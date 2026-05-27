# DDD 通用语言（Ubiquitous Language）

> 本文档定义项目中所有角色（技术负责人、程序员、测试员、领域专家）沟通时使用的统一术语。
> 所有人说同一个词时，指向同一个概念。新增术语须在此文档注册。

---

## 一、限界上下文 (Bounded Context)

| 上下文 | 英文名 | 职责 | 核心聚合 |
|--------|--------|------|----------|
| 烹饪控制 | Cooking | 炉头状态、定时、档位、模式管理 | CookingZone |
| 功率控制 | PowerCtrl | 功率爬坡/缓降/下发指令 | PowerRamp |
| 温度控制 | TempCtrl | NTC回读、PID计算、功率修正 | TempLoop |
| 安全保护 | Protection | 过温/过流/故障检测、紧急停机 | SafetyTrip |
| 人机交互 | HMI | 显示、按键、蜂鸣、UI状态机 | DisplayPage |
| 通讯网关 | CommGateway | MODBUS轮询、编解码、帧管理 | ModbusFrame |

## 二、聚合 (Aggregate)

| 聚合 | 英文 | 说明 |
|------|------|------|
| 炉头 | CookingZone | 1-4号炉头，各自独立的状态机 |
| 功率斜坡 | PowerRamp | 目标功率达到前按斜率逼近 |
| 温度回路 | TempLoop | 一个炉头对应一个PID控制回路 |
| 安全脱扣 | SafetyTrip | 故障条件→紧急指令的判决链路 |
| 显示页面 | DisplayPage | 数码管+LED的组合状态 |
| 通讯帧 | ModbusFrame | MODBUS请求/响应的最小完整单元 |
| 按键事件 | KeyEvent | 触摸芯片输出的去抖后键码 |
| 调度时隙 | ExecSlot | 10槽轮转中一个模块的执行机会 |

## 三、实体 (Entity)

| 实体 | 英文 | 标识方式 | 生命周期 |
|------|------|----------|----------|
| 炉头 | CookingZone | zone_id (1-4) | 上电初始化 → 待机 → 工作中 → 关机 |
| 选中炉头 | SelectedHead | zone_id（最多1个） | 按炉头键进入选中闪烁 → 15s确认 / 切换炉头 / 自按确认 |
| 热点炉头 | HotHead | zone_id 或 -1（清空） | LED 档位灯的唯一数据源。选中炉头一定是热点炉头。若 hotHead=-1 表示热点炉头已清空，系统处于待机，档位 LED 全灭。0 功率（power=0 但曾有过非 0 输出）的炉头仍是热点炉头（LED 亮 0 档灯），与待机（hotHead=-1，LED 全灭）有本质区别。hotHead 只有确认后（15s 超时/自按/切换）且栈空时才清空。 |
| 定时器 | CookingTimer | zone_id | 用户设置 → 倒计时 → 归零关机 |
| UI状态机 | UIStateMachine | 单例 | 上电 → 全显 → 版本 → 关机 → 工作 → 关机 |

## 四、值对象 (Value Object)

| 值对象 | 英文 | 字段 | 不可变性 |
|--------|------|------|----------|
| 档位 | PowerLevel | level: 0-9, is_boost: bool | 是 |
| 温度 | Temperature | raw_adc: u16, deg_c: i16 | 是 |
| 功率 | Power | watt: u16 | 是 |
| 时间间隔 | Duration | ms: u32 | 是 |
| 键码 | KeyCode | raw: u8, mapped: enum | 是 |
| 段码 | SegCode | bitmap: u8[11] | 是 |
| 通讯地址 | ModbusAddr | slave_id: u8, register: u16 | 否（轮询指针可推进） |
| 消息 | Msg | id: MsgId, param: u16, data_ptr: void* | 是（投递后不可改） |

## 五、命令 (Command) — "做XX"

| 命令 | 英文 | 触发者 | 执行者 |
|------|------|--------|--------|
| 开机 | PowerOn | 电源键 | UI → Cooking |
| 关机 | PowerOff | 电源键 / 定时归零 / 保护 | Cooking |
| 设档 | SetPowerLevel | 档位键 0-9 | Cooking |
| 设定时 | SetTimer | 定时键 + +/- | Cooking |
| 加热开 | HeaterOn | Cooking / PowerCtrl | Actuator |
| 加热关 | HeaterOff | Cooking / Protection | Actuator |
| 风机开 | FanOn | Cooking | Actuator |
| 风机关 | FanOff | Cooking（延时） | Actuator |
| 蜂鸣 | Beep | 任意按键 | Buzzer |
| 刷新显示 | RefreshDisplay | UI | Display |
| 轮询 | PollSlave | CommGateway (100ms) | Modbus |
| 急停 | EmergencyStop | Protection | Actuator（全炉头断电） |

