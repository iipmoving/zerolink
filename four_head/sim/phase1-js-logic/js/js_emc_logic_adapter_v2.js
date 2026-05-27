/**
 * JS逻辑层适配器（重写版）
 * 
 * 将EmcLogic包装为标准的LogicLayerAdapter接口
 * 确保JS和WASM使用完全相同的调用接口
 */

class JSEmcLogicAdapter extends LogicLayerAdapter {
    constructor() {
        super();
        this.emcLogic = null;
    }

    /**
     * 初始化JS逻辑层
     * @returns {Promise<void>}
     */
    async init() {
        console.log('[JSEmcLogicAdapter] 初始化JS逻辑层...');
        
        // 创建EmcLogic实例
        this.emcLogic = new EmcLogic();
        console.log('[JSEmcLogicAdapter] ✅ EmcLogic创建成功');
        
        // 注册回调函数到EmcLogic
        if (this.callbacks.onDisplayOutput) {
            this.emcLogic.registerCallbacks(
                this.callbacks.onDisplayOutput,
                this.callbacks.onBuzzerOutput,
                this.callbacks.onStateChange,
                this.callbacks.onHardwareInputs
            );
            console.log('[JSEmcLogicAdapter] ✅ 回调函数已注册');
        }
        
        // 初始化EmcLogic状态机（启动上电流程：S_POWER_ON → S_VERSION → S_STANDBY）
        this.emcLogic.init();
        console.log('[JSEmcLogicAdapter] ✅ EmcLogic状态机已初始化');
        
        this.initialized = true;
        console.log('[JSEmcLogicAdapter] ✅ 初始化完成');
    }

    /**
     * 清理资源
     */
    cleanup() {
        console.log('[JSEmcLogicAdapter] 清理JS逻辑层...');
        
        // 停止周期运行
        this.stopCycle();
        
        // 释放EmcLogic引用
        if (this.emcLogic) {
            this.emcLogic = null;
        }
        
        this.initialized = false;
        console.log('[JSEmcLogicAdapter] ✅ 清理完成');
    }

    /**
     * 运行单个周期
     */
    runCycle() {
        if (!this.initialized || !this.emcLogic) {
            console.warn('[JSEmcLogicAdapter] ⚠️ 未初始化，跳过周期');
            return;
        }
        
        this.emcLogic.runCycle();
    }

    /**
     * 模拟按键按下
     * @param {number} keyCode - 按键代码
     * @param {number} eventType - 事件类型（PRESS/SHORT/LONG/RELEASE）
     */
    pressKey(keyCode, eventType) {
        if (!this.initialized || !this.emcLogic) {
            console.warn('[JSEmcLogicAdapter] ⚠️ 未初始化，无法按键');
            return;
        }
        
        // 直接调用EmcLogic的keyInput方法
        this.emcLogic.keyInput(keyCode, eventType);
    }

    /**
     * 获取当前状态码
     * @returns {number} 状态码
     */
    getState() {
        if (!this.emcLogic) {
            return -1;
        }
        return this.emcLogic.getState();
    }

    /**
     * 获取显示输出
     * @returns {Object} { seg: [4], dp: number, colon: number, display: string }
     */
    getDisplay() {
        if (!this.emcLogic) {
            return { seg: [0x7F, 0x7F, 0x7F, 0x7F], dp: 0, colon: 0, display: '----' };
        }
        return this.emcLogic.getDisplay();
    }

    /**
     * 获取LED状态
     * @returns {Object} LED状态对象
     */
    getLedState() {
        if (!this.emcLogic) {
            return {};
        }
        return this.emcLogic.getLedState();
    }

    /**
     * 获取状态名称映射表
     * @returns {Object} { stateCode: stateName }
     */
    getStateNames() {
        return STATE_NAMES;
    }
}

// 导出适配器类
if (typeof module !== 'undefined' && module.exports) {
    module.exports = JSEmcLogicAdapter;
}
