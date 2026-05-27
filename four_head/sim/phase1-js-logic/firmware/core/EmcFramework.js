/**
 * EMC框架主类 - 统一入口
 */
import { EventBus } from './EventBus.js';
import { StateManager } from './StateManager.js';
import { WasmLogicAdapter } from '../adapters/WasmLogicAdapter.js';
import { JsLogicAdapter } from '../adapters/JsLogicAdapter.js';
import { ControlFactory } from '../controls/ControlFactory.js';
import { ConfigParser } from '../config/ConfigParser.js';

class EmcFramework {
    constructor(options = {}) {
        // 初始化核心组件
        this.eventBus = new EventBus();
        this.stateManager = new StateManager();
        this.controlFactory = new ControlFactory(this.eventBus);
        this.configParser = new ConfigParser();
        
        // 逻辑适配器
        this.logicAdapter = null;
        
        // 配置选项
        this.options = {
            defaultConfig: options.defaultConfig || 'configs/default.json',
            logicType: options.logicType || 'js',  // 'js' | 'wasm'
            autoStart: options.autoStart || false,
            ...options
        };

        // 绑定事件处理
        this._bindEvents();

        console.log('[EmcFramework] 初始化完成');
    }

    /**
     * 初始化框架
     */
    async init() {
        // 初始化状态管理器
        this.stateManager.init(this.eventBus);

        // 根据配置选择逻辑适配器
        if (this.options.logicType === 'wasm') {
            this.logicAdapter = new WasmLogicAdapter();
        } else {
            this.logicAdapter = new JsLogicAdapter();
        }

        // 初始化逻辑适配器
        await this.logicAdapter.init();

        // 注册显示更新回调
        this.logicAdapter.setDisplayUpdateCallback((seg, led) => {
            this._onDisplayUpdate(seg, led);
        });

        // 如果有默认配置，加载它
        if (this.options.defaultConfig) {
            await this.loadConfig(this.options.defaultConfig);
        }

        // 如果自动启动，开始周期运行
        if (this.options.autoStart) {
            this.startDemo();
        }

        this.eventBus.emit('frameworkReady');
        console.log('[EmcFramework] 框架准备就绪');
    }

    /**
     * 加载配置
     * @param {string|Object} config - 配置URL或配置对象
     */
    async loadConfig(config) {
        let parsedConfig;

        if (typeof config === 'string') {
            // 从URL加载
            parsedConfig = await this.configParser.loadFromUrl(config);
        } else {
            // 直接解析对象
            parsedConfig = this.configParser.parse(config);
        }

        // 更新状态管理器
        this.stateManager.loadConfig(parsedConfig);

        // 加载控件到控件工厂
        this.controlFactory.loadControls(parsedConfig.controls || []);

        // 获取配置摘要
        const summary = this.configParser.getSummary(parsedConfig);
        console.log('[EmcFramework] 配置加载完成:', summary);

        this.eventBus.emit('configLoaded', {
            config: parsedConfig,
            summary: summary
        });

        return parsedConfig;
    }

    /**
     * 启动演示模式
     */
    startDemo() {
        if (!this.logicAdapter) {
            throw new Error('逻辑适配器未初始化');
        }

        // 设置模式为演示模式
        this.stateManager.set('mode', 'demo');

        // 启动周期运行
        this.logicAdapter.startCycle(10);

        console.log('[EmcFramework] 演示模式已启动');
        this.eventBus.emit('demoStarted');
    }

    /**
     * 停止演示模式
     */
    stopDemo() {
        if (!this.logicAdapter) return;

        // 停止周期运行
        this.logicAdapter.stopCycle();

        // 设置模式为编辑模式
        this.stateManager.set('mode', 'edit');

        console.log('[EmcFramework] 演示模式已停止');
        this.eventBus.emit('demoStopped');
    }

