# 代码规范模板

> 用法: 复制到 `.claude/specs/code-style.md`，按项目调整。

## 命名规则
- **变量**: snake_case（`s_prog_slot`, `frame_len`, `key_code`）
- **函数**: PascalCase + 模块前缀（`Drv_Key_Init`, `App_Hmi_Init`）
- **宏/常量**: UPPER_SNAKE_CASE（`SLOT_DEPTH`, `MAX_HANDLERS`）
- **类型定义**: PascalCase + `_t` 后缀（`KeyCode_t`, `Msg_t`）
- **结构体后缀**: `_IN` = 本模块接收, `_OUT` = 本模块输出, 无后缀 = 内部私有（`StateGlobal_OUT_t`, `Display_StateInput_t`, `DisplayBuffer_t`）
- **枚举值**: UPPER_SNAKE_CASE 或 PascalCase + 模块前缀
- **文件**: 小写 + 下划线（`drv_key.c`, `app_hmi.c`, `hal_comm.c`）

## __weak 回调命名
- **格式**: `{ModulePrefix}_On{Event}(uint16_t param, void *data_ptr)`
- **APP 层**: `App{Name}_On{Event}` 如 `AppHmi_OnKey`
- **DRV 层**: `Drv{Name}_On{Event}` 如 `DrvDisplay_OnRefresh`

## 注释规范
- 所有 public 函数在头文件中注释：功能、调用时机、参数含义
- 复杂算法加行内注释说明意图
- 模块 `.h` 头部注释：模块职责、依赖列表、所属层级
- 注释风格：`//` 用于单行，`/* */` 仅用于多行

## 文件结构
- 每个模块一对 `.h` + `.c` 文件
- 头文件保护 (HAL 层): `#ifndef MODULE_H` / `#define MODULE_H` / `#endif`
- 头文件保护 (DRV/APP/PROTO 层): `#ifndef MODULE_H` / `//#define MODULE_H` / `#endif`
  - **define 必须注释掉** — 禁用 include guard, 同一个 .c 内二次包含直接编译器报错 (L0 阻断)
  - HAL 不适用此规则 — DRV 需要包含 HAL 头文件
- `.c` 文件仅 `#include` 自己的 `.h` + 层级允许的依赖
- 模块分层：
  - `src/` — 入口（main.c）
  - `app/` — 业务应用层
  - `drv/` — 设备驱动层
  - `hal/` — 硬件抽象层
  - `proto/` — 协议解析层
  - `cfg/` — 配置数据（JSON→C 自动生成）

## 模块依赖规则
- 业务模块之间**零交叉 include**
- __weak 回调是唯一跨模块通信机制
- HAL 层不 include 任何业务头文件
- DRV 层只 include HAL 接口
- APP 模块只通过 __weak 回调通信，禁止直接调用

## 格式要求
- 缩进：4 空格
- 括号风格：K&R
- 行宽：{80 / 100 / 120} 字符
- 操作符两侧空格

## 编译约束
- {C89: 无 _Static_assert, 无 C++ 风格注释 /* 仅用于多行 */}
- {C99: 可用 // 注释}
