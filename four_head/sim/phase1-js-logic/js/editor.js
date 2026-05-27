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
let lastClickTime = 0;
let lastClickPos = { x: 0, y: 0 };

function handleEditMouseDown(event) {
    const pos = getMousePos(AppState.canvas, event);
    const control = hitTest(pos.x, pos.y);
    
    // ✅ 检测双击：300ms内同一位置点击
    const now = Date.now();
    const isDoubleClick = (now - lastClickTime < 300) && 
                          (Math.abs(pos.x - lastClickPos.x) < 5) &&
                          (Math.abs(pos.y - lastClickPos.y) < 5);
    
    lastClickTime = now;
    lastClickPos = { x: pos.x, y: pos.y };
    
    // ✅ 双击打开属性编辑器
    if (isDoubleClick && control) {
        console.log('[DEBUG] Double click detected on:', control.name);
        showPropertyEditor(control);
        return;  // 不执行后续的选中/拖拽逻辑
    }
    
    // ✅ Shift+点击：开始框选
    if (event.shiftKey && !control) {
        AppState.isBoxSelecting = true;
        AppState.boxSelectStart = { x: pos.x, y: pos.y };
        AppState.boxSelectEnd = { x: pos.x, y: pos.y };
        AppState.selectedControls = [];  // 清空之前的选择
        log('开始框选...', 'info');
        return;
    }
    
    if (control) {
        // ✅ 如果按住Shift且控件已在选中列表中，保持多选状态
        if (event.shiftKey && AppState.selectedControls.includes(control.id)) {
            // 已经在选中列表中，不做操作
        } else if (event.shiftKey) {
            // 添加到选中列表
            AppState.selectedControls.push(control.id);
            selectControl(control.id);  // 更新UI
            log(`已选中: ${control.name} (共${AppState.selectedControls.length}个)`, 'info');
        } else {
            // 单选模式：清空之前的选择
            selectControl(control.id);
            AppState.selectedControls = [control.id];
        }
        
        // 开始拖拽（支持单个或整体移动）
        AppState.isDragging = true;
        AppState.draggedControlId = control.id;
        
        // ✅ 记录所有选中控件的初始位置（用于整体移动）
        AppState.dragInitialPositions = {};
        AppState.selectedControls.forEach(id => {
            const ctrl = AppState.controls.find(c => c.id === id);
            if (ctrl) {
                AppState.dragInitialPositions[id] = { x: ctrl.x, y: ctrl.y };
            }
        });
        
        // 记录鼠标起始位置
        AppState.dragStartPos = { x: pos.x, y: pos.y };
        
        log(`选中控件: ${control.name}`, 'info');
    } else {
        // 点击空白处，取消选中
        clearSelection();
        AppState.selectedControls = [];
    }
}

/**
 * 处理Canvas鼠标移动事件（编辑模式）
 */
function handleEditMouseMove(event) {
    const pos = getMousePos(AppState.canvas, event);
    
    // ✅ 框选模式：更新框选矩形
    if (AppState.isBoxSelecting) {
        AppState.boxSelectEnd = { x: pos.x, y: pos.y };
        scheduleRender();
        return;
    }
    
    if (!AppState.isDragging || !AppState.draggedControlId) return;
    
    const control = AppState.controls.find(c => c.id === AppState.draggedControlId);
    
    if (!control) return;
    
    // ✅ 计算鼠标位移量
    const deltaX = pos.x - AppState.dragStartPos.x;
    const deltaY = pos.y - AppState.dragStartPos.y;
    
    // ✅ 移动所有选中的控件（包括单选）
    AppState.selectedControls.forEach(id => {
        const ctrl = AppState.controls.find(c => c.id === id);
        if (ctrl && AppState.dragInitialPositions[id]) {
            // 基于初始位置 + 位移量
            let newX = AppState.dragInitialPositions[id].x + deltaX;
            let newY = AppState.dragInitialPositions[id].y + deltaY;
            
            // 边界限制
            newX = clamp(newX, 0, AppState.canvas.width - ctrl.width);
            newY = clamp(newY, 0, AppState.canvas.height - ctrl.height);
            
            // 吸附网格
            if (AppState.showGrid) {
                newX = snapToGrid(newX, AppState.gridSize);
                newY = snapToGrid(newY, AppState.gridSize);
            }
            
            ctrl.x = newX;
            ctrl.y = newY;
        }
    });
    
    // 重绘
    scheduleRender();
}

/**
 * 处理Canvas鼠标松开事件（编辑模式）
 */
