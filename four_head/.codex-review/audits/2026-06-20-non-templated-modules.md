---
name: non-templated-modules-2026-06-20
description: 未使用 std_module.h 模板的模块清单
metadata:
  node_type: memory
  type: audit
  auditor: Codex
---

## 审查范围

- 数据源：src/include_io/ 11 个模块 _io.h + src/core/data_switcher.c Slot 注册表
- 判定标准：模块 .c 文件是否包含 std_module.h、MODULE_SKELETON、MODULE_EXPORT
- 排除：hal/ 层（底层驱动，不使用模块范式）

## 结果

**总计 11 个已注册模块，1 个未范式化。**

| # | 模块 | 层 | std_module.h | MODULE_SKELETON | MODULE_EXPORT | 状态 |
|---|------|-----|:-:|:-:|:-:|------|
| 1 | app_comm_mgr | app | ✅ | ✅ | ✅ | 已范式化 |
| 2 | app_cooking | app | ✅ | ✅ | ✅ | 已范式化 |
| 3 | app_hmi | app | ✅ | ✅ | ✅ | 已范式化 |
| 4 | app_power | app | ✅ | ✅ | ✅ | 已范式化 |
| 5 | app_protect | app | ✅ | ✅ | ✅ | 已范式化 |
| 6 | app_seg_align | app | ✅ | ✅ | ✅ | 已范式化 |
| 7 | drv_buzzer | drv | ✅ | ✅ | ✅ | 已范式化 |
| 8 | drv_comm_mgr | drv | ✅ | ✅ | ✅ | 已范式化 |
| 9 | drv_display | drv | ✅ | ✅ | ✅ | 已范式化 |
| 10 | drv_key | drv | ✅ | ✅ | ✅ | 已范式化 |
| 11 | proto_modbus | proto | ❌ | ❌ | ❌ | **未范式化** |

## proto_modbus 现状

- data_switcher.c 中已注册 SLOT_ProtoModbus = 1，有 SLOT_GETIO(ProtoModbus)
- 但 proto/proto_modbus.c 仍使用旧架构（手动 g_in/g_out / Constructor / GetIO）
- 是 Switcher 11 个槽位中唯一一个未走 std_module.h 范式的模块

## 建议

1. 按 modify-module SOP 迁移 proto_modbus.c 到 v2.3 范式
2. 迁移后重跑 check_deps.py 验证无新增违规
3. 更新基线：未范式化模块从 1 降为 0

---
*审查时间：2026-06-20 | 审查者：Codex*
