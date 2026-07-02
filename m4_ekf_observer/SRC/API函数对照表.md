# 半桥与移相全桥API函数对照表

## 说明
本文档列出半桥（Half-Bridge）和移相全桥（Full-Bridge）HRTIM API函数的对应关系，便于后续使用条件编译进行切换。

**使用方式示例：**
```c
#ifdef USE_FULL_BRIDGE
    #include "API_hrtim_fullbridge.h"
    #define API_HRTIM1_Init           API_FB_HRTIM1_Init
    #define API_PPG_setPeriod         API_FB_PPG_setPeriod
    #define API_PPG_setValue          API_FB_PPG_setValue
    // ... 其他映射
#else
    #include "API_hrtim.h"
#endif
```

---

## 一、初始化函数对照

| 功能 | 半桥函数 | 全桥函数 | 说明 |
|------|---------|---------|------|
| HRTIM初始化 | `API_HRTIM1_Init()` | `API_FB_HRTIM1_Init()` | 全桥需配置MASTER+TimerA+TimerC |
| 系统时钟初始化 | `API_SystemClocks_Init()` | `API_FB_SystemClocks_Init()` | 基本相同 |

---

## 二、PPG设置函数对照

### 2.1 模式设置

| 功能 | 半桥函数 | 全桥函数 | 差异说明 |
|------|---------|---------|---------|
| 连续输出模式 | `API_PPG_SET_CONTINUOUS(ch)` | `API_FB_PPG_SET_CONTINUOUS(ch)` | 参数兼容，内部实现不同 |
| 单次脉冲模式 | `API_PPG_SET_SINGLE(ch)` | `API_FB_PPG_SET_SINGLE(ch)` | 参数兼容 |

### 2.2 周期和占空比设置

| 功能 | 半桥函数 | 全桥函数 | 差异说明 |
|------|---------|---------|---------|
| 设置脉冲值 | `API_PPG_setPluse(ch, value)` | `API_FB_PPG_setPluse(ch, value)` | **全桥需同步更新多个定时器** |
| 设置通道周期 | `API_PPG_setPeriodChx(ch, value)` | `API_FB_PPG_setPeriodChx(ch, value)` | **全桥所有通道共享MASTER周期** |
| 设置周期和占空比 | `API_PPG_setValueChx(ch, period, duty)` | `API_FB_PPG_setValueChx(ch, period, duty)` | **全桥需同时设置移相角** |
| 设置周期 | `API_PPG_setPeriod(value)` | `API_FB_PPG_setPeriod(value)` | **全桥更新MASTER周期** |
| 设置死区 | `API_PPG_DeadTime(ch, upDts, downDts)` | `API_FB_PPG_DeadTime(ch, upDts, downDts)` | 参数兼容，ch=0超前臂，ch=1滞后臂 |
| 设置PPG值 | `API_PPG_setValue(ch, PPGvalueDef)` | `API_FB_PPG_setValue(ch, FB_PPGvalueDef)` | **结构体增加phase_shift字段** |

### 2.3 获取函数

| 功能 | 半桥函数 | 全桥函数 | 差异说明 |
|------|---------|---------|---------|
| 获取PPG值 | `API_PPG_getValue(ch)` → `PPGvalueDef` | `API_FB_PPG_getValue(ch)` → `FB_PPGvalueDef` | **返回结构体增加phase_shift** |
| 获取死区后DUTY | `API_PPG_getValueDeadTime(ch)` → `PPGpointDef` | `API_FB_PPG_getValueDeadTime(ch)` → `FB_PPGpointDef` | **结构体增加phaseShiftPoint** |
| 获取周期 | `API_PPG_GetPeroid(ch)` | `API_FB_PPG_GetPeroid(ch)` | 基本相同 |
| 获取死区值 | `API_PPG_getDeadTime(ch)` | `API_FB_PPG_getDeadTime(ch)` | 基本相同 |

### 2.4 开关控制

| 功能 | 半桥函数 | 全桥函数 | 差异说明 |
|------|---------|---------|---------|
| PWM开关 | `API_PPG_OnOff(ch, flag)` | `API_FB_PPG_OnOff(ch, flag)` | **全桥需注意启动顺序：Slave→MASTER** |

---

## 三、移相控制函数（全桥特有）

| 功能 | 半桥 | 全桥函数 | 说明 |
|------|------|---------|------|
| 设置移相角 | ❌ 无 | `API_FB_SetPhaseShift(angle_deg)` | **核心功能**：通过MASTER CMP2控制 |
| 获取移相角 | ❌ 无 | `API_FB_GetPhaseShift()` | 返回当前移相角度 |
| 设置频率 | ❌ 间接 | `API_FB_SetFrequency(freq_hz)` | 便捷接口，自动计算周期 |
| 获取频率 | ❌ 间接 | `API_FB_GetFrequency()` | 返回当前频率 |

