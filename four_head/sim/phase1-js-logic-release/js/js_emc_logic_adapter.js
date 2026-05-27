/**
 * JS逻辑层适配器
 * 
 * 包装EmcLogic，提供与EmsWasmAdapter完全一致的接口
 * 继承LogicLayerAdapter基类，实现标准接口
 */

class JSEmcLogicAdapter extends LogicLayerAdapter {
    constructor() {
        super();  // ✅ 调用父类构造函数
        
        this.emcLogic = null;
        
        // 按键和事件常量（与WASM一致）
        this.KEY_CODES = {
            KEY_POWER: 0,
            KEY_M1: 1, KEY_M2: 2, KEY_M3: 3, KEY_M4: 4, KEY_M5: 5,
            KEY_M6: 6, KEY_M7: 7, KEY_M8: 8, KEY_M9: 9, KEY_M10: 10,
            KEY_ADD_WATER: 11,
            KEY_ADD_TIME: 12,
            KEY_START_PAUSE: 13
        };
        
        this.EVENTS = {
            EVENT_NULL: 0,
            EVENT_PRESS: 1,
            EVENT_SHORT: 2,
            EVENT_LONG: 3,
            EVENT_REPEAT: 4,
            EVENT_RELEASE: 5
        };
        
        // 状态枚举
        this.STATES = {
            S_POWER_ON: 0,
            S_VERSION: 1,
            S_SHUTDOWN: 2,
            S_DEMO: 3,
            S_STANDBY: 4,
            S_FUNC_SELECT: 5,
            S_COOKING: 6,
            S_PAUSE: 7,
            S_FAULT: 8
        };
        
        this.STATE_NAMES = {
            0: "S_POWER_ON",
            1: "S_VERSION",
            2: "S_SHUTDOWN",
            3: "S_DEMO",
            4: "S_STANDBY",
            5: "S_FUNC_SELECT",
            6: "S_COOKING",
            7: "S_PAUSE",
            8: "S_FAULT"
        };
        
        // 段码映射表
        this.SEG_MAP = {
            0x3F: '0', 0x06: '1', 0x5B: '2', 0x4F: '3',
            0x66: '4', 0x6D: '5', 0x7D: '6', 0x07: '7',
            0x7F: '8', 0x6F: '9',
            0x77: 'A', 0x7C: 'B', 0x39: 'C', 0x5E: 'd',
            0x79: 'E', 0x71: 'F', 0x3D: 'G', 0x76: 'H',
            0x1E: 'J', 0x38: 'L', 0x37: 'N', 0x73: 'P',
            0x3E: 'U',
            0x58: 'c', 0x54: 'n', 0x5C: 'o', 0x50: 'r',
            0x78: 't', 0x6E: 'y',
            0x40: '-', 0x00: ' ', 0x08: '_'
        };
    }
    
