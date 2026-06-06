/**
 * interface_map.h —— v2.0 Data Switcher 回调函数配对映射表
 *
 * ★ 本文件仅作文档参考，任何 .c/.h 不得 #include 本文件 ★
 *
 * v2.0 架构铁律: 
 *   1. 模块间零依赖 —— 所有跨模块通信通过 Data Switcher 路由
 *   2. Producer 定义 {module}_io.h 输出接口（保留 #define）
 *   3. Consumer 通过 _OnInput() 回调接收数据
 *   4. Switcher 负责检测 status bit1 → 调用回调 → 清零标志
 *   5. 普通 .h 文件使用 //#define 禁止跨模块 include
 *
 * Status Bit 协议:
 *   bit0 = 已构造 (模块构造函数设置，永不清除)
 *   bit1 = 新数据就绪 (Producer 设置，Switcher 清除)
 *
 * 构造函数标准范式:
 *   static void Module_Construct(void) { ... }
 *   首次进入 Module_Run() 时自动调用
 */

/* ====================================================================
 * v2.0 Switcher 配对表 (Switcher 路由模式)
 * ==================================================================== */

/* Pair 0: drv_key → app_hmi */
/* 发送方: data_switcher.c  __weak void AppHmi_OnInput_Key(uint16_t key_code) {} */
/* 接收方: app/app_hmi.c  void AppHmi_OnInput_Key(uint16_t key_code) */
/* 数据流: drv_key 产生按键事件 → Switcher 路由 → app_hmi 处理 */

/* Pair 1: drv_key → app_cooking */
/* 发送方: data_switcher.c  __weak void AppCooking_OnInput_Key(uint16_t key_code) {} */
/* 接收方: app/app_cooking.c  void AppCooking_OnInput_Key(uint16_t key_code) */

/* Pair 2: app_cooking → app_power */
/* 发送方: data_switcher.c  __weak void AppPower_OnInput_PowerCtrl(...) {} */
/* 接收方: app/app_power.c  void AppPower_OnInput_PowerCtrl(...) */

/* Pair 3: app_hmi → drv_display */
/* 发送方: data_switcher.c  __weak void DrvDisplay_OnInput_Refresh(...) {} */
/* 接收方: drv/drv_display.c  void DrvDisplay_OnInput_Refresh(...) */

/* Pair 4: drv_comm_mgr → app_comm_mgr (DataUpdate) */
/* 发送方: data_switcher.c  __weak void AppCommMgr_OnInput_DataUpdate(...) {} */
/* 接收方: app/app_comm_mgr.c  void AppCommMgr_OnInput_DataUpdate(...) */

/* Pair 5: drv_comm_mgr → app_comm_mgr (TxDone) */
/* 发送方: data_switcher.c  __weak void AppCommMgr_OnInput_TxDone(void) {} */
/* 接收方: app/app_comm_mgr.c  void AppCommMgr_OnInput_TxDone(void) */

/* Pair 6: app_protect → app_power */
/* 发送方: data_switcher.c  __weak void AppPower_OnInput_SystemError(...) {} */
/* 接收方: app/app_power.c  void AppPower_OnInput_SystemError(...) */

/* Pair 7: app_comm_mgr → app_power (RegData) */
/* 发送方: data_switcher.c  __weak void AppPower_OnInput_RegData(...) {} */
/* 接收方: app/app_power.c  void AppPower_OnInput_RegData(...) */

/* Pair 8: app_comm_mgr → app_cooking (RegData) */
/* 发送方: data_switcher.c  __weak void AppCooking_OnInput_RegData(...) {} */
/* 接收方: app/app_cooking.c  void AppCooking_OnInput_RegData(...) */

/* Pair 9: app_comm_mgr → app_protect (RegData) */
/* 发送方: data_switcher.c  __weak void AppProtect_OnInput_RegData(...) {} */
/* 接收方: app/app_protect.c  void AppProtect_OnInput_RegData(...) */

/* Pair 10: app_hmi → drv_buzzer */
/* 发送方: data_switcher.c  __weak void DrvBuzzer_OnInput_Ctrl(uint16_t cmd) {} */
/* 接收方: drv/drv_buzzer.c  void DrvBuzzer_OnInput_Ctrl(uint16_t cmd) */

/* Pair 11: app_comm_mgr → drv_comm_mgr */
/* 发送方: data_switcher.c  __weak void DrvCommMgr_OnInput_SendReq(...) {} */
/* 接收方: drv/drv_comm_mgr.c  void DrvCommMgr_OnInput_SendReq(...) */

/* Pair 12: app_power → app_comm_mgr */
/* 发送方: data_switcher.c  __weak void AppCommMgr_OnInput_PowerCmd(...) {} */
/* 接收方: app/app_comm_mgr.c  void AppCommMgr_OnInput_PowerCmd(...) */

/* ====================================================================
 * v2.0 模块类型分类
 * ==================================================================== */

/* Producers (输出数据供其他模块消费) */
/* - drv_key: 按键数据 */
/* - drv_comm_mgr: 通信数据 */
/* - app_power: 电源状态 */
/* - app_protect: 保护事件 */
/* - app_cooking: 烹饪控制 */
/* - app_hmi: HMI 显示数据 */

/* Consumers (消费其他模块数据) */
/* - app_hmi: 消费 drv_key */
/* - app_cooking: 消费 drv_key, app_power, app_comm_mgr */
/* - app_power: 消费 app_cooking, app_protect, app_comm_mgr */
/* - drv_display: 消费 app_hmi */
/* - app_comm_mgr: 消费 drv_comm_mgr, app_power */
/* - drv_buzzer: 消费 app_hmi */
/* - drv_comm_mgr: 消费 app_comm_mgr */

/* ====================================================================
 * v2.0 文件权限规则
 * ==================================================================== */

/* _io.h 文件 (公开接口) */
/* - 保留 #define include guard */
/* - 仅允许 data_switcher.c 全路径 include */
/* - 示例: #include "../../include/drv_key_io.h" */

/* 普通 .h 文件 (私有接口) */
/* - 使用 //#define include guard (L0 blocking) */
/* - 仅允许本模块 .c 文件 include */
/* - 禁止跨模块 include */

/* ====================================================================
 * v2.0 三段式结构标准
 * ==================================================================== */

/* void Module_Run(void) {
 *     // 1. INIT: 首次进入时调用构造函数
 *     static uint8_t _constructed = 0;
 *     if (!_constructed) { _constructed = 1; Module_Construct(); }
 *     
 *     // 2. INPUT: 集中获取外部数据 (调用 _OnInput() 回调)
 *     
 *     // 3. COMPUTE: 纯计算，无外部调用
 *     
 *     // 4. OUTPUT: 集中输出结果 (设置 status bit1)
 * }
 */

/* 本文件不被任何代码引用 —— 仅供 AI 和人类阅读 */
#ifdef INCLUDE_INTERFACE_MAP
#error "interface_map.h 是文档文件，禁止被 #include"
#endif