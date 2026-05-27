/**
 * EMC Logic JS版 - 完整状态机实现
 * 
 * 设计原则：
 * 1. 使用数组索引访问，枚举值作为索引
 * 2. 保持与C代码完全一致的状态转移逻辑
 * 3. 回调接口兼容原有API
 */

// ============================================================================
// 常量定义（与C代码完全一致）
// ============================================================================

const TIME_CONSTANTS = {
    POWER_ON_SELFTEST_MS: 1500,
    VERSION_DISPLAY_MS: 1500,
    LONG_PRESS_MS: 1500,
    FUNC_SELECT_TIMEOUT_MS: 60000,
    TEMP_LOCK_THRESHOLD: 80,
    INDICATOR_BLINK_MS: 500,
    TEMP_HINT_MS: 3000,
    WATER_RESUME_WAIT_MIN_S: 30,
    WATER_RESUME_WAIT_MAX_S: 120,
    CYCLE_PERIOD_MS: 10
};

const PARAM_BOUNDS = {
    MIN_WATER_ML: 100,
    MAX_WATER_ML: 500,
    WATER_STEP_ML: 20,
    MIN_TIME_S: 0,
    MAX_TIME_S: 480,
    TIME_STEP_S: 30
};

// ============================================================================
// 枚举定义（使用对象模拟C的enum，保证索引对齐）
// ============================================================================

/** 系统状态枚举 - 9个状态 */
const SystemState = {
    S_POWER_ON: 0,
    S_VERSION: 1,
    S_SHUTDOWN: 2,
    S_DEMO: 3,
    S_STANDBY: 4,
    S_FUNC_SELECT: 5,
    S_COOKING: 6,
    S_PAUSE: 7,
    S_FAULT: 8,
    STATE_MAX: 9
};

const STATE_NAMES = [
    'S_POWER_ON', 'S_VERSION', 'S_SHUTDOWN', 'S_DEMO',
    'S_STANDBY', 'S_FUNC_SELECT', 'S_COOKING', 'S_PAUSE', 'S_FAULT'
];

/** 按键枚举 - 14个按键 */
const KeyCode = {
    KEY_POWER: 0,
    KEY_M1: 1,
    KEY_M2: 2,
    KEY_M3: 3,
    KEY_M4: 4,
    KEY_M5: 5,
    KEY_M6: 6,
    KEY_M7: 7,
    KEY_M8: 8,
    KEY_M9: 9,
    KEY_M10: 10,
    KEY_ADD_WATER: 11,
    KEY_ADD_TIME: 12,
    KEY_START_PAUSE: 13,
    KEY_MAX: 14
};

// KeyEvent已在key_handler.js中定义，此处不再重复
// const KeyEvent = { ... };

/** 蜂鸣器命令 */
const BuzzerCmd = {
    BUZZ_CLICK: 1,
    BUZZ_DOUBLE: 2,
    BUZZ_LONG: 3,
    BUZZ_CHORD: 4,
    BUZZ_ERROR: 5
};

// ============================================================================
// 功能配置表（数组索引1-10对应M1-M10）
// ============================================================================

const FUNC_CONFIGS = new Array(11).fill(null);
FUNC_CONFIGS[1] = { default_water: 200, default_time: 180 };   // M1
FUNC_CONFIGS[2] = { default_water: 250, default_time: 240 };   // M2
FUNC_CONFIGS[3] = { default_water: 300, default_time: 300 };   // M3
FUNC_CONFIGS[4] = { default_water: 200, default_time: 120 };   // M4
FUNC_CONFIGS[5] = { default_water: 350, default_time: 360 };   // M5
FUNC_CONFIGS[6] = { default_water: 400, default_time: 420 };   // M6
FUNC_CONFIGS[7] = { default_water: 150, default_time: 90 };    // M7
FUNC_CONFIGS[8] = { default_water: 450, default_time: 480 };   // M8
FUNC_CONFIGS[9] = { default_water: 250, default_time: 200 };   // M9
FUNC_CONFIGS[10] = { default_water: 300, default_time: 250 };  // M10

// ============================================================================
// 按键掩码表（按状态定义）
// ============================================================================

const KEY_MASKS = new Array(SystemState.STATE_MAX).fill(null).map(() => ({ short_mask: 0, long_mask: 0 }));