    /**
     * 初始化JS逻辑层
     * @returns {Promise<void>}
     */
    async init() {
        if (this.initialized) {
            console.log('[JSEmcLogicAdapter] 已初始化，跳过');
            return;
        }
        
        console.log('[JSEmcLogicAdapter] 初始化JS逻辑层...');
        
        // 创建EmcLogic实例
        this.emcLogic = new EmcLogic();
        console.log('[JSEmcLogicAdapter] ✅ EmcLogic创建成功');
        
        // 注册回调函数到EmcLogic（添加转换层）
        if (this.callbacks.onDisplayOutput || this.callbacks.onBuzzerOutput || 
            this.callbacks.onStateChange || this.callbacks.onHardwareInputs) {
            
            // ✅ 显示输出回调转换：将EmcLogic的(seg对象, led数组)转换为标准格式
            const displayCallback = this.callbacks.onDisplayOutput ? (segObj, ledArray) => {
                // segObj: {seg: [...], dp_mask: 0, colon_mask: 0}
                // ledArray: [0, 0, 0, ...] (16位)
                
                // 提取段码数组
                const seg = segObj.seg || segObj;
                
                // 将LED数组转换为LED对象
                const ledValue = ledArray.reduce((sum, bit, index) => {
                    return sum | (bit << index);
                }, 0);
                
                const led = {
                    power: !!(ledValue & 0x01),
                    func_leds: (ledValue >> 1) & 0x03FF,
                    add_water: !!(ledValue & (1 << 11)),
                    add_time: !!(ledValue & (1 << 12)),
                    water_disp: !!(ledValue & (1 << 13)),
                    time_disp: !!(ledValue & (1 << 14))
                };
                
                // 调用标准回调
                this.callbacks.onDisplayOutput(seg, led);
            } : null;
            
            // ✅ 状态变化回调转换：EmcLogic传递的是直接的状态值
            const stateCallback = this.callbacks.onStateChange ? (type, value) => {
                // EmcLogic传递: type=1 (STATUS_STATE_CHANGE), value=状态码
                // 标准接口期望: type=1, valPtr=指向状态值的指针
                // 这里直接传递value作为valPtr
                this.callbacks.onStateChange(type, value);
            } : null;
            
            this.emcLogic.registerCallbacks(
                displayCallback,
                this.callbacks.onBuzzerOutput,
                stateCallback,
                this.callbacks.onHardwareInputs
            );
            console.log('[JSEmcLogicAdapter] ✅ 回调函数已注册（带转换层）');
        }
        
        // 初始化EmcLogic状态机（启动上电流程：S_POWER_ON → S_VERSION → S_SHUTDOWN）
        this.emcLogic.init();
        console.log('[JSEmcLogicAdapter] ✅ EmcLogic状态机已初始化');
        
        // ❌ 不在这里调用runCycle，由startPeriodicRun持续调用
        // this.emcLogic.runCycle();
        
        this.initialized = true;
        console.log('[JSEmcLogicAdapter] ✅ 初始化完成');
    }
    
    /**
     * 清理资源（重写父类方法）
     */
    cleanup() {
        // 先调用父类清理（停止周期、重置状态）
        super.cleanup();
        
        // 清理EmcLogic引用
        this.emcLogic = null;
        
        console.log('[JSEmcLogicAdapter] 清理完成');
    }
    
    /**
     * 运行单个10ms周期
     */
    runCycle() {
        if (!this.initialized || !this.emcLogic) {
            throw new Error('JS逻辑层未初始化，请先调用init()');
        }
        this.emcLogic.runCycle();
    }
    
    /**
     * 发送按键事件
     * @param {number} keyCode - 按键代码
     * @param {number} event - 事件类型
     */
    pressKey(keyCode, event = this.EVENTS.EVENT_SHORT) {
        if (!this.initialized || !this.emcLogic) {
            throw new Error('JS逻辑层未初始化');
        }
        
        // 📊 详细日志：记录按键输入
        const eventName = ['NULL', 'PRESS', 'SHORT', 'LONG', 'REPEAT', 'RELEASE'][event] || `UNKNOWN(${event})`;
        console.log('[JSEmcLogicAdapter.pressKey] 发送按键事件:', {
            keyCode: keyCode,
            event: event,
            eventName: eventName,
            initialized: this.initialized
        });
        
        // 直接调用EmcLogic的keyInput方法
        this.emcLogic.keyInput(keyCode, event);
        
        console.log('[JSEmcLogicAdapter.pressKey] ✅ keyInput调用完成');
    }
    
    /**
     * 按键输入（与EmcLogic.keyInput兼容）
     * @param {number} keyCode - 按键代码
     * @param {number} event - 事件类型
     */
    keyInput(keyCode, event) {
        return this.pressKey(keyCode, event);
    }
    
    /**
     * 获取当前状态码
     * @returns {number} 状态码
     */
    getState() {
        if (!this.initialized || !this.emcLogic) {
            throw new Error('JS逻辑层未初始化');
        }
        return this.emcLogic.current_state;
    }
    
