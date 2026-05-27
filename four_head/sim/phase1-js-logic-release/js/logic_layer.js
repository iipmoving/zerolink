/**
 * JS逻辑层 - 简化版状态机实现
 * 第一阶段用于快速验证，第二阶段将被C DLL替换
 * 
 * 注意：SystemState、STATE_NAMES和KeyCode已在emc_logic_js.js中定义，此处不再重复定义
 */

/**
 * JS状态机类
 */
class JSLogicLayer {
    constructor() {
        this.currentState = SystemState.S_POWER_ON;
        this.segDisplay = [0x7F, 0x7F, 0x7F, 0x7F];  // 初始8888
        this.dpMask = 0;
        this.colonMask = 0;
        this.ledBits = new Array(16).fill(0);
        
        // 回调函数
        this.onDisplayCallback = null;
        this.onLedCallback = null;
        this.onBuzzerCallback = null;
        this.onStateCallback = null;
        
        // 定时器
        this.timer = null;
        this.cycleCount = 0;
        
        log('JS逻辑层已初始化', 'success');
    }
    
    /**
     * 初始化状态机
     */
    initialize() {
        this.currentState = SystemState.S_POWER_ON;
        this.segDisplay = [0x7F, 0x7F, 0x7F, 0x7F];
        this.ledBits.fill(0);
        
        // 触发状态变化回调
        if (this.onStateCallback) {
            this.onStateCallback(this.currentState, null);
        }
        
        // 上电后1.5秒进入待机状态
        setTimeout(() => {
            if (this.currentState === SystemState.S_POWER_ON) {
                this.transitionToState(SystemState.S_STANDBY);
            }
        }, 1500);
        
        log('状态机启动：POWER_ON', 'info');
    }
    
    /**
     * 注册回调
     */
    registerCallbacks(displayCb, ledCb, buzzerCb, stateCb) {
        this.onDisplayCallback = displayCb;
        this.onLedCallback = ledCb;
        this.onBuzzerCallback = buzzerCb;
        this.onStateCallback = stateCb;
    }
    
    /**
     * 模拟按键按下
     */
    simulateKeyPress(keyCode, eventType = KeyEvent.EVENT_SHORT) {
        log(`按键事件: keyCode=${keyCode}, event=${eventType === KeyEvent.EVENT_SHORT ? 'SHORT' : 'LONG'}`, 'info');
        
        // 根据当前状态和按键处理逻辑
        this.handleKeyInput(keyCode, eventType);
        
        // 运行一个周期更新显示
        this.runCycle();
    }
    
    /**
     * 处理按键输入（简化版状态机逻辑）
     */
    handleKeyInput(keyCode, eventType) {
        switch (this.currentState) {
            case SystemState.S_POWER_ON:
            case SystemState.S_VERSION:
                // 上电阶段不响应按键
                break;
                
            case SystemState.S_STANDBY:
                if (keyCode === KeyCode.KEY_START_PAUSE) {
                    // 开始键 → 进入功能选择
                    this.transitionToState(SystemState.S_FUNC_SELECT);
                    this.playBuzzer(1);
                } else if (keyCode >= KeyCode.KEY_M1 && keyCode <= KeyCode.KEY_M10) {
                    // M1-M10键 → 直接进入对应功能
                    this.transitionToState(SystemState.S_COOKING);
                    this.playBuzzer(1);
                }
                break;
                
            case SystemState.S_FUNC_SELECT:
                if (keyCode >= KeyCode.KEY_M1 && keyCode <= KeyCode.KEY_M10) {
                    // 选择功能 → 进入烹饪
                    this.transitionToState(SystemState.S_COOKING);
                    this.playBuzzer(1);
                } else if (keyCode === KeyCode.KEY_START_PAUSE) {
                    // 暂停键 → 返回待机
                    this.transitionToState(SystemState.S_STANDBY);
                    this.playBuzzer(1);
                }
                break;
                
            case SystemState.S_COOKING:
                if (keyCode === KeyCode.KEY_START_PAUSE) {
                    // 暂停键 → 暂停
                    this.transitionToState(SystemState.S_PAUSE);
                    this.playBuzzer(2);
                }
                break;
                
            case SystemState.S_PAUSE:
                if (keyCode === KeyCode.KEY_START_PAUSE) {
                    // 暂停键 → 继续
                    this.transitionToState(SystemState.S_COOKING);
                    this.playBuzzer(2);
                }
                break;
        }
    }
    
