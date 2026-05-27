/**
 * WASM 版本的对比测试工具
 * 直接使用编译好的 emc_core.wasm，无需后端服务器
 */

class WasmComparisonTester {
    constructor() {
        this.jsLogic = null;
        this.wasmModule = null;
        this.wasmFunctions = {};
        this.testResults = [];
        this.isRunning = false;
        
        // 输出记录
        this.jsOutputs = [];
        this.wasmOutputs = [];
    }
    
    /**
     * 初始化JS逻辑层
     */
    async initJSLogic() {
        console.log('[WasmComparisonTester] 初始化JS逻辑层...');
        
        if (typeof EmcLogic === 'undefined') {
            throw new Error('EmcLogic未定义，请确保emc_logic_js.js已加载');
        }
        
        this.jsLogic = new EmcLogic();
        
        // 注册回调
        this.jsLogic.registerCallbacks(
            (seg, led) => this.captureJSOutput('display', { seg, led }),
            (cmd) => this.captureJSOutput('buzzer', { cmd }),
            (type, value) => this.captureJSOutput('status', { type, value })
        );
        
        // 初始化
        this.jsLogic.init();
        
        console.log('[WasmComparisonTester] JS逻辑层初始化完成');
    }
    
    /**
     * 初始化WASM逻辑层
     */
    async initWasmLogic() {
        console.log('[WasmComparisonTester] 初始化WASM逻辑层...');
        
        // 如果已经初始化过，直接返回
        if (this.wasmFunctions && this.wasmFunctions.init) {
            console.log('[WasmComparisonTester] WASM已初始化，跳过');
            return;
        }
        
        return new Promise((resolve, reject) => {
            // 检查Module是否已存在且完全初始化
            if (typeof Module !== 'undefined' && typeof Module.cwrap === 'function') {
                console.log('[WasmComparisonTester] Module已存在，直接使用');
                this.setupWasmFunctions();
                this.wasmFunctions.init();
                console.log('[WasmComparisonTester] WASM逻辑层初始化完成');
                resolve();
                return;
            }
            
            // 加载WASM模块
            const script = document.createElement('script');
            script.src = 'wasm/emc_test.js'; // 使用本地复制的WASM文件
            
            script.onload = () => {
                console.log('[WasmComparisonTester] WASM脚本加载成功，等待运行时初始化...');
                
                // WASM模块加载完成后
                window.Module.onRuntimeInitialized = () => {
                    console.log('[WasmComparisonTester] WASM运行时初始化完成');
                    this.setupWasmFunctions();
                    this.wasmFunctions.init();
                    console.log('[WasmComparisonTester] WASM逻辑层初始化完成');
                    resolve();
                };
            };
            
            script.onerror = (error) => {
                console.error('[WasmComparisonTester] WASM脚本加载失败:', error);
                reject(new Error('WASM模块加载失败，请检查wasm/emc_test.js是否存在'));
            };
            
            document.head.appendChild(script);
        });
    }
    
    /**
     * 设置WASM函数
     */
    setupWasmFunctions() {
        this.wasmFunctions = {
            init: Module.cwrap('emc_initialize', null, []),
            runCycle: Module.cwrap('emc_run_cycle', null, []),
            keyInput: Module.cwrap('emc_simulate_key_press', null, ['number', 'number']),
            getState: Module.cwrap('emc_get_current_state', 'number', []),
            // 使用静态缓冲区API（无需malloc）
            getSegBufferPtr: Module.cwrap('emc_get_seg_buffer_ptr', 'number', []),
            getDpMask: Module.cwrap('emc_get_dp_mask', 'number', []),
            getColonMask: Module.cwrap('emc_get_colon_mask', 'number', [])
        };
        console.log('[WasmComparisonTester] WASM函数设置完成');
    }
    
    /**
     * 捕获JS输出
     */
    captureJSOutput(type, data) {
        const record = {
            timestamp: Date.now(),
            source: 'JS',
            type: type,
            data: JSON.parse(JSON.stringify(data))
        };
        
        this.jsOutputs.push(record);
    }
    
    /**
     * 捕获WASM输出（主动查询模式，与Python版本一致）
     */
    captureWasmState() {
        // 主动查询当前状态
        const state = this.wasmFunctions.getState();
        
        // 记录状态变化
        if (this.lastWasmState !== state) {
            this.captureWasmOutput('status', { type: 1, value: state });
            this.lastWasmState = state;
        }
        
        // 主动查询显示输出（使用静态缓冲区API）
        const segPtr = this.wasmFunctions.getSegBufferPtr();
        const dpMask = this.wasmFunctions.getDpMask();
        const colonMask = this.wasmFunctions.getColonMask();
        
        // 从WASM内存中读取段码数据
        const seg = [];
        for (let i = 0; i < 4; i++) {
            seg.push(Module.HEAPU8[segPtr + i]);
        }
        
        // 记录显示更新
        this.captureWasmOutput('display', {
            seg: seg,
            dp_mask: dpMask,
            colon_mask: colonMask
        });
    }
    
    /**
     * 记录WASM输出
     */
    captureWasmOutput(type, data) {
        const record = {
            timestamp: Date.now(),
            source: 'WASM',
            type: type,
            data: JSON.parse(JSON.stringify(data))
        };
        
        this.wasmOutputs.push(record);
    }
    
