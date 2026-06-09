# APP_ADC ↔ app_power 解耦决策记录

> 每对函数一种解法: __weak 桩置换 / Msg_Post / 移至正确层 / 删除

---

## 方向A: APP_ADC → app_power (2个)

| # | 函数 | 判定 | 方案 |
|---|------|------|------|
| F1 | `APP_POWER_SetTxaAwdValue()` | 应在 API_ADC 实例层, 不到 app_power | APP_ADC 声明 `__weak void Adc_SetTxaAwdValue(...)` → API_ADC 提供 STRONG |
| F2 | `APP_POWER_GetPanDmaBuffAddress()` | 数据查询 | 走 Msg_Post 消息 |

---

## 方向B: app_power → APP_ADC (10个)

| # | 函数 | 次数 | 判定 | 方案 |
|---|------|------|------|------|
| F3 | `APP_ADC_GetPowerTxa()` | 6处 | 统一用消息 | Msg_Post 替代 |
| F4 | `APP_ADC_TxaAvgReset()` | 2处 | 无参清零回调 | `__weak void Adc_TxaAvgReset(void) {}` → APP_ADC STRONG |
| F5 | `APP_ADC_WaitTxaCalOver()` | 2处 | **没用** | 删除 |
| F6 | `APP_ADC_ClearCeilQAvg()` | 2处 | **没用** | 删除 |
| F7 | `APP_ADC_GetHrtimSyncBuffAdr()` | 2处 | 数据查询 | 走 Msg_Post |
| F8 | `APP_ADC_IsTxaDmaStart()` | 1处 | 状态查询 | 回调或消息, 待看调用上下文 |
| F9 | `APP_ADC_PanSwChange()` | 3处 | 可移到 POWER | 函数体迁入 app_power.c, APP_ADC 不再持有 |
| F10 | `APP_ADC_DMA_RecoverPan()` | 1处 | 可移到 POWER | 函数体迁入 app_power.c |
| F11 | `APP_ADC_getOverAdcChannel()` | 2处 | 实际在 POWER, 没调 | 死代码, 删除 |
| F12 | `APP_ADC_Power_ZeroIrqFun()` | 1处 | **没用** | 删除 |

---

## 方向C: app_power 提供 STRONG (__weak 回调, 5个)

| # | 函数 | 判定 | 方案 |
|---|------|------|------|
| F13 | `APP_ADC_DebugValueCallBack` | 待定 | |
| F14 | `APP_ADC_IRQ_PPGstepDecT12aCallBack` | 待定 | |
| F15 | `APP_ADC_IRQ_PPGstepChangeCallBack` | 待定 | |
| F16 | `APP_ADC_IRQ_PPGstepDecTxA` | 待定 | |
| F17 | `APP_ADC_IRQ_PPGstepDecTxACallBack` | 待定 | |
