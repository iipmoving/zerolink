# HRTIM移相全桥改造详细方案

## 一、项目初始化状态

✅ **已完成**：
1. Git仓库初始化完成
2. .gitignore配置文件已创建
3. 初始版本已提交（commit: "初始版本：半桥感应加热"）
4. MCU规格书位置确认：`Projects/DOC/RX32G410_Reference_Manual_v0p7.pdf`

## 二、当前半桥系统分析

### 2.1 现有HRTIM配置特点

**文件位置**：`Projects/LIB/API/API_hrtim.c`

**关键配置**：
```c
// 时钟频率定义
#define HRTIM_INPUT_CLOCK 192000000  // 192MHz输入时钟

// 预分频配置
HRTIM_PRESCALERRATIO_MUL4  // 4倍频 = 768MHz工作频率

// 定时器分配（6个独立定时器）
Pot1 -> Timer B
Pot2 -> Timer E  
Pot3 -> Timer A
Pot4 -> Timer D
Test1 -> Timer C
Test2 -> Timer F

// 每个定时器独立运行，使用各自的时钟源
// 问题：存在相位偏移，无法精确同步
```

### 2.2 死区时间配置
```c
// 死区预设值
#define DEAD0_TIME  2000  // ns
#define DEAD1_TIME  2000  // ns
#define DEAD0_1_TIME 2000 // ns

// 实际配置结构
HRTIM_DeadTimeCfgTypeDef PPGDeadTimeCfg = {
  .Prescaler = HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV4,
  .RisingValue = 0,   // 需动态计算
  .FallingValue = 0,  // 需动态计算
};
```

### 2.3 电流采集现状

**文件位置**：`Projects/BaseClass/src/power_calculator.c`

**采集方式**：
- ADC + DMA方式采集谐振电流
- HRTIM时间戳通过DMA传递到power_calculator
- 当前半桥只判断单管导通窗口

## 三、移相全桥改造方案

### 3.1 核心改造目标

| 项目 | 半桥（当前） | 移相全桥（目标） |
|------|-------------|-----------------|
| 时钟源 | 各定时器独立 | MASTER统一时钟 |
| PWM路数 | 4路独立输出 | 4路互补输出（2对） |
| 相位关系 | 无固定关系 | Q1/Q2互补，Q3/Q4互补，AC间可移相 |
| 死区控制 | 单通道死区 | 互补对死区 |
| 电流窗口 | 单管导通 | 对角管同时导通 |

### 3.2 MASTER定时器配置详解

#### 3.2.1 MASTER时基配置

```c
// MASTER定时器作为统一时钟基准
HRTIM_TimeBaseCfgTypeDef MasterTimeBaseCfg = {
    .Period = FULLBRIDGE_PERIOD,           // 全桥工作周期
    .RepetitionCounter = 0,                // 不重复
    .PrescalerRatio = HRTIM_PRESCALERRATIO_MUL4,  // 4倍频保持精度
    .Mode = HRTIM_MODE_CONTINUOUS,         // 连续模式
};

// 周期计算示例（假设25kHz开关频率）
// Period = (192MHz * 4) / 25kHz = 30720
#define FULLBRIDGE_FREQ     25000
#define FULLBRIDGE_PERIOD   ((HRTIM_INPUT_CLOCK * 4) / FULLBRIDGE_FREQ)
```

#### 3.2.2 MASTER比较寄存器用于移相控制

```c
// MASTER CMP1：TimerA复位触发点（超前臂基准）
// MASTER CMP2：TimerC复位触发点（滞后臂，可调移相角）
// MASTER CMP3/CMP4：预留其他功能

HRTIM_CompareCfgTypeDef MasterCompareCfg = {
    .CompareValue = 0,  // 动态设置
    .AutoDelayedMode = HRTIM_AUTODELAYEDMODE_REGULAR,
    .AutoDelayedTimeout = 0,
};

// 移相角计算（单位：HRTIM计数值）
// PhaseShift_Count = (PhaseShift_Angle / 360) * Period
// 例如：移相90度 = Period / 4
```

### 3.3 Slave定时器配置（TimerA和TimerC）