## 六、领域事件 (Domain Event) — "XX发生了"

| 事件 | 英文 | 发布者 | 订阅者 |
|------|------|--------|--------|
| 按键按下 | KeyPressed | Key Driver | UI / Cooking |
| 按键长按 | KeyLongPress | Key Driver | Cooking (Boost) |
| 炉头选中 | ZoneSelected | UI | Display (高亮) |
| 档位变更 | PowerLevelChanged | Cooking | Display / Comm (下发) |
| 温度到达 | TemperatureReached | TempCtrl | Cooking (恒温) |
| 定时归零 | TimerExpired | Cooking Timer | Cooking (关机) |
| 功率下发 | PowerDispatched | PowerCtrl | Comm (封帧发送) |
| 温度回读 | TemperatureReadBack | Comm | TempCtrl |
| 故障检测 | FaultDetected | Protection | Cooking (停机) + Display (故障码) |
| 故障清除 | FaultCleared | Protection | Cooking (允许恢复) |
| 帧接收 | FrameReceived | Comm | Cooking / TempCtrl (数据分发) |
| 帧发送完成 | FrameSent | Comm | Comm (推进轮询) |
| 通讯超时 | CommTimeout | Comm | Protection |

## 七、系统分层术语

| 中文 | 英文 | 定位 |
|------|------|------|
| HAL 层 | Hardware Abstraction Layer | 寄存器操作、外设驱动（不知上层存在） |
| DRV 层 | Driver Layer | 设备封装、ISR 衔接（只知 HAL + 消息调度器） |
| PROTO 层 | Protocol Layer | 通讯协议编解码 |
| API 层 | API / Facade Layer | 对外接口、缓冲管理、逻辑抽象 |
| APP 层 | Application Layer | 业务逻辑（只通过消息调度器通信） |
| CORE 层 | Core / Message Bus | 消息定义、调度、队列 |
| 消息调度器 | MsgScheduler | 环形队列 + 回调注册 + 逐条投递 |
| 消息 | Msg | 16字节结构化数据包 |
| 回调 | Handler | 注册到调度器的消息处理函数 |
| 队列 | Queue | 环形缓冲，深度8 |
| 时隙 | Slot | 10槽轮转，每槽1ms，每模块每10ms执行1次 |

## 八、硬件领域术语

| 中文 | 英文 | 说明 |
|------|------|------|
| 炉头 | Head / Zone | 1-4号加热区 |
| 档位 | Gear / PowerLevel | 0-9 档 + Boost(P) |
| 数码管 | 7-Seg Display | 两个4位实体数码管，物理上下排列，8段×11COM，IO直推 |
| 顶屏 | Top Panel | 上方的4位数码管整体（物理实体，不是显示区） |
| 底屏 | Bottom Panel | 下方的4位数码管整体（同上） |
| 显示区 | Display Slot | 映射到某个炉头的2位数码管区域，英文 Zone N Slot，如 Z1 Slot |
| COM | Common cathode | 数码管公共扫描极，共11个 |
| SEG | Segment | 数码管段选线（A-H），共8个 |
| 偷亮 | Ghosting | COM切换时序不当导致的残影 |
| 触摸通道 | Touch Channel | ch7-24, ch28-30 |
| 蜂鸣器 | Buzzer | TIM1 PWM 驱动 |
| 时基 | Time Base | TIM0 125us ISR → 1ms 标志 |
| DMA | Direct Memory Access | UART 收发使用 DMA0/1 |
| 帧间隔 | Frame Gap | 20ms（2次×10ms轮询）判定 MODBUS 帧结束 |
| MODBUS RTU | MODBUS RTU | Master轮询，4从机 |

### 8.1 显示布局（炉头→显示区映射）

```
         ┌──────────────┬──────────────┐
         │    Z1 Slot    │    Z4 Slot    │
  Top    │   (2位数码管)  │   (2位数码管)  │
  Panel  │              │              │
         ├──────────────┼──────────────┤
         │    Z2 Slot    │    Z3 Slot    │
  Bottom │   (2位数码管)  │   (2位数码管)  │
  Panel  │              │              │
         └──────────────┴──────────────┘
```