    /**
     * 获取当前状态名称
     * @returns {string} 状态名称
     */
    getStateName() {
        const code = this.getState();
        return this.STATE_NAMES[code] || `UNKNOWN(${code})`;
    }
    
    /**
     * 获取状态名称映射表（实现父类接口）
     * @returns {Object} { stateCode: stateName }
     */
    getStateNames() {
        return this.STATE_NAMES;
    }
    
    /**
     * 检查当前状态是否符合预期
     * @param {number|string} expectedState - 期望的状态码或状态名称
     * @returns {boolean} 是否匹配
     */
    checkState(expectedState) {
        const current = this.getState();
        const currentName = this.getStateName();
        
        if (typeof expectedState === 'number') {
            return current === expectedState;
        } else {
            return currentName === expectedState;
        }
    }
    
    /**
     * 读取数码管显示
     * @returns {Object} 显示信息
     *   - seg: 段码数组 [seg0, seg1, seg2, seg3]
     *   - dp: 小数点掩码
     *   - colon: 冒号掩码
     *   - display: 可读字符串
     */
    getDisplay() {
        if (!this.initialized || !this.emcLogic) {
            throw new Error('JS逻辑层未初始化');
        }
        
        const segArray = [...this.emcLogic.seg.seg];
        const dpMask = this.emcLogic.seg.dp_mask;
        const colonMask = this.emcLogic.seg.colon_mask;
        
        return {
            seg: segArray,
            dp: dpMask,
            colon: colonMask,
            display: this.segToString(segArray, dpMask, colonMask)
        };
    }
    
    /**
     * 获取数码管显示字符串
     * @returns {string} 显示字符串
     */
    getDisplayString() {
        return this.getDisplay().display;
    }
    
    /**
     * 检查数码管显示是否符合预期
     * @param {string} expectedStr - 期望的显示字符串
     * @returns {boolean} 是否匹配
     */
    checkDisplay(expectedStr) {
        const actual = this.getDisplayString();
        return actual.toUpperCase() === expectedStr.toUpperCase();
    }
    
    /**
     * 检查段码值是否符合预期
     * @param {Array} expectedCodes - 期望的段码数组
     * @returns {boolean} 是否匹配
     */
    checkSegCodes(expectedCodes) {
        const actual = this.getDisplay().seg;
        return JSON.stringify(actual) === JSON.stringify(expectedCodes);
    }
    
    /**
     * 读取LED状态
     * @returns {Object} LED状态
     */
    getLedState() {
        if (!this.initialized || !this.emcLogic) {
            throw new Error('JS逻辑层未初始化');
        }
        
        // 从EmcLogic读取LED状态（16位）
        const ledValue = this.emcLogic.led.reduce((sum, bit, index) => {
            return sum | (bit << index);
        }, 0);
        
        // 解析位域
        return {
            power: !!(ledValue & 0x01),
            func_leds: (ledValue >> 1) & 0x03FF,
            add_water: !!(ledValue & (1 << 11)),
            add_time: !!(ledValue & (1 << 12)),
            water_disp: !!(ledValue & (1 << 13)),
            time_disp: !!(ledValue & (1 << 14))
        };
    }
    
    /**
     * 检查LED状态
     * @param {Object} expectedLed - 期望的LED状态
     * @returns {boolean} 是否匹配
     */
    checkLed(expectedLed) {
        const actual = this.getLedState();
        
        for (const key in expectedLed) {
            if (actual[key] !== expectedLed[key]) {
                return false;
            }
        }
        
        return true;
    }
    
