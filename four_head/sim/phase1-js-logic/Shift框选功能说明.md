# Shift+框选多选功能说明

## 🎯 功能概述

在编辑模式下，支持通过Shift键进行多选和整体移动控件。

---

## 📋 使用方法

### 1. **框选多个控件**

```
1. 按住Shift键
2. 在空白处按下鼠标
3. 拖动鼠标绘制选择框
4. 松开鼠标，框内的所有控件被选中
```

**效果**：
- 显示蓝色半透明选择框
- 框内所有控件被高亮显示
- 控制台显示选中数量

### 2. **逐个添加/移除控件**

```
按住Shift键 + 点击控件：
- 如果控件未选中 → 添加到选中列表
- 如果控件已选中 → 保持选中状态
```

### 3. **整体移动**

```
1. 选中多个控件（框选或Shift+点击）
2. 拖拽任意一个选中的控件
3. 所有选中控件一起移动，保持相对位置
```

**特点**：
- 所有控件同步移动
- 保持相对位置不变
- 每个控件都有边界检查
- 支持网格吸附

### 4. **取消选中**

```
点击空白处（不按Shift）→ 取消所有选中
```

---

## 🎨 视觉效果

### 框选矩形
- **填充色**: 半透明蓝色 `rgba(0, 120, 255, 0.15)`
- **边框**: 蓝色虚线 `#0078ff`
- **线型**: 虚线 `[5, 3]`

### 选中控件
- 所有选中的控件都有高亮边框
- 主选中控件（最后点击的）有特殊标记
- 其他选中控件也有选中状态显示

---

## 💡 使用场景

### 场景1：批量调整布局

```
问题：需要将一组按键整体向右移动50px

操作：
1. Shift+框选所有需要移动的按键
2. 拖拽任意一个按键向右移动
3. 所有按键同步移动，保持间距
```

### 场景2：对齐多个控件

```
问题：需要将多个LED灯对齐到同一水平线

操作：
1. Shift+点击所有需要对齐的LED灯
2. 手动调整Y坐标（或使用属性编辑器）
3. 所有LED灯保持X坐标不变，Y坐标统一
```

### 场景3：复制布局模式

```
问题：需要复制一组控件的布局到其他位置

操作：
1. Shift+框选源控件组
2. 记录相对位置
3. 创建新控件并设置相同的位置关系
```

---

## 🔧 技术实现

### 1. 状态管理（state.js）

```javascript
AppState = {
    // 框选状态
    isBoxSelecting: false,      // 是否正在框选
    boxSelectStart: { x: 0, y: 0 },  // 框选起点
    boxSelectEnd: { x: 0, y: 0 },    // 框选终点
    selectedControls: [],       // 选中的控件ID数组
}
```

### 2. 框选逻辑（editor.js）

**MouseDown**:
```javascript
if (event.shiftKey && !control) {
    // 开始框选
    AppState.isBoxSelecting = true;
    AppState.boxSelectStart = { x: pos.x, y: pos.y };
}
```

**MouseMove**:
```javascript
if (AppState.isBoxSelecting) {
    // 更新框选矩形
    AppState.boxSelectEnd = { x: pos.x, y: pos.y };
    scheduleRender();
}
```

**MouseUp**:
```javascript
if (AppState.isBoxSelecting) {
    // 计算框选矩形
    const x1 = Math.min(start.x, end.x);
    const y1 = Math.min(start.y, end.y);
    const x2 = Math.max(start.x, end.x);
    const y2 = Math.max(start.y, end.y);
    
    // 查找框内的所有控件
    AppState.controls.forEach(control => {
        if (control.x < x2 && control.x + control.width > x1 &&
            control.y < y2 && control.y + control.height > y1) {
            selectedIds.push(control.id);
        }
    });
}
```

### 3. 整体移动（editor.js）

