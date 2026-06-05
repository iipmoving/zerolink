/**
 * @file    interface-patterns.h
 * @brief   零耦合接口模式 — 实例参考 (不参与编译, 仅供 AI/人 阅读)
 *
 * 本文档用实际代码展示四种核心接口模式, 以及常见的坑.
 * 每个模式标注了 AI 自动维护的区域 (AI-MANAGED).
 *
 * 模式:
 *   Pattern 1: Owner .h — 生产者声明 @STRUCT
 *   Pattern 2: Consumer .h — 消费者副本 (AI 自动生成)
 *   Pattern 3: Consumer .c — __weak 桩 (AI 自动生成)
 *   Pattern 4: interface_map.h — 配对注册
 *
 * 陷阱:
 *   Pitfall A: comment 格式错误 → 解析器不识别
 *   Pitfall B: 文件名大小写 (.C vs .c)
 *   Pitfall C: owner struct 变更后 consumer 未同步
 */

#ifndef INTERFACE_PATTERNS_H
#define INTERFACE_PATTERNS_H

/* ================================================================
 *  PATTERN 1: Owner .h — 生产者声明 @STRUCT
 *
 *  本模块是结构体的"拥有者", 定义原始结构体.
 *  INTERFACE STRUCTS 段由 check_structs.py 解析.
 *  消费者由 AI 读取此段后自动生成副本.
 *
 *  规则:
 *    - 每个字段注释 /* offset=N, size=N */
 *    - 结构体结束注释 /* sizeof=N */
 *    - @STRUCT 标记: name, owner, suffix
 *    - 嵌套结构体也需声明 @STRUCT
 * ================================================================ */

/* 示例: ih_elec_params.h 的 INTERFACE STRUCTS 段 */

#if 0  /* 仅作参考, 不实际编译 */

/* === INTERFACE STRUCTS (本模块是以下结构体的 Owner) ===
 * 消费者需独立声明副本, AI 通过 /new-module Step 4 自动生成.
 * @STRUCT 标记由 check_structs.py 解析验证.
 */

/* @STRUCT IH_HrtimState    owner=ih_elec_params  suffix=OUT */
typedef struct {
    uint16_t highOn;                   /* offset=0, size=2 */
    uint16_t highOff;                  /* offset=2, size=2 */
    uint16_t lowOn;                    /* offset=4, size=2 */
    uint16_t lowOff;                   /* offset=6, size=2 */
} IH_HrtimState;                       /* sizeof=8 */

/* @STRUCT IH_CycleDataDef  owner=ih_elec_params  suffix=OUT */
typedef struct {
    IH_HrtimState hrtim;               /* offset=0,  size=8 — nested */
    uint16_t peak_current;             /* offset=8,  size=2 */
    uint16_t active_current;           /* offset=10, size=2 */
    uint16_t voltage;                  /* offset=12, size=2 */
    uint16_t zero_cross_high;          /* offset=14, size=2 */
} IH_CycleDataDef;                     /* sizeof=16 */

/* === END INTERFACE STRUCTS === */

/* public API — void* 解耦, 调用方不 include 本 .h */
void IhElecParams_Calculate(void *in, void *out);

#endif


/* ================================================================
 *  PATTERN 2: Consumer .h — 消费者副本 (AI 自动生成)
 *
 *  当模块需要消费 owner 的结构体时, AI 读取 owner .h 的
 *  INTERFACE STRUCTS 段, 自动生成消费者副本.
 *
 *  消费者结构体:
 *    - 新类型名: {ConsumerModule}_{StructKey}_IN_t
 *    - source= 指向 owner 结构体 (check_structs.py 链式验证)
 *    - 字段 offset/size 与 owner 完全一致
 *    - 这段代码放在 AI-MANAGED 标记内, 人不手改
 * ================================================================ */

#if 0  /* 仅作参考 */

/* === AI-MANAGED: INTERFACE STRUCTS (消费者副本) ================
 *
 *   由 /new-module Step 4 自动生成, 不手动编辑.
 *   source= 指向 owner 结构体, check_structs.py 验证一致性.
 *
 *   生成时间: 2026-06-05
 *   源: base_class/ih_elec_params/ih_elec_params.h
 * ================================================================ */

/* @STRUCT Adc_HrtimState_IN_t  owner=app_adc  suffix=IN
 * source=IH_HrtimState */
typedef struct {
    uint16_t highOn;                   /* offset=0, size=2 */
    uint16_t highOff;                  /* offset=2, size=2 */
    uint16_t lowOn;                    /* offset=4, size=2 */
    uint16_t lowOff;                   /* offset=6, size=2 */
} Adc_HrtimState_IN_t;                 /* sizeof=8, source=IH_HrtimState */

/* @STRUCT Adc_CycleData_IN_t   owner=app_adc  suffix=IN
 * source=IH_CycleDataDef */
typedef struct {
    Adc_HrtimState_IN_t hrtim;         /* offset=0,  size=8 — nested, source=IH_HrtimState */
    uint16_t peak_current;             /* offset=8,  size=2 */
    uint16_t active_current;           /* offset=10, size=2 */
    uint16_t voltage;                  /* offset=12, size=2 */
    uint16_t zero_cross_high;          /* offset=14, size=2 */
} Adc_CycleData_IN_t;                  /* sizeof=16, source=IH_CycleDataDef */

