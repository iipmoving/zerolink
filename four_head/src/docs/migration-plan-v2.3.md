# Four_Head V2.3 迁移规划

## 一、项目现状

### 1.1 已有基础设施
- `std_module.h` — v2.3 骨架宏（MODULE_SKELETON / MODULE_EXPORT）
- `data_switcher.c/h` — PULL 路由调度器
- `interface_map.h` — 路由配对映射（文档）

### 1.2 已迁移模块（v2.2）
| 模块 | 状态 | 说明 |
|------|------|------|
| AppPower | ✅ 已迁移 | MODULE_SKELETON |
| AppProtect | ✅ 已迁移 | MODULE_SKELETON |
| AppHmi | ✅ 已迁移 | MODULE_SKELETON |
| DrvKey | ✅ 已迁移 | MODULE_SKELETON |

### 1.3 待迁移模块（v1.x → v2.3）
| 模块 | 当前模式 | 问题 |
|------|---------|------|
| AppCommMgr | v1.x 传统 | 未上 MODULE_SKELETON |
| AppCooking | v1.x 传统 | 未上 MODULE_SKELETON |
| AppSegAlign | v1.x 传统 | 未上 MODULE_SKELETON |
| DrvDisplay | v1.x __weak | 未上 MODULE_SKELETON |
| DrvBuzzer | v1.x __weak | 保留 @OUTPUT_CALLBACK 例外 |
| DrvCommMgr | v1.x 传统 | 未上 MODULE_SKELETON |

---

## 二、V2.3 目标架构

### 2.1 核心变化
1. **三层结构定义**：PARAMS → LINK → OUTPUT
2. **指针直穿**：Consumer 回调不做 memcpy，直接赋指针
3. **独立有效值管理**：每个 LINK 有独立的 status/valid 标志
4. **宏定义生成**：用 `MODULE_OUTPUT_PARAMS/LINK/OUTPUT` 宏生成结构体名

### 2.2 宏定义模板

```c
/* ===== 三层宏定义 ===== */

/* 第一层：PARAMS — 纯数据参数 */
#define MODULE_OUTPUT_PARAMS(owner, consumer) \
    owner##_to_##consumer##_Output_Params

/* 第二层：LINK — status + *params 指针 */
#define MODULE_OUTPUT_LINK(owner, consumer) \
    owner##_to_##consumer##_Output_Link

/* 第三层：OUTPUT — 模块总输出槽 */
#define MODULE_OUTPUT(owner) \
    owner##_Output

/* 输入槽 */
#define MODULE_INPUT(owner) \
    owner##_Input
```

### 2.3 指针直穿示例

```c
/* Producer: AppCommMgr */
typedef struct {
    uint8_t  head_idx;
    uint8_t  online;
    uint16_t igbt_temp;
    uint16_t bot_temp;
    uint16_t voltage;
} MODULE_OUTPUT_PARAMS(CommMgr, Power);

typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(CommMgr, Power) *params;
} MODULE_OUTPUT_LINK(CommMgr, Power);

typedef struct {
    MODULE_OUTPUT_LINK(CommMgr, Power)    *to_power;
    MODULE_OUTPUT_LINK(CommMgr, Cooking) *to_cooking;
    MODULE_OUTPUT_LINK(CommMgr, Protect) *to_protect;
} MODULE_OUTPUT(CommMgr);

/* Consumer: AppPower */
typedef struct {
    MODULE_OUTPUT_LINK(CommMgr, Power)  *comm;
    MODULE_OUTPUT_LINK(Protect, Power)  *protect;
    MODULE_OUTPUT_LINK(Cooking, Power)   *cooking;
} MODULE_INPUT(Power);

/* Consumer 回调 — 指针直穿 */
void AppPower_OnCommMgrData(Para_Grp_t *pOut)
{
    MODULE_INPUT(Power) *in = (MODULE_INPUT(Power) *)g_input.para;
    MODULE_OUTPUT(CommMgr) *out = (MODULE_OUTPUT(CommMgr) *)pOut->para;
    
    in->comm = out->to_power;       /* 指针直穿，零拷贝 */
    in->comm->status |= ST_NEW;
}
```

