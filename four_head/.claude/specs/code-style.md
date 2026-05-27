# 代码规范

## 命名规则
- **变量**: snake_case（`s_prog_slot`, `frame_len`, `key_code`）
- **函数**: PascalCase + 模块前缀（`Drv_Key_Init`, `HAL_Buzzer_Init`, `Msg_Post`）
- **宏/常量**: UPPER_SNAKE_CASE（`MSG_SLOT_DEPTH`, `KEY_STATE_PRESS`, `MAX_HANDLERS`）
- **类型定义**: PascalCase + `_t` 后缀（`MsgId_t`, `Msg_t`, `KeyCode_t`）
- **枚举值**: UPPER_SNAKE_CASE 或 PascalCase + 模块前缀（`MSG_KEY_EVENT`, `KEY_POWER_0`）
- **文件**: 小写 + 下划线（`drv_key.c`, `msg_scheduler.h`, `hal_comm.c`）

## 注释规范
- 所有 public 函数在头文件中注释：功能、调用时机、参数含义
- 复杂算法加行内注释说明意图
- 模块 `.h` 头部注释：模块职责、依赖列表、所属层级
- 注释风格：`//` 用于单行注释，`/* */` 仅用于多行注释（兼容 C89）

## 文件结构
- 每个模块一对 `.h` + `.c` 文件
- 头文件保护：`#ifndef MODULE_H` / `#define MODULE_H` / `#endif` 宏保护
- `.c` 文件仅 `#include` 自己的 `.h` + 层级允许的依赖
- 模块分层：
  - `src/` — 入口（main.c）
  - `core/` — 消息调度器
  - `drv/` — 设备驱动层
  - `hal/` — 硬件抽象层
  - `app/` — 业务应用层
  - `proto/` — 协议解析层

## 模块依赖规则
- 业务模块之间**零交叉 include**
- 消息调度器是唯一桥梁（`Msg_Post` / `MsgScheduler_Register`）
- HAL 层不 include 任何业务头文件
- DRV 层只 include `msg_scheduler.h` + HAL 接口
- ISR 只设时基标志，不在中断中做业务处理
- app/ 模块只通过 `Msg_Post()` 通信，禁止直接调用

## 错误处理
- 参数校验使用 `CT_ASSERT` 编译期断言
- `Msg_t` 大小 ≤ 16 字节（编译期断言）
- 队列深度/回调数量有编译期上限检查

## 格式要求
- 缩进：4 空格
- 括号风格：K&R
- 行宽：80 字符
- 操作符两侧空格
- `#pragma pack(4)` 用于跨模块结构体（`Msg_t`）

---

*最后更新：2026-05-21，技术负责人*
