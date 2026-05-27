/**
 * 按键处理模块 - 长按松开检测模式
 */

// 按键事件常量（完整定义）
const KeyEvent = {
    KEY_EVENT_NULL: 0,
    KEY_EVENT_PRESS: 1,
    EVENT_SHORT: 2,      // 兼容旧名称
    KEY_EVENT_SHORT: 2,
    EVENT_LONG: 3,       // 兼容旧名称
    KEY_EVENT_LONG: 3,
    KEY_EVENT_REPEAT: 4,
    KEY_EVENT_RELEASE: 5
};

/**
 * 按键配置
 */
class KeyConfig {
    constructor(keyCode, name = "", triggerMode = "press", thresholdMs = 1500) {
        this.keyCode = keyCode;
        this.name = name;
        this.triggerMode = triggerMode;  // "press" 或 "release"
        this.thresholdMs = thresholdMs;
    }
    
    isLongPressMode() {
        return this.triggerMode === "release";
    }
}

/**
 * 按键处理类
 */
class KeyHandler {
    constructor() {
        // 按键配置表 {keyCode: KeyConfig}
        this.configs = {};
        
        // 当前按下状态 {keyCode: pressTimeMs}
        this.pressedKeys = {};
        
        // 事件队列
        this.eventQueue = [];
    }
    
    /**
     * 设置按键配置表
     */
    setConfigs(configs) {
        this.configs = {};
        if (Array.isArray(configs)) {
            configs.forEach(cfg => {
                this.configs[cfg.keyCode] = cfg;
            });
        } else {
            Object.assign(this.configs, configs);
        }
    }
    
    /**
     * 获取按键配置
     */
    getConfig(keyCode) {
        return this.configs[keyCode];
    }
    
    /**
     * 按键按下
     */
    pressKey(keyCode) {
        const cfg = this.configs[keyCode];
        
        if (cfg && cfg.isLongPressMode()) {
            // release模式：只记录时间，不触发
            this.pressedKeys[keyCode] = Date.now();
            return null;
        } else {
            // press模式：立即触发短按
            return { keyCode, eventType: KeyEvent.EVENT_SHORT };
        }
    }
    
    /**
     * 按键松开
     */
    releaseKey(keyCode) {
        const pressTime = this.pressedKeys[keyCode];
        if (pressTime === undefined) {
            return null;
        }
        
        delete this.pressedKeys[keyCode];
        
        // release模式：计算按住时长
        const elapsedMs = Date.now() - pressTime;
        const cfg = this.configs[keyCode];
        
        if (cfg && elapsedMs >= cfg.thresholdMs) {
            return { keyCode, eventType: KeyEvent.EVENT_LONG };
        } else {
            return { keyCode, eventType: KeyEvent.EVENT_SHORT };
        }
    }
    
    /**
     * 检查按键是否正在按住
     */
    isPressed(keyCode) {
        return keyCode in this.pressedKeys;
    }
    
    /**
     * 获取按键已按住时间（毫秒）
     */
    getPressElapsed(keyCode) {
        const pressTime = this.pressedKeys[keyCode];
        if (pressTime === undefined) {
            return 0;
        }
        return Date.now() - pressTime;
    }
}
