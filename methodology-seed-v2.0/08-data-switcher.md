# 08 — 数据交换机：周期性结构化数据路由

---

## 一、解决什么问题

`__weak` 回调在模块间传结构化数据时，consumer 模块需要在 `.h` 里声明 owner struct 的副本（`_LINK_t`），AI 负责保持两者布局一致。consumer 越多，维护负担越重，出错概率越大。

数据交换机把这个责任从"AI 编译前维护"移到"中间层运行时搬运"——模块不再需要 consumer struct 副本，数据路由集中在唯一的 Switcher 里完成。

**不是替代 `__weak`。** ISR 回调、同槽同步计算仍然走 `__weak`。交换机接管的是周期性的、模块间的结构化数据路由。

---

## 二、三机制分工

```
__weak 回调    → ISR 驱动、同槽同步计算        （链接器接线，零开销）
Msg_Post      → 跨时间片异步事件通知           （环形队列缓冲）
数据交换机     → 周期性模块间结构化数据路由     （中间层指针搬运，按调度槽执行）
```

选择规则扩展为两层：

```
1. 必须在同一次调度槽内完成？
   → 是: __weak 直调
   → 否: 进入下一问

2. 传递的是事件通知还是结构化数据块？
   → 事件通知: Msg_Post
   → 结构化数据块: 数据交换机
```

---

## 三、文件组织

所有模块的公开 IO 接口放在 `include/` 文件夹，**不加入编译器 `-I` path**。

| 文件 | 内容 | include guard | 谁引用 |
|------|------|--------------|--------|
| `include/module_a_io.h` | Input_t, Output_t, GetIO(), DoWork() 声明 | `#define` **保留** | 本模块 .c + Switcher.c（全路径） |
| `module_a.h`（可选） | 内部配置常量、私有枚举 | `//#define` **注释** | 仅 `module_a.c` 自己 |

**`_io.h` 和模块 `.h` 是两种文件，include guard 规则相反。** 详见 §4.3。

`check_include.py` 扫描全路径 include —— **除 Switcher.c 外任何文件用全路径 include 其他模块的 `_io.h` 即阻断提交。** 模块引用自己的 `_io.h` 也用全路径，工具按文件名区分自己/他人的 `_io.h`。

模块无内部配置项时，不需要 `module_a.h`。只有 `_io.h` 作为对外接口。

---

## 四、模块标准结构

### 4.1 组成部分

每个参与交换机的模块对外暴露两个指针 + 一个函数：

| 组成部分 | 职责 | 形式 |
|----------|------|------|
| 输入槽 | 本模块需要的外部数据，头部嵌 status 字节 | `static ModuleA_Input_t g_in` |
| 输出槽 | 本模块产出的数据，头部嵌 status 字节 | `static ModuleA_Output_t g_out` |
| 主逻辑 | 每周期业务处理 | `ModuleA_DoWork()`，在 `_io.h` 声明 |

构造函数不暴露——模块首次 DoWork 时自检 `g_in.status & 0x01`，懒惰初始化。

### 4.2 状态字协议

IN 和 OUT 各自独立的 status 字节，嵌在 struct 头部。32 字节对齐，status + res[3] 占 4 字节：

```
g_in.status   bit0=输入槽已构造  bit1=Switcher 设（新输入到达）→ 模块消费后自清
g_out.status  bit0=保留          bit1=模块设（新输出就绪）→ 模块进入时自清 + Switcher 搬运后清
```

**关键**: 模块 DoWork 进入时**先清自己的 g_out bit1**（每帧都清）。有产出才在末尾置位。100ms 输出一次的模块：9 帧清完 return，1 帧置位。

一个完整的生产和消费周期贯穿三方：

```
1. Producer DoWork 进入 → g_out.status &= ~0x02           (每帧先清)
2. Producer DoWork 结束 → g_out.status |= 0x02            (本帧有产出才置)
3. 下一帧 Switcher 检测 producer g_out.status bit1=1 → 读取输出槽 → 搬运到 consumer 输入槽
4. Switcher 清 producer g_out.status bit1                  (数据已被取走)
5. Switcher 设 consumer g_in.status bit1=1                 (新输入到达)
6. Consumer DoWork 进入 → g_out.status &= ~0x02            (每帧先清)
7. Consumer DoWork 检查 g_in.status bit1=1 → 消费 → g_in.status &= ~0x02 (消费完毕)
```

