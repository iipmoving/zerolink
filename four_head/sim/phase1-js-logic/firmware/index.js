/**
 * EMC Firmware - 标准固件模块导出
 * 
 * 提供统一的固件接口，供其他项目快速引用
 * 
 * 使用方式：
 * import { EmcFramework, EventBus, StateManager, 
 *          WasmLogicAdapter, JsLogicAdapter, 
 *          ControlFactory, ConfigParser } from './firmware/index.js';
 */

// 核心模块
export { EventBus } from './core/EventBus.js';
export { StateManager } from './core/StateManager.js';
export { EmcFramework } from './core/EmcFramework.js';

// 适配器模块
export { LogicAdapter } from './adapters/LogicAdapter.js';
export { WasmLogicAdapter } from './adapters/WasmLogicAdapter.js';
export { JsLogicAdapter } from './adapters/JsLogicAdapter.js';

// 控件模块
export { ControlFactory } from './controls/ControlFactory.js';

// 配置模块
export { ConfigParser } from './config/ConfigParser.js';

// 默认导出
export default {
    EventBus,
    StateManager,
    EmcFramework,
    LogicAdapter,
    WasmLogicAdapter,
    JsLogicAdapter,
    ControlFactory,
    ConfigParser
};