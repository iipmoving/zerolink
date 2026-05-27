/**
 * 对比测试工具 - 同时运行C DLL和JS逻辑
 * 
 * 功能：
 * 1. 向两个逻辑层发送相同的按键序列
 * 2. 捕获并对比所有输出（状态、数码管、LED、蜂鸣器）
 * 3. 生成详细的差异报告
 */

class ComparisonTester {
    constructor() {
        this.jsLogic = null;
        this.dllLogic = null;
        this.testResults = [];
        this.isRunning = false;
        
        // 输出记录
        this.jsOutputs = [];
        this.dllOutputs = [];
    }
    
    /**
     * 初始化JS逻辑层
     */
    async initJSLogic() {
        console.log('[ComparisonTester] 初始化JS逻辑层...');
        
        // 加载EmcLogic类
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
        
        console.log('[ComparisonTester] JS逻辑层初始化完成');
    }
    
    /**
     * 初始化DLL逻辑层（通过Python后端）
     */
    async initDLLLogic() {
        console.log('[ComparisonTester] 初始化DLL逻辑层...');
        
        try {
            // 调用后端API初始化DLL
            const controller = new AbortController();
            const timeoutId = setTimeout(() => controller.abort(), 5000); // 5秒超时
            
            const response = await fetch('http://localhost:5000/api/init', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                signal: controller.signal
            });
            
            clearTimeout(timeoutId);
            
            if (!response.ok) {
                throw new Error(`HTTP错误: ${response.status} ${response.statusText}`);
            }
            
            const result = await response.json();
            if (!result.success) {
                throw new Error(result.error || 'DLL初始化失败');
            }
            
            console.log('[ComparisonTester] DLL逻辑层初始化完成');
            
        } catch (error) {
            console.error('[ComparisonTester] DLL初始化失败:', error);
            throw error;
        }
    }
    
    /**
     * 捕获JS输出
     */
    captureJSOutput(type, data) {
        const record = {
            timestamp: Date.now(),
            source: 'JS',
            type: type,
            data: JSON.parse(JSON.stringify(data)) // 深拷贝
        };
        
        this.jsOutputs.push(record);
        console.log(`[JS Output] ${type}:`, data);
    }
    
    /**
     * 捕获DLL输出（从后端获取）
     */
    async captureDLLOutputs() {
        try {
            const response = await fetch('http://localhost:5000/api/get_outputs');
            if (response.ok) {
                const data = await response.json();
                if (data.success && data.outputs) {
                    this.dllOutputs = data.outputs.map(output => ({
                        timestamp: Date.now(),
                        source: 'DLL',
                        type: output.type,
                        data: output.data
                    }));
                }
            }
        } catch (error) {
            console.error('[ComparisonTester] 获取DLL输出失败:', error);
        }
    }
    
    /**
     * 运行测试序列
     */
    async runTestSequence(testName, sequence) {
        console.log(`\n========== 开始测试: ${testName} ==========`);
        
        this.isRunning = true;
        this.jsOutputs = [];
        this.dllOutputs = [];
        
        // 重置两个逻辑层
        this.jsLogic.init();
        
        // 清空DLL输出
        await fetch('http://localhost:5000/api/clear_outputs', {
            method: 'POST'
        });
        
        // 重新初始化DLL
        await fetch('http://localhost:5000/api/init', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' }
        });
        
        // 启动DLL周期运行
        await fetch('http://localhost:5000/api/start_cycle', {
            method: 'POST'
        });
        
        // 执行测试序列
        for (const step of sequence) {
            await this.executeStep(step);
        }
        
        // 停止DLL周期运行
        await fetch('http://localhost:5000/api/stop_cycle', {
            method: 'POST'
        });
        
        // 获取DLL输出
        await this.captureDLLOutputs();
        
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
                    this.jsLogic.runCycle();
                    // DLL由后端自动运行周期
                }
                await this.sleep(step.duration);
                break;
                
            case 'key_press':
                // 按键按下 - JS
                this.jsLogic.keyInput(step.params.keyCode, 1); // KEY_EVENT_PRESS
                
                // 按键按下 - DLL（通过后端）
                await fetch('http://localhost:5000/api/key_input', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({
                        key_code: step.params.keyCode,
                        event: 1
                    })
                });
                
                await this.sleep(step.params.holdTime || 50);
                break;
                
            case 'key_release':
                // 按键释放 - JS
                this.jsLogic.keyInput(step.params.keyCode, 5); // KEY_EVENT_RELEASE
                
                // 按键释放 - DLL（通过后端）
                await fetch('http://localhost:5000/api/key_input', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({
                        key_code: step.params.keyCode,
                        event: 5
                    })
                });
                
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
        const dllStates = this.dllOutputs.filter(o => o.type === 'status');
        
        if (jsStates.length !== dllStates.length) {
            differences.push({
                type: 'state_count',
                js: jsStates.length,
                dll: dllStates.length,
                message: `状态变化次数不同: JS=${jsStates.length}, DLL=${dllStates.length}`
            });
        }
        
        // 对比显示输出
        const jsDisplays = this.jsOutputs.filter(o => o.type === 'display');
        const dllDisplays = this.dllOutputs.filter(o => o.type === 'display');
        
        if (jsDisplays.length !== dllDisplays.length) {
            differences.push({
                type: 'display_count',
                js: jsDisplays.length,
                dll: dllDisplays.length,
                message: `显示更新次数不同: JS=${jsDisplays.length}, DLL=${dllDisplays.length}`
            });
        }
        
        // 对比蜂鸣器输出
        const jsBuzzers = this.jsOutputs.filter(o => o.type === 'buzzer');
        const dllBuzzers = this.dllOutputs.filter(o => o.type === 'buzzer');
        
        if (jsBuzzers.length !== dllBuzzers.length) {
            differences.push({
                type: 'buzzer_count',
                js: jsBuzzers.length,
                dll: dllBuzzers.length,
                message: `蜂鸣器触发次数不同: JS=${jsBuzzers.length}, DLL=${dllBuzzers.length}`
            });
        }
        
        const passed = differences.length === 0;
        
        console.log(`\n[对比结果]`);
        console.log(`  状态变化: JS=${jsStates.length}, DLL=${dllStates.length}`);
        console.log(`  显示更新: JS=${jsDisplays.length}, DLL=${dllDisplays.length}`);
        console.log(`  蜂鸣器:   JS=${jsBuzzers.length}, DLL=${dllBuzzers.length}`);
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
     * 生成测试报告
     */
    generateReport() {
        console.log('\n========== 测试报告 ==========');
        
        const totalTests = this.testResults.length;
        const passedTests = this.testResults.filter(r => r.passed).length;
        const failedTests = totalTests - passedTests;
        
        console.log(`总测试数: ${totalTests}`);
        console.log(`通过: ${passedTests}`);
        console.log(`失败: ${failedTests}`);
        console.log(`通过率: ${(passedTests / totalTests * 100).toFixed(1)}%`);
        
        if (failedTests > 0) {
            console.log('\n失败的测试:');
            this.testResults
                .filter(r => !r.passed)
                .forEach(r => {
                    console.log(`  ❌ ${r.testName}`);
                    r.differences.forEach(diff => {
                        console.log(`     - ${diff.message}`);
                    });
                });
        }
        
        console.log('================================\n');
        
        return {
            total: totalTests,
            passed: passedTests,
            failed: failedTests,
            passRate: (passedTests / totalTests * 100).toFixed(1),
            details: this.testResults
        };
    }
    
    /**
     * 睡眠辅助函数
     */
    sleep(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
}

