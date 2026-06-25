# modules/APP_ADC

## 职责

- 汇总并发布 IH 控制相关 ADC 数据（电压/谐振电流/功率/Q/相位/温度等），以 20ms 为统计周期产出一帧“处理后的输入向量”
- 提供 1ms 时间片入口 `Adc_DoWork()`，对外以 Data Switcher 输出槽 `g_out` 发布 `inputValue[]`
- 承接若干中断/回调：100us 周期采样、过零点切换、ADC EOC、DMA 采样搬运、HRTIM UPD 转发等

## 入口路径（穷举）

- 对外入口头文件： [APP_ADC.H](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/inc/APP_ADC.H)
- 主要实现文件（库目录）：[APP_ADC.C](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/LIB/APP/APP_ADC.C)
- 详细流程分解（主入口→中断→独立函数）：[APP_ADC_APP_ADC.C.md](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/modules/APP_ADC_APP_ADC.C.md)

## 关键入口与函数

- 主入口（1ms）：`Adc_DoWork()`：[APP_ADC.C:L2956-L2963](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/LIB/APP/APP_ADC.C#L2956-L2963)
- 输出获取：`Adc_GetIO()`：[APP_ADC.C:L2951-L2954](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/LIB/APP/APP_ADC.C#L2951-L2954)
- 20ms 结算：`APP_ADC_AVG_Fun()`：[APP_ADC.C:L576-L640](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/LIB/APP/APP_ADC.C#L576-L640)
- 发布判定：`AdcValueFun()`：[APP_ADC.C:L685-L759](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/LIB/APP/APP_ADC.C#L685-L759)