| 术语 | 英文 | 含义 | 使用场景 |
|------|------|------|----------|
| Z{N} Slot | Zone N Display Slot | 映射到第N号炉头的2位数码管区域 | 日常讨论：更新档位、显示定时、闪烁控制 |
| Top Panel | Top Panel | 上方完整4位数码管物理实体 (Z1+Z2) | 全显测试、版本号 "V1.0" |
| Bottom Panel | Bottom Panel | 下方完整4位数码管物理实体 (Z3+Z4) | 全显测试、版本号 "P1.0" |

**规则**:
- 日常一律用 "Z1 Slot" / "Z2 Slot" / "Z3 Slot" / "Z4 Slot"
- 只有需要把4位当整体使用时才用 "Top Panel" / "Bottom Panel"
- 禁止使用"上数码管/下数码管/左数码管/右数码管"——物理上两屏是上下排列，数据映射按 Slot
- **硬件IO映射** (COM1-8): IO[0,1]=Z1(上左) IO[2,3]=Z2(上右) IO[4,5]=Z3(下左) IO[6,7]=Z4(下右), 来源 参考程序 Disp_data_Exchange_Hardware()
- **DP 点 (小数点)**: 每个 2 位数码管 Slot 下方有两个小数点 (DP)，作为**热点炉头指示器**。当前热点炉头的 Slot 两个 DP 常亮（不闪烁），非热点炉头的 Slot DP 灭。热点炉头队列更新时 DP 跟随更新。
  - 选中炉头 (selecting) 时：数字闪烁，DP **保持常亮**（不跟随闪烁相位）
  - 非热点炉头 Slot：DP 灭

## 九、过程术语

| 中文 | 英文 | 说明 |
|------|------|------|
| 消息投递 | Msg_Post | 将消息写入环形队列 |
| 消息消费 | Consume | 调度器从队列取出一条消息并调用回调 |
| 注册回调 | Register | 绑定 MsgId → Handler 函数 |
| 轮转 | Rotate | 10个槽按1ms节拍依次执行 |
| 爬坡 | Ramp Up | 功率从当前值按斜率升至目标值 |
| 缓降 | Ramp Down | 功率从当前值按斜率降至目标值 |
| 全显 | All-On Test | 上电后所有段全亮3秒 |
| 去抖 | Debounce | 按键信号稳定后才判定有效 |
| 过零 | Zero Cross | 加热IGBT的过零检测（本项目未实现） |

## 十一、系统工作状态 (System States)

> 状态按系统全局和单个炉头分开。全局状态描述整机当前在哪个阶段，
> 炉头状态描述每个 Zone 自己的运行态。两者同时存在、互不覆盖。

### 11.1 全局状态 (System State)

系统全局只有一个当前状态。枚举值：

| 状态 | 英文 | 进入条件 | 显示特征 | 可用操作 |
|------|------|----------|----------|----------|
| 上电 | `POWERING_UP` | MCU 上电复位 | 全显3s（所有段+LED全亮） | 无，自动执行 |
| 版本显示 | `VERSION_SHOW` | 全显3s结束 | Top Panel "V1.0", Bottom Panel "P1.0" | 无，持续3s后自动转入 `POWERED_OFF` |
| 关机 | `POWERED_OFF` | 版本显示结束 / 任何状态长按开关 | "--" 不闪烁，电源灯闪烁 | 仅长按开关 1.5s |
| 工作 | `WORKING` | `POWERED_OFF` 下长按开关 1.5s | 各炉头按自身状态独立显示，LED 跟热点炉头 | 全操作可用 |
| 暂停 | `PAUSED` | WORKING 态按暂停键 | 工作 Zone 显示 "PA" | 按暂停恢复 / 长按开关关机 |
| 休眠 | `DEEP_SLEEP` | `POWERED_OFF` 下 30s 无操作 | 全灭 | 仅长按开关 1.5s → WORKING |

- **WORKING**：取代旧术语"待机"。系统可操控的总称，包含所有 Zone 的 idle/selecting/cooking 状态。
- **0 功率**（0-power）：一个炉头 power_level=0 且 node=cooking（或 idle）的状态。0 功率是合法工作模式，只是不输出功率。LED 档位灯亮 0 档。
- **PAUSED = 冻结**：不改任何炉头状态/进程。功率输出为 0。CHILD_LOCK = 按键过滤器，不改模式。
- **不存在"待机"这个全局状态**。旧称"待机"的情况 = WORKING 下四头全部 idle（0 功率）。显示 "0 0 0 0"。

