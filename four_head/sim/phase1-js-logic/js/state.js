/**
 * 全局状态管理
 */

const AppState = {
    // 当前模式: 'edit' | 'demo' | 'hardware_test'
    mode: 'edit',
    
    // 当前加载的配置ID
    currentConfigId: 'default',
    
    // 控件数组
    controls: [],
    
    // 选中的控件ID（编辑模式）
    selectedControlId: null,
    
    // Canvas相关
    canvas: null,
    ctx: null,
    
    // 显示选项
    showGrid: false,
    showGuides: true,
    gridSize: 10,
    
    // 参考图（已废弃，使用backgroundImage代替）
    referenceImage: null,
    referenceOpacity: 0.5,
    
    // ✅ 底图
    backgroundImage: null,        // 底图Image对象
    backgroundImagePath: null,    // 底图文件路径
    backgroundScale: 1.0,         // 底图缩放比例
    backgroundOffsetX: 0,         // 底图X偏移（居中用）
    backgroundOffsetY: 0,         // 底图Y偏移（居中用）
    
    // ✅ 控件填充/透明模式
    controlFillMode: false,       // true=填充模式(黑色底色+白色文字), false=透明模式(灰色边框+无文字)
    
    // 编辑模式状态
    isDragging: false,
    dragOffset: { x: 0, y: 0 },
    draggedControlId: null,
    
    // ✅ 框选状态
    isBoxSelecting: false,      // 是否正在框选
    boxSelectStart: { x: 0, y: 0 },  // 框选起点
    boxSelectEnd: { x: 0, y: 0 },    // 框选终点
    selectedControls: [],       // 选中的控件ID数组（支持多选）
    
    // ✅ 拖拽状态（用于整体移动）
    dragInitialPositions: {},   // 记录所有选中控件的初始位置 {id: {x, y}}
    dragStartPos: { x: 0, y: 0 },  // 鼠标起始位置
    
    // 演示模式状态
    logicLayer: null,
    keyHandler: null,
    periodicTimer: null,  // 周期运行定时器
    useWasm: true,  // ✅ 默认使用WASM逻辑层
    wasmAdapter: null,  // WASM适配器实例
    controlBinder: null,  // 控件绑定管理器
    cycleCount: 0,  // 周期计数
    dllState: {
        state: null,
        stateName: '',
        seg: [0x7F, 0x7F, 0x7F, 0x7F],  // 初始8888
        dp_mask: 0,
        colon_mask: 0,
        ledBits: new Array(16).fill(0),
    },
    
    // 按键按下状态（用于长按检测）
    pressedKeys: {},  // {keyName: pressTimestamp}
    
    // 撤销/重做历史栈
    history: {
        past: [],
        future: [],
        maxSize: 20
    },
    
    // 音频上下文（蜂鸣器）
    audioContext: null
};

/**
 * 设置模式
 */
function setMode(newMode) {
    if (AppState.mode === newMode) {
        log(`已在${newMode === 'edit' ? '编辑' : newMode === 'demo' ? '演示' : '硬件调试'}模式`, 'info');
        return;
    }
    
    const oldMode = AppState.mode;
    log(`正在从${oldMode}切换到${newMode}...`, 'info');
    
    try {
        // ✅ 先更新模式状态，确保清理和初始化时使用正确的模式
        AppState.mode = newMode;
        
        // 清理旧模式
        if (oldMode === 'demo') {
            log('正在停止演示模式...', 'info');
            stopDemo();
            log('✅ 演示模式已停止', 'success');
        } else if (oldMode === 'hardware_test') {
            log('正在停止硬件调试模式...', 'info');
            stopHardwareTest();
            log('✅ 硬件调试模式已停止', 'success');
        } else if (oldMode === 'edit') {
            clearSelection();
        }
        
        // 初始化新模式
        if (newMode === 'demo') {
            log('正在启动演示模式...', 'info');
            // ✅ 默认使用WASM逻辑层（可通过AppState.useWasm切换）
            const logicType = AppState.useWasm !== false ? 'wasm' : 'js';
            startDemo(logicType);
        } else if (newMode === 'hardware_test') {
            log('正在启动硬件调试模式...', 'info');
            startHardwareTest();
        } else if (newMode === 'edit') {
            log('正在启用编辑模式...', 'info');
            enableEditing();
            log('✅ 编辑模式已启用', 'success');
        }
        
        // 更新UI
        updateModeIndicator();
        renderCanvas();
        log(`成功切换到${newMode === 'edit' ? '编辑' : newMode === 'demo' ? '演示' : '硬件调试'}模式`, 'success');
    } catch (error) {
        log(`模式切换失败: ${error.message}`, 'error');
        console.error('setMode错误详情:', error);
        // 恢复原模式
        AppState.mode = oldMode;
        throw error;
    }
}