#### 3.3.1 TimerA配置（超前臂Q1/Q2）

```c
// 时基配置 - 与MASTER同步
HRTIM_TimeBaseCfgTypeDef TimerATimeBaseCfg = {
    .Period = FULLBRIDGE_PERIOD,
    .RepetitionCounter = 0,
    .PrescalerRatio = HRTIM_PRESCALERRATIO_MUL4,
    .Mode = HRTIM_MODE_CONTINUOUS,
};

// 定时器控制配置 - 关键同步设置
HRTIM_TimerCfgTypeDef TimerACfg = {
    .InterruptRequests = HRTIM_TIM_IT_UPD | HRTIM_TIM_IT_CMP2,
    .DMARequests = HRTIM_TIM_DMA_NONE,
    
    // ★ 关键：使用MASTER更新触发
    .UpdateTrigger = HRTIM_TIMUPDATETRIGGER_MASTER,
    
    // ★ 关键：使用MASTER CMP1复位，实现同步
    .ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_CMP1,
    
    // 使能死区插入
    .DeadTimeInsertion = HRTIM_TIMDEADTIMEINSERTION_ENABLED,
    
    // 故障保护
    .FaultEnable = HRTIM_TIMFAULTENABLE_FAULT1 | HRTIM_TIMFAULTENABLE_FAULT2,
    
    // 其他配置保持默认
    .HalfModeEnable = HRTIM_HALFMODE_DISABLED,
    .PushPull = HRTIM_TIMPUSHPULLMODE_DISABLED,
    .BalancedIdleAutomaticResume = HRTIM_OUTPUTBIAR_DISABLED,
};

// 输出配置 - 互补PWM
HRTIM_OutputCfgTypeDef TimerAOutput1Cfg = {  // TA1 -> Q1
    .Polarity = HRTIM_OUTPUTPOLARITY_HIGH,
    .SetSource = HRTIM_OUTPUTSET_TIMPER,      // 周期开始时置位
    .ResetSource = HRTIM_OUTPUTRESET_TIMCMP2, // CMP2时复位（占空比控制）
    .IdleMode = HRTIM_OUTPUTIDLEMODE_NONE,
    .IdleLevel = HRTIM_OUTPUTIDLELEVEL_INACTIVE,
    .FaultLevel = HRTIM_OUTPUTFAULTLEVEL_INACTIVE,
    .ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED,
};

HRTIM_OutputCfgTypeDef TimerAOutput2Cfg = {  // TA2 -> Q2（互补）
    .Polarity = HRTIM_OUTPUTPOLARITY_HIGH,
    .SetSource = HRTIM_OUTPUTSET_TIMCMP2,     // CMP2时置位（与TA1相反）
    .ResetSource = HRTIM_OUTPUTRESET_TIMPER,  // 周期开始时复位
    .IdleMode = HRTIM_OUTPUTIDLEMODE_NONE,
    .IdleLevel = HRTIM_OUTPUTIDLELEVEL_INACTIVE,
    .FaultLevel = HRTIM_OUTPUTFAULTLEVEL_INACTIVE,
    .ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED,
};
```

#### 3.3.2 TimerC配置（滞后臂Q3/Q4）

```c
// TimerC配置与TimerA类似，关键区别：
HRTIM_TimerCfgTypeDef TimerCCfg = {
    // ... 其他配置相同 ...
    
    // ★ 使用MASTER CMP2复位，实现移相
    .ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_CMP2,
    
    // 故障通道不同
    .FaultEnable = HRTIM_TIMFAULTENABLE_FAULT3 | HRTIM_TIMFAULTENABLE_FAULT4,
};

// 输出配置
HRTIM_OutputCfgTypeDef TimerCOutput1Cfg = {  // TC1 -> Q3
    .Polarity = HRTIM_OUTPUTPOLARITY_HIGH,
    .SetSource = HRTIM_OUTPUTSET_TIMPER,
    .ResetSource = HRTIM_OUTPUTRESET_TIMCMP2,
    // ... 其他相同 ...
};

HRTIM_OutputCfgTypeDef TimerCOutput2Cfg = {  // TC2 -> Q4（互补）
    .Polarity = HRTIM_OUTPUTPOLARITY_HIGH,
    .SetSource = HRTIM_OUTPUTSET_TIMCMP2,
    .ResetSource = HRTIM_OUTPUTRESET_TIMPER,
    // ... 其他相同 ...
};
```

