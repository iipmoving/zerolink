# modules/Users

## 职责

- 定义应用的最外层入口 `main()`
- 承接芯片向量表 ISR 名称，并把中断转发到 `API_*` 弱符号处理函数
- 汇总工程级配置（例如系统时钟、UART 选择、BSP 开关）

## 入口路径（穷举）

- 源文件
  - [main.c](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/Users/src/main.c)
  - [rx32g4xx_it.c](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/Users/src/rx32g4xx_it.c)
- 头文件
  - [main.h](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/Users/inc/main.h)
  - [rx32g4xx_it.h](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/Users/inc/rx32g4xx_it.h)
  - [rx32g4xx_config_def.h](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/Users/inc/rx32g4xx_config_def.h)
  - [rx32g4xx_hal_conf.h](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/Users/inc/rx32g4xx_hal_conf.h)

## 关键函数

### main()

`main()` 不做外设初始化，直接进入应用路由：

- [main.c:L69-L76](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/Users/src/main.c#L69-L76)

依赖：

- `AppTaskRoute()` 来自 `Projects/APP/POWER`（业务调度总入口）

### rx32g4xx_it.c（中断转发）

该文件的关注点建议分两块读：

- **弱符号声明区**：定义 `API_*` 空实现，给上层覆盖 [rx32g4xx_it.c:L62-L141](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/Users/src/rx32g4xx_it.c#L62-L141)
- **ISR 转发区**：把具体中断（ADC/DMA/HRTIM/UART/I2C/FM AC 等）转发到 `API_*` [rx32g4xx_it.c:L255-L469](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/Users/src/rx32g4xx_it.c#L255-L469)

## 关键配置（建议从这里开始改）

- 系统时钟、UART 输出口选择、BSP LED/按键开关等： [rx32g4xx_config_def.h](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/Users/inc/rx32g4xx_config_def.h#L52-L111)

