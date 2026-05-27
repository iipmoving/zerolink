# 接口约定

## 模块间通信

本项目使用两种通信机制，以 **1ms 为分界**：

### 同步回调（≤1ms 必须完成的操作）

- **机制**: DRV 层在初始化时将回调函数指针注册给 APP 半层装配器，装配器在 1ms 调度槽内同步调用
- **适用场景**:
  - 显示帧下发：`Drv_Display_Commit(DisplayFrame_t*)` — COM 扫描每 1ms 一次，走消息队列来不及
  - 键码获取：驱动层轮询触摸芯片后直接返回当前键态
- **约束**: 回调内不得阻塞、不得嵌套投递消息、不得访问业务层数据
- **注册方式**: 装配器暴露 `RegisterCommitCallback(fn)` 供 DRV 在初始化时注册

### 消息队列（跨越多个 1ms 的操作）

- **机制**: `Msg_Post` 投递 / `MsgScheduler_Register` 注册处理器，异步消费
- **消息结构**: `Msg_t { MsgId_t id; uint16_t param; void *data_ptr; }`
- **大小限制**: `sizeof(Msg_t) ≤ 16` 字节（编译期断言）
- **队列深度**: 环形队列，深度 8
- **最大回调**: 16 个处理器注册
- **适用场景**: 按键事件、状态变更、模块间通知、定时事件

### 选择规则

```
问: "这个操作必须在同一次 1ms 调度内完成？"
  → 是: 用同步回调
  → 否: 用消息队列
```

## 函数签名规范
- **返回值**: 全部 `void`（异步消息模型，无同步返回值）
- **参数**: 值传递 `uint16_t param`（轻量数据）+ `void *data_ptr`（复杂数据）
- **调度器透传**: 调度器只传递 `void*`，永不解包 `data_ptr` 内容
- **数据所有权**: 发送方负责数据生命周期，接收方在回调返回后不得再访问

## 数据格式
- **内部数据**: 各模块自定义结构体，通过 `void*` 传递，在模块自己的 `.h` 中定义
- **外部通信**: MODBUS RTU 协议
  - 物理层: UART0, 57600-10B, DMA0 RX（256B 不定长）+ DMA1 TX
  - 帧检测: 轮询帧间隔（2次 × 10ms = 20ms）判定帧结束
  - 协议解析: `proto/proto_modbus` 模块负责

## 模块依赖规则
```
src/ (main.c)
 ├── core/msg_scheduler  ← 消息总线
 ├── drv/{key,display,buzzer}  ← 驱动层
 │   └── hal/{8个模块}  ← 硬件抽象层
 ├── app/{cooking,power,protect}  ← 业务层 (纯逻辑, 只输出抽象值)
 ├── app/api_display  ← 显示装配器 (半层: 抽象值→段码, 回调传 DRV)
 │   └── 回调 ──→ drv_display (接收 DisplayFrame_t 映射 IO)
 ├── proto/modbus_parser  ← 协议层
 └── cfg/hmi_data  ← JSON 配置数据的 C 结构体落点
```
- 禁止循环依赖
- **APP 业务层不直接调用 DRV 函数**（禁止 `#include "drv_*.h"` 后直接调）
- APP → DRV 通信只有两条合法路径：
  1. **同步回调**（≤1ms）：装配器通过 DRV 注册的回调指针传 `DisplayFrame_t`
  2. **消息队列**（>1ms）：`Msg_Post` → 调度器 → 处理器
- HAL 层不知道 MsgScheduler 和回调机制的存在
- 新增模块必须先在 `msg_def.h` 中注册消息 ID

### 显示装配器职责 (api_display)

装配器是 APP 层内的一个半层模块，它不参与业务决策，只做机械转换：

```
业务逻辑 (app_hmi)
  输出: {zone:1, content:"5", effect:BLINK, led_level:5, hot_head:1, ...}
        │
        ▼ 调用多个 Set*() 设置抽象值
显示装配器 (api_display)
  内部: 缓存抽象值 → SMG Lib 查表 → 组装 DisplayFrame_t
        │
        ▼ 通过 DRV 注册的回调指针传参
DRV (drv_display)
  收到: DisplayFrame_t { seg_code[11], led_bits, dp_map, blink_mask }
  只做: 映射到 IO 引脚，不知道段码来源、不知道为什么亮
```

| 层 | 知道什么 | 不知道什么 |
|----|---------|-----------|
| APP 业务逻辑 | 状态机、路由、功率、定时 | 段码、LED bitmap、IO 引脚 |
| 显示装配器 | 抽象值→段码转换、SMG Lib | 业务决策（为什么选这个模式） |
| DRV | IO 引脚映射、COM 扫描时序 | 段码含义、SMG Lib 存在 |

## 调度模式
- **时基**: TIM0 125us → 每 8 次 ISR = 1ms 标志
- **10 槽轮转**: 每槽 1ms，10ms 完整周期
- **模块调用**: 每 10ms 被调用一次，自计数计时（不依赖 `hal_timer`）
- **消息消费**: 每 1ms 消费 1 条消息

## 版本兼容
- 消息 ID 枚举值不可变更顺序，新增只能加在 `MSG_COUNT` 之前
- `Msg_t` 结构体不可增删字段
- 新增模块回调通过 `MsgScheduler_Register` 动态注册，不修改调度器核心

---

*最后更新：2026-05-26，技术负责人 — 新增同步回调机制与显示装配器半层*