### 3.4 死区时间精确配置

```c
// 死区时间计算公式
// DeadTime_Count = (DeadTime_ns * fHRCK) / 1e9
// 其中 fHRCK = 192MHz * 4 = 768MHz
// DeadTime_Count = (2000ns * 768MHz) / 1e9 = 1536

// 但需要使用合适的预分频器
// 使用DIV4预分频：fDTG = 768MHz / 4 = 192MHz
// DeadTime_Count = (2000ns * 192MHz) / 1e9 = 384

HRTIM_DeadTimeCfgTypeDef FullBridgeDeadTimeCfg = {
    .Prescaler = HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV4,
    .RisingValue = 384,   // 上升沿死区
    .RisingSign = HRTIM_TIMDEADTIME_RISINGSIGN_POSITIVE,
    .RisingLock = HRTIM_TIMDEADTIME_RISINGLOCK_WRITE,
    .RisingSignLock = HRTIM_TIMDEADTIME_RISINGSIGNLOCK_WRITE,
    
    .FallingValue = 384,  // 下降沿死区（对称）
    .FallingSign = HRTIM_TIMDEADTIME_FALLINGSIGN_POSITIVE,
    .FallingLock = HRTIM_TIMDEADTIME_FALLINGLOCK_WRITE,
    .FallingSignLock = HRTIM_TIMDEADTIME_FALLINGSIGNLOCK_WRITE,
};

// 宏定义便于调整
#define DEADTIME_NS         2000
#define DEADTIME_PRESCALER  HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV4
#define DEADTIME_COUNT      ((DEADTIME_NS * (HRTIM_INPUT_CLOCK/4)) / 1000000000)
```

### 3.5 移相角动态调整API

```c
/**
 * @brief 设置移相角
 * @param phase_angle_deg 移相角度（0-180度）
 * @note 移相角通过调整MASTER CMP2实现
 */
void API_FullBridge_SetPhaseShift(uint16_t phase_angle_deg)
{
    uint32_t phase_shift_count;
    
    // 限制移相角范围
    if (phase_angle_deg > 180) {
        phase_angle_deg = 180;
    }
    
    // 计算移相计数值
    // PhaseShift_Count = (Angle / 360) * Period
    phase_shift_count = ((uint32_t)phase_angle_deg * FULLBRIDGE_PERIOD) / 360;
    
    // 设置MASTER CMP2（TimerC复位点）
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, 
                          HRTIM_TIMERINDEX_MASTER, 
                          HRTIM_COMPAREUNIT_2, 
                          phase_shift_count);
}

/**
 * @brief 设置占空比
 * @param duty_percent 占空比百分比（0-100，实际最大约45%考虑死区）
 */
void API_FullBridge_SetDutyCycle(uint16_t duty_percent)
{
    uint32_t duty_count;
    
    // 限制占空比（考虑死区，最大约45%）
    if (duty_percent > 45) {
        duty_percent = 45;
    }
    
    // 计算占空比计数值
    duty_count = ((uint32_t)duty_percent * FULLBRIDGE_PERIOD) / 100;
    
    // 设置TimerA和TimerC的CMP2（占空比控制点）
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, 
                          HRTIM_TIMERINDEX_TIMER_A, 
                          HRTIM_COMPAREUNIT_2, 
                          duty_count);
    
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, 
                          HRTIM_TIMERINDEX_TIMER_C, 
                          HRTIM_COMPAREUNIT_2, 
                          duty_count);
}
```

### 3.6 GPIO引脚映射

根据现有代码推断（需硬件确认）：

| 信号 | HRTIM输出 | GPIO引脚 | 说明 |
|------|-----------|----------|------|
| Q1 | TA1 | 待定 | 超前臂高端 |
| Q2 | TA2 | 待定 | 超前臂低端（互补） |
| Q3 | TC1 | 待定 | 滞后臂高端 |
| Q4 | TC2 | 待定 | 滞后臂低端（互补） |