// 辅助函数：生成掩码
function mask(keys) {
    return keys.reduce((sum, key) => sum | (1 << key), 0);
}

KEY_MASKS[SystemState.S_POWER_ON] = { short_mask: 0, long_mask: 0 };
KEY_MASKS[SystemState.S_VERSION] = { 
    short_mask: mask([KeyCode.KEY_POWER, KeyCode.KEY_START_PAUSE]), 
    long_mask: 0 
};
KEY_MASKS[SystemState.S_SHUTDOWN] = { 
    short_mask: mask([KeyCode.KEY_POWER, KeyCode.KEY_START_PAUSE]), 
    long_mask: 0 
};
KEY_MASKS[SystemState.S_DEMO] = { 
    short_mask: mask([KeyCode.KEY_POWER, KeyCode.KEY_START_PAUSE]), 
    long_mask: 0 
};
KEY_MASKS[SystemState.S_STANDBY] = {
    short_mask: mask([1,2,3,4,5,6,7,8,9,10, KeyCode.KEY_ADD_TIME]),  // M1-M10 + ADD_TIME
    long_mask: mask([KeyCode.KEY_POWER, KeyCode.KEY_START_PAUSE])     // 长按关机
};
KEY_MASKS[SystemState.S_FUNC_SELECT] = {
    short_mask: mask([1,2,3,4,5,6,7,8,9,10, KeyCode.KEY_ADD_WATER, KeyCode.KEY_ADD_TIME, KeyCode.KEY_START_PAUSE]),
    long_mask: mask([KeyCode.KEY_START_PAUSE])  // 长按取消
};
KEY_MASKS[SystemState.S_COOKING] = {
    short_mask: mask([KeyCode.KEY_START_PAUSE]),
    long_mask:  mask([KeyCode.KEY_START_PAUSE])  // ✅ 长按取消烹饪
};
KEY_MASKS[SystemState.S_PAUSE] = {
    short_mask: mask([KeyCode.KEY_START_PAUSE]),
    long_mask:  mask([KeyCode.KEY_START_PAUSE])  // ✅ 长按取消烹饪
};
KEY_MASKS[SystemState.S_FAULT] = {
    short_mask: 0,
    long_mask: 0
};

// ============================================================================
// 主控制结构体（模拟C的EmcCtrl_t）
// ============================================================================

class EmcLogic {
    constructor() {
        // 系统时间
        this.system_time_ms = 0;
        this.state_enter_time_ms = 0;
        
        // 状态
        this.current_state = SystemState.S_POWER_ON;
        this.previous_state = SystemState.S_POWER_ON;
        
        // 标志位（用对象模拟位域）
        this.flags = {
            is_initialized: false,
            is_water_phase: false,
            is_end_phase: false,
            temp_lock_active: false,
            qr_lock_active: false,
            hint_active: false,
            show_time_disp: true,
            any_key_processed: false,  // ✅ End状态退出标志
            reserved: 0
        };
        
        // 运行时参数
        this.params = {
            selected_func_id: -1,
            current_water_ml: 0,
            current_time_s: 0,
            remain_water_ml: 0,
            remain_time_s: 0
        };
        
        // 锁定状态
        this.lock = {
            water_low_wait_s: 0
        };
        
        // 临时提示
        this.hint = {
            hint_type: 0,
            start_time_ms: 0,
            restore_digit: [0, 0, 0, 0],
            restore_dp: 0,
            restore_colon: 0
        };
        
        // 硬件输入
        this.hw = {
            water_temp: 850,      // 默认85.0°C
            water_level_ok: 1,
            pot_present: 1,
            fault_code: 0
        };
        
        // 显示缓冲区
        this.disp = {
            digit: [0x38, 0x38, 0x38, 0x38],  // 初始"---"
            dp_mask: 0,
            colon_mask: 0,
            special_mode: 0
        };
        
        // 段码输出
        this.seg = {
            seg: [0x7F, 0x7F, 0x7F, 0x7F],  // 初始8888
            dp_mask: 0,
            colon_mask: 0
        };
        
        // LED状态（16位）
        this.led = new Array(16).fill(0);
        
        // 回调函数
        this.callbacks = {
            disp_output: null,
            buzzer: null,
            status_change: null,
            get_hw_inputs: null
        };
        
        // 按键缓冲
        this.key_buffer = [];
        this.key_buffer_head = 0;
        this.key_buffer_tail = 0;
        this.KEY_BUFFER_SIZE = 8;
        
        // 蜂鸣器请求（待处理）
        this.pending_buzzer_cmd = 0;
        this.pending_buzzer_duration = 0;
        
        console.log('[EmcLogic] 初始化完成');
    }
    