    /**
     * 捕获当前完整状态快照
     * @param {string} stepName - 步骤名称
     * @returns {Object} 状态快照
     */
    captureSnapshot(stepName = '') {
        const state = this.getState();
        const display = this.getDisplay();
        const led = this.getLedState();
        
        return {
            step: stepName,
            state_code: state,
            state_name: this.STATE_NAMES[state] || `UNKNOWN(${state})`,
            seg_codes: display.seg,
            seg_hex: display.seg.map(c => `0x${c.toString(16).toUpperCase().padStart(2, '0')}`).join(', '),
            seg_display: display.display,
            dp_mask: display.dp,
            colon_mask: display.colon,
            led: led
        };
    }
    
    /**
     * 打印状态报告（类似Python DLL测试格式）
     * @param {string} phaseName - 阶段名称
     * @param {Array} snapshots - 状态快照数组
     */
    printReport(phaseName, snapshots) {
        console.log(`\n${'='.repeat(80)}`);
        console.log(`  阶段 ${phaseName}`);
        console.log('='.repeat(80));
        console.log(`${'步骤'.padEnd(25)} | ${'状态'.padEnd(15)} | ${'数码管'.padEnd(10)} | ${'DP'.padEnd(6)} | ${'COLON'.padEnd(6)} | LED摘要`);
        console.log('-'.repeat(25) + '-+-' + '-'.repeat(15) + '-+-' + '-'.repeat(10) + '-+-' + '-'.repeat(6) + '-+-' + '-'.repeat(6) + '-+-' + '-'.repeat(30));
        
        snapshots.forEach(snap => {
            const ledSummary = this._formatLedSummary(snap.led);
            console.log(
                `${snap.step.padEnd(25)} | ${snap.state_name.padEnd(15)} | ${snap.seg_display.padEnd(10)} | ` +
                `0x${snap.dp_mask.toString(16).toUpperCase().padStart(2, '0')}    | ` +
                `0x${snap.colon_mask.toString(16).toUpperCase().padStart(2, '0')}    | ${ledSummary}`
            );
        });
    }
    
    /**
     * 将段码数组转换为字符串
     * @param {Array} segArray - 段码数组
     * @param {number} dpMask - 小数点掩码
     * @param {number} colonMask - 冒号掩码
     * @returns {string} 显示字符串
     */
    segToString(segArray, dpMask = 0, colonMask = 0) {
        let result = [];
        for (let i = 0; i < 4; i++) {
            let char = this.SEG_MAP[segArray[i]] || '?';
            if (dpMask & (1 << i)) char += '.';
            if (colonMask & (1 << i)) char = ':';
            result.push(char);
        }
        return result.join('');
    }
    
    /**
     * 格式化LED摘要
     * @private
     */
    _formatLedSummary(led) {
        const p = led.power ? '✓' : '✗';
        const w = led.add_water ? '✓' : '✗';
        const t = led.add_time ? '✓' : '✗';
        const wd = led.water_disp ? '✓' : '✗';
        const td = led.time_disp ? '✓' : '✗';
        
        return `P:${p} F:${led.func_leds.toString(2).padStart(10, '0')} W:${w} T:${t} WD:${wd} TD:${td}`;
    }
    
    /**
     * 延时函数
     * @param {number} ms - 延时毫秒数
     * @returns {Promise}
     */
    sleep(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
    
    /**
     * 等待直到达到目标状态
     * @param {number|string} targetState - 目标状态码或名称
     * @param {number} maxCycles - 最大周期数
     * @returns {Promise<boolean>} 是否成功达到目标状态
     */
    async waitForState(targetState, maxCycles = 1000) {
        for (let i = 0; i < maxCycles; i++) {
            this.runCycle();
            
            if (typeof targetState === 'number') {
                if (this.getState() === targetState) {
                    return true;
                }
            } else {
                if (this.getStateName() === targetState) {
                    return true;
                }
            }
        }
        
        console.warn(`[JSEmcLogicAdapter] 等待状态超时: ${targetState}`);
        return false;
    }
}

// 导出适配器类（如果在模块环境中）
if (typeof module !== 'undefined' && module.exports) {
    module.exports = JSEmcLogicAdapter;
}
