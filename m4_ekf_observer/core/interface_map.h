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


/* === __weak CHANNELS === */

/* Pair A: APP_ADC → ih_elec_params (算法集调用, void* 解耦) */
 * 发送方: APP_ADC.C  WEAK void IhElecParams_Calculate(void *in, void *out) {}
 * 接收方: ih_elec_params.c  void IhElecParams_Calculate(void *in, void *out)
 * 注: APP_ADC 不 include ih_elec_params.h, 链接器自动接线
 * 注: void* 解耦类型名, 内部强制转换为 IH_ElecInputDef*/IH_ElecResult*


#endif /* INTERFACE_MAP_H */