    // ========================================================================
    // API方法
    // ========================================================================
    
    /**
     * 初始化逻辑层（匹配C版本emc_logic_init）
     */
    init() {
        this.current_state = SystemState.S_POWER_ON;
        this.previous_state = SystemState.S_POWER_ON;
        this.system_time_ms = 0;
        this.state_enter_time_ms = 0;  // ✅ 与system_time_ms保持一致
        this.flags.is_initialized = true;
        
        console.log(`[EmcLogic] 初始化: state=S_POWER_ON, time=0ms`);
        
        // 触发状态变化回调
        if (this.callbacks.status_change) {
            this.callbacks.status_change(1, this.current_state);  // STATUS_STATE_CHANGE
        }
    }
    
    /**
     * 注册回调函数
     */
    registerCallbacks(dispCb, buzzerCb, statusCb, hwCb) {
        this.callbacks.disp_output = dispCb;
        this.callbacks.buzzer = buzzerCb;
        this.callbacks.status_change = statusCb;
        this.callbacks.get_hw_inputs = hwCb;
    }
    
    /**
     * 10ms周期主循环
     */
    runCycle() {
        // ✅ 每个周期都输出调试信息（前5秒）
        if (this.system_time_ms <= 5000) {
            console.log(`[EmcLogic] runCycle #${this.system_time_ms/10}: state=${this.current_state}(${STATE_NAMES[this.current_state]}), time_before=${this.system_time_ms}ms`);
        }
        
        this.system_time_ms += TIME_CONSTANTS.CYCLE_PERIOD_MS;
        
        // 1. 拉取硬件输入
        if (this.callbacks.get_hw_inputs) {
            this.callbacks.get_hw_inputs(this.hw);
        }
        
        // 2. 处理按键队列
        this.processKeyEvents();
        
        // 3. 超时检测
        this.checkTimeouts();
        
        // 4. 倒计时（烹饪中）
        this.updateCountdown();
        
        // 5. ✅ End状态退出检测
        this.checkEndPhaseExit();
        
        // 6. 故障检测
        this.checkFaults();
        
        // 7. 更新显示
        this.updateDisplay();
        
        // 8. 输出回调
        this.outputCallbacks();
    }
    
    /**
     * 按键输入回调
     */
    keyInput(keyCode, event) {
        // 📊 详细日志：记录按键输入
        const eventName = ['NULL', 'PRESS', 'SHORT', 'LONG', 'REPEAT', 'RELEASE'][event] || `UNKNOWN(${event})`;
        console.log('[EmcLogic.keyInput] 收到按键事件:', {
            keyCode: keyCode,
            event: event,
            eventName: eventName,
            currentState: this.current_state,
            stateName: STATE_NAMES[this.current_state],
            bufferHead: this.key_buffer_head,
            bufferTail: this.key_buffer_tail
        });
        
        // 写入按键缓冲
        const keyEvent = {
            key_code: keyCode,
            event: event,
            timestamp_ms: this.system_time_ms
        };
        
        this.key_buffer[this.key_buffer_tail] = keyEvent;
        this.key_buffer_tail = (this.key_buffer_tail + 1) % this.KEY_BUFFER_SIZE;
        
        // 如果缓冲区满，覆盖最旧的
        if (this.key_buffer_tail === this.key_buffer_head) {
            this.key_buffer_head = (this.key_buffer_head + 1) % this.KEY_BUFFER_SIZE;
            console.warn('[EmcLogic.keyInput] ⚠️ 按键缓冲区已满，丢弃旧事件');
        }
        
        console.log('[EmcLogic.keyInput] ✅ 事件已写入缓冲区');
    }
    
    // ========================================================================
    // 内部方法
    // ========================================================================
    
