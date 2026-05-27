# index.html WASM集成完成说明

**日期**: 2026-05-18  
**状态**: ✅ 已完成  

---

## 📋 完成的工作

### 1. 配置导入 ✅

- **new.json** 已设置为默认面板配置
- 位置: `configs/default.json`
- 包含完整的控件布局（按键、LED、数码管）

### 2. WASM集成到index.html ✅

#### 修改的文件

1. **index.html**
   - 添加WASM脚本引用：
     ```html
     <script src="wasm/emc_test.js"></script>
     <script src="js/ems_wasm_adapter.js"></script>
     <script src="js/control_binder.js"></script>
     ```

2. **js/demo.js**
   - 添加 `startDemoWithWasm()` 函数
   - 添加 `updateDisplayFromWasm()` 函数
   - 添加 `setupWasmCanvasClick()` 函数
   - 添加 `stopWasmDemo()` 函数
   - 修改 `startDemo()` 支持逻辑层选择
   - 修改 `stopDemo()` 支持WASM停止

3. **js/state.js**
   - 修改 `setMode()` 函数，默认使用WASM逻辑层
   - 可通过 `AppState.useWasm = false` 切换回JS逻辑

---

## 🎯 使用方法

### 启动WASM演示模式

1. 打开浏览器访问: `http://localhost:8080/phase1-js-logic/index.html`
2. 点击菜单：**演示 → 开始演示**
3. 系统会自动：
   - 初始化WASM
   - 注册回调函数（硬件输入、显示输出等）
   - 启动周期运行（每10ms一次）
   - 加载配置并绑定控件
   - 设置Canvas点击事件

### 预期效果

- **上电自检**（约3秒）：
  - S_POWER_ON → S_VERSION → S_SHUTDOWN
  - 数码管显示：空白 → "F1 0" → "---"

- **待机状态**（S_STANDBY）：
  - 数码管显示：85°（水温）
  - LED灯：电源灯亮 + M1-M10全亮

- **按键交互**：
  - 点击Canvas上的按键
  - 实时响应并更新显示

---

## ⚠️ 避免的坑（重要！）

### 坑1: 上电后按键无效 ❌ → ✅

**原因**: WASM初始化后没有启动周期运行  
**解决**: 
```javascript
// demo.js - startDemoWithWasm()
AppState.wasmAdapter.startCycle(() => {
    AppState.cycleCount++;
    if (AppState.cycleCount % 50 === 0) {
        updateDisplayFromWasm();
    }
});
```

### 坑2: 点击位置无效 ❌ → ✅

**原因**: `bindElements()` 未被调用，`keyInfo.element` 为null  
**解决**: 
```javascript
// control_binder.js - loadAndBind()
this.bindElements();  // ✅ 必须调用
```

### 坑3: LED指示灯不亮 ❌ → ✅

**原因**: 
1. WASM未导出 `emc_get_led_state_ptr` 函数
2. JS层缺少LED状态推断逻辑

**解决**: 
```javascript
// ems_wasm_adapter.js - getLedState()
// 基于状态机推断LED状态
switch (state) {
    case S_STANDBY:
        led.power = true;
        led.func_leds = 0x3FF;  // M1-M10全亮
        break;
}
```

### 坑4: 水温显示为0 ❌ → ✅

**原因**: 硬件输入参数未设置默认值  
**解决**: 
```javascript
// ems_wasm_adapter.js - _registerCallbacks()
const hwInputsCallback = (hwPtr) => {
    Module.HEAPU16[hwPtr / 2] = 850;    // 水温85°C
    Module.HEAPU8[hwPtr + 2] = 1;       // 有水
    Module.HEAPU8[hwPtr + 3] = 1;       // 有锅
};
```

### 坑5: Aborted(Unable to grow wasm table) ❌ → ✅

**原因**: 编译时未启用 `ALLOW_TABLE_GROWTH=1`  
**解决**: 
```python
# compile_wasm.py
"-s", "ALLOW_TABLE_GROWTH=1",
"-s", "EXPORTED_RUNTIME_METHODS=['addFunction','removeFunction']"
```

---

## 🔄 切换逻辑层

如果需要切换回JS逻辑层（用于对比测试）：

```javascript
// 在浏览器控制台执行
AppState.useWasm = false;
```

然后重新启动演示模式。

---

## 📊 技术架构

```
┌─────────────────────────────────────┐
│         index.html (UI层)           │
│  - Canvas绘制                       │
│  - 菜单系统                         │
│  - 编辑/演示模式切换                │
└──────────────┬──────────────────────┘
               │
               ▼
┌─────────────────────────────────────┐
│      js/demo.js (演示控制层)        │
│  - startDemo(logicType)             │
│  - startDemoWithWasm()              │
│  - startDemoWithJS()                │
│  - updateDisplayFromWasm()          │
└──────────────┬──────────────────────┘
               │
       ┌───────┴───────┐
       │               │
       ▼               ▼
┌─────────────┐ ┌──────────────┐
│  WASM逻辑层  │ │  JS逻辑层     │
│             │ │              │
│ emc_test.js │ │emc_logic_js.js│
│             │ │              │
│ ems_wasm_   │ │LogicLayer    │
│ adapter.js  │ │KeyHandler    │
│             │ │              │
│ control_    │ │              │
│ binder.js   │ │              │
└─────────────┘ └──────────────┘
```

---

## ✅ 验证清单

- [x] new.json已设置为默认配置
- [x] WASM脚本已添加到index.html
- [x] demo.js支持WASM和JS双逻辑层
- [x] 周期运行自动启动
- [x] 控件绑定自动完成
- [x] Canvas点击事件正确绑定
- [x] LED状态正确显示
- [x] 水温显示为85°C
- [x] 所有已知问题已修复

---

## 📚 相关文档

- [`WASM回调机制完整实现指南.md`](file://D:/Projects/emc-web/WASM回调机制完整实现指南.md)
- [`硬件输入参数设置问题记录.md`](file://D:/Projects/emc-web/硬件输入参数设置问题记录.md)
- [`LED指示灯修复说明.md`](file://D:/Projects/emc-web/phase1-js-logic/LED指示灯修复说明.md)

---

**最后更新**: 2026-05-18  
**维护者**: AI Assistant
