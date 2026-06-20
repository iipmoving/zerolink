---
name: zerolink
description: "ZeroLink (零耦合) — 统一嵌入式方法论 skill。整合架构约束、管道配对、代码生成、合规审计全流程。"
user-invocable: true
---

# /zerolink — 零耦合嵌入式方法论统一 SKILL

> **整合**: methodology-seed-v2.0（架构规则）+ codeGen/（代码生成引擎）
> **核心**: std_module.h — 模块的骨架宏就是范式


---

## 一、方法论核心

### 1.1 五层架构

```
APP  → 业务逻辑层
  ↓ __weak 直调
DRV  → 设备驱动层
  ↓
HAL  → 硬件抽象层
PROTO → 纯函数协议层
API   → 装配器半层
```

### 1.2 六大铁律

| # | 铁律 | 阻断层级 | 工具 |
|---|------|---------|------|
| 1 | 层间隔离: APP 不得 include DRV/HAL | L1 | check_deps.py |
| 2 | 三种通信互补: __weak/Msg_Post/数据交换机 | L0+L1 | check_weak_pairs.py |
| 3 | 头文件私有化: APP/DRV .h 用 //#define 注释 | L0 | 编译器阻断 |
| 4 | 独立类型声明 | L2 | generate_structs.py |
| 5 | 骨架宏即范式: MODULE_SKELETON + MODULE_EXPORT | L1 | check_paradigm.py |
| 6 | 数据契约先行 | L3 | code review |

---
## 二、管道配对 (PIPE PAIRING)

### 2.1 三层定义

```
第一层: MODULE_xxx_PARAMS(Producer, Consumer)  → 纯数据字段
第二层: MODULE_xxx_LINK(Producer, Consumer)    → 管道 (status + params[])
第三层: MODULE_INPUT(Consumer) / MODULE_OUTPUT(Producer)  → 聚合

Producer (输出方)                              Consumer (输入方)
MODULE_OUTPUT_PARAMS(Producer, Consumer)       MODULE_INPUT_PARAMS(Producer, Consumer)
  └── 字段布局                                    └── 字段布局完全一致

MODULE_OUTPUT_LINK(Producer, Consumer)          MODULE_INPUT_LINK(Producer, Consumer)
  └── status + params[] 实例                    └── 布局一致的实例

MODULE_OUTPUT(Producer)                         MODULE_INPUT(Consumer)
  └── *{Consumer}_params (指针)              └── *{Producer}_params (指针)
```

### 2.2 状态位协议

| 位 | 常量 | 置位 | 清零 |
|----|------|------|------|
| bit0 | ST_INIT (0x01) | Constructor | — |
| bit1 | ST_NEW (0x02) | InputCallback | ProcessInput 消费后 |
| bit2 | ST_OUT (0x04) | ProcessInput 输出段 | OutputCallback 后 |

### 2.3 LINK 样式

| style | C 声明 | 含义 |
|-------|--------|------|
| pointer | *params | 指针引用外部数据 |
| array | params[N] | 定长数组 |
| pointer_array | *params[N] | 指针数组 |

### 2.4 对称命名规则

```
Producer OUTPUT 成员名: out->{Consumer}_params
Consumer INPUT  成员名: in->{Producer}_params
LINK 类型名: Calculator_to_ElecParams_Output_Link
```

---
## 三、MODULE_SKELETON 展开

```c
#define STD_MODULE_ENABLE_ISR  0
#include "std_module.h"

MODULE_SKELETON(ModuleName);
// 展开: g_input/g_output + Init/ProcessInput 前向声明
//       + InputCallback/OutputCallback weak 空壳
//       + Constructor (memset + Init + ST_INIT)
//       + DoWork (Constructor → InputCallback → ProcessInput → OutputCallback)

MODULE_EXPORT(ModuleName);
// 展开: ModuleName_GetIO(Para_Grp_t**, Para_Grp_t**, void(**)(void))
```

### 模块标准结构

```c
#include "std_module.h"
#include "module_io.h"
#include <string.h>

static MODULE_INPUT(ModuleName)  s_in;
static MODULE_OUTPUT(ModuleName) s_out;

MODULE_SKELETON(ModuleName);

static void Init(void) {
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    g_input.para  = &s_in;
    g_output.para = &s_out;
}

static void ProcessInput(void) {
    /* ====== 输入段 ====== */
    /* ====== 计算段 (调纯函数) ====== */
    /* ====== 输出段 (写输出 + 置 ST_OUT) ====== */
}

MODULE_EXPORT(ModuleName);
```

---
## 四、数据交换机 (Switcher)

```
Switcher_Init():
  SLOT_GETIO(AppAdc);
  SLOT_GETIO(Calculator);
  ...

Switcher_Run():
  for each slot in slot_order:
    slot.pDoWork();
    _route_slot();  // 检查输出 → 调消费者 InputCallback

InputCallback (中间层):
  in->{Producer}_params = (void*)&out->{Consumer}_params;  // 指针直穿
```

### include 权限

| 文件 | 权限 |
|------|------|
| Switcher.c | 唯一合法全路径 include _io.h |
| 模块 .c | 只引自己 _io.h |
| 任何 .h | 不得 include 其他模块的 .h |

---
## 五、完整工作流

### 5.1 新项目 (/zerolink init)

1. 交互问答: 项目名/MCU/编译器/层名
2. 创建目录结构 (app/drv/hal/proto/core/include/cfg/docs)
3. 复制填充模板 (CLAUDE.md, structs.json, interface_map.h, std_module.h)
4. 配置 deps_config.json + pre-commit hook
5. 验证: check_deps.py --self-test PASS

### 5.2 新增模块 (/zerolink new-module)