    /**
     * 处理按键事件
     */
    processKeyEvents() {
        while (this.key_buffer_head !== this.key_buffer_tail) {
            const keyEvent = this.key_buffer[this.key_buffer_head];
            this.key_buffer_head = (this.key_buffer_head + 1) % this.KEY_BUFFER_SIZE;
            
            // ✅ End状态特殊处理：任意有效按键标记退出
            if (this.flags.is_end_phase) {
                console.log('[EmcLogic] 🔍 End状态检测到按键:', keyEvent.key_code, keyEvent.event);
                this.flags.any_key_processed = true;
                continue;  // End状态下不执行其他逻辑
            }
            
            this.handleKeyEvent(keyEvent.key_code, keyEvent.event);
        }
    }
    
    /**
     * 处理单个按键事件
     */
    handleKeyEvent(keyCode, event) {
        // 检查按键掩码
        const keyMask = KEY_MASKS[this.current_state];
        const keyBit = 1 << keyCode;
        
        let isValid = false;
        if (event === KeyEvent.KEY_EVENT_SHORT) {
            isValid = (keyMask.short_mask & keyBit) !== 0;
        } else if (event === KeyEvent.KEY_EVENT_LONG) {
            isValid = (keyMask.long_mask & keyBit) !== 0;
        }
        
        if (!isValid) {
            return;  // 按键被屏蔽
        }
        
        // 根据当前状态和按键执行状态转移
        this.executeTransition(keyCode, event);
    }
    
    /**
     * 执行状态转移（简化版，完整版需要状态转移表）
     */
    executeTransition(keyCode, event) {
        const state = this.current_state;
        
        switch (state) {
            case SystemState.S_POWER_ON:
                // 上电自检完成后自动进入S_VERSION
                if (this.system_time_ms >= TIME_CONSTANTS.POWER_ON_SELFTEST_MS) {
                    this.transitionTo(SystemState.S_VERSION);
                }
                break;
                
            case SystemState.S_VERSION:
                // 版本号显示完成后进入S_SHUTDOWN（匹配C代码）
                if (this.system_time_ms - this.state_enter_time_ms >= TIME_CONSTANTS.VERSION_DISPLAY_MS) {
                    this.transitionTo(SystemState.S_SHUTDOWN);
                }
                break;
                
            case SystemState.S_SHUTDOWN:
                // ✅ 关机状态下，短按电源键或START_PAUSE开机
                if ((keyCode === KeyCode.KEY_POWER || keyCode === KeyCode.KEY_START_PAUSE) && 
                    event === KeyEvent.KEY_EVENT_SHORT) {
                    console.log('[EmcLogic] 🔌 检测到开机事件:', keyCode, event);
                    this.transitionTo(SystemState.S_STANDBY);
                    this.playBuzzer(BuzzerCmd.BUZZ_CLICK);
                }
                break;
                
            case SystemState.S_STANDBY:
                if (keyCode >= KeyCode.KEY_M1 && keyCode <= KeyCode.KEY_M10) {
                    // 选择功能
                    this.params.selected_func_id = keyCode;
                    const config = FUNC_CONFIGS[keyCode];
                    this.params.current_water_ml = config.default_water;
                    this.params.current_time_s = config.default_time;
                    this.params.remain_water_ml = config.default_water;
                    this.params.remain_time_s = config.default_time;
                    
                    this.transitionTo(SystemState.S_FUNC_SELECT);
                    this.playBuzzer(BuzzerCmd.BUZZ_CLICK);
                } else if (keyCode === KeyCode.KEY_START_PAUSE || keyCode === KeyCode.KEY_POWER) {
                    // ✅ 电源键或START_PAUSE长按关机
                    if (event === KeyEvent.KEY_EVENT_LONG || event === KeyEvent.KEY_EVENT_REPEAT) {
                        console.log('[EmcLogic] 🔑 检测到长按关机事件:', keyCode, event);
                        this.transitionTo(SystemState.S_SHUTDOWN);
                        this.playBuzzer(BuzzerCmd.BUZZ_CLICK);
                    }
                }
                break;
                
            case SystemState.S_FUNC_SELECT:
                if (keyCode >= KeyCode.KEY_M1 && keyCode <= KeyCode.KEY_M10) {
                    // 切换功能
                    this.params.selected_func_id = keyCode;
                    const config = FUNC_CONFIGS[keyCode];
                    this.params.current_water_ml = config.default_water;
                    this.params.current_time_s = config.default_time;
                    this.params.remain_water_ml = config.default_water;
                    this.params.remain_time_s = config.default_time;
                    
                    this.playBuzzer(BuzzerCmd.BUZZ_CLICK);
                } else if (keyCode === KeyCode.KEY_ADD_WATER) {
                    // 调整水量
                    this.adjustWater(PARAM_BOUNDS.WATER_STEP_ML);
                    this.playBuzzer(BuzzerCmd.BUZZ_CLICK);
                } else if (keyCode === KeyCode.KEY_ADD_TIME) {
                    // 调整时间
                    this.adjustTime(PARAM_BOUNDS.TIME_STEP_S);
                    this.playBuzzer(BuzzerCmd.BUZZ_CLICK);
                } else if (keyCode === KeyCode.KEY_START_PAUSE) {
                    if (event === KeyEvent.KEY_EVENT_SHORT) {
                        // 短按启动
                        this.transitionTo(SystemState.S_COOKING);
                        this.playBuzzer(BuzzerCmd.BUZZ_CHORD);
                    } else if (event === KeyEvent.KEY_EVENT_LONG) {
                        // 长按取消
                        this.transitionTo(SystemState.S_STANDBY);
                        this.playBuzzer(BuzzerCmd.BUZZ_CLICK);
                    }
                }
                break;
                
            case SystemState.S_COOKING:
                if (keyCode === KeyCode.KEY_START_PAUSE) {
                    if (event === KeyEvent.KEY_EVENT_SHORT) {
                        // 短按暂停
                        this.transitionTo(SystemState.S_PAUSE);
                        this.playBuzzer(BuzzerCmd.BUZZ_DOUBLE);
                    } else if (event === KeyEvent.KEY_EVENT_LONG || event === KeyEvent.KEY_EVENT_REPEAT) {
                        // ✅ 长按取消烹饪，回到待机状态
                        console.log('[EmcLogic] 🛑 检测到长按取消烹饪事件:', keyCode, event);
                        this.resetCookingParams();
                        this.transitionTo(SystemState.S_STANDBY);
                        this.playBuzzer(BuzzerCmd.BUZZ_CLICK);
                    }
                }
                break;
                
            case SystemState.S_PAUSE:
                if (keyCode === KeyCode.KEY_START_PAUSE) {
                    if (event === KeyEvent.KEY_EVENT_SHORT) {
                        // 短按恢复
                        this.transitionTo(SystemState.S_COOKING);
                        this.playBuzzer(BuzzerCmd.BUZZ_CHORD);
                    } else if (event === KeyEvent.KEY_EVENT_LONG || event === KeyEvent.KEY_EVENT_REPEAT) {
                        // ✅ 长按取消烹饪，回到待机状态
                        console.log('[EmcLogic] 🛑 检测到长按取消烹饪事件:', keyCode, event);
                        this.resetCookingParams();
                        this.transitionTo(SystemState.S_STANDBY);
                        this.playBuzzer(BuzzerCmd.BUZZ_CLICK);
                    }
                }
                break;
        }
    }
    
