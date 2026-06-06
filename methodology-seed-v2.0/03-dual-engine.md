# 03 — 双引擎同源检测体系

> **核心思想**: 一套 JSON 规则驱动两个独立实现的引擎（JS 参考 + C 生产），给相同输入逐字段比对输出，保证行为等价。

---

## 一、三层同源金字塔

```
         ┌────────────────────────────┐
         │  JSON 规则                  │  ← 唯一权威数据源
         │  (global/zone/process)      │
         ├────────────────────────────┤
         │  JS 引擎 (参考实现)          │  ← 先行验证, 快速迭代
         ├────────────────────────────┤
         │  WASM C 引擎 (生产等价)      │  ← 跟随实现, 同源对比
         └────────────────────────────┘
```

**流程铁律**:
```
JSON 规则变更
  → JS 引擎先改, JS 独立测试必须通过
  → C 模块跟随修改
  → 双引擎对比测试必须通过
  → 只有两步都通过, 变更才能合并
```

---

## 二、归一化接口: 保证 JS ↔ C 同源

### 2.1 状态结构归一

JS 侧和 C 侧使用相同语义的状态结构，通过适配层做 C enum → JS string 的机械映射:

```javascript
// wasm_adapter.js — 归一化映射表 (唯一翻译点)
const GLOBAL_MODE_C2JS = ['powered_off', 'deep_sleep', 'working', 'paused'];
const ZONE_NODE_C2JS   = ['idle', 'selecting', 'cooking'];
const SEG_MODE_C2JS    = ['power', 'dash', 'off', 'ascii', 'timer_setting'];
```

**规则**: 任何新增状态值，必须在 C enum、JS string、适配层映射表中**同步新增**。三者顺序严格一致。

### 2.2 按键码归一

```
C 侧:  drv_key.h         KEY_POWER_1 = 11
JS 侧: wasm_adapter.js   KEY_CODE.POWER_1 = 11
JSON:  路由规则 keyCode   keyCode: 11
```

**规则**: 键码值来源于硬件定义（C 侧），JS 和 JSON 跟随 C 侧。

### 2.3 JSON 规则归一

```
rules.json  ──→ JS 引擎运行时加载
            ──→ json_to_c.py ──→ hmi_data.c ──→ C 引擎编译时嵌入
```

**规则**: JSON 是唯一数据源。禁止在 C 代码中硬编码 JSON 已有的配置值。

### 2.4 测试断言归一

每条断言对比 JS 引擎和 C 引擎的**同名字段**:

```javascript
const jsSt = jsEngine.getState();
const wmSt = wasmEngine.getState();
assertEq('T05 确认cooking', jsSt.heads[0].node, wmSt.heads[0].node);
```

**已知差异豁免**: 显式声明跳过字段 + 原因注释:
```javascript
assertStateCompat('T27 定时', jsSt, wmSt,
    ['node','power_level','timer_value'],  // 比对字段
    ['timer_setting','timer_active']);      // 跳过字段 (原因: C侧不自动确认)
```

---

## 三、新增 HMI 规则 SOP

```
步骤 1: 修改 JSON 规则
步骤 2: JS 引擎先行验证 → JS 独立测试 100% PASS
步骤 3: json_to_c.py 重新生成 C 数据
步骤 4: C 模块跟随实现
步骤 5: 双引擎对比测试 → 100% PASS
步骤 6: check_deps.py + check_weak_pairs.py → 0 violations
```

---

## 四、新增 WASM 导出函数 SOP

```
步骤 1: 在对应模块 .c 中实现函数
步骤 2: 在对应模块 .h 中声明函数
步骤 3: 在 Makefile EXPORTED_FUNCS 中添加 _function_name
步骤 4: 在 build 脚本中同步添加
步骤 5: 如需 JS 侧调用: 在适配层中添加包装函数
步骤 6: emcc 编译 → 双引擎测试 → 全部通过
```

---

## 五、接口一致性检查清单

每次扩展后必须验证:

| # | 检查项 | 方法 |
|---|--------|------|
| 1 | JSON → C 数据一致 | 重运行 `json_to_c.py` |
| 2 | C enum ↔ JS string 映射 | 比对适配层映射表 |
| 3 | 键码三处一致 (C/JS/JSON) | grep 键码定义 |
| 4 | __weak 签名配对 | `check_weak_pairs.py` |
| 5 | 新增导出函数两处同步 | 比对 Makefile + build 脚本 |
| 6 | 层依赖无违规 | `check_deps.py` → 0 |
| 7 | 双引擎全部通过 | `node test_wasm_basic.js` → 100% PASS |
