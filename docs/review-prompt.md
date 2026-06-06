# four_head v2.0 架构审查提示词

> 用途: 第三方技术审查 — 方法论、SKILL、检查工具、模块架构、测试用例

---

## 审查范围

| 模块 | 路径 | 行数 | 说明 |
|------|------|------|------|
| 方法论种子 | `methodology-seed-v2.0/` | — | 零耦合嵌入式开发约束系统 |
| 统一骨架 | `four_head/src/core/std_module.h` | 83 | 模块化宏骨架（核心产出） |
| 数据交换机 | `four_head/src/core/data_switcher.c/h` | 120 | 函数指针调度 + 输出路由 |
| 检查工具 | `four_head/tools/check_deps.py` | — | 层依赖审计 |
| 检查工具 | `four_head/tools/check_msgs.py` | — | 消息通道一致性 |
| 检查工具 | `four_head/tools/check_all.py` | — | 一键审计 + pre-commit 入口 |
| SKILL | `.reasonix/skills/check.md` | — | 合规检查 playbook |
| SKILL | `.reasonix/skills/new-module.md` | — | 模块创建向导 |
| SKILL | `.reasonix/skills/modify-module.md` | — | 模块迁移转换 SOP |
| 已迁移模块 | `four_head/src/app/app_power.c` | 274 | 功率管理 |
| 已迁移模块 | `four_head/src/app/app_cooking.c` | 282 | 烹饪状态机 |
| 已迁移模块 | `four_head/src/app/app_hmi.c` | 1841 | HMI JSON 引擎 |
| 已迁移模块 | `four_head/src/app/app_comm_mgr.c` | 293 | 通信轮询 |
| 已迁移模块 | `four_head/src/app/app_protect.c` | 308 | 故障检测 |
| 已迁移模块 | `four_head/src/app/app_seg_align.c` | 527 | 段位对齐 |
| 已迁移模块 | `four_head/src/drv/drv_key.c` | 210 | 按键驱动 |
| 已迁移模块 | `four_head/src/drv/drv_display.c` | 496 | 显示驱动 |
| 已迁移模块 | `four_head/src/drv/drv_buzzer.c` | 429 | 蜂鸣器驱动 |
| 已迁移模块 | `four_head/src/drv/drv_comm_mgr.c` | 114 | 通讯管理 |
| 测试用例 | `four_head/sim/test_hmi/run_tests_node.js` | — | 30 条 HMI 数据流测试 |
| 测试用例 | `four_head/sim/test_hmi/wasm/test_wasm_basic.js` | — | 35 项 WASM 双引擎对比 |
| 主入口 | `four_head/src/main.c` | — | 10 槽调度 + Switcher 接线 |
| pre-commit | `.git/hooks/pre-commit` | — | L1 阻断 |

---

## 审查要点

### 一、方法论审查 (`methodology-seed-v2.0/`)

1. **约束层级是否完整** — L0 编译器阻断 (`//#define` include guard)、L1 提交阻断 (pre-commit + check_all.py)、L2 生成一致性、L3 文档约定 四级是否都在？
2. **`std_module.h` 是否符合方法论** — 宏骨架是否替代了手写 `g_in/g_out`、`GetIO`、`DoWork`？三个通信机制（__weak/Msg_Post/数据交换机）中 Switcher 的角色是否清晰？
3. **违规是否真的被阻断** — 找一个故意的层违规（比如 APP 层 include DRV 头文件），看 check_deps.py 能否检出、pre-commit 能否阻断？

### 二、SKILL 审查 (`.reasonix/skills/`)

1. **`new-module.md`** — Step 0 路径检索是否覆盖了 I/O 表规划？模板代码是否使用了 `core/std_module.h`？
2. **`modify-module.md`** — 6 步 SOP 是否覆盖：找入口 → 查路径 → 提取 I/O → 搭骨架 → 填逻辑 → 验证？
3. **`check.md`** — 5 项检查是否完整：层依赖 + 消息通道 + 编码规范 + 结构体 + 编译？编码规范审计能否检出裸 `__weak`、缺 `_Constructor`、缺 `//#define`？
4. **SKILL 一致性** — 三个 SKILL 是否都指向同一个范式（`core/std_module.h`）？

### 三、检查工具审查 (`tools/`)

