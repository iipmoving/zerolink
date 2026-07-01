/**
 * app_seg_align.h —— 通用数码管段位对齐模块
 *
 * 依赖: <stdint.h> (无其他模块依赖 — 全部通过 __weak 对接)
 * 层级: APP —— 对齐/校准工具
 *
 * 设计原则:
 *   1. 零业务依赖 — 可复制到任何数码管项目
 *   2. 不链接不占 Flash — 所有对外通道 __weak
 *   3. 可拦截正常显示 — AppSegAlign_IsActive() 返回 1 时 HMI 刷新被抑制
 *   4. 支持双入口 — 组合键长按 / 串口指令 ##ALIGN
 *
 * 适配新项目: 修改下方 "项目可配置项" 区域的 #define 即可
 */

#ifndef APP_SEG_ALIGN_H
//#define APP_SEG_ALIGN_H

#include <stdint.h>

/* ================================================================
 * 项目可配置项 — 适配新项目时修改此区域
 * ================================================================ */

/* 数码管硬件参数 */
#define SEG_ALIGN_DIGIT_COUNT   8u    /* SMG 位数 (IO[0..7])            */
#define SEG_ALIGN_COM_COUNT     11u   /* 总 COM 数 (含 LED: IO[8..10])  */
#define SEG_ALIGN_LED_COUNT     3u    /* LED COM 数                      */

/*
 * 按键配置 (数值必须与项目 drv_key.h 的 KeyCode_t 枚举一致)
 *
 *   四头电磁炉默认值:
 *     KEY_BIND = 6   (双圈绑定键)
 *     KEY_ADD  = 3   (加键)
 *     KEY_SUB  = 2   (减键)
 *
 *   适配新项目时修改以下数值为对应项目的键码
 */
#define SEG_ALIGN_KEY_ENTER     6u    /* 进入/退出对齐模式 (长按3秒)    */
#define SEG_ALIGN_KEY_EXIT      6u    /* 同上                          */
#define SEG_ALIGN_KEY_CONFIRM   3u    /* 确认: 当前步骤正确             */
#define SEG_ALIGN_KEY_REJECT    2u    /* 报错: 当前步骤不正确           */

/*
 * 按键状态 (数值必须与项目 drv_key.h 的 KEY_STATE_* 一致)
 */
#define SEG_ALIGN_KEY_LONG      0x02u
#define SEG_ALIGN_KEY_RELEASE   0x04u
#define SEG_ALIGN_KEY_REPEAT    0x08u
#define SEG_ALIGN_KEY_TAP       0x10u

/* 长按判定: LONG(1次) + REPEAT(N次) ≈ 3秒 */
#define SEG_ALIGN_HOLD_REPEAT   3u

/* 串口指令: 设为 0 禁用串口入口 */
#define SEG_ALIGN_SERIAL_ENABLE  1u

/* ================================================================
 * 段位映射表 (12 字节, 可存入 Flash)
 * ================================================================ */
typedef struct {
    uint8_t seg_order[8];         /* seg_order[logical]=physical_bit    */
    uint8_t com_order[11];        /* com_order[logical]=physical_com    */
    uint8_t common_type;          /* 0=共阴, 1=共阳                    */
    uint8_t crc8;                 /* 简易校验: 前12字节异或            */
} SegAlignMap_t;

/* ================================================================
 * 公共接口
 * ================================================================ */
uint8_t AppSegAlign_IsActive(void);
void    AppSegAlign_Init(void);
void    AppSegAlign_Run(void);         /* 每10ms调用(串口轮询+长按计时)  */

/* __weak 桩: DRV 层调用, 对齐模块提供强符号 */
void AppSegAlign_OnKey(uint16_t param, void *data_ptr);

/* __weak 桩: 对齐模块调用, DRV 层提供强符号 — 单COM独立写段码 */
void DrvSegAlign_WriteCom(uint8_t com, uint8_t seg_mask);

/* __weak 桩: 对齐模块调用, DRV 层提供强符号 — 阻塞/恢复 HMI 刷新 */
void DrvSegAlign_BlockHmi(uint8_t block);

/* 进入门禁: 默认返回1(允许), HMI 可覆盖强符号限制仅 POWERED_OFF 进入 */
uint8_t AppSegAlign_CanEnter(void);

/* __weak 桩: 对齐模块调用, 存储层提供强符号 */
void SegAlign_OnSave(const SegAlignMap_t *map);

/* __weak 桩: 对齐模块调用, 存储层提供强符号 (返回1=加载成功) */
uint8_t SegAlign_OnLoad(SegAlignMap_t *map);

#endif /* APP_SEG_ALIGN_H */
