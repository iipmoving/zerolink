# RX32G410_FW_HAL_V1.3N Code Wiki

本 Wiki 面向“快速读懂 + 快速定位代码入口”，以模块为单位给出：

- 仓库整体分层与依赖方向
- 各模块职责与入口路径（优先以 `inc/` 头文件作为对外入口）
- 关键函数/中断路径（从 `main()` 与 `Sys_TaskService()` 主循环出发）
- 构建与运行方式（Keil uVision 工程）

## 快速导航

- [01_整体架构](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/01_%E6%95%B4%E4%BD%93%E6%9E%B6%E6%9E%84.md)
- [02_构建与运行](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/02_%E6%9E%84%E5%BB%BA%E4%B8%8E%E8%BF%90%E8%A1%8C.md)
- [03_主入口与调度](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/03_%E4%B8%BB%E5%85%A5%E5%8F%A3%E4%B8%8E%E8%B0%83%E5%BA%A6.md)
- [04_中断与回调](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/04_%E4%B8%AD%E6%96%AD%E4%B8%8E%E5%9B%9E%E8%B0%83.md)
- [05_模块总览（入口路径穷举）](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/05_%E6%A8%A1%E5%9D%97%E6%80%BB%E8%A7%88_%E5%85%A5%E5%8F%A3%E8%B7%AF%E5%BE%84%E7%A9%B7%E4%B8%BE.md)

## 模块页面

- [modules/Users](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/modules/Users.md)
- [modules/APP_ADC](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/modules/APP_ADC.md)
- [modules/APP_POWER](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/modules/APP_POWER.md)
- [modules/APP_sys](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/modules/APP_sys.md)
- [modules/API](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/modules/API.md)
- [modules/DRV](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/modules/DRV.md)
- [modules/BaseClass](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/modules/BaseClass.md)
- [modules/INTERFACE](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/modules/INTERFACE.md)
- [modules/modbus](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/modules/modbus.md)
- [modules/Middlewares_System](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/modules/Middlewares_System.md)
- [modules/Drivers](file:///D:/OBSIDIAN/MOVING%20IH/%E4%BD%8E%E8%80%A6%E5%90%88%E7%A8%8B%E5%BA%8F%E6%9E%B6%E6%9E%84/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/CodeWiki/modules/Drivers.md)