bit1 在两个角色手里翻转：生产者设 1，Switcher 清 0；Switcher 设 1，消费者清 0。

### 4.3 GetIO() 接口

**`_io.h` 是公开接口文件，保留 `#define` include guard。** 它与模块自身 `.h` 的规则相反：

| 文件类型 | include guard | 原因 |
|----------|--------------|------|
| **`_io.h`** (公开 IO 接口) | `#define` **保留** | 公开接口，恰好两个合法 include 方（本模块 + Switcher），需要正常的防重入保护 |
| 模块自身 `.h` (内部配置) | `//#define` **注释** | 私有头文件，禁止他人 include，注释 guard = L0 编译阻断 |

**为什么容易搞反**：铁律七说"DRV/APP/PROTO 的每个 .h 必须注释 #define"——但这个规则的对象是**模块自身 .h**，不是 `_io.h`。`_io.h` 是故意暴露的公共接口，属于不同类别。

```c
// ===== include/module_a_io.h =====
#ifndef MODULE_A_IO_H
#define MODULE_A_IO_H       /* ← 保留: _io.h 是公开接口, 两个合法 include 方 */

#include <stdint.h>

#pragma pack(4)

typedef struct {
    uint8_t  status;        // bit0=构造, bit1=新数据到达 (Switcher 设, 本模块消费后清)
    uint8_t  res[3];        // 32位对齐
    uint16_t power;         // 外部输入的功率值
    uint8_t  mode;          // 外部输入的模式
} ModuleA_Input_t;           // sizeof 须为 4 的倍数

typedef struct {
    uint8_t  status;        // bit0=保留, bit1=输出就绪 (本模块设, Switcher 取走后清)
    uint8_t  res[3];
    uint16_t result;
    uint8_t  flag;
} ModuleA_Output_t;

#pragma pack()

void ModuleA_GetIO(ModuleA_Input_t **ppIn, ModuleA_Output_t **ppOut);
void ModuleA_DoWork(void);

#endif /* MODULE_A_IO_H */
```

### 4.4 模块内部实现

```c
// ===== module_a.c =====
#include "module_a.h"                          // 可选 — 内部配置
#include "../include/module_a_io.h"            // 自己的 IO 接口

static ModuleA_Input_t  g_in;
static ModuleA_Output_t g_out;

static void _Constructor(void)
{
    memset(&g_in,  0, sizeof(g_in));
    memset(&g_out, 0, sizeof(g_out));
    g_out.mode = MODE_DEFAULT;
}

void ModuleA_GetIO(ModuleA_Input_t **ppIn, ModuleA_Output_t **ppOut)
{
    *ppIn  = &g_in;
    *ppOut = &g_out;
}

void ModuleA_DoWork(void)
{
    if (!(g_in.status & 0x01)) {               // 首次进入
        _Constructor();
        g_in.status |= 0x01;
    }

    g_out.status &= ~0x02;                      // ★ 每帧先清输出标志

    if (!(g_in.status & 0x02)) return;          // 无新输入

    // ... 消费 g_in → 计算 → 更新 g_out ...

    g_in.status &= ~0x02;                       // 消费完毕
    g_out.status |= 0x02;                       // 本帧有产出，置位
}
```

---

## 五、中间层（Switcher）

### 5.1 定位

**全项目唯一有权 include 所有 `_io.h` 的文件。** 模块之间互不知道对方存在。

### 5.2 初始化：Switcher_Init()

```c
// ===== data_switcher.c =====
#include "../include/module_a_io.h"
#include "../include/module_b_io.h"
// ... 所有模块的 _io.h

static ModuleA_Input_t  *pA_In;
static ModuleA_Output_t *pA_Out;
static ModuleB_Input_t  *pB_In;
static ModuleB_Output_t *pB_Out;
// ...

void Switcher_Init(void)
{
    ModuleA_GetIO(&pA_In, &pA_Out);
    ModuleB_GetIO(&pB_In, &pB_Out);
    // ...

    /* 只清输入槽状态字节 → 模块首次 DoWork 自检 bit0=0 → 调 _Constructor() */
    pA_In->status = 0;
    pB_In->status = 0;
    // ...
}
```

### 5.3 运行时：Switcher_Run()

每 10ms 调度槽内调用一次。遍历所有模块：搬运数据 → 调 DoWork。

