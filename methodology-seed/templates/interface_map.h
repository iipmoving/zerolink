/* interface_map.h — __weak 回调配对文档 (v2.0 零依赖架构)
 *
 * 本文件是 __weak 配对的唯一真相源 (single source of truth)。
 * 每对包含: 发送方(WEAK空壳) → 接收方(强符号实现)
 * AI 保证双方函数签名一致。
 *
 * ⛔ 任何 .c/.h 不得 include 本文件。本文档仅 AI 维护，不参与编译。
 *
 * 使用方法:
 *   新增通道 → 在对应的区段添加配对注释
 *   修改通道 → 更新注释中的函数签名
 *   删除通道 → 移除该条目
 *   新增结构体 → 在结构体配对表添加条目
 */

#ifndef INTERFACE_MAP_H
#define INTERFACE_MAP_H

/* ================================================================
 * 一、结构体配对表
 *
 * 跨模块传递的数据结构，每模块独立声明，不同名同布局。
 * 约束: sizeof() 相等，所有字段 offsetof() 相等。
 * ================================================================ */

/*
 * Pair S{n}: {用途}
 * 发送方({层}): {TypeName_A} 在 {file_a}.h
 * 接收方({层}): {TypeName_B} 在 {file_b}.h
 * 字段: {field1_type field1; field2_type field2; ...}
 */


/* ================================================================
 * 二、__weak 通道注册表
 *
 * 格式 (必须精确匹配 check_weak_pairs.py 解析器):
 *
 *   /* Pair {ID}: {description} */       ← */ 必须在同一行!
 *    * 发送方: {file}.c  WEAK {ret} {func}({params}) {}
 *    * 接收方: {file}.c  {ret} {func}({params})
 *
 *   - 发送方和接收方之间不能有空行 (空行=解析停止)
 *   - 文件名大小写需匹配实际文件 (.C or .c)
 *   - 函数签名必须与实际代码一致
 * ================================================================ */

/* Pair A: {sender} → {receiver} */
 * 发送方: {sender_file}.c  WEAK void {Receiver}_On{Event}(uint16_t param, void *data_ptr) {}
 * 接收方: {receiver_file}.c  void {Receiver}_On{Event}(uint16_t param, void *data_ptr)
 * 注: {触发条件说明}

#endif /* INTERFACE_MAP_H */