    /**
     * 状态转换
     */
    transitionTo(newState) {
        this.previous_state = this.current_state;
        this.current_state = newState;
        this.state_enter_time_ms = this.system_time_ms;
        
        // ✅ 重置End状态标志
        if (newState !== SystemState.S_COOKING || !this.flags.is_end_phase) {
            this.flags.is_end_phase = false;
            this.flags.any_key_processed = false;
        }
        
        // 进入新状态的初始化
        this.onEnterState(newState);
        
        // 触发状态变化回调
        if (this.callbacks.status_change) {
            this.callbacks.status_change(1, newState);
        }
        
        console.log(`[EmcLogic] 状态转换: ${STATE_NAMES[this.previous_state]} → ${STATE_NAMES[newState]}`);
    }
    
    /**
     * 进入状态的处理
     */
    onEnterState(state) {
        // 清除所有LED
        this.led.fill(0);
        
        switch (state) {
            case SystemState.S_SHUTDOWN:
                // ✅ 关机状态，所有LED熄灭，显示---
                console.log('[EmcLogic] 🔌 进入关机状态');
                this.seg.seg = [0x40, 0x40, 0x40, 0x40];  // ---（匹配C代码）
                break;
                
            case SystemState.S_STANDBY:
                // 待机状态，电源灯亮（bit 0）
                this.led[0] = 1;
                break;
                
            case SystemState.S_FUNC_SELECT:
                // 功能选择，电源灯 + 对应功能灯亮
                this.led[0] = 1;  // 电源灯
                if (this.params.selected_func_id >= 1 && this.params.selected_func_id <= 10) {
                    this.led[this.params.selected_func_id] = 1;  // M1-M10灯
                }
                break;
                
            case SystemState.S_COOKING:
            case SystemState.S_PAUSE:
                // 烹饪中/暂停，电源灯 + 功能灯亮
                this.led[0] = 1;  // 电源灯
                if (this.params.selected_func_id >= 1 && this.params.selected_func_id <= 10) {
                    this.led[this.params.selected_func_id] = 1;  // M1-M10灯
                }
                break;
        }
    }
    
