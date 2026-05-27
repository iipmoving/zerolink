# 控件属性编辑和ID管理功能说明

## 🎯 功能概述

增强控件属性编辑器，支持ID字段编辑，侧边栏列表显示ID信息，方便与C代码枚举绑定。

---

## 📋 使用方法

### 1. **打开属性编辑器**

**方法1：双击控件**
```
在Canvas上双击任意控件 → 打开属性编辑器
```

**方法2：点击侧边栏列表**
```
点击右侧控件列表中的任意项 → 打开属性编辑器
```

### 2. **编辑控件属性**

属性编辑器包含以下字段：

#### 通用属性（所有控件）

| 字段 | 说明 | 示例 |
|------|------|------|
| **ID** | 与程序绑定的标识符 | `KEY_START`, `LED_POWER` |
| **名称** | 显示名称 | "开始 / 暂停" |
| **X坐标** | 水平位置 | `99` |
| **Y坐标** | 垂直位置 | `276` |
| **宽度** | 控件宽度 | `40` |
| **高度** | 控件高度 | `40` |

#### 按键特有属性

| 字段 | 说明 | 示例 |
|------|------|------|
| 背景颜色 | 按键背景色 | `#2a2a2a` |
| 文字颜色 | 按键文字色 | `#cccccc` |
| 字体大小 | 文字字号 | `12` |
| **长按模式** | ✅ 是否支持长按 | 勾选/不勾选 |

#### LED特有属性

| 字段 | 说明 | 示例 |
|------|------|------|
| 点亮颜色 | LED点亮时的颜色 | `#00ff00` |
| 熄灭颜色 | LED熄灭时的颜色 | `#333333` |

#### 数码管特有属性

| 字段 | 说明 | 示例 |
|------|------|------|
| 背景颜色 | 数码管背景色 | `#111111` |
| 文字颜色 | 段码显示颜色 | `#44ff44` |
| 字体大小 | 数字字号 | `32` |

### 3. **设置ID（重要）**

**ID的作用**：
- 与C代码中的枚举值对应
- 用于控件绑定到逻辑层
- 区分不同的控件

**ID命名规范**：
```
按键:   KEY_XXX      (例如: KEY_START, KEY_POWER)
LED:    LED_XXX      (例如: LED_POWER, LED_HEAT)
数码管: SEG_XXX      (例如: SEG_TEMP, SEG_TIME)
```

**示例**：
```
开始/暂停按键:
  ID: KEY_START
  名称: 开始 / 暂停
  
电源灯:
  ID: LED_POWER
  名称: 电源灯
  
温度显示:
  ID: SEG_TEMP
  名称: 温度显示
```

### 4. **保存修改**

```
1. 修改属性
2. 点击"保存"按钮
3. 控件立即更新
4. 修改自动保存到历史栈（支持撤销）
```

---

## 🎨 侧边栏控件列表

### 显示格式

每个控件项显示两行信息：

```
[KEY_START]          ← ID（灰色小字）
🔘 开始 / 暂停       ← 类型图标 + 名称
```

### 状态说明

| 显示内容 | 含义 |
|---------|------|
| `[KEY_START]` | 已设置ID |
| `[未设置ID]` | 未设置ID（需要设置） |

### 操作

- **单击**：选中控件，Canvas高亮显示
- **再次单击**：打开属性编辑器

---

## 💡 使用场景

### 场景1：新建控件后设置ID

```
1. 点击"添加控件"按钮
2. 选择控件类型（按键/LED/数码管）
3. 控件出现在画布中央
4. 双击新控件
5. 在ID字段输入：KEY_NEW_BUTTON
6. 点击保存
7. 侧边栏显示：[KEY_NEW_BUTTON]
```

### 场景2：批量设置ID

```
1. 依次点击侧边栏中的每个控件
2. 在属性编辑器中设置ID
3. 确保ID与C代码枚举一致
4. 完成后所有控件都有正确的ID
```

### 场景3：检查未设置ID的控件

```
1. 查看侧边栏列表
2. 找到显示"[未设置ID]"的控件
3. 点击该控件
4. 在属性编辑器中设置ID
5. 保存
```

### 场景4：修改按键长按功能

```
1. 双击需要修改的按键
2. 找到"长按模式"复选框
3. 勾选 = 支持长按
4. 不勾选 = 不支持长按
5. 点击保存
```

---

## 🔧 技术实现

### 1. 属性编辑器（editor.js）