**需要查阅原理图确认具体GPIO引脚！**

## 四、电流采集改造方案

### 4.1 全桥电流窗口判断逻辑

```c
/**
 * 移相全桥电流采集窗口判断
 * 
 * 正向电流窗口：Q1和Q4同时导通
 *   - TA1高电平 + TC2高电平
 *   - 对应HRTIM状态：TimerA在[0, Duty]区间，TimerC在[PhaseShift, PhaseShift+Duty]区间
 * 
 * 反向电流窗口：Q2和Q3同时导通  
 *   - TA2高电平 + TC1高电平
 *   - 对应HRTIM状态：TimerA在[Duty, Period]区间，TimerC在[PhaseShift+Duty, Period]区间
 */

typedef struct {
    uint16_t hrtim_timestamp;  // HRTIM时间戳
    uint16_t current_value;    // 电流ADC值
} CurrentSampleDef;

/**
 * @brief 判断当前采样点是否在有效电流窗口内
 * @param hrtim_value 当前HRTIM计数器值
 * @param duty_count 占空比计数值
 * @param phase_shift_count 移相计数值
 * @param period 周期计数值
 * @return 0:不在窗口, 1:正向窗口(Q1+Q4), 2:反向窗口(Q2+Q3)
 */
uint8_t FullBridge_GetCurrentWindow(
    uint16_t hrtim_value,
    uint16_t duty_count,
    uint16_t phase_shift_count,
    uint16_t period)
{
    uint16_t window1_start = 0;
    uint16_t window1_end = duty_count;
    uint16_t window2_start = phase_shift_count;
    uint16_t window2_end = phase_shift_count + duty_count;
    
    // 正向窗口：Q1(TA1)和Q4(TC2)同时导通
    // TA1高：[0, duty_count]
    // TC2高：[phase_shift_count, phase_shift_count+duty_count]
    // 交集：[phase_shift_count, duty_count]（如果phase_shift < duty）
    if (hrtim_value >= window1_start && hrtim_value <= window1_end) {
        if (hrtim_value >= window2_start && hrtim_value <= window2_end) {
            return 1;  // 正向电流窗口
        }
    }
    
    // 反向窗口：Q2(TA2)和Q3(TC1)同时导通
    // TA2高：[duty_count, period]
    // TC1高：[0, phase_shift_count] ∪ [phase_shift_count+duty_count, period]
    // 简化判断：在半个周期后的对称位置
    uint16_t half_period = period / 2;
    uint16_t rev_window_start = half_period + phase_shift_count;
    uint16_t rev_window_end = half_period + phase_shift_count + duty_count;
    
    if (hrtim_value >= rev_window_start && hrtim_value <= rev_window_end) {
        return 2;  // 反向电流窗口
    }
    
    return 0;  // 不在有效窗口
}
```

### 4.2 功率计算器改造要点

**新文件**：`power_calculator_fullbridge.c`

**核心改动**：
1. 增加移相角参数
2. 修改窗口判断逻辑（对角管导通判断）
3. 分别累加正向和反向电流
4. 计算平均电流绝对值