    /**
     * 调整水量
     */
    adjustWater(step) {
        this.params.current_water_ml += step;
        if (this.params.current_water_ml > PARAM_BOUNDS.MAX_WATER_ML) {
            this.params.current_water_ml = PARAM_BOUNDS.MIN_WATER_ML;
        }
        this.params.remain_water_ml = this.params.current_water_ml;
    }
    
    /**
     * 调整时间
     */
    adjustTime(step) {
        this.params.current_time_s += step;
        if (this.params.current_time_s > PARAM_BOUNDS.MAX_TIME_S) {
            this.params.current_time_s = PARAM_BOUNDS.MIN_TIME_S;
        }
        this.params.remain_time_s = this.params.current_time_s;
    }
    
    /**
     * ✅ 重置烹饪参数（匹配C版本reset_cooking_params）
     */
    resetCookingParams() {
        console.log('[EmcLogic] 🔄 重置烹饪参数');
        this.params.selected_func_id = -1;
        this.params.current_water_ml = 0;
        this.params.current_time_s = 0;
        this.params.remain_water_ml = 0;
        this.params.remain_time_s = 0;
        this.flags.is_end_phase = 0;
    }
    
    /**
     * 检查状态超时（匹配C版本key_disp_cycle Step 3）
     */
    checkTimeouts() {
        const elapsed = this.system_time_ms - this.state_enter_time_ms;
        
        // 调试：前5秒每500ms输出一次
        if (this.system_time_ms <= 5000 && this.system_time_ms % 500 === 0) {
            console.log(`[EmcLogic] checkTimeouts: state=${this.current_state}, elapsed=${elapsed}ms, system_time=${this.system_time_ms}ms`);
        }
        
        switch (this.current_state) {
            case SystemState.S_POWER_ON:
                // 上电自检1.5秒后自动进入版本显示
                if (elapsed >= TIME_CONSTANTS.POWER_ON_SELFTEST_MS) {
                    console.log(`[EmcLogic] ✅ S_POWER_ON超时: elapsed=${elapsed}ms >= ${TIME_CONSTANTS.POWER_ON_SELFTEST_MS}ms → S_VERSION`);
                    this.transitionTo(SystemState.S_VERSION);
                }
                break;
                
            case SystemState.S_VERSION:
                // 版本显示1.5秒后自动进入关机状态（匹配C代码第998-1002行）
                if (elapsed >= TIME_CONSTANTS.VERSION_DISPLAY_MS) {
                    console.log(`[EmcLogic] ✅ S_VERSION超时: elapsed=${elapsed}ms >= ${TIME_CONSTANTS.VERSION_DISPLAY_MS}ms → S_SHUTDOWN`);
                    this.transitionTo(SystemState.S_SHUTDOWN);
                }
                break;
                
            case SystemState.S_FUNC_SELECT:
                // 功能选择60秒超时回待机
                if (elapsed >= TIME_CONSTANTS.FUNC_SELECT_TIMEOUT_MS) {
                    console.log(`[EmcLogic] S_FUNC_SELECT超时: elapsed=${elapsed}ms`);
                    this.transitionTo(SystemState.S_STANDBY);
                }
                break;
        }
    }
    
