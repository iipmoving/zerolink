# HMI 逻辑标准开发工作流

**概述**: 本文档定义 HMI 逻辑从需求到 MCU 固件的完整开发管线。核心原则是 **JS+JSON 先行验证，C 代码跟随实现**。

---

## 工作流总览

```
需求变更
  │
  ▼
[Phase 1] JS+JSON 闭环验证 (sim/test_hmi/)
  ├─ 修改 JSON 规则声明  (logic/four_head_v4.json)
  ├─ 修改 JS 引擎逻辑    (js/json_logic_engine.js)
  ├─ 运行 JS 测试        (js/test_runner.js → 34/34)
  └─ 交互验证            (index.html Canvas UI)
  │
  ▼ 所有 JS 测试通过 ✓
  │
[Phase 2] C 代码转换
  ├─ 更新 hmi_data.c     (cfg/hmi_data.c, 由 gen_hmi.js 生成)
  ├─ 更新 app_hmi.c      (wasm/app/app_hmi.c)
  └─ 更新 app_hmi.h      (wasm/app/app_hmi.h, 如需新增枚举)
  │
  ▼
[Phase 3] WASM 双引擎对比测试
  ├─ 编译 WASM           (wasm/Makefile → app_logic.js)
  ├─ 编写测试场景        (wasm/test_wasm_basic.js)
  └─ 运行测试            (node test_wasm_basic.js → N/N PASS)
  │
  ▼ 所有 WASM 测试通过 ✓
  │
[Phase 4] MCU 集成
  ├─ 复制到 Project/Claude/app/
  ├─ armcc 编译           (0 error, 0 warning)
  └─ 烧录验证
```

---

## Phase 1: JS+JSON 闭环验证

### 1.1 目录结构

```
sim/test_hmi/
├── index.html              # Canvas UI (交互验证)
├── js/
│   ├── json_logic_engine.js # JS 引擎 (参考实现)
│   ├── message_bus.js       # 消息总线
│   ├── key_handler.js       # 按键注入
│   ├── slot_element.js      # Slot 渲染元素
│   ├── led_element.js       # LED 渲染元素
│   ├── blink_rule.js        # 闪烁规则
│   ├── mode_rule.js         # 模式规则
│   └── test_runner.js       # 测试执行器 (34条)
├── logic/
│   └── four_head_v4.json   # JSON 规则声明 (权威数据源)
└── wasm/
    ├── app/app_hmi.c        # C 引擎
    ├── app/app_hmi.h        # C 引擎头文件
    ├── cfg/hmi_data.c       # JSON→C 数据
    ├── wasm_adapter.js      # WASM↔JS 适配器
    └── test_wasm_basic.js   # 双引擎对比测试
```

### 1.2 修改流程

1. **修改 JSON**: 在 `four_head_v4.json` 中添加/修改规则
2. **修改 JS 引擎**: 如需新增逻辑函数，在 `json_logic_engine.js` 中实现
3. **运行测试**: `node js/test_runner.js` → 确保 34/34 通过
4. **交互验证**: 在浏览器打开 `index.html`，手动操作验证行为

### 1.3 JSON 三层架构

```
Zone Logic (孤岛函数)       ← 标准函数: selectHead, confirmSelect, handlePowerKey...
    ↓
JSON Binding (路由+动作)    ← four_head_v4.json: global_routes, zone_routes, process_routes
    ↓
Presentation Controls       ← SlotElement, LEDElement, BlinkRule, ModeRule
```

**原则**: 可配置的行为→JSON声明；执行函数→单向独立；问题→隔离处理。

### 1.4 测试执行

```bash
cd sim/test_hmi
node js/test_runner.js          # JS 单元测试
```

---

## Phase 2: C 代码转换

### 2.1 转换顺序

1. **数据层** (`hmi_data.c`): 由 `gen_hmi.js` 自动生成，将 JSON 规则转换为 C 静态数组
2. **类型层** (`app_hmi.h`): 确保 enum 和 struct 定义完整
3. **逻辑层** (`app_hmi.c`): 按 JS 函数→C 函数对应表实现

### 2.2 生成 hmi_data.c

当 JSON 规则变更时，运行生成器:
```bash
node sim/test_hmi/wasm/cfg/gen_hmi.js \
  sim/test_hmi/logic/four_head_v4.json \
  > sim/test_hmi/wasm/cfg/hmi_data.c
```

### 2.3 手动转换规则

参考 `docs/ai-guidelines/js-to-c-conversion.md` 和 `docs/architecture/js-json-to-c-encoding-guide.md`。