// ============================================================================
// 预定义的测试用例
// ============================================================================

const TestCases = {
    /**
     * 测试1: 上电自检流程
     */
    powerOnSelftest: [
        { action: 'wait', duration: 3000 } // 等待3秒，观察S_POWER_ON → S_VERSION → S_STANDBY
    ],
    
    /**
     * 测试2: 待机状态下按键响应
     */
    standbyKeyPress: [
        { action: 'wait', duration: 3000 }, // 等待进入待机
        { action: 'key_click', params: { keyCode: 1, holdTime: 50 } }, // M1短按
        { action: 'wait', duration: 500 }
    ],
    
    /**
     * 测试3: 功能选择流程
     */
    functionSelect: [
        { action: 'wait', duration: 3000 }, // 等待进入待机
        { action: 'key_click', params: { keyCode: 1, holdTime: 50 } }, // M1
        { action: 'wait', duration: 500 },
        { action: 'key_click', params: { keyCode: 2, holdTime: 50 } }, // M2
        { action: 'wait', duration: 500 }
    ],
    
    /**
     * 测试4: 参数调整
     */
    parameterAdjust: [
        { action: 'wait', duration: 3000 }, // 等待进入待机
        { action: 'key_click', params: { keyCode: 1, holdTime: 50 } }, // M1进入功能选择
        { action: 'wait', duration: 500 },
        { action: 'key_click', params: { keyCode: 11, holdTime: 50 } }, // 加水
        { action: 'key_click', params: { keyCode: 11, holdTime: 50 } }, // 加水
        { action: 'key_click', params: { keyCode: 12, holdTime: 50 } }, // 加时间
        { action: 'wait', duration: 500 }
    ],
    
    /**
     * 测试5: 启动烹饪
     */
    startCooking: [
        { action: 'wait', duration: 3000 }, // 等待进入待机
        { action: 'key_click', params: { keyCode: 1, holdTime: 50 } }, // M1
        { action: 'wait', duration: 500 },
        { action: 'key_click', params: { keyCode: 13, holdTime: 50 } }, // 开始/暂停
        { action: 'wait', duration: 2000 } // 观察烹饪状态
    ]
};

// 导出
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { ComparisonTester, TestCases };
}