---

## 四、故障和保护函数对照

### 4.1 故障标志

| 功能 | 半桥函数 | 全桥函数 | 说明 |
|------|---------|---------|------|
| Pot1故障标志 | `API_PPG_BkFlag_Pot1()` | `API_FB_PPG_BkFlag_Pot1()` | 基本相同 |
| Pot2故障标志 | `API_PPG_BkFlag_Pot2()` | `API_FB_PPG_BkFlag_Pot2()` | 基本相同 |
| Pot3故障标志 | `API_PPG_BkFlag_Pot3()` | `API_FB_PPG_BkFlag_Pot3()` | 基本相同 |
| Pot4故障标志 | `API_PPG_BkFlag_Pot4()` | `API_FB_PPG_BkFlag_Pot4()` | 基本相同 |

### 4.2 COMP选择

| 功能 | 半桥函数 | 全桥函数 | 说明 |
|------|---------|---------|------|
| 检锅模式 | `API_Comp_SetSelPanCheck(ch)` | `API_FB_Comp_SetSelPanCheck(ch)` | 基本相同 |
| 功率输出模式 | `API_Comp_SetSelPowerOn(ch)` | `API_FB_Comp_SetSelPowerOn(ch)` | 基本相同 |

### 4.3 中断控制

| 功能 | 半桥函数 | 全桥函数 | 说明 |
|------|---------|---------|------|
| 禁用REST中断 | `API_HRTIM_DISABLE_IT_REST()` | `API_FB_HRTIM_DISABLE_IT_REST()` | 基本相同 |
| 使能REST中断 | `API_HRTIM_ENABLE_IT_REST()` | `API_FB_HRTIM_ENABLE_IT_REST()` | 基本相同 |
| 禁用BASE UPD中断 | `API_HRTIM_BASE_DISABLE_IT_UPD()` | `API_FB_HRTIM_BASE_DISABLE_IT_UPD()` | 基本相同 |
| 使能BASE UPD中断 | `API_HRTIM_BASE_ENABLE_IT_UPD()` | `API_FB_HRTIM_BASE_ENABLE_IT_UPD()` | 基本相同 |
| 禁用BASE CMP中断 | `API_HRTIM_BASE_DISABLE_IT_CMP()` | `API_FB_HRTIM_BASE_DISABLE_IT_CMP()` | 基本相同 |
| 使能BASE CMP中断 | `API_HRTIM_BASE_ENABLE_IT_CMP()` | `API_FB_HRTIM_BASE_ENABLE_IT_CMP()` | 基本相同 |
| 获取UPD中断标志 | `API_HRTIM_GET_IT_UPD(ch)` | `API_FB_HRTIM_GET_IT_UPD(ch)` | 基本相同 |
| 清除UPD中断标志 | `API_HRTIM_CLEAR_IT_UPD(ch)` | `API_FB_HRTIM_CLEAR_IT_UPD(ch)` | 基本相同 |

### 4.4 计数器读取

| 功能 | 半桥函数 | 全桥函数 | 说明 |
|------|---------|---------|------|
| 获取Test计数 | `API_HRTIM_GetAddressTestCnt()` | `API_FB_HRTIM_GetAddressTestCnt()` | 基本相同 |
| 获取Txa地址计数 | `API_HRTIM_GetAddressTxaCnt(ch)` | `API_FB_HRTIM_GetAddressTxaCnt(ch)` | 基本相同 |
| 获取Txa计数 | `API_HRTIM_GetTxaCnt(ch)` | `API_FB_HRTIM_GetTxaCnt(ch)` | 基本相同 |

### 4.5 同步和清除

| 功能 | 半桥函数 | 全桥函数 | 说明 |
|------|---------|---------|------|
| 检锅脉冲检查 | `API_HRTIM_CHECK_PAN_PLUSE(ch)` | `API_FB_HRTIM_CHECK_PAN_PLUSE(ch)` | 基本相同 |
| HRTIM同步 | `TIMsynchronous()` | `API_FB_TIMsynchronous()` | **全桥需确保MASTER同步** |
| 功率同步 | `TIMsynchronousPower()` | `API_FB_TIMsynchronousPower()` | **全桥需确保MASTER同步** |
| 清除CMP2标志 | `API_HRTIM_PAN_CLEAR_FLAG(ch)` | `API_FB_HRTIM_PAN_CLEAR_FLAG(ch)` | 基本相同 |
| 设置DMA句柄 | `API_HRTIM_SetDmaHandle(addr)` | `API_FB_HRTIM_SetDmaHandle(addr)` | 基本相同 |