| 步骤 | 操作 | 产出 |
|------|------|------|
| 1 | 确定模块名 + 层 + 数据契约 | docs/data-contract.md |
| 1.5 | I/O 对齐检查 | 布局一致确认 |
| 2 | 生成 _io.h (PARAMS/LINK 三层) | include/{module}_io.h |
| 3 | 生成 .c (Skeleton/Init/ProcessInput/Export) | {layer}/{module}.c |
| 4 | 注册到 Switcher (SLOT + GETIO + 路由) | data_switcher.c |
| 5 | 验证 | check 全 PASS |

### 5.3 修改模块 (/zerolink modify-module)

| 步骤 | 操作 |
|------|------|
| 0 | 检查数据契约, 先更新 docs/data-contract.md |
| 1 | 找入口: 识别所有外部调用点 |
| 2 | 查路径: 追踪每条 I/O 数据流 |
| 3 | 提取 I/O 参数: 定义 PARAMS/LINK/OUTPUT |
| 4 | 建骨架: 改名备份旧文件, 生成模板, 搬功能 |
| 5 | 填逻辑: 移业务代码入 ProcessInput |
| 6 | 验证: check 全 PASS |

### 5.4 合规检查 (/zerolink check)

```
/zerolink check [--quick]
  1. check_deps.py       — 层依赖审计
  2. check_weak_pairs.py — __weak 配对一致性
  3. check_structs.py    — 结构体一致性
  4. check_include.py    — _io.h 权限审计
  5. 编译检查
  --quick: 跳过第 5 步
```

### 5.5 扫描与 JSON 维护

```
/zerolink scan
  → 扫描项目 → project.json
    遗留项目: 读 .c/.h 推断
    v2.3项目: 读 _io.h + data_switcher.c 机械提取

/zerolink update-json
  1. python codeGen/scan_project.py <项目路径>
  2. python codeGen/gui_editor.py (用户确认)
  3. AI 编辑 JSON/project.json
  4. python codeGen/code_gen.py gen --config ... --output ...
  5. 验证并提交

/zerolink check-json
  → Schema 合规 / 源文件存在 / 管道对称 / slot_order 完整
```

### 5.6 一键生成 (/zerolink gen)

```
/zerolink gen --config project.json --output <项目/src>
  → include/*_io.h
  → core/data_switcher.c
  → {layer}/*.c + *.h

子命令:
  --only-io        仅 io.h
  --only-switcher  仅 data_switcher.c
  --only-modules   仅模块 .c/.h
```

---
## 六、工具路径

```
codeGen/                        # 生成工具
├── code_gen.py                 # CLI 主入口 (init + gen)
├── gen_io_h.py                 # io.h 生成器
├── gen_module_c.py             # 模块骨架生成器
├── gen_switcher.py             # data_switcher.c 生成器
├── gui_editor.py               # Tkinter GUI 编辑器
├── scan_project.py             # 扫描项目 → project.json
├── check_json.py               # JSON 正确性验证
├── keil_parser.py              # KEIL .uvprojx 解析器
└── config_schema.json          # project.json Schema

.claude/tools/                  # 检查工具
├── check_deps.py               # 层依赖审计
├── check_weak_pairs.py         # __weak 配对一致性
├── check_structs.py            # 结构体一致性验证
├── check_include.py            # _io.h include 权限
├── check_output_callback.py    # @OUTPUT_CALLBACK 审批
├── check_paradigm.py           # 骨架宏配对检查
├── check_switcher_macros.py    # Switcher 宏检查
└── generate_structs.py         # structs.json → types.h
```

---
## 七、方法论文档索引

| 文件 | 主题 | 阅读时机 |
|------|------|---------|
| methodology-seed/01-architecture.md | 五层架构 + 六大铁律 | 新项目第一课 |
| methodology-seed/02-weak-callback.md | __weak + Msg_Post + 数据交换机 | 模块间通信设计 |
| methodology-seed/05-interface-management.md | interface_map.h 维护 | 新增 __weak 通道 |
| methodology-seed/07-struct-generation.md | JSON→types.h 生成 | 跨模块结构体变更 |
| methodology-seed/08-data-switcher.md | 数据交换机 PULL 路由 | Switcher 项目 |
| methodology-seed/09-std-module.md | MODULE_SKELETON 详解 | 新模块 / 模块迁移 |
| methodology-seed/10-data-contract.md | 数据交互说明书 | 编码前 I/O 对齐 |
| methodology-seed/03-dual-engine.md | JS/C 双引擎同源 | HMI 项目 |
| methodology-seed/04-golden-output.md | 黄金输出三阶段验证 | 模块分解重构 |
| methodology-seed/06-json-driven.md | JSON 三层驱动架构 | HMI 项目 |

---
## 八、速查表

### 通信机制选择

```
必须在同一次调度槽内完成—？
  ┌─ 是 → __weak 直调
  └─ 否 → 事件→Msg_Post / 结构化数据→数据交换机
```

### 关键约束

- 跨模块 struct: #pragma pack(4) + res[] 填充到 4 倍数
- APP/PROTO 的 .h: 用 //#define include guard (L0 阻断)
- _io.h: 保留 #define include guard (公开接口)
- 任何 .h 不得 include 其他模块的 .h
- ProcessInput 不得内联计算, 调纯函数
- res/reserved 字段跳过 JSON 导出

### 提交前自检清单

- [ ] check_deps.py → 0 violations
- [ ] check_weak_pairs.py → 0 violations
- [ ] check_structs.py → PASS
- [ ] check_include.py → 0 violations (Switcher 项目)
- [ ] 编译 0 error 0 warning
- [ ] 数据契约与代码一致
- [ ] 无裸 __weak, 全部 __attribute__((weak))
- [ ] git reset 前 stash 或建备份分支