    /**
     * 调整水量（匹配C版本adjust_water）
     */
    adjustWater(step) {
        let newVal = this.params.current_water_ml + step;
        if (newVal > PARAM_BOUNDS.MAX_WATER_ML) {
            newVal = PARAM_BOUNDS.MIN_WATER_ML;  // 循环到最小值
        }
        if (newVal < PARAM_BOUNDS.MIN_WATER_ML) {
            newVal = PARAM_BOUNDS.MAX_WATER_ML;  // 循环到最大值
        }
        
        this.params.current_water_ml = newVal;
        this.params.remain_water_ml = newVal;
        
        // 切换到水量显示
        this.flags.show_time_disp = false;
        
        console.log(`[EmcLogic] 💧 调整水量: ${this.params.current_water_ml}ml`);
    }
    
    /**
     * 调整时间（匹配C版本adjust_time）
     */
    adjustTime(step) {
        let newVal = this.params.current_time_s + step;
        if (newVal > PARAM_BOUNDS.MAX_TIME_S) {
            newVal = PARAM_BOUNDS.MIN_TIME_S;  // 循环到最小值
        }
        if (newVal < PARAM_BOUNDS.MIN_TIME_S) {
            newVal = PARAM_BOUNDS.MAX_TIME_S;  // 循环到最大值
        }
        
        this.params.current_time_s = newVal;
        this.params.remain_time_s = newVal;
        
        // 切换到时间显示
        this.flags.show_time_disp = true;
        
        console.log(`[EmcLogic] ⏱️ 调整时间: ${this.params.current_time_s}s`);
    }
    
    /**
     * 更新倒计时（匹配C版本update_cooking_countdown）
     */
    updateCountdown() {
        if (this.current_state === SystemState.S_COOKING) {
            // 每100ms减少1秒（简化处理）
            if (this.system_time_ms % 100 === 0) {
                if (this.params.remain_time_s > 0) {
                    this.params.remain_time_s--;
                }
                
                if (this.params.remain_time_s === 0) {
                    // ✅ 烹饪完成 → 进入End子状态，不直接跳转STANDBY
                    console.log('[EmcLogic] ⏰ 烹饪时间归零，进入End状态');
                    this.flags.is_end_phase = 1;
                    this.flags.is_water_phase = 0;
                    this.playBuzzer(BuzzerCmd.BUZZ_CHORD);
                    // 注意：保持在S_COOKING状态，但显示End
                }
            }
        }
    }
    
    /**
     * 检查故障
     */
    checkFaults() {
        if (this.hw.fault_code !== 0) {
            this.transitionTo(SystemState.S_FAULT);
        }
    }
    
    /**
     * ✅ End状态退出检测（匹配C版本check_end_phase_exit）
     */
    checkEndPhaseExit() {
        if (this.flags.is_end_phase && this.flags.any_key_processed) {
            console.log('[EmcLogic] 🔑 End状态检测到按键，退出到STANDBY');
            this.resetCookingParams();
            this.transitionTo(SystemState.S_STANDBY);
            
            // 触发状态变化回调
            if (this.callbacks.status_change) {
                this.callbacks.status_change(2, null);  // STATUS_COOK_END = 2
            }
        }
    }
    
    /**
     * 更新显示
     */
    updateDisplay() {
        // 根据当前状态更新数码管显示
        switch (this.current_state) {
            case SystemState.S_POWER_ON:
                this.setDisplay('8888');
                break;
                
            case SystemState.S_VERSION:
                this.setDisplay('F1.0');
                break;
                
            case SystemState.S_STANDBY:
                this.setDisplay('85  ');
                break;
                
            case SystemState.S_FUNC_SELECT:
                if (this.flags.show_time_disp) {
                    this.setDisplay(this.formatTime(this.params.current_time_s));
                } else {
                    this.setDisplay(this.formatWater(this.params.current_water_ml));
                }
                break;
                
            case SystemState.S_COOKING:
                // ✅ 检查End状态（匹配C版本update_display）
                if (this.flags.is_end_phase) {
                    // 显示 "End"
                    this.setDisplay('End ');
                } else {
                    this.setDisplay(this.formatTime(this.params.remain_time_s));
                }
                break;
                
            case SystemState.S_PAUSE:
                this.setDisplay('P   ');
                break;
                
            default:
                this.setDisplay('----');
        }
        
        // 更新LED
        this.updateLeds();
    }
    