---

## 五、全桥特有高级功能

| 功能 | 全桥函数 | 说明 |
|------|---------|------|
| 电流窗口判断 | `API_FB_GetCurrentWindow(...)` | **核心功能**：判断Q1+Q4或Q2+Q3导通窗口 |
| 软启动 | `API_FB_SoftStart(freq, duty, step_ms)` | 逐步增加频率和占空比 |
| 软停止 | `API_FB_SoftStop(step_ms)` | 逐步降低后停止 |
| 故障恢复 | `API_FB_FaultRecovery()` | 清除故障并重新初始化 |
| 获取DMA状态 | `API_FB_GetHrtimStatusForDMA(status)` | 提供HRTIM时间戳给ADC DMA |
| 获取运行状态 | `API_FB_GetStatus(status)` | 获取完整运行状态 |
| 中断回调 | `API_FB_IRQHandler_Callback(timer_id, flag)` | 弱定义，用户可重写 |

---

## 六、数据结构对照

### 6.1 PPG值结构体

**半桥：**
```c
typedef struct {
    uint16_t prioed;    // PPG周期
    uint16_t duty;      // PPG占空比
} PPGvalueDef;
```

**全桥：**
```c
typedef struct {
    uint16_t period;        // PPG周期（对应prioed）
    uint16_t duty;          // PPG占空比
    uint16_t phase_shift;   // 移相角（新增）
} FB_PPGvalueDef;
```

### 6.2 PPG时间点结构体

**半桥：**
```c
typedef struct {
    uint16_t highOn;    // 死区后高端开通
    uint16_t highOff;   // 高端关闭
    uint16_t lowOn;     // 死区后低端开通
    uint16_t lowOff;    // 低端关闭
} PPGpointDef;
```

**全桥：**
```c
typedef struct {
    uint16_t highOn;            // 死区后高端开通
    uint16_t highOff;           // 高端关闭
    uint16_t lowOn;             // 死区后低端开通
    uint16_t lowOff;            // 低端关闭
    uint16_t phaseShiftPoint;   // 移相点（新增）
} FB_PPGpointDef;
```

---

## 七、关键差异总结

### 🔴 必须注意的差异

1. **时钟源不同**
   - 半桥：各定时器独立时钟
   - 全桥：MASTER统一时钟，Slave同步

2. **启动/停止顺序**
   - 半桥：可独立启动各定时器
   - 全桥：启动时先Slave后MASTER，停止时先MASTER后Slave

3. **周期同步**
   - 半桥：各定时器周期可独立设置
   - 全桥：所有定时器周期必须一致（由MASTER决定）

4. **占空比设置**
   - 半桥：单通道独立设置
   - 全桥：需同时设置TimerA和TimerC的CMP2

5. **移相控制**
   - 半桥：无此功能
   - 全桥：通过MASTER CMP2控制TimerC复位点

6. **电流采集窗口**
   - 半桥：单管导通窗口
   - 全桥：对角管同时导通窗口（Q1+Q4或Q2+Q3）

### 🟡 兼容性设计

为了便于条件编译切换，全桥API设计遵循以下原则：

1. **函数名对应**：所有半桥函数都有对应的全桥版本（加`FB_`前缀）
2. **参数兼容**：大部分函数参数保持一致
3. **结构体扩展**：全桥结构体在半桥基础上增加字段
4. **新增功能独立**：全桥特有功能使用新函数名，不影响半桥

---

## 八、条件编译示例

### 8.1 头文件包含