1. **`check_deps.py`** — 层白名单是否正确？APP 层禁止 include DRV/HAL，DRV 层禁止 include APP，是否都覆盖？
2. **`check_msgs.py`** — 在 `msg_scheduler` 已删除的背景下，0 个活跃消息通道是否合理？
3. **`check_all.py`** — 是否在 pre-commit hook 中被自动调用？失败时 exit 1 阻断提交？
4. **误报/漏报** — 现有 46 个文件中有没有漏检的违规？有没有误报？

### 四、骨架及模块架构审查 (`core/std_module.h` + 已迁移模块)

1. **`std_module.h` 设计**：
   - `Para_Grp_t` 的 `void *para` 使用是否类型安全？
   - `MODULE_SKELETON()` 生成的 `g_input`/`g_output` 生命周期是否正确？
   - `MODULE_EXPORT(name)` 生成的 `__attribute__((weak)) name_OnOutput()` 能否被强符号覆盖？
   - `__attribute__((constructor))` 注册 `_onOutput` 的顺序是否可靠？
   - `ST_INIT`/`ST_NEW`/`ST_OUT` 三个状态位是否覆盖了所有场景？

2. **已迁移模块的一致性**：
   - 每个模块是否都实现了：`Init()` + `ProcessInput()` + `MODULE_EXPORT()`？
   - 输入来源是否通过强符号 `{Producer}_OnOutput()` 写入 `s_in`？
   - 输出去向是否通过 `g_output.info.status |= ST_OUT` 触发？
   - 旧入口桥接是否完整（`App_Power_Init → Constructor()` 等）？
   - 是否有模块遗留了 `__weak` 裸关键字（应全部替换为 `__attribute__((weak))`）？

3. **Switcher 设计**：
   - `data_switcher.c` 是否只通过 `GetIO` 获取函数指针，不直接认知模块名？
   - 输出路由强符号（`DrvKey_OnOutput`、`AppHmi_OnOutput`）是否正确实现了一对多分发？
   - Switcher 是否完全不碰模块的 `g_input.info` / `g_output.info`？

### 五、测试用例审查 (`sim/test_hmi/`)

1. **JS 数据流测试**— `node run_tests_node.js` 是否 33/34 通过（1 个预期失败 Flow-15）？
2. **WASM 双引擎对比** — 从 C 源码重新编译 `emcc ... modules/*.c` → `node test_wasm_basic.js` 是否 66/66 通过？
3. **测试覆盖缺口** — 测试覆盖了 HMI 状态机的全部流程，但**没有覆盖**：
   - 功率控制链（PowerCtrl → IGBT降功率 → 软启动 → 间断加热）
   - SWD/通信模块（MODBUS 轮询/响应）
   - 故障检测（IGBT/炉面高温/通讯断开）
   - 按键去抖/长按/连发逻辑
   - 段位对齐模块
   这些是否需要补充测试？

### 六、迁移完整性审查

1. **是否还有 `__weak` 裸关键字残留？** — `grep -n "__weak " src/` 应该只有 `__attribute__((weak))` 或 0 结果。
2. **是否还有 `Msg_Post` / `MsgScheduler` 残留？** — 文件已删除，但注释中可能有引用。
3. **`interface_map.h` 是否还有保留价值？** — 以前是双向维护表，现在由 `MODULE_EXPORT` + 强符号覆盖自动管理。
4. **`include/` 目录是否空？** — 旧 `_io.h` 已删除，`std_module.h` 在 `src/core/` 下。
5. **main.c 的 Init 序列是否正确？** — 所有 `XX_Init` 是否都委托给了 `Constructor()`？

---

## 审查输出要求

按以下格式输出每项发现：

```
[严重/警告/建议] 标题
  位置: file:line
  问题: 一句话描述
  建议: 具体修复方案
```

- **严重**: 运行时可能崩溃或数据错误
- **警告**: 不符合方法论约定，但当前不会崩溃
- **建议**: 可优化项

---

## 环境准备

```bash
# 1. 层依赖审计
cd <project_root>
python four_head/tools/check_deps.py four_head/src

# 2. 消息通道审计
python four_head/tools/check_msgs.py four_head/src

# 3. 一键合规审计
python four_head/tools/check_all.py

# 4. JS 测试
node four_head/sim/test_hmi/run_tests_node.js

# 5. WASM 测试（需 emcc）
cd four_head/sim/test_hmi/wasm
# 备份旧文件后重新编译
emcc -O1 -s MODULARIZE=1 -s EXPORT_NAME="AppLogicModule" ... modules/*.c cfg/hmi_data.c -o app_logic.js
node test_wasm_basic.js
```