---

## 三、数据流图

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         V2.3 指针直穿数据流                                  │
└─────────────────────────────────────────────────────────────────────────────┘

                    ┌──────────────┐
                    │  AppCommMgr  │ (Producer)
                    │  g_output    │
                    │  ┌──────────┐│
                    │  │*to_power ││───────────────────────────────┐
                    │  └──────────┘│                               │
                    │  ┌──────────┐│                               │
                    │  │*to_cook  ││─────────────┐                 │
                    │  └──────────┘│             │                 │
                    │  ┌──────────┐│             │                 │
                    │  │*to_protect││─────┐       │                 │
                    │  └──────────┘│     │       │                 │
                    └──────────────┘     │       │                 │
                                         │       │                 │
                    ┌──────────────┐     │       │                 │
                    │  AppProtect  │←────┘       │                 │
                    │  g_input     │             │                 │
                    │  ┌──────────┐│             │                 │
                    │  │*comm     ││←────────────┘                 │
                    │  └──────────┘│                               │
                    │  g_output    │                               │
                    │  ┌──────────┐│                               │
                    │  │*to_power ││───────────────────────────────┤
                    │  └──────────┘│                               │
                    └──────────────┘                               │
                                         │                         │
                    ┌──────────────┐     │                         │
                    │  AppCooking  │←────┘                         │
                    │  g_input     │                               │
                    │  ┌──────────┐│                               │
                    │  │*comm     ││←──────────────────────────────┘
                    │  │*key      ││←───────────────────────────────┐
                    │  └──────────┘│                               │ │
                    │  g_output    │                               │ │
                    │  ┌──────────┐│                               │ │
                    │  │*to_power ││───────────────────────────────┤ │
                    │  └──────────┘│                               │ │
                    └──────────────┘                               │ │
                                         │                         │ │
                    ┌──────────────┐     │                         │ │
                    │  AppPower    │←────┼─────────────────────────┘ │
                    │  g_input     │     │                           │
                    │  ┌──────────┐│     │                           │
                    │  │*comm     ││←────┘                           │
                    │  │*protect  ││←───────────────────────────────┤
                    │  │*cooking  ││←───────────────────────────────┘
                    │  └──────────┘│
                    │  g_output    │
                    │  ┌──────────┐│
                    │  │*to_drv   ││─────────────────────────────────┐
                    │  └──────────┘│                                 │
                    └──────────────┘                                 │
                                         │                           │
                    ┌──────────────┐     │                           │
                    │  DrvCommMgr  │←────┘                           │
                    │  g_input     │                                 │
                    │  ┌──────────┐│                                 │
                    │  │*power_cmd││←────────────────────────────────┘
                    │  └──────────┘│
                    └──────────────┘

                    ┌──────────────┐
                    │   DrvKey     │ (Producer)
                    │  g_output    │
                    │  ┌──────────┐│
                    │  │*to_hmi   ││─────────────────────────────────┐
                    │  └──────────┘│                                 │
                    │  ┌──────────┐│                                 │
                    │  │*to_cook  ││─────────────────────────────────┤
                    │  └──────────┘│                                 │
                    │  ┌──────────┐│                                 │
                    │  │*to_seg   ││─────────────────────────────────┼─┐
                    │  └──────────┘│                                 │ │
                    └──────────────┘                                 │ │
                                         │                           │ │
                    ┌──────────────┐     │                           │ │
                    │   AppHmi     │←────┘                           │ │
                    │  g_input     │                                 │ │
                    │  ┌──────────┐│                                 │ │
                    │  │*key      ││←────────────────────────────────┘ │
                    │  │*power    ││←───────────────────────────────────┐ (状态反馈)
                    │  └──────────┘│                                   │
                    │  g_output    │                                   │
                    │  ┌──────────┐│                                   │
                    │  │*to_display││───────────────────────────────────┤
                    │  └──────────┘│                                   │
                    │  ┌──────────┐│                                   │
                    │  │*to_buzzer││─── @OUTPUT_CALLBACK ──────────────┼─→ DrvBuzzer
                    │  └──────────┘│   (即时路由, 不走 Switcher)        │
                    └──────────────┘                                   │
                                         │                             │
                    ┌──────────────┐     │                             │
                    │  DrvDisplay  │←────┘                             │
                    │  g_input     │                                   │
                    │  ┌──────────┐│                                   │
                    │  │*hmi      ││←──────────────────────────────────┘
                    │  └──────────┘│
                    └──────────────┘

                    ┌──────────────┐
                    │  AppSegAlign │←──────────────────────────────────┐
                    │  g_input     │                                   │
                    │  ┌──────────┐│                                   │
                    │  │*key      ││←──────────────────────────────────┘
                    │  └──────────┘│
                    └──────────────┘
