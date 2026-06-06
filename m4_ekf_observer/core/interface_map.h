/* ============================================================
   interface_map.h — __weak 配对注册表

   DO NOT #include this file in C code.
   This is for check_weak_pairs.py validation only.

   Format (must match check_weak_pairs.py parser):
    * /* Pair {ID}: {description} */
    *  * 发送方: {file}.c  WEAK {return_type} {func_name}({params}) {}
    *  * 接收方: {file}.c  {return_type} {func_name}({params})

   Struct pairs (S1-S4) are verified by check_structs.py via @STRUCT.
   ============================================================ */

#ifndef INTERFACE_MAP_H
#define INTERFACE_MAP_H

/* === STRUCT PAIRS (verified by check_structs.py @STRUCT markers) === */

/* Pair S1: HRTIM 时序状态
 * owner:    ih_elec_params (base_class) — IH_HrtimState        (sizeof=8)
 * consumer: app_adc        (app)        — Adc_HrtimState_IN_t  (sizeof=8)
 * constraint: sizeof/offsetof 一致, check_structs.py 自动验证
 */

/* Pair S2: 单周期输入
 * owner:    ih_elec_params (base_class) — IH_CycleDataDef       (sizeof=16)
 * consumer: app_adc        (app)        — Adc_CycleData_IN_t    (sizeof=16)
 */

/* Pair S3: 20ms 帧输入
 * owner:    ih_elec_params (base_class) — IH_ElecInputDef       (sizeof=321)
 * consumer: app_adc        (app)        — Adc_ElecInput_IN_t    (sizeof=321)
 */

/* Pair S4: 电参数结果
 * owner:    ih_elec_params (base_class) — IH_ElecResult         (sizeof=58)
 * consumer: app_adc        (app)        — Adc_ElecResult_IN_t   (sizeof=58)
 */

/* Pair S5: ADC 功率输出 → app_power 消费
 * owner:    app_adc        (app)        — AdcPowerOut           (sizeof=16)
 * consumer: app_power      (app)        — Power_AdcOut_IN_t     (sizeof=16)
 */

/* Pair S6: [已迁移] ADC 原始值输出 → Data Switcher v2.0
 * v1.0: Power_RawInput_LINK_t (已删除)
 * v2.0: include/app_adc_io.h → Adc_Output_t.inputValue[]
 * 路由: data_switcher.c → memcpy(pPower_In->inputValue, pAdc_Out->inputValue)
 */

/* Pair S7: [已迁移] AWD DNTR 类型 → 本地定义
 * v1.0: Power_AwdDntr_LINK_t (app_power.h, 已删除)
 * v2.0: app_power.c 本地 typedef (ISR 路径使用)
 */

/* Pair S8: [已迁移] ADC 全量数据 → Data Switcher v2.0
 * v1.0: Power_AdcDef_LINK_t (app_power.h, 已删除)
 * v2.0: static const Power_Input_t *_adc (指向 g_in, Switcher 直接填入)
 * 路由: data_switcher.c → memcpy(g_in.inputValue, Adc_Output_t.inputValue) → Power_DoWork 消费
 */


/* === __weak CHANNELS === */

/* Pair A: APP_ADC → ih_elec_params (算法集调用, void* 解耦) */
 * 发送方: APP_ADC.C  WEAK void IhElecParams_Calculate(void *in, void *out) {}
 * 接收方: ih_elec_params.c  void IhElecParams_Calculate(void *in, void *out)
 * 注: APP_ADC 不 include ih_elec_params.h, 链接器自动接线
 * 注: void* 解耦类型名, 内部强制转换为 IH_ElecInputDef*/IH_ElecResult*

/* Pair C: app_power → app_adc (TXA 滤波重置) */
 * 发送方: app_power.c  WEAK void Adc_TxaAvgReset(uint8_t ch) {}
 * 接收方: APP_ADC.C  void Adc_TxaAvgReset(uint8_t ch)
 * 注: 按通道清零 TXA 滤波值

/* Pair E: app_power → app_adc (Q 滤波重置, 倍频切换时防误判) */
 * 发送方: app_power.c  WEAK void Adc_ClearCeilQAvg(uint8_t ch) {}
 * 接收方: APP_ADC.C  void Adc_ClearCeilQAvg(uint8_t ch)
 * 注: 倍频切换时清除 Q 值一阶滤波缓存

/* Pair F: app_power → app_adc (单通道功率查询) */
 * 发送方: app_power.c  WEAK uint32_t Adc_GetPowerTxa(uint8_t ch) { return 0; }
 * 接收方: APP_ADC.C  uint32_t Adc_GetPowerTxa(uint8_t ch)
 * 注: 分别查询各通道谐振电流功率, 后续可合并到 Pair O 的 AdcFunRam 全量推送

/* Pair G: app_power → app_adc (HRTIM 同步缓冲地址) */
 * 发送方: app_power.c  WEAK uint16_t* Adc_GetHrtimSyncBuffAdr(void) { return 0; }
 * 接收方: APP_ADC.C  uint16_t* Adc_GetHrtimSyncBuffAdr(void)
 * 注: 后续可合并到 Pair O 的 AdcFunRam 全量推送 → sync_addr

/* Pair H: app_power → app_adc (TXA DMA 状态) */
 * 发送方: app_power.c  WEAK uint8_t Adc_IsTxaDmaStart(void) { return 0; }
 * 接收方: APP_ADC.C  uint8_t Adc_IsTxaDmaStart(void)
 * 注: 后续可合并到 Pair O 的 AdcFunRam 全量推送 → dma_ready

