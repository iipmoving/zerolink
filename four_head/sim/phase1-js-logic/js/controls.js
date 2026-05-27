/**
 * 控件绘制模块
 */

/**
 * 绘制按键
 */
function drawButton(ctx, control, isSelected = false, isPressed = false) {
    const { x, y, width, height, style } = control;
    ctx.save();
    
    // ✅ 优先使用控件独立标记，未设置则跟随全局开关
    const isTransparent = control.transparent !== undefined ? control.transparent : !AppState.controlFillMode;
    
    if (isTransparent) {
        ctx.fillStyle = 'rgba(0, 0, 0, 0.2)';
        ctx.fillRect(x, y, width, height);
        ctx.strokeStyle = '#999999';
        ctx.lineWidth = 2;
        ctx.strokeRect(x, y, width, height);
        
        ctx.fillStyle = '#cccccc';
        ctx.font = `${style.font_size || 12}px Microsoft YaHei`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText(control.name, x + width / 2, y + height / 2);
    } else {
        // 填充模式：黑色底色 + 白色文字
        // 按下效果
        if (isPressed) {
            ctx.shadowColor = 'rgba(0, 0, 0, 0.3)';
            ctx.shadowBlur = 5;
            ctx.shadowOffsetX = 2;
            ctx.shadowOffsetY = 2;
        }
        
        // 绘制背景
        ctx.fillStyle = '#000000';  // 黑色底色
        ctx.fillRect(x, y, width, height);
        
        // 绘制边框
        ctx.strokeStyle = isSelected ? '#0066cc' : '#666666';
        ctx.lineWidth = isSelected ? 3 : 2;
        ctx.strokeRect(x, y, width, height);
        
        // 绘制文字
        ctx.fillStyle = '#ffffff';  // 白色文字
        ctx.font = `${style.font_size || 12}px Microsoft YaHei`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText(control.name, x + width / 2, y + height / 2);
    }
    
    // 选中指示器
    if (isSelected) {
        ctx.strokeStyle = '#0066cc';
        ctx.lineWidth = 1;
        ctx.setLineDash([5, 3]);
        ctx.strokeRect(x - 3, y - 3, width + 6, height + 6);
        ctx.setLineDash([]);
    }
    
    ctx.restore();
}

/**
 * 绘制LED
 */
function drawLED(ctx, control, isSelected = false, isOn = false) {
    const { x, y, width, height, style } = control;
    ctx.save();
    
    const isTransparent = control.transparent !== undefined ? control.transparent : !AppState.controlFillMode;
    
    const centerX = x + width / 2;
    const centerY = y + height / 2;
    const radius = Math.min(width, height) / 2;
    
    if (isTransparent) {
        // 透明模式：半透明底色 + 灰色边框 + LED灯内容
        ctx.beginPath();
        ctx.arc(centerX, centerY, radius, 0, Math.PI * 2);
        ctx.fillStyle = 'rgba(0, 0, 0, 0.3)';
        ctx.fill();
        ctx.strokeStyle = '#999999';
        ctx.lineWidth = 2;
        ctx.stroke();
        
        // 绘制LED灯（保留点亮状态）
        ctx.beginPath();
        ctx.arc(centerX, centerY, radius - 2, 0, Math.PI * 2);
        ctx.fillStyle = isOn ? (style.on_color || '#00ff00') : (style.off_color || '#666666');
        ctx.fill();
        if (isOn) {
            ctx.shadowColor = style.on_color || '#00ff00';
            ctx.shadowBlur = 10;
            ctx.fill();
            ctx.shadowBlur = 0;
        }
    } else {
        // 填充模式：黑色底色 + LED灯
        // 绘制外圈（黑色背景）
        ctx.beginPath();
        ctx.arc(centerX, centerY, radius, 0, Math.PI * 2);
        ctx.fillStyle = '#000000';  // 黑色底色
        ctx.fill();
        
        // 绘制LED灯
        ctx.beginPath();
        ctx.arc(centerX, centerY, radius - 2, 0, Math.PI * 2);
        ctx.fillStyle = isOn ? (style.on_color || '#00ff00') : (style.off_color || '#666666');
        ctx.fill();
        
        // 发光效果
        if (isOn) {
            ctx.shadowColor = style.on_color || '#00ff00';
            ctx.shadowBlur = 10;
            ctx.beginPath();
            ctx.arc(centerX, centerY, radius - 2, 0, Math.PI * 2);
            ctx.fill();
            ctx.shadowBlur = 0;
        }
        
        // 边框
        ctx.strokeStyle = '#666666';
        ctx.lineWidth = 1;
        ctx.stroke();
    }
    
    // 选中指示器
    if (isSelected) {
        ctx.strokeStyle = '#0066cc';
        ctx.lineWidth = 1;
        ctx.setLineDash([3, 2]);
        ctx.beginPath();
        ctx.arc(centerX, centerY, radius + 4, 0, Math.PI * 2);
        ctx.stroke();
        ctx.setLineDash([]);
    }
    
    ctx.restore();
}