    /**
     * 设置显示文本
     */
    setDisplay(text) {
        // 这里应该转换为段码，简化版直接存储ASCII
        for (let i = 0; i < 4; i++) {
            this.disp.digit[i] = text.charCodeAt(i) || 0x20;  // 空格
        }
        
        // 转换为段码（简化映射）
        this.seg.seg[0] = this.charToSeg(text[0] || ' ');
        this.seg.seg[1] = this.charToSeg(text[1] || ' ');
        this.seg.seg[2] = this.charToSeg(text[2] || ' ');
        this.seg.seg[3] = this.charToSeg(text[3] || ' ');
    }
    
    /**
     * 字符转段码（简化版）
     */
    charToSeg(char) {
        const map = {
            '0': 0x3F, '1': 0x06, '2': 0x5B, '3': 0x4F, '4': 0x66,
            '5': 0x6D, '6': 0x7D, '7': 0x07, '8': 0x7F, '9': 0x6F,
            'A': 0x77, 'B': 0x7C, 'C': 0x39, 'D': 0x5E, 'E': 0x79,
            'F': 0x71, 'P': 0x73, '-': 0x40, ' ': 0x00
        };
        return map[char] || 0x00;
    }
    
    /**
     * 格式化时间
     */
    formatTime(seconds) {
        const min = Math.floor(seconds / 60);
        const sec = seconds % 60;
        return `${min.toString().padStart(2, '0')}${sec.toString().padStart(2, '0')}`;
    }
    
    /**
     * 格式化水量
     */
    formatWater(ml) {
        return ml.toString().padStart(4, ' ');
    }
    
    /**
     * 更新LED状态
     */
    updateLeds() {
        // 清除所有LED
        this.led.fill(0);
        
        // 根据状态设置LED
        switch (this.current_state) {
            case SystemState.S_STANDBY:
                this.led[0] = 1;  // 电源灯
                break;
                
            case SystemState.S_FUNC_SELECT:
                this.led[0] = 1;  // 电源灯
                if (this.params.selected_func_id >= 1 && this.params.selected_func_id <= 10) {
                    this.led[this.params.selected_func_id] = 1;  // 功能灯
                }
                break;
                
            case SystemState.S_COOKING:
            case SystemState.S_PAUSE:
                this.led[0] = 1;  // 电源灯
                if (this.params.selected_func_id >= 1 && this.params.selected_func_id <= 10) {
                    this.led[this.params.selected_func_id] = 1;  // 功能灯
                }
                break;
        }
    }
    
    /**
     * 播放蜂鸣器
     */
    playBuzzer(cmd) {
        if (this.callbacks.buzzer) {
            this.callbacks.buzzer(cmd);
        }
    }
    
    /**
     * 输出回调
     */
    outputCallbacks() {
        // 显示输出（包含段码和LED）
        if (this.callbacks.disp_output) {
            // C版本签名: disp_output(SegCode_t *p_seg, LedState_t *p_led)
            // JS版本: 传递段码对象和LED数组（保持结构一致）
            
            // ✅ 调试日志：每个周期都输出（前10秒）
            if (this.system_time_ms <= 10000) {
                console.log(`[EmcLogic] outputCallbacks: state=${this.current_state}(${STATE_NAMES[this.current_state]}), seg=[${this.seg.seg.map(v => '0x'+v.toString(16).toUpperCase().padStart(2,'0')).join(', ')}]`);
            }
            
            this.callbacks.disp_output(this.seg, this.led);
        } else {
            // ✅ 警告：没有注册显示回调
            if (this.system_time_ms <= 10000 && this.system_time_ms % 1000 === 0) {
                console.warn('[EmcLogic] ⚠️ disp_output回调未注册！');
            }
        }
        
        // 蜂鸣器输出（如果有待处理的蜂鸣请求）
        if (this.pending_buzzer_cmd > 0 && this.callbacks.buzzer) {
            // C版本签名: buzzer(uint8_t cmd) - 只有cmd参数
            this.callbacks.buzzer(this.pending_buzzer_cmd);
            this.pending_buzzer_cmd = 0;
        }
        
        // 状态变化回调（仅在状态改变时触发）
        // 注意：status_change在change_state中已经调用，这里不需要重复调用
    }
    
    /**
     * 获取当前状态
     */
    getState() {
        return this.current_state;
    }
    
    /**
     * 获取状态名称
     */
    getStateName() {
        return STATE_NAMES[this.current_state] || 'UNKNOWN';
    }
}

// 导出
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { EmcLogic, SystemState, KeyCode, KeyEvent, BuzzerCmd };
}
