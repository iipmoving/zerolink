# modules/INTERFACE

## 职责

- 定义对外通信接口层（当前主要是 I2C 从机协议）
- 提供初始化入口与回调注册接口，供 APP 层绑定“控制数据处理/初始化数据处理/上送数据生成”等逻辑

## 入口路径（穷举）

- 头文件
  - [Proto_I2C.H](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/INTERFACE/inc/Proto_I2C.H)
- 源文件
  - [Proto_SlaveI2C.C](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/INTERFACE/src/Proto_SlaveI2C.C)

## 关键接口（Proto_I2C.H）

- `I2cSlaveInit()`：协议层初始化入口 [Proto_I2C.H:L95-L96](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/INTERFACE/inc/Proto_I2C.H#L93-L99)
- `iic_bus_updata()`：I2C 总线轮询/更新（在 `Task_Tk()` 中被周期调用）[Proto_I2C.H:L93-L96](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/INTERFACE/inc/Proto_I2C.H#L93-L96)
- 回调注册：
  - `SetI2cRxControlCallBackFun(...)`：控制数据回调 [Proto_I2C.H:L97-L99](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/INTERFACE/inc/Proto_I2C.H#L97-L99)
  - `SetI2cRxInitCallBackFun(...)`：初始化数据回调 [Proto_I2C.H:L97-L99](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/INTERFACE/inc/Proto_I2C.H#L97-L99)
  - `SetI2cSetTxValueCallBackFun(...)`：生成上送数据 [Proto_I2C.H:L99-L99](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/INTERFACE/inc/Proto_I2C.H#L99-L99)

## 与其他模块的关系

- APP 入口 `AppTask_Init()` 会调用 `I2cSlaveInit()` [app_task.c](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/src/app_task.c#L358-L383)
- 底层硬件 I2C 初始化由 API 层完成：`API_I2C_Init()`、以及 `API_i2c_slave`（I2C 外设/中断配置）

