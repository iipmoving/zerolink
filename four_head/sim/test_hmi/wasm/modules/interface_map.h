/* interface_map.h — __weak 回调配对文档 (v2.0 零依赖架构)
 *
 * 本文件是__weak配对的唯一真相源。
 * 每对包含: 发送方(声明WEAK空壳) → 接收方(提供强符号实现)
 * AI保证双方函数签名一致。
 *
 * 接口摘要:
 *   key_module    ──HmiState_OnKey──→ state_module
 *   state_module  ──Display_OnStateChange──→ display_module
 *   state_module  ──Timer_OnStateChange──→ timer_module
 *   timer_module  ──HmiState_OnSelectTimeout──→ state_module
 *   timer_module  ──HmiState_OnTimerExpired──→ state_module
 *   timer_module  ──HmiState_OnBoostTimeout──→ state_module
 *   display_module──DrvDisplay_OnRefresh──→ main.c (DRV桩)
 */

#ifndef INTERFACE_MAP_H
#define INTERFACE_MAP_H

/* Pair A: key→state */
/* 发送方: key_module.c  WEAK void HmiState_OnKey(uint16_t param, uint16_t unused, void *data_ptr) {} */
/* 接收方: state_module.c void HmiState_OnKey(uint16_t param, uint16_t evt_packed, void *data_ptr) */
/* 注: param = ((uint16_t)evt << 8) | key, 打包传递 */

/* Pair B: state→display */
/* 发送方: state_module.c   WEAK void Display_OnStateChange(uint8_t zone, const HmiHead_t *head, const HmiGlobalState_t *global) {} */
/* 接收方: display_module.c void Display_OnStateChange(uint8_t zone, const HmiHead_t *head, const HmiGlobalState_t *global) */

/* Pair C: state→timer */
/* 发送方: state_module.c  WEAK void Timer_OnStateChange(uint8_t zone, const HmiHead_t *head) {} */
/* 接收方: timer_module.c  void Timer_OnStateChange(uint8_t zone, const HmiHead_t *head) */

/* Pair D: timer→state (select timeout) */
/* 发送方: timer_module.c   WEAK void HmiState_OnSelectTimeout(uint8_t zone) {} */
/* 接收方: state_module.c   void HmiState_OnSelectTimeout(uint8_t zone) */

/* Pair E: timer→state (timer expired) */
/* 发送方: timer_module.c   WEAK void HmiState_OnTimerExpired(uint8_t zone) {} */
/* 接收方: state_module.c   void HmiState_OnTimerExpired(uint8_t zone) */

/* Pair F: timer→state (boost timeout) */
/* 发送方: timer_module.c   WEAK void HmiState_OnBoostTimeout(uint8_t zone) {} */
/* 接收方: state_module.c   void HmiState_OnBoostTimeout(uint8_t zone) */

/* Pair G: display→DRV */
/* 发送方: display_module.c WEAK void DrvDisplay_OnRefresh(const HmiDisplayCache_t *cache) {} */
/* 接收方: main.c           void DrvDisplay_OnRefresh(const HmiDisplayCache_t *cache) */

#endif
