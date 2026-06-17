# modules/APP_POWER

## 职责

- 作为业务层总入口：`AppTaskRoute()`（被 `main()` 调用）
- 定义任务函数表 `TaskFun[]`，并绑定到 `APP/sys` 的协作式调度器
- 负责系统外设初始化 `SystemInitial()`（通过 API 层完成各外设配置）
- 功率控制子系统（头文件在 `APP/POWER/inc`，主要实现落在 `Projects/LIB/APP/app_power.c`）

## 入口路径（穷举）

- 入口头文件（`Projects/APP/POWER/inc`）
  - [app_task.h](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/inc/app_task.h)
  - [app_power.h](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/inc/app_power.h)
  - [app_power_route.h](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/inc/app_power_route.h)
  - [app_protect.h](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/inc/app_protect.h)
  - [app_work.h](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/inc/app_work.h)
  - [APP_ADC.H](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/inc/APP_ADC.H)
  - [APP_ZERO.H](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/inc/APP_ZERO.H)
- 源文件
  - 路由与任务表： [app_task.c](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/src/app_task.c)
  - 过零相关： [APP_ZERO.C](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/src/APP_ZERO.C)
  - 功率主实现（库目录）： [app_power.c](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/LIB/APP/app_power.c)

## 关键入口与函数

- [APP_POWER/app_power.c 流程分解](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/modules/APP_POWER_app_power.c.md)

### AppTaskRoute()

入口在： [app_task.c:L61-L76](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/src/app_task.c#L61-L76)

职责：

- 先调用 `AppTask_Init()` 与 `SystemInitial()`
- 进入循环，持续调用 `Sys_TaskService()`（来自 `APP/sys`）

### AppTask_Init()

入口在： [app_task.c:L358-L383](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/src/app_task.c#L358-L383)

关键动作：

- `Sys_SetTskFunAddress((TSK_FUN*)TaskFun);`：把任务表交给调度器
- `Switcher_Init()`：弱符号（可由数据路由/采样路由模块提供强实现）
- `I2cSlaveInit()`：I2C 从机协议初始化（`Projects/INTERFACE`）
- `u_power_init()`：功率子系统初始化（声明于 `app_power.h`）
- `Sys_Init()`：调度器状态清零（`APP/sys`）

### SystemInitial()

入口在： [app_task.c:L248-L343](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/src/app_task.c#L248-L343)

初始化序列的“读法”建议：

- 先把 `API_*_Init()` 当作“外设资源节点”，按顺序理解依赖（如 DMA 要早于 UART/ADC）
- 再回到 `API_HRTIM`，理解 HRTIM 与 ADC/DMA 的同步关系（中断、DMA 缓冲地址、MasterSync 等）

## 与其他模块的依赖

- 依赖调度器：`Projects/APP/sys`
- 依赖外设抽象：`Projects/API`（`API_GPIO_PORT_INIT`、`API_DMA_Init`、`API_ADC_Init`、`API_HRTIM1_Init`、`API_I2C_Init` 等）
- 依赖协议：`Projects/INTERFACE`（I2C 从机）、`Projects/modbus`（Modbus）
- 依赖通用组件：`Projects/BaseClass`（如 `wave_capture`、`printMessage`、功率计算等）
