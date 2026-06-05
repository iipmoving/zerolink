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

/* Pair S6: ADC 原始值输出 → app_power 消费 (替代 getADCinputValue)
 * owner:    app_adc        (app)        — AdcRawInput           (sizeof=52)
 * consumer: app_power      (app)        — Power_RawInput_LINK_t   (sizeof=52)
 */

/* Pair S7: AWD DNTR 类型 → app_power 消费 (ISR 回调参数)
 * owner:    app_adc        (app)        — APP_ADC_AWD_DNTR_DEF  (sizeof=24)
 * consumer: app_power      (app)        — Power_AwdDntr_LINK_t    (sizeof=24)
 */


/* Pair S8: ADC 全量数据 → app_power 消费 (Pair O PUSH 指针)
 * owner:    app_adc        (app)        — APP_ADC_DEF
 * consumer: app_power      (app)        — Power_AdcDef_LINK_t    (inputValue[30] @ offset 4)
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

/* Pair O: APP_ADC → app_power (ADC 数据就绪, PUSH 范式: void *input) */
 * 发送方: APP_ADC.C  WEAK void AppAdc_OnDataReady(void *input) { (void)input; }
 * 接收方: app_power.c  void AppAdc_OnDataReady(void *input)
 * 传: &AdcFunRam.inputValue → 存到 _adc.input_value, 调 APP_PPG_SetIcVcOk()


#endif /* INTERFACE_MAP_H */
