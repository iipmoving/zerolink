/**
 * interface_map.h —— v2.2 PULL 路由配对映射表
 *
 * ★ 本文件仅作文档参考，任何 .c/.h 不得 #include 本文件 ★
 *
 * v2.2 架构铁律:
 *   1. 模块间零依赖 —— 所有跨模块通信由 InputCallback 弱符号路由
 *   2. Producer 写 g_output.para + 置 ST_OUT
 *   3. 中间层覆盖 InputCallback 强符号, 从上游拉数据写入下游 g_input + 置 ST_NEW
 *   4. ProcessInput 消费 g_input.para → 计算 → 写 g_output.para
 *   5. 输出回调例外须 @OUTPUT_CALLBACK 标记 + 用户确认
 *
 * Status Bit 协议:
 *   bit0 ST_INIT (0x01) = 已构造 (Constructor 置位, 永不清除)
 *   bit1 ST_NEW  (0x02) = 新输入到达 (InputCallback 置, ProcessInput 消费后自清)
 *   bit2 ST_OUT  (0x04) = 输出就绪 (ProcessInput 输出段置, DoWork 宏自动消费)
 *
 * 使用:
 *   MODULE_SKELETON(name) — 展开 g_input/g_output/InputCallback/OutputCallback/DoWork
 *   MODULE_EXPORT(name)   — 展开 GetIO
 */

/* ====================================================================
 * v2.2 PULL 路由表 (Switcher 显式调用)
 * ==================================================================== */

/* === Producer: AppCommMgr → Consumers: AppPower, AppCooking, AppProtect === */
/* Route: Switcher 检查 OutData_t.has_reg → 调 consumer 回调 */
/* Consumer v2.2: app_power.c  void AppPower_OnCommMgrData(Para_Grp_t *pOut) */
/* Consumer v1.x: app_cooking.c  void AppCooking_OnRegData(uint16_t, void*) */
/* Consumer v1.x: app_protect.c  void AppProtect_OnRegData(uint16_t, void*) */

/* === Producer: AppProtect → Consumer: AppPower === */
/* Route: Switcher 检查 OutData_t.has_err → 调 consumer 回调 */
/* Consumer v2.2: app_power.c  void AppPower_OnProtectData(Para_Grp_t *pOut) */

/* === Producer: AppCooking → Consumer: AppPower === */
/* Route: Switcher 检查 OutData_t.has_power → 调 consumer 回调 */
/* Consumer v2.2: app_power.c  void AppPower_OnCookingData(Para_Grp_t *pOut) */

/* === Producer: DrvKey → Consumers: AppHmi, AppCooking, AppSegAlign === */
/* Route: Switcher 检查 OutData_t.has_key → 调 consumer 回调 */
/* Consumer v1.x: app_hmi.c  void AppHmi_OnKey(uint16_t, void*) */
/* Consumer v1.x: app_cooking.c  void AppCooking_OnKey(uint16_t, void*) */
/* Consumer v1.x: app_seg_align.c  void AppSegAlign_OnKey(uint16_t, void*) */

/* === Producer: AppHmi → Consumers: DrvDisplay (buzzer via @OUTPUT_CALLBACK) === */
/* Route: Switcher 检查 OutData_t.has_display → DrvDisplay_OnRefresh */
/* @OUTPUT_CALLBACK: app_hmi.c → DrvBuzzer_OnCtrl (实时蜂鸣, 用户确认) */

/* ====================================================================
 * v2.2 Switcher 调度顺序 (Switcher_Run)
 * ==================================================================== */

/*
 * 1. AppCommMgr_DoWork  → _route_comm_mgr() → AppPower_OnCommMgrData + v1.x compat
 * 2. AppPower_DoWork     (消费 CommMgr 数据 + 计算功率)
 * 3. AppProtect_DoWork   → _route_protect()  → AppPower_OnProtectData
 * 4. AppHmi_DoWork       → _route_hmi()      → DrvDisplay_OnRefresh
 * 5. AppSegAlign_DoWork
 * 6. DrvKey_DoWork       → _route_key()      → AppHmi_OnKey + AppCooking_OnKey + AppSegAlign_OnKey
 * 7. DrvCommMgr_DoWork
 * 8. DrvBuzzer_DoWork
 * 9. DrvDisplay_DoWork
 */

/* ====================================================================
 * v2.2 模块类型分类
 * ==================================================================== */

/* Producers (写 g_output.para, Switcher 路由到 consumers) */
/* - AppCommMgr: 寄存器数据 */
/* - AppProtect: 保护事件 */
/* - AppCooking: 烹饪控制 */
/* - DrvKey: 按键数据 */
/* - AppHmi: 显示数据 */

/* Consumers (Switcher 回调写 g_input.para, ProcessInput 消费) */
/* - AppPower: AppPower_OnCommMgrData, AppPower_OnProtectData, AppPower_OnCookingData */
/* - AppCooking: AppCooking_OnRegData, AppCooking_OnKey */
/* - AppProtect: AppProtect_OnRegData */
/* - AppHmi: AppHmi_OnKey */
/* - AppSegAlign: AppSegAlign_OnKey */
/* - DrvDisplay: DrvDisplay_OnRefresh */

/* ====================================================================
 * 文件权限规则 (_io.h)
 * ==================================================================== */

/* _io.h 文件 (公开接口) */
/* - 保留 #define include guard */
/* - 仅允许 data_switcher.c 和模块自身 include */
/* - check_include.py 审计 */

/* 普通 .h 文件 (私有接口) */
/* - 使用 //#define include guard (L0 blocking) */
/* - 仅允许本模块 .c include */

/* ====================================================================
 * @OUTPUT_CALLBACK 例外白名单
 * ==================================================================== */

/* 以下输出回调因实时性要求保留直接调用 (不走 Switcher PULL):
 * - app_hmi.c: DrvBuzzer_OnCtrl() — 蜂鸣即时反馈
 *
 * 新增例外须: 用户确认 + @OUTPUT_CALLBACK 标记 + 本文档注册
 */

/* 本文件不被任何代码引用 —— 仅供 AI 和人类阅读 */
#ifdef INCLUDE_INTERFACE_MAP
#error "interface_map.h 是文档文件，禁止被 #include"
#endif