```javascript
if (AppState.selectedControls.length > 1) {
    // 计算位移量
    const deltaX = pos.x - dragOffset.x - control.x;
    const deltaY = pos.y - dragOffset.y - control.y;
    
    // 移动所有选中的控件
    AppState.selectedControls.forEach(id => {
        const ctrl = AppState.controls.find(c => c.id === id);
        if (ctrl) {
            ctrl.x += deltaX;
            ctrl.y += deltaY;
            
            // 边界检查
            ctrl.x = clamp(ctrl.x, 0, canvas.width - ctrl.width);
            ctrl.y = clamp(ctrl.y, 0, canvas.height - ctrl.height);
        }
    });
}
```

### 4. 绘制框选矩形（canvas.js）

```javascript
function drawBoxSelection(ctx) {
    const x1 = Math.min(start.x, end.x);
    const y1 = Math.min(start.y, end.y);
    const width = Math.abs(end.x - start.x);
    const height = Math.abs(end.y - start.y);
    
    // 填充半透明蓝色
    ctx.fillStyle = 'rgba(0, 120, 255, 0.15)';
    ctx.fillRect(x1, y1, width, height);
    
    // 蓝色虚线边框
    ctx.strokeStyle = '#0078ff';
    ctx.lineWidth = 2;
    ctx.setLineDash([5, 3]);
    ctx.strokeRect(x1, y1, width, height);
}
```

---

## ✅ 功能特性

### 已实现

- ✅ Shift+空白处框选
- ✅ 蓝色半透明选择框
- ✅ 自动选中框内控件
- ✅ Shift+点击添加/移除控件
- ✅ 多选控件整体移动
- ✅ 保持相对位置
- ✅ 边界检查（每个控件独立）
- ✅ 网格吸附（如果启用）
- ✅ 所有选中控件高亮显示
- ✅ 控制台日志提示

### 待优化（可选）

- ⏸️ Ctrl+A 全选
- ⏸️ Delete 删除选中控件
- ⏸️ 方向键微调选中控件
- ⏸️ 撤销/重做支持多选操作
- ⏸️ 选中控件数量显示在UI上

---

## 📝 注意事项

### 1. 边界检查

每个控件都有独立的边界检查：
```javascript
newX = clamp(newX, 0, canvas.width - control.width);
newY = clamp(newY, 0, canvas.height - control.height);
```

**结果**：如果某个控件到达边界，它会停止移动，但其他控件继续移动。

### 2. 网格吸附

如果启用了网格显示，所有控件都会吸附到网格：
```javascript
if (AppState.showGrid) {
    newX = snapToGrid(newX, gridSize);
    newY = snapToGrid(newY, gridSize);
}
```

### 3. 历史记录

整体移动会作为一次操作推入历史栈：
```javascript
pushHistory();  // 在MouseUp时调用
```

**结果**：撤销时会恢复所有控件的原始位置。

---

## 🚀 使用示例

### 示例1：快速选择一行按键

```
场景：有10个菜单键排成一行，需要整体下移

步骤：
1. 按住Shift
2. 在这行按键上方点击
3. 向下拖动，框住所有按键
4. 松开鼠标
5. 拖拽任意一个按键向下移动
6. 所有按键同步下移
```

### 示例2：调整LED灯位置

```
场景：有5个LED灯需要整体右移并对齐

步骤：
1. Shift+点击第一个LED灯
2. Shift+点击第二个LED灯
3. ... 依次点击所有LED灯
4. 拖拽任意一个LED灯向右移动
5. 使用属性编辑器统一Y坐标
```

### 示例3：复制布局

```
场景：需要在另一侧创建相同的按键布局

步骤：
1. Shift+框选源按键组
2. 查看控制台日志，记住坐标
3. 创建新按键，设置相同的相对位置
4. 或者手动计算偏移量并应用
```

---

## 🎉 总结

Shift+框选功能大大提升了编辑效率：

✅ **快速选择**：框选比逐个点击快得多  
✅ **整体移动**：保持布局关系，避免逐个调整  
✅ **视觉反馈**：蓝色选择框清晰可见  
✅ **灵活操作**：支持框选和逐个添加两种方式  

现在您可以高效地编辑复杂的面板布局了！
