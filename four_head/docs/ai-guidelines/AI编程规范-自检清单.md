# AI 编程规范与自检清单

> 本文档供 AI 生成/检查代码时使用。规则按编译阻断级别分级。

---

## 一、命名规范

### 1.1 文件命名

```
<layer>_<submodule>.h/.c

层前缀:
  hal_   硬件抽象层    hal_gpio.c   hal_uart.c   hal_timer.c
  drv_   驱动层        drv_display.c  drv_touch.c  drv_io.c
  proto_ 协议层        proto_modbus.c
  api_   功能接口层    api_display.c  api_key.c  api_comm.c  api_buzzer.c  api_io.c
  app_   应用层        app_ui.c  app_cook.c  app_power_ctrl.c  app_temp_ctrl.c
                      app_protect.c  app_actuator.c
```

### 1.2 函数命名

```
<layer>_<action>_from_<source>()

规则:
  - 用 _from_ 注明底层来源
  - 动作动词前缀: init / get / set / send / recv / ctrl / check

示例:
  api_key_get_from_touch()          // API层按键获取，来源触摸库
  api_comm_send_from_uart1()        // API层通信发送，来源UART1
  drv_display_set_segment()         // Drv层设置段码
  app_cook_ctrl_from_ui()           // APP层烹饪控制，来自UI
  proto_modbus_decode_from_uart()   // PROTO层MODBUS解码，来自UART
```

### 1.3 类型与宏命名

```
类型:  大驼峰 + _t     HeadState_t   PowerCtrl_t   ModbusData_t
枚举值: 全大写+下划线   MSG_KEY_EVENT   COOK_CMD_START
宏:    全大写+下划线   MSG_SLOT_DEPTH   MAX_HANDLERS
局部变量: 小驼峰         headCount     powerWatt
模块级static: s_前缀    s_headState   s_msgSlot
```

---

## 二、分层与include规则（阻断级）

```
┌──────┬────────────────────┬──────────────────────────────────────┐
│ 层    │ 可 include          │ 禁止 include                         │
├──────┼────────────────────┼──────────────────────────────────────┤
│ APP   │ msg_def.h           │ 任何 hal_  drv_  proto_  api_        │
│       │ 本模块自己的.h      │ 其他 app_ 模块的 .h                   │
├──────┼────────────────────┼──────────────────────────────────────┤
│ API   │ msg_def.h           │ 任何 app_                            │
│       │ drv_*.h  proto_*.h  │ 其他 api_ 模块的 .h                  │
├──────┼────────────────────┼──────────────────────────────────────┤
│ PROTO │ hal_*.h  drv_*.h    │ msg_def.h(禁止)  app_  api_          │
├──────┼────────────────────┼──────────────────────────────────────┤
│ DRV   │ hal_*.h             │ msg_def.h(禁止)  proto_  api_  app_  │
├──────┼────────────────────┼──────────────────────────────────────┤
│ HAL   │ MCU寄存器头文件     │ 所有上层                              │
├──────┼────────────────────┼──────────────────────────────────────┤
│ CORE  │ 无                  │ 任何模块                              │
└──────┴────────────────────┴──────────────────────────────────────┘

编译期分层检查:
  #ifdef MSG_DEF_H
  #error "Drv/Proto层禁止包含msg_def.h"
  #endif
```

---

## 三、类型系统（阻断级）

### 3.1 强制规则

- 只用 `<stdint.h>` 类型：`uint8_t` `uint16_t` `uint32_t` `int16_t`
- **禁用** `int` `char` `short` `long`
- **禁用** `float` `double`
- 按范围选最小类型：0-255→uint8_t，256-65535→uint16_t
- M0平台：优先uint16_t，uint32_t仅在必须时使用

### 3.2 定点数（替代浮点）

```c
// 温度：int16_t，单位 0.01°C，范围 -327.68 ~ 327.67°C
int16_t temperature = 2530;   // 表示 25.30°C

// 功率：uint16_t，单位 W
uint16_t power = 2000;        // 表示 2000W

// 电流：uint16_t，单位 mA
uint16_t current = 8500;      // 表示 8.5A

// 电压：uint16_t，单位 10mV
uint16_t voltage = 2200;      // 表示 220.0V

// ★ 定点数必须在声明/定义处注释单位和精度
```

