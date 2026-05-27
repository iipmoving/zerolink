# HMI JSON逻辑专用测试程序 — 实施计划

## 背景

- four_head 嵌入式C骨架已完成（消息调度器 + HAL/DRV/APP/PROTO层）
- emc-web 仿真平台已迁入 `sim/`，界面JSON `fourSave.json` 已就位
- 原 WEB 端使用回调机制（LogicLayerAdapter），与嵌入式消息机制不一致
- 决定：重写测试界面，以消息机制为核心，全新设计JSON逻辑格式

## 关键约定

| 项目 | 约定 |
|------|------|
| Segment 映射 | POT1显示上：左2位=炉头1(左上)，右2位=炉头4(右上) |
|  | POT1显示下：左2位=炉头2(左下)，右2位=炉头3(右下) |
| 消息系统 | 复用嵌入式 `msg_def.h` 的 `MsgId_t` 枚举 |
| JSON格式 | 全新设计 V4.0，面向消息机制 |
| 原适配器代码 | 不保留，参考控件属性后重写 |

## 架构

```
sim/test_hmi/test_hmi.html
├── Canvas 渲染层（加载 fourSave.json 控件布局）
├── KeyHandler（Canvas点击 → 按键检测 → 长按/短按识别）
├── MessageBus（JS版 MsgScheduler：Msg_Post / 注册回调 / 环形队列）
├── JsonLogicEngine（加载逻辑JSON → 解析状态机 → 处理消息 → 执行动作）
└── TestCommandPanel（文本命令注入消息 / 查看状态 / 查看显示）
```

数据流：
```
Canvas点击 → KeyHandler → Msg_Post(MSG_KEY_EVENT) → JsonLogicEngine
    → 状态转移 + 动作执行 → Msg_Post(MSG_DISPLAY_REFRESH) → Canvas重绘
                          → Msg_Post(MSG_BUZZER_CTRL) → Buzzer输出
```

## 实施步骤

### 步骤1：创建测试程序骨架

创建 `sim/test_hmi/` 目录，包含：
- `test_hmi.html` — 主页面（Canvas + 测试命令面板）
- `js/message_bus.js` — JS版消息调度器（Msg_Post / 注册 / 环形队列）
- `js/key_handler.js` — 按键处理器（短按/长按检测，生成 MSG_KEY_EVENT）
- `js/renderer.js` — Canvas渲染器（加载 fourSave.json，绘制控件，更新显示）
- `js/json_logic_engine.js` — JSON逻辑引擎（解析JSON → 状态机运转 → 消息分发）

### 步骤2：设计新JSON逻辑格式 V4.0

核心设计原则：
- 消息驱动：事件的 trigger 是消息ID，动作通过 post 消息执行
- 头尾分离：全局状态机（开关机/待机）+ 炉头模板（闲置/选中/工作/Boost/定时/Pause）
- 独立进程：定时器、Boost计时、暂停控制作为可复用进程

格式规范：`sim/test_hmi/spec/logic_json_v4_schema.json`

### 步骤3：编写四头电磁炉逻辑JSON

`sim/test_hmi/logic/four_head_v4.json`：
- 上电序列 → 待机（-- 闪烁）
- 长按开关 → 开机（0档，炉头灯全亮）
- 按炉头键 → 选中炉头（闪烁）
- 按0-9 → 设置功率 → 工作
- 按定时 → 定时设置
- 长按9 → Boost（显示P，5分钟超时）
- 按暂停 → 暂停（显示PA）
- 长按童锁 → 锁键
- 长按开关 → 关机

### 步骤4：测试命令验证

测试命令面板支持：
```
key <head> <key_name> <event>   → 注入 MSG_KEY_EVENT
state [head]                    → 查看当前状态
display [head]                  → 查看显示缓存
msg <msg_id> <param>            → 直接注入消息
led                             → 查看LED状态
```

先用命令跑通全部状态转移路径，用户确认逻辑后，再启用 Canvas 点击交互。

### 步骤5：用户确认 + Canvas 交互

用测试命令验证逻辑正确后，启用 Canvas 点击模式，通过 `KeyHandler → MessageBus → LogicEngine` 完成端到端交互。

### 步骤6（后续）：移植到嵌入式C

- 逻辑JSON → C常量表（离线编译工具）
- JS MessageBus → 已有的 `msg_scheduler.c`
- JS JsonLogicEngine → C状态机解释器

## 文件清单

| 文件 | 说明 |
|------|------|
| `sim/test_hmi/test_hmi.html` | 主测试页面 |
| `sim/test_hmi/js/message_bus.js` | JS版消息调度器 |
| `sim/test_hmi/js/key_handler.js` | 按键处理器 |
| `sim/test_hmi/js/renderer.js` | Canvas渲染器 |
| `sim/test_hmi/js/json_logic_engine.js` | JSON逻辑引擎 |
| `sim/test_hmi/spec/logic_json_v4_schema.json` | JSON格式规范 |
| `sim/test_hmi/logic/four_head_v4.json` | 四头电磁炉逻辑规则 |

## 验证

1. 启动 HTTP 服务器，浏览器打开 `test_hmi.html`
2. 页面渲染 fourSave.json 的控件布局
3. 测试命令 `key 0 POWER long` → 状态变为 POWER_ON
4. 测试命令 `key 1 HEAD_1 short` → 炉头1选中闪烁
5. 测试命令 `key 1 5 short` → 炉头1显示5档，进入工作态
6. 遍历全部状态转移路径
