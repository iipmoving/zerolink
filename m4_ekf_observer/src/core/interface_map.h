/**
 * interface_map.h —— __weak 回调函数配对映射表 (文档，非编译单元)
 *
 * ★ 本文件仅作文档参考，任何 .c/.h 不得 #include 本文件 ★
 *
 * 架构铁律: 新模块零依赖。所有跨模块通信通过 __weak 回调实现:
 *   调用方定义 __weak void Target_OnEvent(...) {} (空壳)
 *   实现方定义       void Target_OnEvent(...) {} (强符号)
 *   链接器自动接线 —— 无运行时开销, 无注册, 无队列。
 *
 * 一致性由 AI 保证——每次修改任一方函数签名时, AI 同步检查并更新配对。
 *
 * ====================================================================
 * __weak 回调通道注册表
 * ====================================================================
 *
 * ┌──────┬──────────────────────────────────────┬──────────────────────────────────────┬──────────────────┐
 * │  #   │ Caller (__weak 定义处)                │ Implementer (强符号定义处)            │ Purpose          │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │  0   │ Modbus_Lib_Init_An_Analysis.c:       │ wave_capture.c:                      │ MODBUS 0x5000    │
 * │      │   __weak WaveCapture_GetFramePtr()   │   void* WaveCapture_GetFramePtr()    │ 帧缓冲区指针     │
 * │      │   __weak WaveCapture_OnAckWrite()    │   uchar WaveCapture_OnAckWrite()    │ ACK 解冻回调    │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │  1   │ Modbus_Lib_Init_An_Analysis.c:       │ raw_capture.c:                       │ MODBUS 0x5100    │
 * │      │   __weak RawCapture_GetFramePtr()    │   void* RawCapture_GetFramePtr()     │ 帧缓冲区指针     │
 * │      │   __weak RawCapture_OnAckWrite()     │   uchar RawCapture_OnAckWrite()     │ ACK 解冻回调    │
 * ├──────┼──────────────────────────────────────┼──────────────────────────────────────┼──────────────────┤
 * │  2   │ Modbus_Lib_Init_An_Analysis.c:       │ (厂商固件)                            │ UART 回调        │
 * │      │   __weak API_UART_RxControlCallback  │   API_UART_RxControlCallback          │                  │
 * │      │   __weak API_UART_RxInitCallback     │   API_UART_RxInitCallback             │                  │
 * │      │   __weak API_UART_TxStatusCallback   │   API_UART_TxStatusCallback           │                  │
 * │      │   __weak API_UART_TxInitCallback     │   API_UART_TxInitCallback             │                  │
 * └──────┴──────────────────────────────────────┴──────────────────────────────────────┴──────────────────┘
 *
 * ====================================================================
 * 跨模块结构体配对 (独立声明, 同布局不同名)
 * ====================================================================
 *
 * ┌─────────────────────────────────┬─────────────────────────────────┐
 * │ modbus 层 (本地常量)              │ capture 层 (struct 定义)         │
 * ├─────────────────────────────────┼─────────────────────────────────┤
 * │ WAVE_FRAME_WORDS = 4006         │ sizeof(WaveCaptureFrame)         │
 * │ RAW_FRAME_WORDS  = 4006         │ sizeof(RawCaptureFrame)          │
 * │ 布局: header(6w) + data(4000w)  │ ack,status,id,count,dw,max+data │
 * ├─────────────────────────────────┼─────────────────────────────────┤
 * │ AI 保证: 常量与 sizeof() 一致    │                                  │
 * └─────────────────────────────────┴─────────────────────────────────┘
 *
 * ====================================================================
 * AI 管理规则
 * ====================================================================
 *
 * 1. 新增跨模块通信: 调用方定义 __weak 空壳, 实现方定义强符号
 * 2. 不在 .h 中声明回调函数 (链接器负责符号解析)
 * 3. 模块内部状态 static, 只通过 API 函数暴露
 * 4. 修改任一方函数签名时, 同步更新配对
 * 5. 新增通道时, 在本文档注册
 */

/* 本文件不被任何代码引用 —— 仅供 AI 和人类阅读 */
#ifdef INCLUDE_INTERFACE_MAP
#error "interface_map.h 是文档文件，禁止被 #include"
#endif
