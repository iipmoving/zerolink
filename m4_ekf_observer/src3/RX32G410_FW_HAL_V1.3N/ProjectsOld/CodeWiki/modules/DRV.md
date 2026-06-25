# modules/DRV

## 职责

该目录更偏“板级/底层驱动与辅助处理”，与 `Projects/API` 的区别通常是：

- API：对 HAL 的统一抽象接口，面向整个工程对外输出
- DRV：更贴近具体板卡、具体数据处理（例如 ADC 处理汇编/定时器细节、I2C 细节等）

## 入口路径（穷举）

- 头文件（`Projects/DRV/INC`）
  - [DRV_GPIO.H](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/DRV/INC/DRV_GPIO.H)
  - [DRV_I2C.H](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/DRV/INC/DRV_I2C.H)
  - [DRV_I2C_DEFINE.H](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/DRV/INC/DRV_I2C_DEFINE.H)
  - [drv_tim.h](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/DRV/INC/drv_tim.h)
  - [adc_processing.h](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/DRV/INC/adc_processing.h)
  - [commClass.H](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/DRV/INC/commClass.H)
- 源文件（`Projects/DRV/SRC`）
  - [DRV_GPIO.C](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/DRV/SRC/DRV_GPIO.C)
  - [DRV_I2C.C](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/DRV/SRC/DRV_I2C.C)
  - [DRV_TIM.C](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/DRV/SRC/DRV_TIM.C)
  - [drv_timDefine.c](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/DRV/SRC/drv_timDefine.c)
  - [adc_processing.s](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/DRV/SRC/adc_processing.s)
  - [adc_processing1.c](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/DRV/SRC/adc_processing1.c)

## 与其他模块的依赖关系

- 被 `Projects/API`、`Projects/APP` 间接依赖（例如某些 API/HRTIM/ADC 处理会引用 DRV 层工具）
- 与 `Projects/BaseClass` 的 `commClass.H` 存在交叉引用（该仓库中 commClass 同名头文件在多个目录出现，定位时建议优先看 include 路径/工程配置）