/**
 * 绘制数码管
 */
function drawSegment(ctx, control, isSelected = false, segText = '----') {
    const { x, y, width, height, style } = control;
    ctx.save();
    
    const isTransparent = control.transparent !== undefined ? control.transparent : !AppState.controlFillMode;
    
    if (isTransparent) {
        // 透明模式：半透明底色 + 灰色边框 + 数码管内容
        ctx.fillStyle = 'rgba(0, 0, 0, 0.3)';
        ctx.fillRect(x, y, width, height);
        ctx.strokeStyle = '#999999';
        ctx.lineWidth = 2;
        ctx.strokeRect(x, y, width, height);
        
        // 绘制数字
        ctx.fillStyle = '#00ff00';
        ctx.font = `bold ${style.font_size || 32}px 'Courier New', monospace`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.shadowColor = '#00ff00';
        ctx.shadowBlur = 10;
        ctx.fillText(segText, x + width / 2, y + height / 2);
        ctx.shadowBlur = 0;
    } else {
        // 填充模式：黑色底色 + 绿色文字
        // 绘制背景
        ctx.fillStyle = '#000000';  // 黑色底色
        ctx.fillRect(x, y, width, height);
        
        // 绘制边框
        ctx.strokeStyle = isSelected ? '#0066cc' : '#666666';
        ctx.lineWidth = isSelected ? 3 : 1;
        ctx.strokeRect(x, y, width, height);
        
        // 绘制文字
        ctx.fillStyle = '#00ff00';  // 绿色文字
        ctx.font = `bold ${style.font_size || 32}px 'Courier New', monospace`;
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        
        // 添加发光效果
        ctx.shadowColor = '#00ff00';
        ctx.shadowBlur = 10;
        ctx.fillText(segText, x + width / 2, y + height / 2);
        ctx.shadowBlur = 0;
    }
    
    // 选中指示器
    if (isSelected) {
        ctx.strokeStyle = '#0066cc';
        ctx.lineWidth = 1;
        ctx.setLineDash([5, 3]);
        ctx.strokeRect(x - 3, y - 3, width + 6, height + 6);
        ctx.setLineDash([]);
    }
    
    ctx.restore();
}

/**
 * 根据类型绘制控件
 */
function drawControl(ctx, control, isSelected = false) {
    const isPressed = AppState.mode === 'demo' && AppState.pressedKeys[control.name];
    
    switch (control.type) {
        case 'button':
            drawButton(ctx, control, isSelected, isPressed);
            break;
        case 'led':
            const ledBit = control.config?.led_bit || 0;
            const isOn = AppState.dllState.ledBits[ledBit] === 1;
            drawLED(ctx, control, isSelected, isOn);
            break;
        case 'segment':
            const segText = segArrayToString(
                AppState.dllState.seg,
                AppState.dllState.dp_mask,
                AppState.dllState.colon_mask
            );
            drawSegment(ctx, control, isSelected, segText || '----');
            break;
    }
}
