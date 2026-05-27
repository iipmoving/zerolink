/**
 * 编辑器逻辑 - 编辑模式下的交互
 */

/**
 * 启用编辑模式
 */
function enableEditing() {
    // 隐藏对齐工具
    document.getElementById('align-tools-section')?.classList.remove('hidden');
    document.getElementById('control-list-section')?.classList.remove('hidden');
    
    log('编辑模式已启用', 'info');
}

/**
 * 处理Canvas鼠标按下事件（编辑模式）
 */
function handleEditMouseDown(event) {
    const pos = getMousePos(AppState.canvas, event);
    const control = hitTest(pos.x, pos.y);
    
    if (control) {
        // 选中控件
        selectControl(control.id);
        
        // 开始拖拽
        AppState.isDragging = true;
        AppState.draggedControlId = control.id;
        AppState.dragOffset = {
            x: pos.x - control.x,
            y: pos.y - control.y
        };
        
        log(`选中控件: ${control.name}`, 'info');
    } else {
        // 点击空白处，取消选中
        clearSelection();
    }
}

/**
 * 处理Canvas鼠标移动事件（编辑模式）
 */
function handleEditMouseMove(event) {
    if (!AppState.isDragging || !AppState.draggedControlId) return;
    
    const pos = getMousePos(AppState.canvas, event);
    const control = AppState.controls.find(c => c.id === AppState.draggedControlId);
    
    if (!control) return;
    
    // 计算新位置
    let newX = pos.x - AppState.dragOffset.x;
    let newY = pos.y - AppState.dragOffset.y;
    
    // 边界限制
    newX = clamp(newX, 0, AppState.canvas.width - control.width);
    newY = clamp(newY, 0, AppState.canvas.height - control.height);
    
    // 吸附网格
    if (AppState.showGrid) {
        newX = snapToGrid(newX, AppState.gridSize);
        newY = snapToGrid(newY, AppState.gridSize);
    }
    
    // 更新位置
    control.x = newX;
    control.y = newY;
    
    // 重绘
    scheduleRender();
}

/**
 * 处理Canvas鼠标松开事件（编辑模式）
 */
function handleEditMouseUp(event) {
    if (AppState.isDragging) {
        AppState.isDragging = false;
        AppState.draggedControlId = null;
        
        // 推入历史栈
        pushHistory();
        
        log('控件位置已更新', 'success');
    }
}

/**
 * 处理Canvas双击事件（编辑模式）- 打开属性编辑器
 */
function handleEditDoubleClick(event) {
    const pos = getMousePos(AppState.canvas, event);
    const control = hitTest(pos.x, pos.y);
    
    if (control) {
        showPropertyEditor(control);
    }
}

/**
 * 显示属性编辑器
 */
function showPropertyEditor(control) {
    const modal = document.getElementById('property-modal');
    const form = document.getElementById('property-form');
    
    if (!modal || !form) return;
    
    // 生成表单
    form.innerHTML = generatePropertyForm(control);
    
    // 显示模态框
    modal.classList.add('show');
    
    // 绑定保存按钮
    document.getElementById('modal-save').onclick = () => {
        savePropertyEditor(control);
        modal.classList.remove('show');
    };
    
    // 绑定取消按钮
    document.getElementById('modal-cancel').onclick = () => {
        modal.classList.remove('show');
    };
    
    // 绑定关闭按钮
    document.getElementById('modal-close').onclick = () => {
        modal.classList.remove('show');
    };
}

/**
 * 生成属性表单HTML
 */
