/**
 * 事件总线 - 实现组件间解耦通信
 */
class EventBus {
    constructor() {
        this.listeners = new Map();
    }

    /**
     * 订阅事件
     * @param {string} eventName - 事件名称
     * @param {Function} callback - 回调函数
     */
    on(eventName, callback) {
        if (!this.listeners.has(eventName)) {
            this.listeners.set(eventName, []);
        }
        this.listeners.get(eventName).push(callback);
    }

    /**
     * 取消订阅
     * @param {string} eventName - 事件名称
     * @param {Function} callback - 回调函数（可选）
     */
    off(eventName, callback) {
        const callbacks = this.listeners.get(eventName);
        if (!callbacks) return;
        
        if (callback) {
            this.listeners.set(eventName, callbacks.filter(cb => cb !== callback));
        } else {
            this.listeners.delete(eventName);
        }
    }

    /**
     * 触发事件
     * @param {string} eventName - 事件名称
     * @param {*} data - 事件数据
     */
    emit(eventName, data) {
        const callbacks = this.listeners.get(eventName);
        if (!callbacks) return;
        
        callbacks.forEach(callback => {
            try {
                callback(data);
            } catch (error) {
                console.error(`[EventBus] 事件处理失败 ${eventName}:`, error);
            }
        });
    }

    /**
     * 一次性订阅
     * @param {string} eventName - 事件名称
     * @param {Function} callback - 回调函数
     */
    once(eventName, callback) {
        const onceCallback = (data) => {
            callback(data);
            this.off(eventName, onceCallback);
        };
        this.on(eventName, onceCallback);
    }
}

// 导出单例
const eventBus = new EventBus();
export { EventBus, eventBus };