**状态流转（主路径）**：

```
POWERING_UP(全显3s) → VERSION_SHOW(3s) → POWERED_OFF ←→ DEEP_SLEEP
                                              │              ↑
                                   长按开关 1.5s ↓              │30s无操作
                                              │              │
                                           WORKING ←──────────┘
                                              ↑↓ 暂停键
                                           PAUSED
```

### 11.2 炉头状态 (Zone State)

每个 Zone(1-4) 独立维护自己的 node。仅 3 个状态：

| 状态 | 英文 | Z{N} Slot 显示 | seg_blink | 触发条件 | 超时 |
|------|------|----------------|-----------|----------|------|
| 空闲 | `idle` | "0" 常亮 | false | WORKING 进入默认 / cooking 下设 0 档并确认 | — |
| 选中闪烁 | `selecting` | 当前档位数字闪烁（含 0） | **仅本炉头** true | 按对应 Zone 键 | **15s** 无操作 → 自动确认 |
| 加热 | `cooking` | 档位数字常亮 | false | selecting 确认时 power_level > 0 / 单头快捷设档 | — |

**关键区分**：
- `selecting` = 炉头被选中，Z{N} Slot **在闪烁**。此期间该炉头必为热点炉头，LED 档位灯跟随它（含 0 档）。
- `cooking` = 炉头在工作（可能 0 功率也可能非 0 功率），Z{N} Slot **常亮不闪烁**。
- `idle` = 炉头空闲且未被选中。
- 0 功率的炉头**只有在确认后才退出热点炉头位置**。在 selecting 态下即使设了 0 档，仍是热点炉头。

### 11.3 正交进程（叠加在 Zone 状态上）

进程不改变 node，是独立标志 + 生命周期：

| 进程 | 英文 | 显示效果 | 进入条件 | 终止条件 |
|------|------|----------|----------|----------|
| 定时设置 | `timer_setting` | 显示时间数字（替代档位） | selecting/cooking 态按定时键 | 15s 超时 / 按定时键确认 / 长按定时键取消 |
| 定时运行 | `timer_active` | 交替显示：档位/时间各 5s | timer_setting 确认 | 倒计时归零 → idle / 长按定时键取消 |
| 强火 | `boost_active` | 显示 "P"（替代档位数字） | selecting/cooking 态长按 9 | 5min 超时 / 按任意档位键 |

### 11.4 双层状态示例

```
系统=POWERED_OFF  →  4 个 Zone 全部不可操作
系统=WORKING      →  Zone1=idle   Zone2=idle   Zone3=idle   Zone4=idle   (显示 "0 0 0 0")
按 Zone1 键       →  Zone1=selecting(0闪烁), hotHead=1, LED档位灯=0
设档 5            →  Zone1=selecting(5闪烁, 重置15s), hotHead=1, LED=5
15s 确认          →  Zone1=cooking(显示5), LED=5(栈顶=炉头1)
按 Zone2 键       →  Zone2=selecting(0闪烁), hotHead=2, LED=0
设档 9            →  Zone2=selecting(9闪烁), hotHead=2, LED=9
长按 9            →  Zone2=selecting + boost_active, 显示"P"闪烁, LED=9-P
按 Zone2 自确认   →  Zone2=cooking + boost_active, 显示"P", LED=9-P
暂停键            →  系统=PAUSED, 所有 Zone 冻结, 功率=0
```
> 禁止说 "S0/S1/S2" 等旧编号——状态用英文名，不用数字编号。

## 十、TDD 协作术语

| 中文 | 英文 | 说明 |
|------|------|------|
| 任务卡 | Task Card | 技术负责人分解的最小工作单元 (T001, T002...) |
| 完成标准 | Acceptance Criteria | 技术负责人为每个任务定的客观验收条件 |
| 测试用例 | Test Case | 每条完成标准对应的可执行验证步骤（正向/边界/异常） |
| 编码完成 | Dev Done | 程序员声明代码写好，请求测试员验证 |
| 测试通过 | Test Passed (✅) | 测试员验证全部用例通过，任务可关闭 |
| 测试驳回 | Test Rejected (❌) | 测试员发现问题，程序员修复后重新提交 |
| 循环 | Cycle | 程序员编码 → 测试员验证 → 修复 → 再验证 → 通过 |

---

*维护规则：任何人在讨论中引入新术语时，须在此文件注册。术语变更须技术负责人审批。*