    /**
     * 运行测试序列
     */
    async runTestSequence(testName, sequence) {
        console.log(`\n========== 开始测试: ${testName} ==========`);
        
        this.isRunning = true;
        this.jsOutputs = [];
        this.wasmOutputs = [];
        this.lastWasmState = -1; // 重置上次状态
        
        // 重置两个逻辑层
        this.jsLogic.init();
        this.wasmFunctions.init();
        
        // 运行初始周期，让系统进入稳定状态（与Python版本一致）
        console.log('[初始化] 运行50个周期等待系统稳定...');
        for (let i = 0; i < 50; i++) {
            this.jsLogic.runCycle();
            this.wasmFunctions.runCycle();
            this.captureWasmState();
        }
        
        // 执行测试序列
        for (const step of sequence) {
            await this.executeStep(step);
        }
        
        // 对比结果
        const result = this.compareOutputs();
        this.testResults.push({
            testName: testName,
            passed: result.passed,
            differences: result.differences
        });
        
        this.isRunning = false;
        
        console.log(`========== 测试完成: ${testName} ==========\n`);
        
        return result;
    }
    
    /**
     * 执行单个步骤
     */
    async executeStep(step) {
        console.log(`[Step] ${step.action}`, step.params);
        
        switch (step.action) {
            case 'wait':
                // 等待指定时间，期间运行多个周期
                const cycles = Math.floor(step.duration / 10);
                for (let i = 0; i < cycles; i++) {
                    // JS执行
                    this.jsLogic.runCycle();
                    
                    // WASM执行
                    this.wasmFunctions.runCycle();
                    
                    // 捕获WASM状态（每个周期后查询）
                    this.captureWasmState();
                }
                await this.sleep(step.duration);
                break;
                
            case 'key_press':
                // 按键按下 - JS
                this.jsLogic.keyInput(step.params.keyCode, 1); // KEY_EVENT_PRESS
                
                // 按键按下 - WASM
                this.wasmFunctions.keyInput(step.params.keyCode, 1);
                
                // 运行一个周期处理按键（与Python版本一致）
                this.jsLogic.runCycle();
                this.wasmFunctions.runCycle();
                this.captureWasmState();
                
                await this.sleep(step.params.holdTime || 50);
                break;
                
            case 'key_release':
                // 按键释放 - JS
                this.jsLogic.keyInput(step.params.keyCode, 5); // KEY_EVENT_RELEASE
                
                // 按键释放 - WASM
                this.wasmFunctions.keyInput(step.params.keyCode, 5);
                
                // 运行一个周期处理按键释放
                this.jsLogic.runCycle();
                this.wasmFunctions.runCycle();
                this.captureWasmState();
                
                await this.sleep(50);
                break;
                
            case 'key_click':
                // 完整按键（按下+释放）
                await this.executeStep({
                    action: 'key_press',
                    params: step.params
                });
                await this.executeStep({
                    action: 'key_release',
                    params: step.params
                });
                break;
                
            default:
                console.warn(`[Warning] 未知的动作: ${step.action}`);
        }
    }
    
    /**
     * 对比输出
     */
    compareOutputs() {
        const differences = [];
        
        // 对比状态变化
        const jsStates = this.jsOutputs.filter(o => o.type === 'status');
        const wasmStates = this.wasmOutputs.filter(o => o.type === 'status');
        
        if (jsStates.length !== wasmStates.length) {
            differences.push({
                type: 'state_count',
                js: jsStates.length,
                wasm: wasmStates.length,
                message: `状态变化次数不同: JS=${jsStates.length}, WASM=${wasmStates.length}`
            });
        }
        
        // 对比显示输出
        const jsDisplays = this.jsOutputs.filter(o => o.type === 'display');
        const wasmDisplays = this.wasmOutputs.filter(o => o.type === 'display');
        
        if (jsDisplays.length !== wasmDisplays.length) {
            differences.push({
                type: 'display_count',
                js: jsDisplays.length,
                wasm: wasmDisplays.length,
                message: `显示更新次数不同: JS=${jsDisplays.length}, WASM=${wasmDisplays.length}`
            });
        }
        
        // 对比蜂鸣器输出
        const jsBuzzers = this.jsOutputs.filter(o => o.type === 'buzzer');
        const wasmBuzzers = this.wasmOutputs.filter(o => o.type === 'buzzer');
        
        if (jsBuzzers.length !== wasmBuzzers.length) {
            differences.push({
                type: 'buzzer_count',
                js: jsBuzzers.length,
                wasm: wasmBuzzers.length,
                message: `蜂鸣器触发次数不同: JS=${jsBuzzers.length}, WASM=${wasmBuzzers.length}`
            });
        }
        
        const passed = differences.length === 0;
        
        console.log(`\n[对比结果]`);
        console.log(`  状态变化: JS=${jsStates.length}, WASM=${wasmStates.length}`);
        console.log(`  显示更新: JS=${jsDisplays.length}, WASM=${wasmDisplays.length}`);
        console.log(`  蜂鸣器:   JS=${jsBuzzers.length}, WASM=${wasmBuzzers.length}`);
        console.log(`  差异数:   ${differences.length}`);
        console.log(`  测试结果: ${passed ? '✅ PASS' : '❌ FAIL'}\n`);
        
        if (!passed) {
            differences.forEach(diff => {
                console.log(`  [差异] ${diff.message}`);
            });
        }
        
        return { passed, differences };
    }
    
    /**
     * 睡眠辅助函数
     */
    sleep(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
}

// 导出
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { WasmComparisonTester };
}