/* @STRUCT Adc_ElecInput_IN_t   owner=app_adc  suffix=IN
 * source=IH_ElecInputDef */
typedef struct {
    Adc_CycleData_IN_t cycle[20];      /* offset=0,  size=320 — nested array */
    uint8_t count;                     /* offset=320, size=1 */
} Adc_ElecInput_IN_t;                  /* sizeof=321, source=IH_ElecInputDef */

/* === END AI-MANAGED === */

#endif


/* ================================================================
 *  PATTERN 3: Consumer .c — __weak 桩 + AI-MANAGED 区
 *
 *  .c 文件中有两个 AI-MANAGED 区:
 *   a) __weak stubs  — 声明本模块需要的外部函数
 *   b) test functions — 临时测试入口 (可选)
 *
 *  规则:
 *    - __weak 桩放文件顶部, 不隐藏
 *    - 调用方不 include 提供方 .h
 *    - void* 解耦类型名
 * ================================================================ */

#if 0  /* 仅作参考 — 消费者 .c 文件结构 */

/* === AI-MANAGED: __weak stubs ===================================
 *
 *   由 /new-module Step 5 自动生成, 与 interface_map.h 配对.
 *   不 include 提供方 .h — 链接器根据 STRONG 符号自动接线.
 * ================================================================ */

__weak void IhElecParams_Calculate(void *in, void *out) {}

/* === END AI-MANAGED === */


/* ================================================================
 *  test: 电参数计算 (4炉头循环调用)
 *
 *  本函数为临时测试入口 — 调用 IhElecParams_Calculate,
 *  通过 __weak 链接到 base_class/ih_elec_params.c 的 STRONG 符号.
 * ================================================================ */
void APP_ADC_ComputeElecParams(void)
{
    static uint8_t elec_buf[4][64];  /* 64B/head, IH_ElecResult=58B */
    for (uint8_t i = 0; i < PotNum; i++) {
        IhElecParams_Calculate(&powerResult20ms[i], elec_buf[i]);
    }
}

#endif


/* ================================================================
 *  PATTERN 4: interface_map.h — 配对注册
 *
 *  格式必须精确匹配 check_weak_pairs.py 解析器的期望:
 *    - Pair 行: /* Pair {ID}: {description} */  (单行, */ 必须在同行)
 *    - 发送方行:  * 发送方: {file}.c  WEAK {ret} {func}({params}) {}
 *    - 接收方行:  * 接收方: {file}.c  {ret} {func}({params})
 *
 *  check_structs.py 验证 struct pairs (S1-S4).
 * ================================================================ */

#if 0  /* 仅作参考 */

/* === __weak CHANNELS === */

/* Pair A: APP_ADC → ih_elec_params (算法集调用, void* 解耦) */
 * 发送方: APP_ADC.C  WEAK void IhElecParams_Calculate(void *in, void *out) {}
 * 接收方: ih_elec_params.c  void IhElecParams_Calculate(void *in, void *out)
 * 注: APP_ADC 不 include ih_elec_params.h, 链接器自动接线

#endif


/* ================================================================
 *  PITFALL A: comment 格式错误 → 解析器不识别
 *
 *  错误1: Pair 行不闭合 */ (解析器要求同行结束)
 *     /* Pair A: description        ← 缺少 */
 *      * 发送方: ...
 *      */                           ← 块注释闭合不算
 *
 *  正确:
 *     /* Pair A: description */
 *      * 发送方: ...
 *
 *  错误2: 空行打断解析 (空行 strip 后不以 * 开头 → break)
 *      * 发送方: file.c  WEAK ...
 *                              ← 空行 = 解析停止!
 *      * 接收方: file.c  ...
 *
 *  正确: 发送方和接收方之间不能有空行.
 * ================================================================ */


/* ================================================================
 *  PITFALL B: 文件名大小写 (.C vs .c)
 *
 *  check_weak_pairs.py 的 SENDER_RE/RECEIVER_RE 使用 (\S+\.[cC])
 *  匹配 .c 和 .C. 如果工具版本过旧 (仅匹配 \.c), 则 APP_ADC.C
 *  会被漏掉.
 *
 *  修复: 确认 check_weak_pairs.py 使用 (\S+\.[cC]) 而非 (\S+\.c).
 * ================================================================ */


/* ================================================================
 *  PITFALL C: owner struct 变更后 consumer 未同步
 *
 *  当 owner 修改 @STRUCT (增删字段/改类型), 所有 consumer 副本
 *  必须同步更新. 如果 consumer 是 AI 自动生成的, 重新运行生成流程.
 *
 *  验证: check_structs.py 自动检测 offset/type/sizeof 不一致.
 *  嵌套结构体通过 source= 链验证.
 * ================================================================ */


/* ================================================================
 *  快速参考: 哪个工具检查什么
 * ================================================================
 *
 *  | 检查项              | 工具                  | 检查内容              |
 *  |---------------------|-----------------------|-----------------------|
 *  | 层依赖规则          | check_deps.py         | #include 不跨层        |
 *  | __weak 配对         | check_weak_pairs.py   | weak→strong 成对存在  |
 *  | struct owner/consumer| check_structs.py     | @STRUCT 布局一致性    |
 *  | 编译                | armcc / gcc           | 0 error, 0 warning    |
 *
 *  全部通过才能提交. pre-commit hook 自动执行前 3 项.
 * ================================================================ */


#endif /* INTERFACE_PATTERNS_H */
