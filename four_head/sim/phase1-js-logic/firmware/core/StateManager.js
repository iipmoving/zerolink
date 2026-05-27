/**
 * 状态管理器 - 统一状态管理
 */
class StateManager {
    constructor() {
        this.state = {
            mode: 'edit',              // edit | demo | hardware_test
            config: null,              // 当前加载的配置
            controls: [],              // 控件列表
            logicState: {              // 逻辑层状态
                state: null,
                stateName: '',
                seg: [0x7F, 0x7F, 0x7F, 0x7F],
                dp_mask: 0,
                colon_mask: 0,
                ledBits: new Array(16).fill(0)
            },
            uiState: {                 // UI状态
                selectedControlId: null,
                isDragging: false,
                showGrid: false,
                showGuides: true
            },
            history: {                 // 撤销/重做历史
                past: [],
                future: [],
                maxSize: 20
            }
        };
        
        this.eventBus = null;
        this.historyEnabled = true;
    }

    /**
     * 初始化
     * @param {EventBus} eventBus - 事件总线
     */
    init(eventBus) {
        this.eventBus = eventBus;
    }

    /**
     * 获取状态
     * @param {string} path - 状态路径（如 'mode', 'logicState.seg'）
     * @returns {*} 状态值
     */
    get(path) {
        if (!path) return this.state;
        
        const keys = path.split('.');
        let value = this.state;
        for (const key of keys) {
            value = value?.[key];
        }
        return value;
    }

    /**
     * 设置状态
     * @param {string|Object} path - 状态路径或状态对象
     * @param {*} value - 状态值
     */
    set(path, value) {
        if (typeof path === 'object') {
            // 批量设置
            this._deepMerge(this.state, path);
        } else {
            // 单路径设置
            const keys = path.split('.');
            const lastKey = keys.pop();
            let target = this.state;
            for (const key of keys) {
                if (!target[key]) target[key] = {};
                target = target[key];
            }
            
            // 保存历史
            if (this.historyEnabled && path.startsWith('controls')) {
                this._saveHistory();
            }
            
            target[lastKey] = value;
        }
        
        // 触发状态变化事件
        this.eventBus?.emit('stateChange', { path, value });
    }

    /**
     * 加载配置
     * @param {Object} config - 配置对象
     */
    loadConfig(config) {
        this._saveHistory();
        this.state.config = config;
        this.state.controls = config.controls || [];
        this.eventBus?.emit('configLoaded', config);
    }

    /**
     * 更新逻辑层状态
     * @param {Object} logicState - 逻辑层状态
     */
    updateLogicState(logicState) {
        Object.assign(this.state.logicState, logicState);
        this.eventBus?.emit('logicStateChange', logicState);
    }

    /**
     * 更新UI状态
     * @param {Object} uiState - UI状态
     */
    updateUiState(uiState) {
        Object.assign(this.state.uiState, uiState);
        this.eventBus?.emit('uiStateChange', uiState);
    }

    /**
     * 撤销
     */
    undo() {
        if (this.state.history.past.length === 0) return false;
        
        this.state.history.future.push(JSON.parse(JSON.stringify(this.state.controls)));
        this.state.controls = JSON.parse(this.state.history.past.pop());
        this.eventBus?.emit('stateChange', { path: 'controls', value: this.state.controls });
        return true;
    }

    /**
     * 重做
     */
    redo() {
        if (this.state.history.future.length === 0) return false;
        
        this.state.history.past.push(JSON.parse(JSON.stringify(this.state.controls)));
        this.state.controls = JSON.parse(this.state.history.future.pop());
        this.eventBus?.emit('stateChange', { path: 'controls', value: this.state.controls });
        return true;
    }

    /**
     * 保存历史
     * @private
     */
    _saveHistory() {
        this.state.history.past.push(JSON.stringify(this.state.controls));
        
        if (this.state.history.past.length > this.state.history.maxSize) {
            this.state.history.past.shift();
        }
        this.state.history.future = [];
    }

    /**
     * 深度合并
     * @private
     */
    _deepMerge(target, source) {
        for (const key in source) {
            if (source[key] && typeof source[key] === 'object' && !Array.isArray(source[key])) {
                if (!target[key]) target[key] = {};
                this._deepMerge(target[key], source[key]);
            } else {
                target[key] = source[key];
            }
        }
    }

    /**
     * 获取完整状态快照
     */
    getSnapshot() {
        return JSON.parse(JSON.stringify(this.state));
    }
}

export { StateManager };