```c
#ifdef USE_FULL_BRIDGE
    #include "API_hrtim_fullbridge.h"
    
    // 函数名映射
    #define API_HRTIM1_Init               API_FB_HRTIM1_Init
    #define API_SystemClocks_Init         API_FB_SystemClocks_Init
    #define API_PPG_SET_CONTINUOUS        API_FB_PPG_SET_CONTINUOUS
    #define API_PPG_SET_SINGLE            API_FB_PPG_SET_SINGLE
    #define API_PPG_setPluse              API_FB_PPG_setPluse
    #define API_PPG_setPeriodChx          API_FB_PPG_setPeriodChx
    #define API_PPG_setValueChx           API_FB_PPG_setValueChx
    #define API_PPG_setPeriod             API_FB_PPG_setPeriod
    #define API_PPG_DeadTime              API_FB_PPG_DeadTime
    #define API_PPG_setValue              API_FB_PPG_setValue
    #define API_PPG_getValue              API_FB_PPG_getValue
    #define API_PPG_getValueDeadTime      API_FB_PPG_getValueDeadTime
    #define API_PPG_GetPeroid             API_FB_PPG_GetPeroid
    #define API_PPG_getDeadTime           API_FB_PPG_getDeadTime
    #define API_PPG_OnOff                 API_FB_PPG_OnOff
    #define API_PPG_BkFlag_Pot1           API_FB_PPG_BkFlag_Pot1
    #define API_PPG_BkFlag_Pot2           API_FB_PPG_BkFlag_Pot2
    #define API_PPG_BkFlag_Pot3           API_FB_PPG_BkFlag_Pot3
    #define API_PPG_BkFlag_Pot4           API_FB_PPG_BkFlag_Pot4
    #define API_Comp_SetSelPanCheck       API_FB_Comp_SetSelPanCheck
    #define API_Comp_SetSelPowerOn        API_FB_Comp_SetSelPowerOn
    #define API_HRTIM_DISABLE_IT_REST     API_FB_HRTIM_DISABLE_IT_REST
    #define API_HRTIM_ENABLE_IT_REST      API_FB_HRTIM_ENABLE_IT_REST
    #define API_HRTIM_BASE_DISABLE_IT_UPD API_FB_HRTIM_BASE_DISABLE_IT_UPD
    #define API_HRTIM_BASE_ENABLE_IT_UPD  API_FB_HRTIM_BASE_ENABLE_IT_UPD
    #define API_HRTIM_BASE_DISABLE_IT_CMP API_FB_HRTIM_BASE_DISABLE_IT_CMP
    #define API_HRTIM_BASE_ENABLE_IT_CMP  API_FB_HRTIM_BASE_ENABLE_IT_CMP
    #define API_HRTIM_GET_IT_UPD          API_FB_HRTIM_GET_IT_UPD
    #define API_HRTIM_CLEAR_IT_UPD        API_FB_HRTIM_CLEAR_IT_UPD
    #define API_HRTIM_GetAddressTestCnt   API_FB_HRTIM_GetAddressTestCnt
    #define API_HRTIM_GetAddressTxaCnt    API_FB_HRTIM_GetAddressTxaCnt
    #define API_HRTIM_GetTxaCnt           API_FB_HRTIM_GetTxaCnt
    #define API_HRTIM_CHECK_PAN_PLUSE     API_FB_HRTIM_CHECK_PAN_PLUSE
    #define TIMsynchronous                API_FB_TIMsynchronous
    #define TIMsynchronousPower           API_FB_TIMsynchronousPower
    #define API_HRTIM_PAN_CLEAR_FLAG      API_FB_HRTIM_PAN_CLEAR_FLAG
    #define API_HRTIM_SetDmaHandle        API_FB_HRTIM_SetDmaHandle
    
    // 数据类型映射
    #define PPGvalueDef                   FB_PPGvalueDef
    #define PPGpointDef                   FB_PPGpointDef
    
#else
    #include "API_hrtim.h"
#endif
```

### 8.2 使用示例

```c
// 初始化
#ifdef USE_FULL_BRIDGE
    FullBridgeConfigTypeDef fb_config = {
        .switching_freq = 25000,
        .duty_cycle = 30,
        .phase_shift_angle = 90,
        .deadtime_ns = 2000
    };
    API_FB_HRTIM1_Init(&fb_config);
#else
    API_HRTIM1_Init();
#endif

// 设置占空比
#ifdef USE_FULL_BRIDGE
    FB_PPGvalueDef value;
    value.period = API_FB_PPG_GetPeroid(0);
    value.duty = value.period * 30 / 100;
    value.phase_shift = 90;
    API_FB_PPG_setValue(0, value);
#else
    PPGvalueDef value;
    value.prioed = API_PPG_GetPeroid(0);
    value.duty = value.prioed * 30 / 100;
    API_PPG_setValue(0, value);
#endif

// 移相控制（仅全桥）
#ifdef USE_FULL_BRIDGE
    API_FB_SetPhaseShift(90);
#endif
```

---

## 九、后续工作建议

1. ✅ **已完成**：创建`API_hrtim_fullbridge.h`，定义接口框架
2. ⏳ **下一步**：创建`API_hrtim_fullbridge.c`，实现所有函数
3. ⏳ **然后**：创建`power_calculator_fullbridge.h/c`
4. ⏳ **最后**：编写测试代码，分阶段验证功能

**实施策略**：按照用户偏好的"分阶段测试执行策略"，每完成一个模块就进行测试验证。