### 3.3 布尔值

```c
// 禁止单独 bool 变量，集中到位域
typedef struct {
    uint8_t active       : 1;
    uint8_t timer_enabled: 1;
    uint8_t boost_active : 1;
    uint8_t reserved     : 5;   // 未使用位显式命名
} HeadFlags_t;

// 必须验证大小
_Static_assert(sizeof(HeadFlags_t) == 1, "HeadFlags_t size error");
```

---

## 四、结构体对齐

```c
// 通信消息结构体 —— 强制 pack(1)
#pragma pack(1)
typedef struct { ... } ModbusData_t;
#pragma pack()

// 内部运行时结构体 —— pack(4)
#pragma pack(4)
typedef struct { ... } HeadState_t;
#pragma pack()

// 所有关键结构体必须验证大小和偏移
_Static_assert(sizeof(ModbusData_t) == EXPECTED, "ModbusData_t size mismatch");
_Static_assert(offsetof(ModbusData_t, field) == OFFSET, "ModbusData_t offset error");
```

---

## 五、消息机制规范

### 5.1 Msg_t 是唯一通道

```c
// msg_def.h —— 唯一共享头文件
typedef enum { MSG_KEY_EVENT, MSG_POWER_CTRL, ... MSG_COUNT } MsgId_t;

typedef void (*MsgHandler_t)(MsgId_t id, uint16_t param, void *data_ptr);

typedef struct {
    MsgId_t id;
    uint16_t param;      // 简单数值（键码、档位），复杂数据不用此字段
    void   *data_ptr;    // 指向模块自定义结构体，调度器不解包
} Msg_t;
```

### 5.2 消息发送（APP/API层）

```c
// 简单消息（只传param）
Msg_Post(MSG_KEY_EVENT, keyCode, NULL);

// 复杂消息（传data_ptr）
PowerCtrl_t ctrl = { .head_id = 0, .power_watt = 2000, .ramp = 0 };
Msg_Post(MSG_POWER_CTRL, 0, &ctrl);
//                         ↑ param不用则填0
```

### 5.3 回调注册（只能在本模块Init中）

```c
void App_Cook_Init(void) {
    MsgScheduler_Register(MSG_COOKING_CTRL, App_Cook_OnCtrl);
    MsgScheduler_Register(MSG_TIMER_1S,     App_Cook_OnTimer);
    MsgScheduler_Register(MSG_COMM_DATA_UPDATE, App_Cook_OnData);
}
```

### 5.4 回调函数内禁止阻塞和耗时操作

```c
// 正确：只做数据更新+发消息，快速返回
static void App_Cook_OnCtrl(MsgId_t id, uint16_t param, void *data_ptr) {
    CookCtrl_t *ctrl = (CookCtrl_t *)data_ptr;
    if (ctrl == NULL) return;                    // 先检查指针
    s_heads[ctrl->head_id].active = 1;           // 更新状态
    Msg_Post(MSG_POWER_CTRL, 0, &powerCtrl);    // 发消息
}

// 错误：阻塞循环、长时间计算、嵌套消息风暴
```

---

## 六、阻塞与等待（阻断级）

- **禁止** `while(xxx){}` 无退出条件的循环
- 所有循环必须有：**超时退出 + 最大重试次数**
- 延时用状态机，不用 `delay_ms()` 类函数

```c
// 正确：状态机延时
static uint32_t s_timeout_ms;
if (timer_expired(s_timeout_ms, 100)) {  // 100ms超时
    state = NEXT;
}

// 错误：
delay_ms(100);     // 阻塞延时
while(!flag) {}    // 无超时死循环
for(i=0;i<100000;i++) {}  // 软件延时
```

---

## 七、输入确认与数据有效性

### 7.1 外部输入

- 连续采样 ≥3次，≥2次一致才算有效
- 数据变化后才开始确认流程
- 适用：按键、ADC采样、MODBUS回读

```c
// 按键去抖示例
static uint8_t key_samples[3];
static uint8_t key_idx = 0;
key_samples[key_idx++ % 3] = HAL_Key_Read();
// 3次中 ≥2次相同 → 确认该键值
```

### 7.2 数据使用前检查