```

---

## 四、迁移步骤

### Phase 1: 定义数据契约
1. 创建 `data-contract.md` — 所有模块的 PARAMS/LINK/INPUT/OUTPUT 定义
2. 创建 `_io.h` 接口文件 — 每个 Producer 一个

### Phase 2: 迁移模块（按依赖顺序）
1. **DrvKey** — 已迁移，补充 LINK 定义
2. **AppCommMgr** — 新迁移，定义 OUTPUT
3. **AppProtect** — 已迁移，补充 LINK 定义
4. **AppCooking** — 新迁移
5. **AppPower** — 已迁移，补充 INPUT 定义
6. **AppHmi** — 已迁移，补充 LINK 定义
7. **AppSegAlign** — 新迁移
8. **DrvDisplay** — 新迁移
9. **DrvCommMgr** — 新迁移
10. **DrvBuzzer** — 保留 @OUTPUT_CALLBACK 例外

### Phase 3: 更新 Switcher
1. 更新 `data_switcher.c` 槽位枚举
2. 更新 `interface_map.h` 路由配对
3. 实现 Consumer 回调（指针直穿）

### Phase 4: 验证
1. 运行 `/check` 五件套
2. 编译验证
3. 功能测试

---

## 五、例外处理

### DrvBuzzer @OUTPUT_CALLBACK
- **原因**：蜂鸣需要即时响应，不能等 Switcher 调度周期
- **处理**：在 `AppHmi_DoWork()` 中直接调用 `DrvBuzzer_OnCtrl()`
- **标记**：`/* @OUTPUT_CALLBACK */` 注释

---

## 六、文件清单

| 文件 | 说明 |
|------|------|
| `docs/migration-plan-v2.3.md` | 本规划文件 |
| `docs/data-contract.md` | 数据契约定义 |
| `core/data_switcher.c` | Switcher 调度器 |
| `core/interface_map.h` | 路由配对映射 |
| `app/app_comm_mgr_io.h` | AppCommMgr 输出接口 |
| `app/app_cooking_io.h` | AppCooking 输出接口 |
| `app/app_power_io.h` | AppPower 输出接口 |
| `drv/drv_key_io.h` | DrvKey 输出接口 |
| ... | ... |

---

## 七、风险与对策

| 风险 | 对策 |
|------|------|
| 结构体布局不一致导致指针直穿失败 | 使用 `check_structs.py` 验证 offset/type/size 一致性 |
| 迁移过程中功能回归 | 分模块迁移，每迁移一个模块后编译验证 |
| Consumer 回调参数类型不匹配 | 统一使用 `Para_Grp_t *` 参数 |

---

**创建日期**：2026-06-12
**版本**：v2.3
**状态**：规划中