**生成表单**：
```javascript
function generatePropertyForm(control) {
    let html = `
        <div class="property-group">
            <label>ID（与程序绑定）</label>
            <input type="text" id="prop-id" value="${control.id || ''}" 
                   placeholder="例如: KEY_START">
        </div>
        <!-- 其他字段... -->
    `;
}
```

**保存属性**：
```javascript
function savePropertyEditor(control) {
    const updates = {
        id: document.getElementById('prop-id').value.trim(),  // ✅ 保存ID
        name: document.getElementById('prop-name').value,
        x: parseInt(document.getElementById('prop-x').value),
        y: parseInt(document.getElementById('prop-y').value),
        width: parseInt(document.getElementById('prop-width').value),
        height: parseInt(document.getElementById('prop-height').value)
    };
    
    updateControl(control.id, updates);
}
```

### 2. 控件列表显示（state.js）

```javascript
function updateControlList() {
    AppState.controls.forEach(control => {
        const item = document.createElement('div');
        
        // ✅ 显示ID和名称
        const displayId = control.id ? `[${control.id}]` : '[未设置ID]';
        item.innerHTML = `
            <div style="font-size: 12px; color: #666;">${displayId}</div>
            <div>${typeIcon} ${control.name}</div>
        `;
        
        item.onclick = () => {
            selectControl(control.id);
        };
        
        listElement.appendChild(item);
    });
}
```

### 3. ID生成（utils.js）

```javascript
function generateId(prefix = 'ctrl') {
    // 生成简洁的ID：前缀_时间戳随机数
    const timestamp = Date.now().toString(36).toUpperCase();
    const random = Math.random().toString(36).substr(2, 4).toUpperCase();
    return `${prefix}_${timestamp}${random}`;
}
```

**生成的ID示例**：
- `btn_JKL8M2N3`
- `led_JKL8M2N4`
- `seg_JKL8M2N5`

---

## 📝 配置文件中的ID

### 保存配置

配置JSON中包含ID字段：

```json
{
  "controls": [
    {
      "id": "KEY_START",
      "type": "按键",
      "name": "开始 / 暂停",
      "x": 99,
      "y": 276,
      "width": 40,
      "height": 40,
      ...
    },
    {
      "id": "LED_POWER",
      "type": "LED灯",
      "name": "电源灯",
      "x": 113,
      "y": 190,
      "width": 10,
      "height": 10,
      ...
    }
  ]
}
```

### 加载配置

加载配置时，ID会被保留：

```javascript
AppState.controls = config.controls || [];
// ID字段自动保留
```

---

## ✅ 最佳实践

### 1. ID命名规范

**推荐**：
```
✅ KEY_START      - 清晰明了
✅ LED_POWER      - 类型+功能
✅ SEG_TEMP       - 类型+用途
```

**不推荐**：
```
❌ btn1           - 无意义
❌ key            - 太泛
❌ start_button   - 冗长
```

### 2. 及时设置ID

```
新建控件后立即设置ID，避免忘记
```

### 3. 保持ID唯一

```
确保每个控件的ID都是唯一的
不要重复使用相同的ID
```

### 4. 与C代码保持一致

```
前端ID应该与C代码中的枚举值对应
例如：
  C代码: typedef enum { KEY_START = 0, KEY_PAUSE = 1, ... }
  前端:  ID: KEY_START, KEY_PAUSE
```

---

## 🚀 完整工作流程

### 步骤1：创建面板布局

```
1. 添加所有需要的控件
2. 调整位置和大小
3. 设置样式（颜色、字体等）
```

### 步骤2：设置ID

```
1. 双击第一个控件
2. 设置ID（例如：KEY_START）
3. 保存
4. 重复直到所有控件都有ID
```

### 步骤3：配置长按功能

```
1. 双击需要长按的按键
2. 勾选"长按模式"
3. 保存
```

### 步骤4：保存配置

```
1. 点击"保存配置"按钮
2. 下载JSON文件
3. ID信息已包含在配置中
```

### 步骤5：加载配置

```
1. 点击"加载配置"按钮
2. 选择JSON文件
3. 所有控件及其ID被恢复
```

---

## 🎉 总结

现在您可以：

✅ **双击控件或点击列表**打开属性编辑器  
✅ **编辑ID字段**，与C代码枚举绑定  
✅ **查看侧边栏ID**，快速识别控件  
✅ **设置长按功能**，控制按键行为  
✅ **保存和加载**，ID信息完整保留  

这些功能让控件管理更加高效，便于与后端程序对接！