每次使用数据前检查：
1. 指针非空
2. 数值在合法范围内
3. 通讯数据 CRC 正确
4. 时间戳未过期（如适用）

```c
static void OnData(MsgId_t id, uint16_t param, void *data_ptr) {
    ModbusData_t *d = (ModbusData_t *)data_ptr;
    if (d == NULL) return;                        // 1. 指针
    if (d->head_id >= 4) return;                  // 2. 范围
    if (d->commStatus != 0) return;               // 3. 通讯状态
    // 使用数据...
}
```

---

## 八、错误处理

每条错误路径必须明确处理，禁止吞掉错误：

| 错误类型 | 处理路径 |
|---|---|
| 通讯超时 | 重试3次 → 降级（停止加热） → 报警（MSG_SYSTEM_ERROR） |
| CRC校验错 | 丢弃当前帧 → commStatus=2 → 等待下一帧 |
| 传感器超限 | 连续确认≥2次 → 紧急停机 → 报警 |
| 参数越界 | 钳位到合法范围 → 不执行 |
| 指针为空 | 直接 return（消息回调中） |

```c
// 错误处理有4条路径之一：重试 / 降级 / 复位 / 报警
```

---

## 九、编译期验证清单

每个模块的 .c 文件末尾或头文件中必须包含：

```c
// 1. 结构体大小验证
_Static_assert(sizeof(ModbusData_t) == 11, "ModbusData_t size");

// 2. 关键偏移验证
_Static_assert(offsetof(ModbusData_t, faultCode) == 9, "faultCode offset");

// 3. 位域大小验证
_Static_assert(sizeof(HeadFlags_t) == 1, "flags too large");

// 4. 数组大小验证
_Static_assert(MSG_SLOT_DEPTH <= 8, "slot too deep");

// 5. 分层检查（Drv/Proto 文件中）
#ifdef MSG_DEF_H
#error "This layer must not include msg_def.h"
#endif
```

---

## 十、Python测试接口约定

### 10.1 测试即契约

每个模块的 Python 测试文件对应 `interface_registry.h` 中的契约行。
测试用例从契约表自动推导：该模块收到消息X → 应发出消息Y。

### 10.2 测试文件命名

```
test_<layer>_<module>.py
例如: test_app_cook.py   test_app_protect.py   test_proto_modbus.py
```

### 10.3 测试三要素

```python
def test_cook_start_sends_power():
    # 1. 注入输入消息
    inject_msg(MSG_COOKING_CTRL, param=0, data_ptr=CookCtrl_t(head_id=0, cmd=1))
    
    # 2. 运行被测模块
    run_module("app_cook")
    
    # 3. 验证输出消息
    output = capture_msg(MSG_POWER_CTRL)
    assert output.data_ptr.head_id == 0
    assert output.data_ptr.power_watt == 2000   # 菜谱第一步功率
```

---

## 十一、AI 自检清单（每次生成代码后逐条验证）

### 编译前检查

- [ ] 每个 .c 的 `#include` 列表符合分层规则
- [ ] APP 层 .c 不包含 hal_/drv_/proto_/api_ 头文件（msg_def.h 除外）
- [ ] DRV/PROTO 层 .c 不包含 msg_def.h
- [ ] 没有使用 `int` `char` `float` `double`
- [ ] 类型宽度选择符合数值范围
- [ ] 定点数附带了单位注释
- [ ] 结构体有 `#pragma pack` 和 `_Static_assert`
- [ ] 位域用 `uint8_t`，有 `reserved` 位

### 逻辑检查

- [ ] 所有循环有超时退出条件
- [ ] 没有阻塞延时（delay_ms / 空循环）
- [ ] 外部输入有≥3次采样确认
- [ ] 数据使用前检查了指针/范围/CRC
- [ ] 所有错误分支有明确处理（重试/降级/复位/报警）
- [ ] 回调函数内无耗时操作，无嵌套消息风暴
- [ ] 函数命名用 `_from_` 注明了底层来源
- [ ] 模块间只通过 Msg_Post / Register 通信，无直接调用

### 契约检查

- [ ] interface_registry.h 中有对应契约行
- [ ] 实际 Msg_Post 的发送方与契约一致
- [ ] 实际 MsgScheduler_Register 的接收方与契约一致
- [ ] data_ptr 指向的类型与契约声明的类型一致