/**
 * 添加控件
 */
function addControl(control) {
    AppState.controls.push(control);
    pushHistory();
    renderCanvas();
    updateControlList();
}

/**
 * 更新控件属性
 */
function updateControl(id, properties) {
    const control = AppState.controls.find(c => c.id === id);
    if (!control) return;
    
    Object.assign(control, properties);
    pushHistory();
    renderCanvas();
    updateControlList();
}

/**
 * 删除控件
 */
function deleteControl(id) {
    AppState.controls = AppState.controls.filter(c => c.id !== id);
    if (AppState.selectedControlId === id) {
        AppState.selectedControlId = null;
    }
    pushHistory();
    renderCanvas();
    updateControlList();
}

/**
 * 选中控件
 */
function selectControl(id) {
    AppState.selectedControlId = id;
    renderCanvas();
    updateControlList();
}

/**
 * 取消选中
 */
function clearSelection() {
    AppState.selectedControlId = null;
    AppState.selectedControls = [];  // ✅ 清空多选列表
    renderCanvas();
    updateControlList();
}

/**
 * 推入历史栈
 */
function pushHistory() {
    const snapshot = deepClone(AppState.controls);
    AppState.history.past.push(snapshot);
    
    // 限制历史栈大小
    if (AppState.history.past.length > AppState.history.maxSize) {
        AppState.history.past.shift();
    }
    
    // 清空未来栈
    AppState.history.future = [];
}

/**
 * 撤销
 */
function undo() {
    if (AppState.history.past.length === 0) {
        log('没有可撤销的操作', 'info');
        return;
    }
    
    // 保存当前状态到未来栈
    AppState.history.future.push(deepClone(AppState.controls));
    
    // 恢复上一个状态
    AppState.controls = AppState.history.past.pop();
    AppState.selectedControlId = null;
    
    renderCanvas();
    updateControlList();
    log('已撤销', 'info');
}

/**
 * 重做
 */
function redo() {
    if (AppState.history.future.length === 0) {
        log('没有可重做的操作', 'info');
        return;
    }
    
    // 保存当前状态到过去栈
    AppState.history.past.push(deepClone(AppState.controls));
    
    // 恢复下一个状态
    AppState.controls = AppState.history.future.pop();
    AppState.selectedControlId = null;
    
    renderCanvas();
    updateControlList();
    log('已重做', 'info');
}

/**
 * 更新模式指示器
 */
function updateModeIndicator() {
    const indicator = document.getElementById('mode-indicator');
    if (!indicator) return;
    
    if (AppState.mode === 'edit') {
        indicator.textContent = '编辑模式';
        indicator.classList.remove('demo-mode');
    } else if (AppState.mode === 'demo') {
        indicator.textContent = '演示模式';
        indicator.classList.add('demo-mode');
    } else {
        indicator.textContent = '硬件调试';
        indicator.classList.add('demo-mode');
    }
}

/**
 * 更新控件列表
 */
// ✅ 用于跟踪快速两次单击
const controlListClickTimes = {};

function updateControlList() {
    const listElement = document.getElementById('control-list');
    if (!listElement) return;
    
    listElement.innerHTML = '';
    
    AppState.controls.forEach(control => {
        const item = document.createElement('div');
        item.className = 'control-list-item';
        if (control.id === AppState.selectedControlId) {
            item.classList.add('selected');
        }
        
        // ✅ 支持中文和英文类型
        let typeIcon = '❓';
        const type = control.type;
        if (type === 'button' || type === '按键') {
            typeIcon = '🔘';
        } else if (type === 'led' || type === 'LED灯') {
            typeIcon = '💡';
        } else if (type === 'segment' || type === '数码管') {
            typeIcon = '🔢';
        }
        
        // ✅ 显示键码和名称，如果未绑定则显示空
        const keyCode = control.config?.key_code;
        const displayKeyCode = keyCode ? `[${keyCode}]` : '';
        item.innerHTML = `<div style="font-size: 12px; color: #666; min-height: 16px;">${displayKeyCode}</div><div>${typeIcon} ${control.name}</div>`;
        
        // 单击选中，如果已选中则打开属性编辑器
        item.onclick = () => {
            const now = Date.now();
            const lastClickTime = controlListClickTimes[control.id] || 0;
            
            if (control.id === AppState.selectedControlId && (now - lastClickTime) < 300) {
                // 快速第二次点击，打开属性编辑器
                console.log('[DEBUG] List double-click detected on:', control.name);
                showPropertyEditor(control);
            } else {
                // 第一次点击，选中
                selectControl(control.id);
                controlListClickTimes[control.id] = now;
            }
        };
        
        listElement.appendChild(item);
    });
}
