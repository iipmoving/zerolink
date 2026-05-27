/**
 * Canvas渲染引擎
 */

/**
 * 初始化Canvas
 */
function initCanvas() {
    AppState.canvas = document.getElementById('main-canvas');
    AppState.ctx = AppState.canvas.getContext('2d');
    
    if (!AppState.canvas || !AppState.ctx) {
        log('Canvas初始化失败', 'error');
        return false;
    }
    
    log('Canvas已初始化', 'success');
    return true;
}

/**
 * 渲染Canvas
 */
function renderCanvas() {
    if (!AppState.ctx) return;
    
    const ctx = AppState.ctx;
    const canvas = AppState.canvas;
    
    // 清空画布
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    
    // 绘制白色背景
    ctx.fillStyle = '#ffffff';
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    
    // ✅ 绘制底图（在所有控件下方）
    if (AppState.backgroundImage) {
        const scale = AppState.backgroundScale || 1.0;
        const offsetX = AppState.backgroundOffsetX || 0;
        const offsetY = AppState.backgroundOffsetY || 0;
        
        ctx.save();
        ctx.translate(offsetX, offsetY);
        ctx.scale(scale, scale);
        ctx.drawImage(AppState.backgroundImage, 0, 0);
        ctx.restore();
    }
    
    // 绘制参考图（兼容旧代码）
    if (AppState.referenceImage && AppState.mode === 'edit') {
        ctx.globalAlpha = AppState.referenceOpacity;
        ctx.drawImage(AppState.referenceImage, 0, 0);
        ctx.globalAlpha = 1.0;
    }
    
    // 绘制网格
    if (AppState.showGrid && AppState.mode === 'edit') {
        drawGrid(ctx, canvas);
    }
    
    // 绘制控件
    AppState.controls.forEach(control => {
        const isSelected = control.id === AppState.selectedControlId;
        drawControl(ctx, control, isSelected);
    });
    
    // 绘制辅助线
    if (AppState.showGuides && AppState.mode === 'edit' && AppState.isDragging) {
        drawGuideLines(ctx);
    }
}

/**
 * 绘制网格
 */
function drawGrid(ctx, canvas) {
    ctx.save();
    ctx.strokeStyle = '#e0e0e0';
    ctx.lineWidth = 0.5;
    
    const gridSize = AppState.gridSize;
    
    // 垂直线
    for (let x = 0; x <= canvas.width; x += gridSize) {
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, canvas.height);
        ctx.stroke();
    }
    
    // 水平线
    for (let y = 0; y <= canvas.height; y += gridSize) {
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(canvas.width, y);
        ctx.stroke();
    }
    
    ctx.restore();
}

/**
 * 绘制辅助线
 */
function drawGuideLines(ctx) {
    if (!AppState.draggedControlId) return;
    
    const draggedControl = AppState.controls.find(c => c.id === AppState.draggedControlId);
    if (!draggedControl) return;
    
    ctx.save();
    ctx.strokeStyle = '#ff0000';
    ctx.lineWidth = 1;
    ctx.setLineDash([5, 3]);
    
    const center = {
        x: draggedControl.x + draggedControl.width / 2,
        y: draggedControl.y + draggedControl.height / 2
    };
    
    // 查找对齐的控件
    AppState.controls.forEach(control => {
        if (control.id === AppState.draggedControlId) return;
        
        const otherCenter = {
            x: control.x + control.width / 2,
            y: control.y + control.height / 2
        };
        
        // 水平对齐
        if (Math.abs(center.y - otherCenter.y) < 5) {
            ctx.beginPath();
            ctx.moveTo(0, otherCenter.y);
            ctx.lineTo(AppState.canvas.width, otherCenter.y);
            ctx.stroke();
        }
        
        // 垂直对齐
        if (Math.abs(center.x - otherCenter.x) < 5) {
            ctx.beginPath();
            ctx.moveTo(otherCenter.x, 0);
            ctx.lineTo(otherCenter.x, AppState.canvas.height);
            ctx.stroke();
        }
    });
    
    ctx.restore();
}

/**
 * 命中测试 - 检测点击位置对应的控件
 */
function hitTest(x, y) {
    // 从后往前遍历（后绘制的在上层）
    for (let i = AppState.controls.length - 1; i >= 0; i--) {
        const control = AppState.controls[i];
        if (isPointInRect(x, y, control)) {
            return control;
        }
    }
    return null;
}

/**
 * 请求重绘（防抖）
 */
const scheduleRender = debounce(() => {
    renderCanvas();
}, 16);  // 约60fps
