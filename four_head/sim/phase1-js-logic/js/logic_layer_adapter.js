/**
 * 逻辑层适配器基类
 * 
 * 定义标准的逻辑层接口，所有逻辑层实现（JS/WASM/DLL）都必须遵循此接口
 * 界面层通过此接口与逻辑层交互，无需关心具体实现
 */

class LogicLayerAdapter {
    constructor() {
        this.initialized = false;
        this.cycleTimer = null;
        this.callbacks = {
            onDisplayOutput: null,
            onBuzzerOutput: null,
            onStateChange: null,
            onHardwareInputs: null
        };
    }

    /**
     * 初始化逻辑层
     * @returns {Promise<void>}
     */
    async init() {
        throw new Error('init() must be implemented by subclass');
    }

    /**
     * 清理资源
     */
    cleanup() {
        this.stopCycle();
        this.initialized = false;
    }

    /**
     * 注册回调函数
     * @param {Function} dispCb - 显示输出回调 (segPtr, ledPtr) => void
     * @param {Function} buzzerCb - 蜂鸣器回调 (cmd) => void
     * @param {Function} statusCb - 状态变化回调 (type, value) => void
     * @param {Function} hwCb - 硬件输入回调 (hwPtr) => void
     */
    registerCallbacks(dispCb, buzzerCb, statusCb, hwCb) {
        this.callbacks.onDisplayOutput = dispCb;
        this.callbacks.onBuzzerOutput = buzzerCb;
        this.callbacks.onStateChange = statusCb;
        this.callbacks.onHardwareInputs = hwCb;
    }

    /**
     * 运行单个周期
     */
    runCycle() {
        throw new Error('runCycle() must be implemented by subclass');
    }

    /**
     * 启动周期运行
     * @param {Function} onCycle - 每个周期结束时的回调
     * @param {number} interval - 周期间隔（毫秒），默认10ms
     */
    startCycle(onCycle, interval = 10) {
        if (this.cycleTimer) {
            console.warn('[LogicLayerAdapter] 周期已在运行');
            return;
        }

        this.cycleTimer = setInterval(() => {
            this.runCycle();
            if (onCycle) {
                onCycle();
            }
        }, interval);
    }

    /**
     * 停止周期运行
     */
    stopCycle() {
        if (this.cycleTimer) {
            clearInterval(this.cycleTimer);
            this.cycleTimer = null;
        }
    }

    /**
     * 模拟按键按下
     * @param {number} keyCode - 按键代码
     * @param {number} eventType - 事件类型（短按/长按/释放）
     */
    pressKey(keyCode, eventType) {
        throw new Error('pressKey() must be implemented by subclass');
    }

    /**
     * 获取当前状态码
     * @returns {number} 状态码
     */
    getState() {
        throw new Error('getState() must be implemented by subclass');
    }

    /**
     * 获取显示输出
     * @returns {Object} { seg: [4], dp: number, colon: number, display: string }
     */
    getDisplay() {
        throw new Error('getDisplay() must be implemented by subclass');
    }

    /**
     * 获取LED状态
     * @returns {Object} LED状态对象
     */
    getLedState() {
        throw new Error('getLedState() must be implemented by subclass');
    }

    /**
     * 获取状态名称映射表
     * @returns {Object} { stateCode: stateName }
     */
    getStateNames() {
        throw new Error('getStateNames() must be implemented by subclass');
    }
}

// 导出基类
if (typeof module !== 'undefined' && module.exports) {
    module.exports = LogicLayerAdapter;
}