function generatePropertyForm(control) {
    let html = `
        <div class="property-group">
            <label>名称</label>
            <input type="text" id="prop-name" value="${control.name}">
        </div>
        <div class="property-group">
            <label>X坐标</label>
            <input type="number" id="prop-x" value="${control.x}">
        </div>
        <div class="property-group">
            <label>Y坐标</label>
            <input type="number" id="prop-y" value="${control.y}">
        </div>
        <div class="property-group">
            <label>宽度</label>
            <input type="number" id="prop-width" value="${control.width}">
        </div>
        <div class="property-group">
            <label>高度</label>
            <input type="number" id="prop-height" value="${control.height}">
        </div>
    `;
    
    // 按键特有属性
    if (control.type === 'button') {
        html += `
            <div class="property-group">
                <label>背景颜色</label>
                <input type="color" id="prop-bg-color" value="${control.style.background_color || '#e0e0e0'}">
            </div>
            <div class="property-group">
                <label>文字颜色</label>
                <input type="color" id="prop-text-color" value="${control.style.text_color || '#000000'}">
            </div>
            <div class="property-group">
                <label>字体大小</label>
                <input type="number" id="prop-font-size" value="${control.style.font_size || 12}">
            </div>
            <div class="property-group">
                <label>
                    <input type="checkbox" id="prop-long-press" ${control.config?.long_press ? 'checked' : ''}>
                    长按模式
                </label>
            </div>
        `;
    }
    
    // LED特有属性
    if (control.type === 'led') {
        html += `
            <div class="property-group">
                <label>点亮颜色</label>
                <input type="color" id="prop-on-color" value="${control.style.on_color || '#00ff00'}">
            </div>
            <div class="property-group">
                <label>熄灭颜色</label>
                <input type="color" id="prop-off-color" value="${control.style.off_color || '#666666'}">
            </div>
        `;
    }
    
    // 数码管特有属性
    if (control.type === 'segment') {
        html += `
            <div class="property-group">
                <label>背景颜色</label>
                <input type="color" id="prop-bg-color" value="${control.style.background_color || '#111111'}">
            </div>
            <div class="property-group">
                <label>文字颜色</label>
                <input type="color" id="prop-text-color" value="${control.style.text_color || '#00ff00'}">
            </div>
            <div class="property-group">
                <label>字体大小</label>
                <input type="number" id="prop-font-size" value="${control.style.font_size || 32}">
            </div>
        `;
    }
    
    return html;
}

/**
 * 保存属性编辑器
 */
function savePropertyEditor(control) {
    const updates = {
        name: document.getElementById('prop-name').value,
        x: parseInt(document.getElementById('prop-x').value),
        y: parseInt(document.getElementById('prop-y').value),
        width: parseInt(document.getElementById('prop-width').value),
        height: parseInt(document.getElementById('prop-height').value)
    };
    
    // 更新样式
    if (document.getElementById('prop-bg-color')) {
        control.style.background_color = document.getElementById('prop-bg-color').value;
    }
    if (document.getElementById('prop-text-color')) {
        control.style.text_color = document.getElementById('prop-text-color').value;
    }
    if (document.getElementById('prop-font-size')) {
        control.style.font_size = parseInt(document.getElementById('prop-font-size').value);
    }
    if (document.getElementById('prop-on-color')) {
        control.style.on_color = document.getElementById('prop-on-color').value;
    }
    if (document.getElementById('prop-off-color')) {
        control.style.off_color = document.getElementById('prop-off-color').value;
    }
    
    // 更新配置
    if (document.getElementById('prop-long-press')) {
        if (!control.config) control.config = {};
        control.config.long_press = document.getElementById('prop-long-press').checked;
    }
    
    updateControl(control.id, updates);
    log(`属性已更新: ${control.name}`, 'success');
}

/**
 * 添加新控件
 */
function addNewControl(type) {
    const canvas = AppState.canvas;
    const centerX = canvas.width / 2 - 50;
    const centerY = canvas.height / 2 - 25;
    
    let control;
    
    switch (type) {
        case 'button':
            control = {
                id: generateId('btn'),
                type: 'button',
                name: '新按键',
                x: centerX,
                y: centerY,
                width: 60,
                height: 40,
                style: {
                    background_color: '#e0e0e0',
                    border_color: '#999999',
                    text_color: '#000000',
                    font_size: 12
                },
                config: {
                    long_press: false,
                    key_code: 1,
                    trigger_mode: 'press'
                }
            };
            break;
            
        case 'led':
            control = {
                id: generateId('led'),
                type: 'led',
                name: '新LED',
                x: centerX,
                y: centerY,
                width: 12,
                height: 12,
                style: {
                    on_color: '#00ff00',
                    off_color: '#666666'
                },
                config: {
                    led_bit: 0
                }
            };
            break;
            
        case 'segment':
            control = {
                id: generateId('seg'),
                type: 'segment',
                name: '新数码管',
                x: centerX,
                y: centerY,
                width: 160,
                height: 50,
                style: {
                    background_color: '#111111',
                    text_color: '#00ff00',
                    font_size: 32
                },
                config: {
                    digits: 4
                }
            };
            break;
    }
    
    if (control) {
        addControl(control);
        log(`已添加${type === 'button' ? '按键' : type === 'led' ? 'LED' : '数码管'}`, 'success');
    }
}

/**
 * 删除选中的控件
 */
function deleteSelectedControl() {
    if (!AppState.selectedControlId) {
        log('没有选中的控件', 'info');
        return;
    }
    
    const control = AppState.controls.find(c => c.id === AppState.selectedControlId);
    deleteControl(AppState.selectedControlId);
    log(`已删除控件: ${control?.name}`, 'success');
}