```c
void Switcher_Run(void)
{
    // ===== 数据路由：从 Producer 输出槽 → Consumer 输入槽 =====

    if (pA_Out->status & 0x02) {               // A 有新输出
        pB_In->field_x = pA_Out->field_y;       // 按接线规则搬运
        pB_In->field_z = pA_Out->field_w;
        pA_Out->status &= ~0x02;                // 清 A 输出标志
        pB_In->status  |= 0x02;                 // 通知 B
    }

    if (pB_Out->status & 0x02) {               // B 有新输出
        pA_In->field   = pB_Out->field;
        pB_Out->status &= ~0x02;
        pA_In->status  |= 0x02;
    }

    // ===== 执行模块主逻辑 =====
    ModuleA_DoWork();
    ModuleB_DoWork();
    // ...
}
```

**时序保证**：模块 DoWork 之间无顺序依赖假设。消费者读到的是上一帧生产者输出（自然延迟一帧）。10ms 轮转确保每轮数据都是新的。

### 5.4 接线方式

硬编码在 Switcher 里。Switcher 可以包含转换逻辑、字段映射、多源聚合。传指针还是拷贝数据——按具体场景决定。

后续可升级为 JSON 接线表 + `generate_switcher.py` 生成 `Switcher_Run()`。

---

## 六、结构体使用约束

- **枚举不出模块**：跨模块数据用纯 struct 字段描述，不用枚举值。模块内部遍历用联合体或指针偏移
- **常量不跨模块**：没有公共 `constants.h`。跨模块常量由定义方放入 Output_t，Switcher 填入消费方 Input_t
- **字段变更影响面**：owner 的 Output_t 字段变更 → 只影响 Switcher 接线段。不需要改所有 consumer

---

## 七、与 __weak 的共存

一个模块可以同时有两条入口：

```
模块 X:
  ├── ModuleX_DoWork()  ← Switcher 每周期调用（消费 g_in → 计算 → 更新 g_out）
  └── ModuleX_OnISR()   ← __weak 强符号，ISR 独立入口
```

两条路径互不冲突。ISR 不走 Switcher。

---

## 八、include 权限规则

| 文件 | 权限 | 审计 |
|------|------|------|
| **Switcher.c** | 全路径 `#include "../include/xxx_io.h"` | 允许 |
| **模块 .c** | `#include "../include/xxx_io.h"` | 禁止 — `check_include.py` 阻断 |
| **模块 .c** | `#include "module_a.h"` | 允许 — 只能引自己的头文件 |
| **模块 .c** | `#include "../include/module_a_io.h"` | 允许 — 只能引自己的 `_io.h` |

`check_include.py` 扫描逻辑：
- 非 `Switcher.c` 中检测到全路径 `#include "../include/"` → 阻断
- 模块 .c 引用了非自己的 `_io.h` → 阻断

---

## 九、命名约定

```
文件:       include/module_a_io.h   module_a.h   module_a.c
类型:       ModuleA_Input_t         ModuleA_Output_t
函数:       ModuleA_GetIO()         ModuleA_DoWork()
变量:       g_in                    g_out
结构体成员:  snake_case 单词，不加模块前缀 (power, mode, voltage，不是 a_power)
```

`{Module}{Name}_{type}` — 模块前缀区分命名空间，结构体成员不加前缀。

---

## 十、优势

- **消除 `_LINK` 副本**：模块不需要声明 consumer struct
- **AI 维护量下降**：owner 字段变更 → 只改 Switcher 接线，不改 N 个 consumer
- **数据流集中可见**：所有跨模块路由在 `Switcher_Run()` 一目了然
- **头文件物理隔离**：`_io.h` 统一放 `include/`，全路径可被工具审计
- **初始化一致**：Switcher 上电统一清零，不依赖模块记住

---

## 十一、不适用场景

- ISR 回调（延迟敏感 → 仍走 `__weak`）
- 同槽同步计算（调用即执行 → 仍走 `__weak`）
- 跨进程/跨核通信（需消息持久化 → `Msg_Post`）

---

## 十二、与 interface_map.h 的关系

`interface_map.h` 仍然管 __weak 函数配对。数据交换机引入后，新增一块"接线表"——可硬编码在 Switcher，也可抽成 `wiring.json`（后续工具化）。

```
interface_map.h  →  __weak 函数配对 + 签名验证        → check_weak_pairs.py
Switcher 接线段  →  结构化数据路由（Output_t → Input_t） → 暂无工具（后续 generate_switcher.py）
```

---

*方法论版本: v2.0, 2026-06-06*
