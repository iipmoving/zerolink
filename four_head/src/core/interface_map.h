/**
 * interface_map.h —— __weak 回调函数配对映射表 (文档，非编译单元)
 *
 * ★ 本文件仅作文档参考，任何 .c/.h 不得 #include 本文件 ★
 *
 * 架构铁律: 模块间零依赖。所有跨模块通信通过 __weak 回调实现:
 *   发送方定义 __weak void Receiver_OnXxx(...) {} (空壳)
 *   接收方定义       void Receiver_OnXxx(...) {} (强符号)
 *   链接器自动接线 —— 无运行时开销, 无注册, 无队列。
 *
 * 一致性由 AI 保证——每次修改任一方函数签名时, AI 同步检查并更新配对。
 * 验证方法: 发送方和接收方的函数签名(参数类型/顺序)必须完全一致。
 *
 * 命名约定:
 *   发送动作: {ReceiverModule}_{On}{Purpose}(uint16_t param, void *data_ptr)
 *   例: AppHmi_OnKey, DrvDisplay_OnRefresh, AppCommMgr_OnPowerCmd
 *
 * 前缀约定:
 *   APP 层: App{Module}_On*  (如 AppHmi_OnKey, AppPower_OnPowerCtrl)
 *   DRV 层: Drv{Module}_On*  (如 DrvDisplay_OnRefresh, DrvBuzzer_OnCtrl)
 *
 * ====================================================================
 * 结构体配对表 (跨模块数据传递, 独立声明, 同布局不同名)
 * ====================================================================
 *
 * ┌─────────────────────────────────┬─────────────────────────────────┐
 * │ APP 层 (app/app_hmi.h)           │ DRV 层 (drv/drv_display.c)       │
 * ├─────────────────────────────────┼─────────────────────────────────┤
 * │ HmiDisplayCache_t               │ DisplayFrame_t                   │
 * │   sizeof = 29                   │   sizeof = 29                    │
 * │   字段布局一致, AI 保证           │                                  │
 * ├─────────────────────────────────┼─────────────────────────────────┤
 * │ __weak 通道:                    │ 发送方: app_hmi.c, app_cooking.c │
 * │   DrvDisplay_OnRefresh()        │ 接收方: drv_display.c (强符号)   │
 * └─────────────────────────────────┴─────────────────────────────────┘
 *
 * ┌─────────────────────────────────┬─────────────────────────────────┐
 * │ APP 层 (app/app_hmi.h)           │ DRV 层 (drv/drv_key.h)           │
 * ├─────────────────────────────────┼─────────────────────────────────┤
 * │ HmiKeyCode_t 枚举               │ KeyCode_t 枚举                   │
 * │   数值一致, AI 保证              │                                  │
 * ├─────────────────────────────────┼─────────────────────────────────┤
 * │ __weak 通道:                    │ 发送方: drv_key.c                │
 * │   AppHmi_OnKey()                │ 接收方: app_hmi.c (强符号)       │
 * │   AppCooking_OnKey()            │ 接收方: app_cooking.c (强符号)   │
 * └─────────────────────────────────┴─────────────────────────────────┘
 *
 * ┌─────────────────────────────────┬─────────────────────────────────┐
 * │ APP 层 (app_comm_mgr.c          │ DRV 层 (drv_comm_mgr.h)           │
 * │         /app_power.c)            │                                 │
 * ├─────────────────────────────────┼─────────────────────────────────┤
 * │ CommSendReq_t (各模块独立声明)    │ DrvCommSendReq_t                 │
 * │   uint8_t data[64]              │   uint8_t data[64]               │
 * │   uint16_t len                  │   uint16_t len                   │
 * │ sizeof = 66                     │ sizeof = 66                      │
 * ├─────────────────────────────────┼─────────────────────────────────┤
 * │ __weak 通道:                    │ 发送方: app_comm_mgr.c           │
 * │   DrvCommMgr_OnSendReq()        │ 接收方: drv_comm_mgr.c (强符号)  │
 * └─────────────────────────────────┴─────────────────────────────────┘
 *
 * ┌─────────────────────────────────┬─────────────────────────────────┐
 * │ DRV 层 (drv_comm_mgr.c)          │ APP 层 (app_comm_mgr.c)          │
 * ├─────────────────────────────────┼─────────────────────────────────┤
 * │ DrvCommDataUpdate_t             │ CommDataUpdate_t                 │
 * │   uint8_t data[256]             │   uint8_t data[256]              │
 * │   uint16_t len                  │   uint16_t len                   │
 * ├─────────────────────────────────┼─────────────────────────────────┤
 * │ __weak 通道:                    │ 发送方: drv_comm_mgr.c           │
 * │   AppCommMgr_OnDataUpdate()     │ 接收方: app_comm_mgr.c (强符号)  │
 * └─────────────────────────────────┴─────────────────────────────────┘
 *
 * ┌─────────────────────────────────┬─────────────────────────────────┐
 * │ APP 层 (app_power.c)             │ APP 层 (app_comm_mgr.c)          │
 * ├─────────────────────────────────┼─────────────────────────────────┤
 * │ PowerOutput_t                   │ CommPowerCmd_t                   │
 * │   uint8_t  head_idx             │   uint8_t  head_idx              │
 * │   uint16_t power_watt           │   uint16_t power_watt            │
 * │ sizeof = 4 (packed)             │ sizeof = 4 (packed)              │
 * ├─────────────────────────────────┼─────────────────────────────────┤
 * │ __weak 通道:                    │ 发送方: app_power.c              │
 * │   AppCommMgr_OnPowerCmd()       │ 接收方: app_comm_mgr.c (强符号)  │
 * └─────────────────────────────────┴─────────────────────────────────┘
 *
 * ┌─────────────────────────────────┬─────────────────────────────────┐
 * │ APP 层 (app_cooking.c)           │ APP 层 (app_power.c)             │
 * ├─────────────────────────────────┼─────────────────────────────────┤
 * │ LocalPowerCtrl_t                │ PowerCtrl_t                      │
 * │   uint8_t  head_index           │   uint8_t  head_index            │
 * │   uint8_t  onoff                │   uint8_t  onoff                 │
 * │   uint16_t target_power         │   uint16_t target_power          │
 * │   uint8_t  power_level          │   uint8_t  power_level           │
 * │   uint8_t  work_mode            │   uint8_t  work_mode             │
 * │   uint16_t target_temp          │   uint16_t target_temp           │
 * │ sizeof = 8 (packed)             │ sizeof = 8 (packed)              │
 * ├─────────────────────────────────┼─────────────────────────────────┤
 * │ __weak 通道:                    │ 发送方: app_cooking.c            │
 * │   AppPower_OnPowerCtrl()        │ 接收方: app_power.c (强符号)     │
 * └─────────────────────────────────┴─────────────────────────────────┘
 *
 * ┌─────────────────────────────────┬─────────────────────────────────┐
 * │ APP 层 (app_protect.c)           │ APP 层 (app_power.c)             │
 * ├─────────────────────────────────┼─────────────────────────────────┤
 * │ ProtectEvent_t                  │ (app_power 直接用 raw bytes)     │
 * │   uint8_t  head_index           │   raw[0]=head_index              │
 * │   uint8_t  slave_addr           │   raw[1]=slave_addr              │
 * │   uint16_t fault (ProtectFault_t)│  raw[2..3]=fault                 │
 * ├─────────────────────────────────┼─────────────────────────────────┤
 * │ __weak 通道:                    │ 发送方: app_protect.c            │
 * │   AppPower_OnSystemError()      │ 接收方: app_power.c (强符号)     │
 * └─────────────────────────────────┴─────────────────────────────────┘
 *
 * ====================================================================
 * __weak 回调通道全局注册表 (AI管理, 无数字ID, 纯函数名配对)
 * ====================================================================
 *
 * 每条通道由一对 __weak(发送方)/强符号(接收方) 函数组成。
 * 发送方定义空壳, 接收方定义强实现, 链接器自动接线。
 * AI 保证函数签名一致 (参数类型/顺序/返回值)。
 *
 * ┌──────┬──────────────────────────────────────┬──────────────────────────────────────┬──────────────────┐
 * │  #   │ Sender (__weak 定义处)                │ Receiver (强符号定义处)               │ Data Type        │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │  0   │ drv_key:                             │ app_hmi:                             │ KeyEvent_t       │
 * │      │   __weak AppHmi_OnKey()              │   void AppHmi_OnKey()                │ (u16 param)      │
 * │      │   __weak AppCooking_OnKey()          │ app_cooking:                         │                  │
 * │      │                                      │   void AppCooking_OnKey()            │                  │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │  1   │ (自收, cooking 内部直调)              │ app_cooking:                         │ CookingCmd_t     │
 * │      │   on_cooking_ctrl() 静态函数          │   static on_cooking_ctrl()           │ (u16 param)      │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │  2   │ app_cooking:                         │ app_power:                           │ PowerCtrl_t      │
 * │      │   __weak AppPower_OnPowerCtrl()      │   void AppPower_OnPowerCtrl()        │                  │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │  3   │ (预留)                               │                                      │                  │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │  4   │ app_hmi:                             │ drv_display:                         │ DisplayFrame_t   │
 * │      │   __weak DrvDisplay_OnRefresh()      │   void DrvDisplay_OnRefresh()        │                  │
 * │      │ app_cooking:                         │                                      │                  │
 * │      │   __weak DrvDisplay_OnRefresh()      │                                      │                  │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │  5   │ main:                                │ app_hmi:                             │ (none)           │
 * │      │   __weak AppHmi_OnTimer100ms()       │   void AppHmi_OnTimer100ms()         │                  │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │  6   │ main:                                │ app_hmi:                             │ (none)           │
 * │      │   __weak AppHmi_OnTimer1s()          │   void AppHmi_OnTimer1s()            │                  │
 * │      │   __weak AppCooking_OnTimer1s()      │ app_cooking:                         │                  │
 * │      │                                      │   void AppCooking_OnTimer1s()        │                  │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │  7   │ drv_comm_mgr:                        │ app_comm_mgr:                        │ (none)           │
 * │      │   __weak AppCommMgr_OnTxDone()       │   void AppCommMgr_OnTxDone()         │                  │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │  8   │ drv_comm_mgr:                        │ app_comm_mgr:                        │ CommData_t       │
 * │      │   __weak AppCommMgr_OnDataUpdate()   │   void AppCommMgr_OnDataUpdate()     │                  │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │  9   │ app_protect:                         │ app_power:                           │ ProtectFault_t   │
 * │      │   __weak AppPower_OnSystemError()    │   void AppPower_OnSystemError()      │                  │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │ 10   │ (test 通道, 当前禁用)                 │                                      │                  │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │ 11   │ (test 通道, 当前禁用)                 │                                      │                  │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │ 12   │ (预留)                               │                                      │                  │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │ 13   │ app_comm_mgr:                        │ app_power:                           │ RegData_t        │
 * │      │   __weak AppPower_OnRegData()        │   void AppPower_OnRegData()          │                  │
 * │      │   __weak AppCooking_OnRegData()      │ app_cooking:                         │                  │
 * │      │   __weak AppProtect_OnRegData()      │   void AppCooking_OnRegData()        │                  │
 * │      │                                      │ app_protect:                         │                  │
 * │      │                                      │   void AppProtect_OnRegData()        │                  │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │ 14   │ app_hmi:                             │ drv_buzzer:                          │ (u16 param)      │
 * │      │   __weak DrvBuzzer_OnCtrl()          │   void DrvBuzzer_OnCtrl()            │                  │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │ 15   │ app_comm_mgr:                        │ drv_comm_mgr:                        │ CommSendReq_t    │
 * │      │   __weak DrvCommMgr_OnSendReq()      │   void DrvCommMgr_OnSendReq()        │                  │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │ 16   │ app_power:                           │ app_comm_mgr:                        │ PowerOutput_t    │
 * │      │   __weak AppCommMgr_OnPowerCmd()     │   void AppCommMgr_OnPowerCmd()       │ ↔CommPowerCmd_t   │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │ 17   │ app_comm_mgr:                        │ proto_modbus:                        │ (raw buf)        │
 * │      │   __weak Proto_BuildRead()           │   uint16_t Proto_BuildRead()         │ uint8_t[64]      │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │ 18   │ app_comm_mgr:                        │ proto_modbus:                        │ (raw buf)        │
 * │      │   __weak Proto_Parse()               │   int8_t Proto_Parse()               │ uint8_t[256]     │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │ 19   │ app_comm_mgr:                        │ proto_modbus:                        │ (raw buf)        │
 * │      │   __weak Proto_BuildWriteSingle()    │   uint16_t Proto_BuildWriteSingle()  │ uint8_t[64]      │
 * └──────┴──────────────────────────────────────┴──────────────────────────────────────┴──────────────────┘
 *
 * 模块前缀缩写:
 *   comm=app_comm_mgr, cook=app_cooking, power=app_power
 *   prot=app_protect, hmi=app_hmi, dcomm=drv_comm_mgr
 *   ddisp=drv_display, dkey=drv_key, dbuzz=drv_buzzer
 *   proto=proto_modbus, main=main.c
 *
 * ====================================================================
 * 特殊: PROTO 协议抽象通道 (#17-19, 返回值函数)
 * ====================================================================
 *
 * Proto_BuildRead / Proto_Parse / Proto_BuildWriteSingle 三条通道
 * 不同于标准 void Receiver_OnEvent(uint16_t, void*) 模式:
 *   - 这些是带返回值的协议抽象函数 (uint16_t / int8_t)
 *   - APP 层定义 __weak 空壳 (返回 0/-1 = 无协议支持)
 *   - PROTO 层提供强符号实现 (委托 Proto_Modbus_* 内部函数)
 *   - 换协议只需换 proto/ 模块, APP 层代码不受影响
 *   - 参数使用具体类型 (非 void*), 因为协议帧格式是跨层约定的
 *
 *
 * ====================================================================
 * AI 管理规则 (v2.0 — __weak 时代)
 * ====================================================================
 *
 * 1. 新增跨模块通信:
 *    a. 确定发送方和接收方模块
 *    b. 发送方定义: __weak void ReceiverName_OnEvent(uint16_t param, void *data)
 *       { (void)param; (void)data; }
 *    c. 接收方定义: void ReceiverName_OnEvent(uint16_t param, void *data) { ... }
 *    d. 在本文档注册通道 (发送方/接收方/数据类型)
 *    e. 如需传数据: 两端各独立声明结构体，不同名同布局
 * 2. 命名约定: {ModulePrefix}_On{Event}(uint16_t param, void *data_ptr)
 *    - APP 模块: App{Name}_On{Event}  (如 AppHmi_OnKey)
 *    - DRV 模块: Drv{Name}_On{Event}  (如 DrvDisplay_OnRefresh)
 * 3. 修改任一方函数签名时, AI 必须同步修改配对
 * 4. 新增跨层数据传递时, 先在本文档注册配对, 再在两端独立声明
 * 5. 字段顺序、类型、数组大小必须在两端完全一致
 * 6. 编译时链接器自动验证: 若发送方调用但无接收方强符号 → 空壳运行(静默丢弃)
 * 7. msg_scheduler.h/c 已废弃 —— 全部模块使用 __weak 直接调用
 */

/* 本文件不被任何代码引用 —— 仅供 AI 和人类阅读 */
#ifdef INCLUDE_INTERFACE_MAP
#error "interface_map.h 是文档文件，禁止被 #include"
#endif