    /**
     * 处理按键点击
     * @param {number} x - 点击X坐标
     * @param {number} y - 点击Y坐标
     */
    handleClick(x, y) {
        if (this.stateManager.get('mode') !== 'demo') {
            // 编辑模式：触发控件选择事件
            this.eventBus.emit('editClick', { x, y });
            return;
        }

        // 演示模式：处理按键点击
        const hit = this.controlFactory.handleClick(x, y);
        if (hit) {
            console.log('[EmcFramework] 按键点击已处理');
        }
    }

    /**
     * 发送按键事件
     * @param {number} keyCode - 按键代码
     * @param {number} eventType - 事件类型
     */
    pressKey(keyCode, eventType = 2) {
        if (!this.logicAdapter) {
            throw new Error('逻辑适配器未初始化');
        }

        this.logicAdapter.pressKey(keyCode, eventType);
        console.log(`[EmcFramework] 发送按键: keyCode=${keyCode}, eventType=${eventType}`);
    }

    /**
     * 获取当前状态
     * @returns {Object} 状态对象
     */
    getState() {
        return this.stateManager.get();
    }

    /**
     * 获取逻辑层状态
     * @returns {Object} 逻辑层状态
     */
    getLogicState() {
        if (!this.logicAdapter) return null;

        return {
            state: this.logicAdapter.getState(),
            stateName: this.logicAdapter.getStateNames()[this.logicAdapter.getState()],
            display: this.logicAdapter.getDisplay(),
            led: this.logicAdapter.getLedState()
        };
    }

    /**
     * 获取控件工厂
     * @returns {ControlFactory} 控件工厂
     */
    getControlFactory() {
        return this.controlFactory;
    }

    /**
     * 获取事件总线
     * @returns {EventBus} 事件总线
     */
    getEventBus() {
        return this.eventBus;
    }

    /**
     * 订阅事件
     * @param {string} eventName - 事件名称
     * @param {Function} callback - 回调函数
     */
    on(eventName, callback) {
        this.eventBus.on(eventName, callback);
    }

    /**
     * 取消订阅
     * @param {string} eventName - 事件名称
     * @param {Function} callback - 回调函数
     */
    off(eventName, callback) {
        this.eventBus.off(eventName, callback);
    }

    /**
     * 清理资源
     */
    cleanup() {
        this.stopDemo();
        
        if (this.logicAdapter) {
            this.logicAdapter.cleanup();
            this.logicAdapter = null;
        }

        console.log('[EmcFramework] 资源已清理');
    }

    /**
     * 绑定事件处理
     * @private
     */
    _bindEvents() {
        // 监听按键按下事件
        this.eventBus.on('keyPressed', (data) => {
            if (this.stateManager.get('mode') === 'demo') {
                this.logicAdapter.clickKey(data.keyCode, 50);
            }
        });

        // 监听配置加载完成
        this.eventBus.on('configLoaded', (data) => {
            console.log('[EmcFramework] 配置加载:', data.summary);
        });

        // 监听状态变化
        this.eventBus.on('stateChange', (data) => {
            console.log('[EmcFramework] 状态变化:', data.path);
        });
    }

    /**
     * 显示更新回调
     * @private
     */
    _onDisplayUpdate(seg, led) {
        // 更新状态管理器
        this.stateManager.updateLogicState({
            seg: seg,
            ledBits: this._convertLedToBits(led)
        });

        // 更新控件显示
        this.controlFactory.updateLeds(led);
        this.controlFactory.updateSegmentDisplay(seg, 0, 0);
    }

    /**
     * 将LED状态转换为位数组
     * @private
     */
    _convertLedToBits(led) {
        const bits = new Array(16).fill(0);
        
        bits[0] = led.power ? 1 : 0;
        for (let i = 0; i < 10; i++) {
            bits[i + 1] = (led.func_leds >> i) & 1;
        }
        bits[11] = led.add_water ? 1 : 0;
        bits[12] = led.add_time ? 1 : 0;
        bits[13] = led.water_disp ? 1 : 0;
        bits[14] = led.time_disp ? 1 : 0;

        return bits;
    }
}

export { EmcFramework };