/**
 * hal_key.c —— 触摸按键HAL实现
 *
 * 依赖: TKDriver.h + SC_TK_Scan.h
 * 层级: HAL —— 不include任何业务模块头文件
 *
 * 触摸库时序:
 *   HAL_Key_Poll()  → TIM0 ISR 每1ms调用 → Sys_Scan() 驱动状态机
 *   HAL_Key_Scan()  → 主循环10ms槽位调用 → 检查 Bit_SYS_Read_Key → 读取 KeyFlag
 *
 * Sys_Scan() 内部: TouchKey_Service(处理1通道) → 扫描完成? → 置 KeyFlag
 * 21通道 × 1ms = 21ms 完成一轮全扫描
 */
#include "hal_key.h"
#include "../lib/SC_TK_Scan.h"
#include "../lib/MCU_Drivers/TKDriverNew/TKDriver.h"

/* KeyFlag 由触摸库在扫描完成时更新，声明在 TKDriver.h */
/* Bit_SYS_Read_Key 由 Sys_Scan() 在完成一轮扫描后置1，在 SC_TK_Scan.h */

void HAL_Key_Init(void)
{
    TK_Init();
}

/* ISR安全 —— 驱动触摸状态机一步 */
void HAL_Key_Poll(void)
{
    Sys_Scan();
}

// 主循环调用 —— 驱动状态机 + 返回完整扫描后的键值
// 返回 KEY_SCAN_NOT_READY 表示本轮扫描未完成，调用者应跳过本次结果
uint32_t HAL_Key_Scan(void)
{
    uint32_t result;
    result = (uint32_t)Sys_Scan();
    if (Bit_SYS_Read_Key != 0u) {
        return result;
    }
    return KEY_SCAN_NOT_READY;
}
