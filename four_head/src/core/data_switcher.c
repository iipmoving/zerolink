/**
 * @file    data_switcher.c
 * @brief   Data Switcher — 统一调度
 * @layer   core
 *
 * 所有模块通过 GetIO 注册函数指针，Switcher 只调 pDoWork。
 * 数据通过 __weak 回调自动路由，Switcher 不碰 struct 字段。
 */
#include "core/std_module.h"
#include "data_switcher.h"

/* 模块 GetIO 声明（由 MODULE_EXPORT 生成，在各自 .c 中定义）*/
void AppPower_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));

#define MAX_MODULES  16

typedef struct {
    void (*pDoWork)(void);
} ModuleSlot_t;

static ModuleSlot_t s_slots[MAX_MODULES];
static uint8_t      s_count = 0;

void Switcher_Register(void (*pDoWork)(void))
{
    if (s_count < MAX_MODULES)
        s_slots[s_count++].pDoWork = pDoWork;
}

void Switcher_Init(void)
{
    Para_Grp_t *pIn, *pOut;
    void       (*pWork)(void);

    /* 已迁移到 std_module.h 的模块 */
    AppPower_GetIO(&pIn, &pOut, &pWork);
    /* INFO 是模块私有的，Switcher 不碰 */
    Switcher_Register(pWork);
}

void Switcher_Run(void)
{
    for (uint8_t i = 0; i < s_count; i++)
        if (s_slots[i].pDoWork)
            s_slots[i].pDoWork();
}