function handleEditMouseUp(event) {
    // ✅ 框选模式：计算框选矩形内的控件
    if (AppState.isBoxSelecting) {
        AppState.isBoxSelecting = false;
        
        // 计算框选矩形
        const x1 = Math.min(AppState.boxSelectStart.x, AppState.boxSelectEnd.x);
        const y1 = Math.min(AppState.boxSelectStart.y, AppState.boxSelectEnd.y);
        const x2 = Math.max(AppState.boxSelectStart.x, AppState.boxSelectEnd.x);
        const y2 = Math.max(AppState.boxSelectStart.y, AppState.boxSelectEnd.y);
        
        // 查找框内的所有控件
        const selectedIds = [];
        AppState.controls.forEach(control => {
            // 检查控件是否与框选矩形相交
            if (control.x < x2 && control.x + control.width > x1 &&
                control.y < y2 && control.y + control.height > y1) {
                selectedIds.push(control.id);
            }
        });
        
        if (selectedIds.length > 0) {
            AppState.selectedControls = selectedIds;
            // 选中最后一个控件作为主控件
            selectControl(selectedIds[selectedIds.length - 1]);
            log(`框选完成: 选中${selectedIds.length}个控件`, 'success');
        } else {
            clearSelection();
            AppState.selectedControls = [];
            log('框选完成: 未选中任何控件', 'info');
        }
        
        scheduleRender();
        return;
    }
    
    if (AppState.isDragging) {
        AppState.isDragging = false;
        AppState.draggedControlId = null;
        
        // 推入历史栈
        pushHistory();
        
        if (AppState.selectedControls.length > 1) {
            log(`已移动${AppState.selectedControls.length}个控件`, 'success');
        } else {
            log('控件位置已更新', 'success');
        }
    }
}

/**
 * 处理Canvas双击事件（编辑模式）- 打开属性编辑器
 */
function handleEditDoubleClick(event) {
    // 已由handleEditMouseDown中的双击检测处理
}

/**
 * 显示属性编辑器
 */
function showPropertyEditor(control) {
    console.log('[DEBUG] showPropertyEditor called for:', control.name);
    const modal = document.getElementById('property-modal');
    const form = document.getElementById('property-form');
    
    console.log('[DEBUG] modal element:', modal ? 'found' : 'NOT FOUND');
    console.log('[DEBUG] form element:', form ? 'found' : 'NOT FOUND');
    
    if (!modal || !form) {
        console.error('[ERROR] Modal or form not found!');
        return;
    }
    
    // 生成表单
    form.innerHTML = generatePropertyForm(control);
    console.log('[DEBUG] Form HTML generated');
    
    // 显示模态框
    modal.classList.add('show');
    modal.style.display = 'flex';  // ✅ 强制设置display，覆盖inline style
    console.log('[DEBUG] Modal show class added, modal display:', window.getComputedStyle(modal).display);
    
    // 绑定保存按钮
    document.getElementById('modal-save').onclick = () => {
        savePropertyEditor(control);
        modal.classList.remove('show');
        modal.style.display = 'none';  // ✅ 恢复inline style
    };
    
    // 绑定取消按钮
    document.getElementById('modal-cancel').onclick = () => {
        modal.classList.remove('show');
        modal.style.display = 'none';  // ✅ 恢复inline style
    };
    
    // 绑定关闭按钮
    document.getElementById('modal-close').onclick = () => {
        modal.classList.remove('show');
        modal.style.display = 'none';  // ✅ 恢复inline style
    };
}

/**
 * 生成属性表单HTML
 */
function generatePropertyForm(control) {
    let html = `
        <div class="property-group">
            <label>ID（与程序绑定）</label>
            <input type="text" id="prop-id" value="${control.id || ''}" placeholder="例如: KEY_START">
        </div>
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
    if (control.type === 'button' || control.type === '按键') {
        html += `
            <div class="property-group">
                <label>键码（与逻辑层绑定）</label>
                <input type="text" id="prop-key-code" value="${control.config?.key_code !== undefined ? control.config.key_code : ''}" placeholder="例如: key_addwater">
            </div>
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
        id: document.getElementById('prop-id').value.trim(),  // ✅ 保存ID
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
    if (document.getElementById('prop-key-code')) {
        if (!control.config) control.config = {};
        const keyCodeValue = document.getElementById('prop-key-code').value.trim();
        control.config.key_code = keyCodeValue ? keyCodeValue : undefined;  // ✅ 保存键码，空则删除
    }
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
