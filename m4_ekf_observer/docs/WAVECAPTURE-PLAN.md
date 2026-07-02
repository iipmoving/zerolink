# WaveCapture 模块 — 实施计划

> **状态**: 规划完成, 待实施
> **日期**: 2026-05-30
> **角色**: 技术负责人 → 程序员
> **定位**: 替代 printMessage UART printf, 通过 MODBUS 回读完整谐振电流波形供 MATLAB 验证
> **最终目标**: `BaseClass/src/power_calculator.c` 按 MATLAB 物理模型重写为全新电参量运算模块

---

## 一、数据帧结构 (最终确认版)

### 帧头 (6 words)

| MODBUS地址 | 字段 | 说明 |
|-----------|------|------|
| 0x5004 | WAVE_MAGIC = 0xA5A5 | 帧同步头 |
| 0x5005 | WAVE_FRAME_ID | 帧序号(递增, 主机判断新帧) |
| 0x5006 | WAVE_HEAD_IDX | 炉头号 (0-3) |
| 0x5007 | WAVE_CYCLE_CNT | 本帧实际周期数 |
| 0x5008 | WAVE_F_AVG | 平均开关频率 |
| 0x5009 | WAVE_RES | 保留 |

### PARA块 (5 words, 整帧共用, 每1S更新一次)

| MODBUS地址 | 字段 | 说明 |
|-----------|------|------|
| 0x500A | PARA_PPG_VALUE | **HRTIM PPG占空比值** |
| 0x500B | PARA_HIGH_ON | 上管开通HRTIM值 |
| 0x500C | PARA_POWER | 功率值 (W/25) |
| 0x500D | PARA_PERIOD | PWM周期 HRTIM ticks |
| 0x500E | PARA_RES | 保留 |

### 周期体 (360 × 3 words)

每PWM周期存3个值, 共360周期覆盖完整50Hz交流周期(20ms @18kHz最低频):

```
word[0] = HRTIM_time    ← HRTIM时钟戳, 用于时序对齐
word[1] = Vbus_ADC      ← 母线电压 ADC (观察50Hz纹波)
word[2] = I_FMAC_ADC    ← FMAC滤波谐振电流 (MATLAB主输入)
```

**帧总大小**: 6 + 5 + 1080 = **1091 words** (6K=6144, 余量充足)

---

## 二、波形采集触发逻辑

```
每PWM周期 (在 PowerTypeFun 或 PowerControlFun 内部):
  → 逐周期调用 WaveCapture_Push(ch, hrtim, vbus, i_fmac)

APP_ADC_CalculatePower() 每 ~20ms 执行:
  → MessageCnt++ 累加
  → if (MessageCnt >= 50) {  ← 50×20ms = 1S
      WaveCapture_Freeze()   ← 冻结当前帧, 写帧头+frame_id++, 解锁给MODBUS读
      MessageCnt = 0
    }
```

---

## 三、MODBUS 主机读取流程 (Python)

```
m4_modbus_tool.py COMx --wave --head 0

1. 写 0x5000 = 1  → 启动采集 (配置炉头0)
2. 等待 1S
3. 写 0x5000 = 0  → 停止采集
4. 读 0x5004(1)   → 检查 MAGIC=0xA5A5
5. 读 0x500A(1086) → 批量拉取完整帧 (PARA 5 + 周期体 1080 + 帧头已包含)
6. 解析帧:
   - 帧头: frame_id, head_idx, cycle_cnt, f_sw
   - PARA: ppg_value, high_on, power, period
   - 周期体: 按 cycle_cnt 行展开 [HRTIM, Vbus, I_FMAC]
7. 保存 CSV → data/wave_YYYYMMDD_HHMMSS_headN.csv
8. MATLAB calc_physical_params.m 直接加载分析
```

---

## 四、文件改动清单

| # | 文件 | 改动 | 行数 | 风险 |
|---|------|------|------|------|
| 1 | **`BaseClass/inc/wave_capture.h`** | 新建 — 结构体定义 + API声明 + 6K extern buffer | +55 | 低 |
| 2 | **`BaseClass/src/wave_capture.c`** | 新建 — Push/Freeze/Config 实现 | +200 | 低 |
| 3 | **`modbus/Modbus_Lib_Init_An_Analysis.c`** | AREA_COUNT 5→6, 新增Area5: 0x5000→wave_buffer | +15 | 中 |
| 4 | **`LIB/APP/APP_ADC.C`** | 替换 PrintMessagePush/Out → WaveCapture_Push + Freeze | -5+10 | 中 |
| 5 | **`tools/ekf_tuner/m4_modbus_tool.py`** | 新增 --wave 命令: 拉取→解析→保存CSV | +80 | 低 |

---

## 五、与现有 resonant_msg 的关系

| 模块 | 输出 | 频率 | 用途 |
|------|------|------|------|
| resonant_msg (保留) | 0x4000区 f0/Q/L | 每 ~20ms | 轻量实时监控 |
| **wave_capture (新增)** | **0x5000区 全波形** | 每 **1S** | **MATLAB 重分析/验证** |

两者互补, 不冲突。

---

## 六、编程规则 (本次实施继承)

1. **碰前必读**: 改任何 .c/.h 前完整读完, 不读前50行就改
2. **最小化diff**: 只改任务要求的, 不顺手重构/格式化无关代码
3. **接口兼容**: 已有函数不改签名, 改必须说明影响范围
4. **编译验证**: armcc 0e0w
5. **假设显性化**: 不确定的假设单列, 等确认
6. **自清门户**: 改动导致的死变量/dead include 自行清理
7. **每一行可追溯**: 所有修改行必须直接对应到本计划的需求

---

## 七、实施顺序

```
Step 1: wave_capture.h      ← 头文件定义 (独立, 无依赖)
Step 2: wave_capture.c      ← 实现 (依赖.h)
Step 3: Modbus_Lib注册      ← 指向wave_buffer (依赖Step1)
Step 4: APP_ADC.C 调用      ← 替换PrintMessage (依赖Step2)
Step 5: m4_modbus_tool.py   ← PC端工具 (可独立验证)
```

---

*本计划由技术负责人归档, 程序员按 Step1→5 逐任务实施, 每步编译验证*