    /**
     * 状态转换
     */
    transitionToState(newState) {
        this.currentState = newState;
        log(`状态转换: ${STATE_NAMES[newState]}`, 'success');
        
        // 更新LED显示
        this.updateLeds();
        
        // 更新数码管显示
        this.updateDisplay();
        
        // 触发状态变化回调
        if (this.onStateCallback) {
            this.onStateCallback(newState, null);
        }
    }
    
    /**
     * 更新LED显示
     */
    updateLeds() {
        this.ledBits.fill(0);
        
        switch (this.currentState) {
            case SystemState.S_STANDBY:
                this.ledBits[0] = 1;  // 电源灯亮
                break;
                
            case SystemState.S_FUNC_SELECT:
                this.ledBits[0] = 1;  // 电源灯亮
                break;
                
            case SystemState.S_COOKING:
            case SystemState.S_PAUSE:
                this.ledBits[0] = 1;  // 电源灯亮
                this.ledBits[1] = 1;  // M1灯亮（示例）
                break;
        }
        
        // 触发LED回调
        if (this.onLedCallback) {
            this.onLedCallback(null, this.ledBits);
        }
    }
    
    /**
     * 更新数码管显示
     */
    updateDisplay() {
        switch (this.currentState) {
            case SystemState.S_POWER_ON:
                this.segDisplay = [0x7F, 0x7F, 0x7F, 0x7F];  // 8888
                break;
                
            case SystemState.S_STANDBY:
                this.segDisplay = [0x7F, 0x6D, 0x00, 0x00];  // 85
                break;
                
            case SystemState.S_FUNC_SELECT:
                this.segDisplay = [0x77, 0x00, 0x00, 0x00];  // A
                break;
                
            case SystemState.S_COOKING:
                this.segDisplay = [0x5B, 0x3F, 0x3F, 0x00];  // 200
                break;
                
            case SystemState.S_PAUSE:
                this.segDisplay = [0x73, 0x00, 0x00, 0x00];  // P
                break;
                
            default:
                this.segDisplay = [0x00, 0x00, 0x00, 0x00];
        }
        
        // 触发显示回调
        if (this.onDisplayCallback) {
            this.onDisplayCallback(this.segDisplay, this.dpMask, this.colonMask);
        }
    }
    
    /**
     * 播放蜂鸣器
     */
    playBuzzer(cmd) {
        log(`蜂鸣器: cmd=${cmd}`, 'info');
        
        if (this.onBuzzerCallback) {
            this.onBuzzerCallback(cmd);
        }
    }
    
    /**
     * 运行一个周期（10ms）
     */
    runCycle() {
        this.cycleCount++;
        // 这里可以添加周期性任务
    }
    
    /**
     * 启动周期运行（每100ms跑10个周期）
     */
    startPeriodicRun() {
        if (this.timer) return;
        
        this.timer = setInterval(() => {
            for (let i = 0; i < 10; i++) {
                this.runCycle();
            }
        }, 100);
        
        log('周期运行已启动（100ms/10周期）', 'info');
    }
    
    /**
     * 停止周期运行
     */
    stopPeriodicRun() {
        if (this.timer) {
            clearInterval(this.timer);
            this.timer = null;
            log('周期运行已停止', 'info');
        }
    }
    
    /**
     * 获取当前状态
     */
    getState() {
        return this.currentState;
    }
    
    /**
     * 获取状态名称
     */
    getStateName() {
        return STATE_NAMES[this.currentState] || `UNKNOWN(${this.currentState})`;
    }
    
    /**
     * 清理资源
     */
    cleanup() {
        this.stopPeriodicRun();
        log('JS逻辑层已清理', 'info');
    }
}
