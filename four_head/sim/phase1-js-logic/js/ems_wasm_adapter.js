/**
 * EMC WASM 适配器层
 * 
 * 封装WASM模块的初始化和调用，提供标准化的API接口
 * 完全对齐Python DLL测试的调用模式
 * 继承LogicLayerAdapter基类，实现标准接口
 * 
 * 使用示例:
 * ```javascript
 * const adapter = new EmsWasmAdapter();
 * await adapter.init();
 * 
 * // 运行周期
 * adapter.runCycle();
 * 
 * // 发送按键
 * adapter.pressKey(KEY_POWER, EVENT_SHORT);
 * 
 * // 读取状态
 * const state = adapter.getState();
 * const display = adapter.getDisplay();
 * const led = adapter.getLedState();
 * ```
 */

class EmsWasmAdapter extends LogicLayerAdapter {
    constructor(wasmPath = 'wasm/emc_test.js') {
        super();  // ✅ 调用父类构造函数
        
        this.wasmPath = wasmPath;
        
        // 按键和事件常量（与C代码一致）
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
        
        // WASM函数引用
        this.funcs = {};
        
        // ✅ LED状态缓存（从回调接收）
        this._currentLedState = null;
        
        // ✅ 显示更新回调（单向数据流）
        this._onDisplayUpdate = null;
        
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
     * 初始化WASM模块
     * @returns {Promise} 初始化完成的Promise
     */
    async init() {
        if (this.initialized) {
            console.log('[EmsWasmAdapter] 已初始化，跳过');
            return;
        }
        
        return new Promise((resolve, reject) => {
            // 如果Module已经存在且完全初始化
            if (typeof Module !== 'undefined' && typeof Module.cwrap === 'function') {
                // 检查是否有readyPromise（Emscripten 5.x的标记）
                if (Module.ready) {
                    // WASM正在初始化或已完成，等待ready Promise
                    console.log('[EmsWasmAdapter] Module已存在，等待初始化完成...');
                    Module.ready.then(() => {
                        console.log('[EmsWasmAdapter] Module ready Promise resolved');
                        this._setupFunctions();
                        this._initializeLogic();
                        resolve();
                    }).catch(reject);
                    return;
                } else {
                    // 没有ready Promise，直接初始化
                    console.log('[EmsWasmAdapter] Module已存在，直接初始化');
                    this._setupFunctions();
                    this._initializeLogic();
                    resolve();
                    return;
                }
            }
            
            // 加载WASM脚本
            const script = document.createElement('script');
            script.src = this.wasmPath;
            
            script.onload = () => {
                console.log('[EmsWasmAdapter] WASM脚本加载成功');
                
                // 等待WASM运行时初始化
                if (Module.ready) {
                    Module.ready.then(() => {
                        console.log('[EmsWasmAdapter] WASM运行时初始化完成');
                        this._setupFunctions();
                        this._initializeLogic();
                        resolve();
                    }).catch(reject);
                } else {
                    // 兼容旧版本Emscripten
                    Module.onRuntimeInitialized = () => {
                        console.log('[EmsWasmAdapter] WASM运行时初始化完成');
                        this._setupFunctions();
                        this._initializeLogic();
                        resolve();
                    };
                }
            };
            
            script.onerror = (error) => {
                console.error('[EmsWasmAdapter] WASM脚本加载失败:', error);
                reject(new Error(`WASM加载失败: ${this.wasmPath}`));
            };
            
            document.head.appendChild(script);
        });
    }
    
    /**
     * 设置WASM函数包装
     * @private
     */
    _setupFunctions() {
        this.funcs = {
            // 核心控制
            init: Module.cwrap('emc_initialize', null, []),
            cleanup: Module.cwrap('emc_cleanup', null, []),
            runCycle: Module.cwrap('emc_run_cycle', null, []),
            
            // 按键输入
            keyPress: Module.cwrap('emc_simulate_key_press', null, ['number', 'number']),
            
            // 状态查询
            getState: Module.cwrap('emc_get_current_state', 'number', []),
            
            // 显示输出（使用静态缓冲区）
            getSegPtr: Module.cwrap('emc_get_seg_buffer_ptr', 'number', []),
            getDpMask: Module.cwrap('emc_get_dp_mask', 'number', []),
            getColonMask: Module.cwrap('emc_get_colon_mask', 'number', []),
            
            // 回调注册
            registerCallbacks: Module.cwrap('emc_register_callbacks', null, ['number', 'number', 'number', 'number'])
        };
        
        // ✅ LED状态指针（使用try-catch安全检测）
        try {
            const testFunc = Module.cwrap('emc_get_led_state_ptr', 'number', []);
            // 尝试调用一次验证
            const ptr = testFunc();
            if (ptr) {
                this.funcs.getLedStatePtr = testFunc;
                console.log('[EmsWasmAdapter] getLedStatePtr: ✅ 已导出并可调用');
            } else {
                console.log('[EmsWasmAdapter] getLedStatePtr: ⚠️ 已导出但返回null');
                this.funcs.getLedStatePtr = null;
            }
        } catch (error) {
            console.log('[EmsWasmAdapter] getLedStatePtr: ❌ 未导出，将使用推断机制');
            console.log('  错误:', error.message);
            this.funcs.getLedStatePtr = null;
        }
        
        console.log('[EmsWasmAdapter] 函数包装完成');
    }
    
    /**
     * 初始化逻辑层并注册回调
     * @private
     */
    _initializeLogic() {
        // 初始化EMC逻辑
        this.funcs.init();
        
        // ✅ 注册回调函数（包括硬件输入回调）
        this._registerCallbacks();
        
        // 运行一个周期以确保显示缓冲区初始化
        this.funcs.runCycle();
        
        this.initialized = true;
        console.log('[EmsWasmAdapter] 逻辑层初始化完成');
    }
    
    /**
     * 注册回调函数（必须）
     * @private
     */
    _registerCallbacks() {
        console.log('[EmsWasmAdapter] 注册回调函数...');
        
        // 创建硬件输入回调 - 返回默认值
        const hwInputsCallback = (hwPtr) => {
            // 从HEAPU8中写入默认硬件输入
            // HardwareInputs_t结构：water_temp(2字节), water_level_ok(1字节), pot_present(1字节), fault_code(1字节)
            Module.HEAPU16[hwPtr / 2] = 850;    // water_temp = 85.0°C
            Module.HEAPU8[hwPtr + 2] = 1;       // water_level_ok = 1
            Module.HEAPU8[hwPtr + 3] = 1;       // pot_present = 1
            Module.HEAPU8[hwPtr + 4] = 0;       // fault_code = 0
        };
        
        // 创建显示输出回调（✅ 单向数据流：WASM主动推送显示数据）
        const dispOutputCallback = (segPtr, ledPtr) => {
            // 从WASM内存读取段码
            const seg = [
                Module.HEAPU8[segPtr],
                Module.HEAPU8[segPtr + 1],
                Module.HEAPU8[segPtr + 2],
                Module.HEAPU8[segPtr + 3]
            ];
            
            // ✅ 从WASM内存读取LED状态（16位位域）
            const ledValue = Module.HEAPU8[ledPtr] | (Module.HEAPU8[ledPtr + 1] << 8);
            
            // ✅ 解析LED位域（与C代码LedState_t结构完全对应）
            const led = {
                power: !!(ledValue & 0x01),           // bit 0: power_led
                func_leds: (ledValue >> 1) & 0x03FF,  // bit 1-10: func_leds (10位)
                add_water: !!(ledValue & (1 << 11)),  // bit 11: add_water_led
                add_time: !!(ledValue & (1 << 12)),   // bit 12: add_time_led
                water_disp: !!(ledValue & (1 << 13)), // bit 13: water_disp_led
                time_disp: !!(ledValue & (1 << 14))   // bit 14: time_disp_led
            };
            
            // ✅ 缓存最新LED状态
            this._currentLedState = led;
            
            // ✅ 触发显示更新事件（供UI层监听）
            if (this._onDisplayUpdate) {
                this._onDisplayUpdate(seg, led);
            }
        };
        
        // 创建蜂鸣器回调（✅ 调用JS层的onBuzzerOutput）
        const buzzerCallback = (cmd) => {
            console.log('[EmsWasmAdapter] 🔊 BUZZER回调: cmd=' + cmd);
            try {
                // ✅ 调用全局的onBuzzerOutput函数
                if (typeof onBuzzerOutput === 'function') {
                    onBuzzerOutput(cmd);
                }
            } catch (error) {
                console.error('[EmsWasmAdapter] BUZZER回调错误:', error);
            }
        };
        
        // 创建状态变化回调（✅ 更新AppState.dllState.state）
        const statusChangeCallback = (type, valPtr) => {
            // type: 1=STATUS_STATE_CHANGE, valPtr指向新的状态值（uint8_t*）
            if (type === 1 && valPtr) {
                // ✅ 从WASM内存中读取状态值
                const newState = Module.HEAPU8[valPtr];
                if (typeof AppState !== 'undefined' && AppState.dllState) {
                    AppState.dllState.state = newState;
                    console.log(`[EmsWasmAdapter] 🔄 状态变化: ${newState}`);
                }
            }
        };
        
        // 将JS函数转换为C回调指针
        const hwCallbackPtr = Module.addFunction(hwInputsCallback, 'vi');
        const dispCallbackPtr = Module.addFunction(dispOutputCallback, 'vii');
        const buzzerCallbackPtr = Module.addFunction(buzzerCallback, 'vi');
        const statusCallbackPtr = Module.addFunction(statusChangeCallback, 'vii');
        
        // 注册所有回调
        if (this.funcs.registerCallbacks) {
            this.funcs.registerCallbacks(dispCallbackPtr, buzzerCallbackPtr, statusCallbackPtr, hwCallbackPtr);
            console.log('[EmsWasmAdapter] ✅ 所有回调函数已注册');
            console.log('  - 硬件输入回调: 水温85°C, 有水, 有锅');
            console.log('  - 显示输出回调: 已注册（单向数据流）');
            console.log('  - 蜂鸣器回调: 已注册');
            console.log('  - 状态变化回调: 已注册');
        }
        
        // 保存回调指针以便清理
        this._callbackPointers = [
            hwCallbackPtr,
            dispCallbackPtr,
            buzzerCallbackPtr,
            statusCallbackPtr
        ];
    }
    
    /**
     * 设置显示更新回调（单向数据流）
     * @param {Function} callback - (seg, led) => void
     */
    setDisplayUpdateCallback(callback) {
        this._onDisplayUpdate = callback;
    }
    
    /**
     * 清理资源
     */
    /**
     * 清理资源（重写父类方法）
     */
    cleanup() {
        // ✅ 先调用父类清理（停止周期、重置状态）
        super.cleanup();
        
        // 释放回调函数指针
        if (this._callbackPointers) {
            this._callbackPointers.forEach(ptr => {
                if (ptr) Module.removeFunction(ptr);
            });
            this._callbackPointers = null;
            console.log('[EmsWasmAdapter] 回调函数已释放');
        }
        
        // ✅ WASM可能没有导出cleanup函数，安全调用
        if (this.funcs.cleanup && typeof this.funcs.cleanup === 'function') {
            try {
                this.funcs.cleanup();
                console.log('[EmsWasmAdapter] cleanup调用成功');
            } catch (error) {
                console.warn('[EmsWasmAdapter] cleanup调用失败（可忽略）:', error.message);
            }
        } else {
            console.log('[EmsWasmAdapter] cleanup函数未导出（正常）');
        }
        
        console.log('[EmsWasmAdapter] 清理完成');
    }
    
    // =========================================================================
    // 核心控制方法
    // =========================================================================
    
    /**
     * 运行单个10ms周期
     */
    runCycle() {
        if (!this.initialized) {
            throw new Error('WASM未初始化，请先调用init()');
        }
        this.funcs.runCycle();
    }
    
    /**
     * 运行n个周期
     * @param {number} n - 周期数
     */
    runCycles(n) {
        for (let i = 0; i < n; i++) {
            this.runCycle();
        }
    }
    
    // =========================================================================
    // 按键控制方法
    // =========================================================================
    
    /**
     * 发送按键事件
     * @param {number} keyCode - 按键代码
     * @param {number} event - 事件类型
     */
    pressKey(keyCode, event = this.EVENTS.EVENT_SHORT) {
        if (!this.initialized) {
            throw new Error('WASM未初始化');
        }
        
        // 📊 详细日志：记录按键输入
        const eventName = ['NULL', 'PRESS', 'SHORT', 'LONG', 'REPEAT', 'RELEASE'][event] || `UNKNOWN(${event})`;
        console.log('[EmsWasmAdapter.pressKey] 发送按键事件:', {
            keyCode: keyCode,
            event: event,
            eventName: eventName,
            initialized: this.initialized
        });
        
        this.funcs.keyPress(keyCode, event);
        
        console.log('[EmsWasmAdapter.pressKey] ✅ keyPress调用完成');
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
     * 完整按键（按下+释放）
     * @param {number} keyCode - 按键代码
     * @param {number} holdTime - 按住时间（ms），默认50ms
     */
    async clickKey(keyCode, holdTime = 50) {
        // 按下
        this.pressKey(keyCode, this.EVENTS.EVENT_PRESS);
        this.runCycle();
        
        // 等待
        await this.sleep(holdTime);
        
        // 释放
        this.pressKey(keyCode, this.EVENTS.EVENT_RELEASE);
        this.runCycle();
        
        // 短暂延迟
        await this.sleep(50);
    }
    
    /**
     * 长按按键
     * @param {number} keyCode - 按键代码
     * @param {number} duration - 长按时长（ms），默认1500ms
     */
    async longPressKey(keyCode, duration = 1500) {
        // 按下
        this.pressKey(keyCode, this.EVENTS.EVENT_PRESS);
        
        // 等待长按时间
        await this.sleep(duration);
        
        // 释放
        this.pressKey(keyCode, this.EVENTS.EVENT_RELEASE);
        this.runCycle();
    }
    
    // =========================================================================
    // 状态查询方法
    // =========================================================================
    
    /**
     * 获取当前状态码
     * @returns {number} 状态码
     */
    getState() {
        return this.funcs.getState();
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
    
    // =========================================================================
    // 显示输出方法
    // =========================================================================
    
    /**
     * 读取数码管显示
     * @returns {Object} 显示信息
     *   - seg: 段码数组 [seg0, seg1, seg2, seg3]
     *   - dp: 小数点掩码
     *   - colon: 冒号掩码
     *   - display: 可读字符串
     */
    getDisplay() {
        const segPtr = this.funcs.getSegPtr();
        const dpMask = this.funcs.getDpMask();
        const colonMask = this.funcs.getColonMask();
        
        // 从WASM内存读取段码
        const segArray = [];
        for (let i = 0; i < 4; i++) {
            segArray.push(Module.HEAPU8[segPtr + i]);
        }
        
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
    
    // =========================================================================
    // LED状态方法
    // =========================================================================
    
    /**
     * 读取LED状态
     * @returns {Object} LED状态
     */
    getLedState() {
        // ✅ 返回缓存的LED状态（从回调接收）
        if (this._currentLedState) {
            return this._currentLedState;
        }
        
        // 如果还没有收到回调，返回默认状态
        console.warn('[EmsWasmAdapter] LED状态尚未初始化，返回默认值');
        return {
            power: false,
            func_leds: 0,
            add_water: false,
            add_time: false,
            water_disp: false,
            time_disp: false
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
    
    // =========================================================================
    // 综合报告方法
    // =========================================================================
    
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
    
    // =========================================================================
    // 工具方法
    // =========================================================================
    
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
        
        console.warn(`[EmsWasmAdapter] 等待状态超时: ${targetState}`);
        return false;
    }
}

// 导出适配器类（如果在模块环境中）
if (typeof module !== 'undefined' && module.exports) {
    module.exports = EmsWasmAdapter;
}