核心检查点:
- [ ] 所有 enum 值使用枚举名，无 magic number
- [ ] 路由优先级顺序与 JSON 一致
- [ ] 显示区映射 Z1→[0,1], Z2→[2,3], Z3→[4,5], Z4→[6,7]
- [ ] `sync_leds()` + `post_display()` 成对调用
- [ ] 定时器单位 ms，tick 计数用 `*100u` 转换

---

## Phase 3: WASM 双引擎对比测试

### 3.1 测试原理

```
输入(按键序列) ─┬─→ JS 引擎 → getState()/getDisplay() ─┐
                │                                         ├─→ assert 对比
                └─→ C/WASM 引擎 → getState()/getDisplay()┘
```

### 3.2 编译 WASM

```bash
cd sim/test_hmi/wasm
make build    # 编译 C → app_logic.js + app_logic.wasm
```

或使用批处理:
```bash
cmd /c build_wasm.bat
```

### 3.3 运行测试

```bash
make test     # node test_wasm_basic.js
# 或
make wasm_test  # build + test
```

### 3.4 添加新测试场景

在 `test_wasm_basic.js` 中:

```javascript
/* N. 测试: Flow-XX — 描述 */
console.log('\n--- Test N (Flow-XX): 描述 ---');

/* 注入 WASM 按键 */
wasm.postKey('HEAD_1', 'tap');

/* 注入 JS 按键 (通过 KeyHandler) */
KeyHandler.injectKey(15, 'tap'); /* KEY_LEFT_P_SET_UP, TAP */

/* 消费消息 */
flushMsg();

/* 对比状态 */
jsSt = JsonLogicEngine.getState();
wmSt = wasm.getState();
assertState('TN 描述', jsSt, wmSt);

/* 对比显示 */
jsDisp = JsonLogicEngine.getDisplay();
wmDisp = wasm.getDisplay();
assertDisplay('TN display', jsDisp, wmDisp);
```

**按键映射参考** (WASM string → C key_code):

| wasm.postKey() | KeyHandler.injectKey() | 含义 |
|---------------|------------------------|------|
| `'POWER'` | `4,` | 开关 |
| `'TIMER'` | `5,` | 定时 |
| `'PAUSE'` | `6,` | 暂停 |
| `'CHILD_LOCK'` | `8,` | 童锁 |
| `'HEAD_1'` | `15,` | Z1键 |
| `'HEAD_2'` | `14,` | Z2键 |
| `'HEAD_3'` | `13,` | Z3键 |
| `'HEAD_4'` | `12,` | Z4键 |
| `'0'`-`'9'` | `22`-`31,` | 档位 |

**验收标准**: 所有断言 PASS，0 FAIL。

---

## Phase 4: MCU 集成

### 4.1 复制文件

将 WASM 验证通过的 C 文件复制到 MCU 项目:

```bash
# 从 WASM 到 MCU 项目
cp sim/test_hmi/wasm/app/app_hmi.c   Project/Claude/app/
cp sim/test_hmi/wasm/app/app_hmi.h   Project/Claude/app/
cp sim/test_hmi/wasm/cfg/hmi_data.c  Project/Claude/cfg/
```

### 4.2 编译验证

```bash
cd Project
armcc -c --cpu Cortex-M0+ -DSC32L14xx --c99 \
  -I "../FWLib/SC32F1XXX_Lib/inc" -I "../CMSIS" -I "../MCU_Drivers" \
  -I "../Claude/app" -I "../Claude" -I "../Claude/src" -I "../Claude/test" \
  -I "../Claude/hal" -I "../Claude/core" -I "../Claude/drv" \
  ../Claude/app/app_hmi.c -o ../Claude/app/app_hmi.o
```

**验收标准**: 0 error, 0 warning。

### 4.3 烧录验证

使用 Keil MDK 或 JLink 烧录到 SC32L14T 目标板，手动操作验证关键行为路径。

---

## 关键原则

1. **JSON is the source of truth**: 规则在 JSON 中声明，C 代码只负责执行
2. **JS leads, C follows**: 永远是 JS 先验证通过，再写 C 代码
3. **Test before integrate**: WASM 双引擎测试通过后，才进入 MCU 编译
4. **No silent divergence**: JS 和 C 的行为必须可证明地一致

---

## 相关文档

- 转换规则: `docs/ai-guidelines/js-to-c-conversion.md`
- 完整映射: `docs/architecture/js-json-to-c-encoding-guide.md`
- 架构报告: `docs/architecture/json-architecture-phase-report.md`
- 编码规范: `.claude/specs/code-style.md`
- 通用语言: `.claude/specs/ubiquitous-language.md`

---

*维护规则: 工作流变更时同步更新本文档。*