```c
typedef struct {
    uint32_t forward_current_sum;   // 正向电流累加和
    uint32_t reverse_current_sum;   // 反向电流累加和
    uint16_t forward_sample_count;  // 正向采样点数
    uint16_t reverse_sample_count;  // 反向采样点数
    
    uint16_t avg_forward_current;   // 平均正向电流
    uint16_t avg_reverse_current;   // 平均反向电流
    uint16_t avg_total_current;     // 总平均电流（绝对值和）
    
    uint16_t phase_shift_angle;     // 当前移相角
    uint16_t duty_cycle;            // 当前占空比
} FullBridgePowerResult;

FullBridgePowerResult CalculateFullBridgePower(
    uint16_t* resonant_current,
    uint16_t* hrtim_values,
    uint16_t count,
    uint16_t duty_count,
    uint16_t phase_shift_count,
    uint16_t period)
{
    FullBridgePowerResult result = {0};
    
    for (uint16_t i = 0; i < count; i++) {
        uint8_t window = FullBridge_GetCurrentWindow(
            hrtim_values[i], 
            duty_count, 
            phase_shift_count, 
            period);
        
        if (window == 1) {
            // 正向电流窗口
            result.forward_current_sum += resonant_current[i];
            result.forward_sample_count++;
        } else if (window == 2) {
            // 反向电流窗口
            result.reverse_current_sum += resonant_current[i];
            result.reverse_sample_count++;
        }
    }
    
    // 计算平均值
    if (result.forward_sample_count > 0) {
        result.avg_forward_current = result.forward_current_sum / result.forward_sample_count;
    }
    if (result.reverse_sample_count > 0) {
        result.avg_reverse_current = result.reverse_current_sum / result.reverse_sample_count;
    }
    
    // 总平均电流（取绝对值后求和）
    result.avg_total_current = result.avg_forward_current + result.avg_reverse_current;
    
    return result;
}
```

## 五、实施步骤总结

### 第一阶段：HRTIM配置（优先级最高）

1. ✅ 阅读MCU规格书HRTIM章节（进行中）
2. ⏳ 创建`API_hrtim_fullbridge.h`
   - 定义移相全桥相关宏和数据结构
   - 声明初始化函数和控制API
3. ⏳ 创建`API_hrtim_fullbridge.c`
   - 实现MASTER定时器配置
   - 实现TimerA/TimerC同步配置
   - 实现死区时间配置
   - 实现移相角和占空比控制API
4. ⏳ 编写测试代码验证四路PWM输出
   - 示波器观察Q1/Q2互补波形
   - 示波器观察Q3/Q4互补波形
   - 测量移相角准确性

### 第二阶段：电流采集改造

5. ⏳ 创建`power_calculator_fullbridge.h`
6. ⏳ 创建`power_calculator_fullbridge.c`
   - 实现对角管导通窗口判断
   - 实现正反向电流分离累加
   - 实现平均电流计算
7. ⏳ 集成ADC+DMA数据采集
8. ⏳ 验证电流采集准确性

### 第三阶段：系统集成测试

9. ⏳ 联调HRTIM和电流采集
10. ⏳ 功率闭环测试
11. ⏳ 性能优化和问题修复

## 六、注意事项和风险点

### 6.1 硬件相关
⚠️ **必须确认**：
- Q1-Q4对应的具体GPIO引脚
- 死区时间要求（当前假设2000ns，需根据MOSFET特性确认）
- 电流传感器位置和采样电路参数

### 6.2 软件相关
⚠️ **关键点**：
- MASTER和Slave定时器的启动顺序（先配置Slave，最后启动MASTER）
- 寄存器更新的原子性（使用preload机制）
- 中断优先级配置（HRTIM中断优先级应高于普通定时器）

### 6.3 调试建议
🔧 **分阶段测试**（符合用户偏好）：
1. 先测试MASTER定时器单独运行
2. 再测试TimerA同步到MASTER
3. 然后测试TimerC同步和移相
4. 最后加入死区和故障保护
5. 每步都用示波器验证波形

## 七、参考资料

1. **MCU规格书**：`Projects/DOC/RX32G410_Reference_Manual_v0p7.pdf`
   - 重点章节：HRTIM主从定时器、时钟树、同步机制
   
2. **HAL库文档**：`Drivers/RX32G4xx_HAL_Driver/Inc/rx32g4xx_hal_hrtim.h`
   - 关键API：`HAL_HRTIM_Init`, `HAL_HRTIM_PWM_Start`, `__HAL_HRTIM_SETCOMPARE`

3. **现有代码参考**：
   - `Projects/LIB/API/API_hrtim.c` - 半桥HRTIM实现
   - `Projects/BaseClass/src/power_calculator.c` - 电流采集逻辑

---

**下一步行动**：
根据用户"分阶段测试执行策略"偏好，建议先完成**第一阶段第2步**：创建`API_hrtim_fullbridge.h`头文件，定义接口框架，然后再逐步实现具体功能。