/* Pair J: app_power → app_adc (T12A DMA 缓冲指针, void* 解耦) */
 * 发送方: app_power.c  WEAK uint16_t* Adc_GetCurrentAdc2Ptr(void) { return 0; }
 * 接收方: APP_ADC.C  uint16_t* Adc_GetCurrentAdc2Ptr(void)
 * 注: API_HRTIM1_TEST_CMP1_IRQHandlerCallback 调用时需要 ADC DMA 缓冲指针

/* Pair K: app_power → app_adc (T34A DMA 缓冲指针, void* 解耦) */
 * 发送方: app_power.c  WEAK uint16_t* Adc_GetCurrentAdc3Ptr(void) { return 0; }
 * 接收方: APP_ADC.C  uint16_t* Adc_GetCurrentAdc3Ptr(void)
 * 注: API_HRTIM1_TEST_CMP1_IRQHandlerCallback 调用时需要 ADC DMA 缓冲指针

/* Pair L: API_ADC → app_power (T12A AWD 过流中断回调, 从 APP_ADC 迁入) */
 * 发送方: API_adc.c  WEAK void API_ADC_Current1AWD_IRQHandlerCallBack(void) {}
 * 接收方: app_power.c  void API_ADC_Current1AWD_IRQHandlerCallBack(void)
 * 注: 原在 APP_ADC.C, 现完整迁入 app_power.c

/* Pair M: API_ADC → app_power (T34A AWD 过流中断回调, 从 APP_ADC 迁入) */
 * 发送方: API_adc.c  WEAK void API_ADC_Current2AWD_IRQHandlerCallBack(void) {}
 * 接收方: app_power.c  void API_ADC_Current2AWD_IRQHandlerCallBack(void)
 * 注: 原在 APP_ADC.C, 现完整迁入 app_power.c

/* Pair N: API_HRTIM → app_power (CMP1 过流检测回调, 从 APP_ADC 迁入) */
 * 发送方: API_hrtim.c  WEAK void API_HRTIM1_TEST_CMP1_IRQHandlerCallback(void) {}
 * 接收方: app_power.c  void API_HRTIM1_TEST_CMP1_IRQHandlerCallback(void)
 * 注: 原在 APP_ADC.C, 现完整迁入 app_power.c; 内部直接调用 APP_ADC_IRQ_PPGstepDecTxA

/* Pair O: [已删除] APP_ADC → app_power (AppAdc_OnDataReady)
 * v2.0: 数据由 Switcher 直接 memcpy 到 g_in.inputValue[], Power_DoWork 内部消费.
 *       __weak 回调不再需要.  发送方 WEAK 和接收方 STRONG 均已移除.
 */

/* Pair P: app_task → data_switcher (Slot1 调度入口) */
 * 发送方: app_task.c  WEAK void Switcher_Run_Slot1(void) {}
 * 接收方: data_switcher.c  void Switcher_Run_Slot1(void)
 * v2.0: 替代 Task_TimeChip1 中的 AdcValueFun() + PowerTypeFun()

/* Pair Q: app_task → data_switcher (Switcher 初始化) */
 * 发送方: app_task.c  WEAK void Switcher_Init(void) {}
 * 接收方: data_switcher.c  void Switcher_Init(void)
 * v2.0: AppTask_Init 中调用, 绑定各模块 output 指针

/* Pair R: [已删除] APP_ADC → app_power (Power_OnAdcData)
 * v2.0: Switcher 直接写入 g_in.inputValue[], Power_DoWork 从 g_in 消费.
 *       不需要中间回调转发.
 */

/* Pair T: APP_ADC → app_power (TxaAwd 设置, ISR 路径)
 * 发送方: APP_ADC.C  WEAK void APP_POWERR_SetTxaAwdValue(void) {}
 * 接收方: app_power.c  void APP_POWERR_SetTxaAwdValue(void)
 */

/* Pair U: APP_ADC → app_power (TxaAwd 设置 v2, ISR 路径)
 * 发送方: APP_ADC.C  WEAK void APP_POWER_SetTxaAwdValue(void) {}
 * 接收方: app_power.c  void APP_POWER_SetTxaAwdValue(void)
 */

/* Pair V: APP_ADC → app_power (PAN DMA 缓冲地址, ISR 路径)
 * 发送方: APP_ADC.C  WEAK int16_t* APP_POWER_GetPanDmaBuffAddress(void) { return 0; }
 * 接收方: app_power.c  int16_t* APP_POWER_GetPanDmaBuffAddress(void)
 */

/* Pair W: app_power → APP_ADC (PAN 通道切换, ISR 路径)
 * 发送方: app_power.c  WEAK void APP_ADC_PanSwChange(uint32_t ch) { (void)ch; }
 * 接收方: APP_ADC.C  void APP_ADC_PanSwChange(uint32_t ch)
 */

/* Pair X: app_power → APP_ADC (PAN DMA 恢复, ISR 路径)
 * 发送方: app_power.c  WEAK void APP_ADC_DMA_RecoverPan(uint8_t ch) { (void)ch; }
 * 接收方: APP_ADC.C  void APP_ADC_DMA_RecoverPan(uint8_t ch)
 */


#endif /* INTERFACE_MAP_H */
