# m4_ekf_observer — 项目文件索引

**生成日期**: 2026-06-03  
**项目根**: `d:\OBSIDIAN\MOVING IH\低耦合程序架构\m4_ekf_observer\`

---

## 一、文档层

| 路径 | 用途 |
|------|------|
| [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md) | 项目总结 + 问题记录 + 避免方法 |
| [CLAUDE.md](CLAUDE.md) | 项目级 AI 指令 (待创建) |
| [memory/](memory/) | 项目记忆 (跨会话持久) |
| [docs/architecture/](docs/architecture/) | 架构文档 |

---

## 二、MCU 固件 (`src/RX32G410_FW_HAL_V1.3N/`)

### 2.1 硬件驱动层 — HAL (`Drivers/`)

| 路径 | 说明 |
|------|------|
| `Drivers/CMSIS/` | ARM Cortex-M4 CMSIS 头文件 |
| `Drivers/RX32G4xx_HAL_Driver/` | 瑞芯 HAL 库 |
| `Drivers/Startup/` | 启动文件 + 中断向量表 |

### 2.2 API 层 (`Projects/API/`)

| 文件 | 职责 |
|------|------|
| `inc/API_HRTIM.h` | HRTIM API 声明 (含 Master 同步函数) |
| `inc/API_ADC.h` | ADC API 声明 |
| `inc/API_DMA.H` | DMA API 声明 |
| `inc/API_gpio.h` | GPIO API 声明 |
| `inc/API_TIM.h` | Timer API 声明 |
| `inc/API_COMP.h` | 比较器 API 声明 |
| `inc/API_I2C.H` | I2C API 声明 |
| `inc/API_OPAMP.H` | 运放 API 声明 |
| `inc/API_DAC.H` | DAC API 声明 |
| `inc/API_UART.H` | UART API 声明 |
| `inc/API_FMAC.H` | FMAC 硬件加速器 API 声明 |
| `inc/API_hrtim_fullbridge.h` | 全桥 HRTIM 扩展声明 |
| `inc/API_hrtim_fullbridge1.h` | 全桥 HRTIM 变体声明 |
| `src/API_TIM.C` | Timer 实现 |
| `Projects/LIB/API/API_hrtim.c` | **HRTIM 核心实现** (Master 同步/PREEN/中断) |
| `Projects/LIB/API/API_hrtim_master_sync.c` | ⚠️ 重复文件 — 不编译, 待归档 |
| `Projects/LIB/API/API_hrtim_fullbridge.c` | 全桥 HRTIM 扩展 (半桥模式为空桩) |
| `Projects/LIB/API/API_hrtim_fullbridge1.c` | 全桥 HRTIM 变体 (半桥模式为空桩) |

### 2.3 基础类 (`Projects/BaseClass/`)

| 文件 | 职责 |
|------|------|
| `inc/power_calculator.h` | 功率/电参数结构体 + 函数声明 |
| `src/power_calculator.c` | `CalculatePower_FPU` + `CalculateElecParams_20ms` (KVL 全链) |
| `inc/power_calculator_fullbridge.h` | 全桥功率计算声明 |
| `src/power_calculator_fullbridge.c` | 全桥功率计算实现 |
| `inc/wave_capture.h` | 波形采集声明 |
| `src/wave_capture.c` | 波形采集 0x5000 MODBUS 实现 |
| `inc/phase.h` | 相位计算声明 |
| `src/phase.c` | 相位计算实现 |
| `inc/class_phase.h` | 相位类声明 |
| `src/class_phase.c` | 相位类实现 |
| `inc/class_power_calculator.h` | 功率计算类声明 |
| `src/class_power_calculator.c` | 功率计算类实现 |
| `inc/class_queue.h` | 队列类声明 |
| `src/class_queue.c` | 队列类实现 |
| `inc/class_printMessage.h` | 打印类声明 |
| `src/class_printMessage.c` | 打印类实现 |
| `inc/printMessage.h` | 打印声明 |
| `src/printMessage.c` | 打印实现 |
| `inc/queue.h` | 队列声明 |
| `src/queue.c` | 队列实现 |
| `inc/dma_hrtim.h` | DMA+HRTIM 声明 |
| `src/dma_hrtim.c` | DMA+HRTIM 实现 |
| `inc/CommClass.h` | 通讯类声明 |
| `src/CommClass.c` | 通讯类实现 |
| `inc/resonant_msg.h` | 谐振消息声明 |
| `src/resonant_msg.c` | 谐振消息实现 |
| `src/class_comm.c` | 通讯类补充实现 |
| `src/class_pluse.c` | 脉冲类实现 |
| `src/pluse.c` | 脉冲实现 |

### 2.4 应用层 (`Projects/APP/` + `Projects/LIB/APP/`)

| 路径 | 职责 |
|------|------|
| [Projects/APP/POWER/src/app_task.c](Projects/APP/POWER/src/app_task.c) | 任务调度表 + SystemInitial (Master 同步初始化) |
| [Projects/LIB/APP/APP_ADC.C](Projects/LIB/APP/APP_ADC.C) | ADC 中断回调链 + Master UPD 重使能 |
| `Projects/LIB/APP/app_power.c` | 功率控制状态机 (POWER_CHANGE_CYCLE) |
| `Projects/LIB/APP/APP_ADC - 副本.C` | ⚠️ 备份副本 — 待删除 |
| `Projects/LIB/APP/adc_processing.h` | ADC 数据处理声明 |
| `Projects/DRV/SRC/adc_processing.c` | ADC 数据处理实现 |

### 2.5 中断服务 (`Projects/Users/`)

| 路径 | 职责 |
|------|------|
| [Projects/Users/src/rx32g4xx_it.c](Projects/Users/src/rx32g4xx_it.c) | 所有 ISR (含 HRTIM1_Master_IRQHandler) |
| `Projects/Users/inc/rx32g4xx_it.h` | ISR 声明 |

### 2.6 通讯协议 (`Projects/modbus/`)

| 路径 | 职责 |
|------|------|
| `modbus/Modbus_Lib_Init_An_Analysis.c` | MODBUS 寄存器注册 |
| `modbus/Modbus_Lib_Init_An_Analysis.h` | MODBUS 声明 |

### 2.7 中间件 (`Middlewares/System/`)

| 路径 | 职责 |
|------|------|
| `sys_task.c` | 系统任务调度器 (10槽轮转) |
| `s_time_base.c` | 时基 (1ms) |

---

## 三、仿真与验证 (`sim/`)

### 3.1 MATLAB 半桥模型 (`sim/matlab_half_bridge/`)

#### 核心管道 (14 个)

| 文件 | 职责 |
|------|------|
| [export_golden_batch.m](export_golden_batch.m) | 批量 golden 导出 (47 文件 → golden_batch.json, 含 KVL 全链) |
| [export_golden.m](export_golden.m) | 3 文件 golden 导出 (golden.json) |
| `load_data.m` | CSV 加载 + CNT 绕回检测 |
| `calc_waveform.m` | I_zero + P_W + I_peak |
| `calc_phase.m` | 谷值检测 → φ |
| `calc_ac_cycle.m` | 交流周期参数 |
| `calc_timing.m` | 控制模式判别 (D_std/倍频/同频) |
| `calc_impedance.m` | KVL L + f_res/Q/R/X/Z |
| `calc_power.m` | 功率积分 |
| `calc_rlc.m` | RLC 参数 |
| `calibration.m` | 标定常数 |
| `main.m` | 主流程 (旧) |
| `plot_panel.m` | 可视化面板 |
| `run_ih.m` | 主流程编排 (新) |

#### 调试脚本 (25 个)

| 文件 | 调试对象 |
|------|---------|
| `debug_104850_f7.m` | 特定帧 |
| `debug_concentration.m` | 数据集中度 |
| `debug_cut_cycle.m` | 周期裁剪 |
| `debug_deadzone.m` | 死区 |
| `debug_didt.m` | dI/dt |
| `debug_didt_diag.m` | dI/dt 诊断 |
| `debug_didt_l.m` | dI/dt → L |
| `debug_didt_l2.m` | dI/dt → L (v2) |
| `debug_file_avg.m` | 文件均值 |
| `debug_fmac_impact.m` | FMAC 影响 |
| `debug_frame1.m` | 单帧 |
| `debug_f_res.m` | f_res 谐振频率 |
| `debug_hv_band.m` | 高压带 |
| `debug_hv_dist.m` | 高压分布 |
| `debug_iron_fft.m` | 铁锅 FFT |
| `debug_iron_fft2.m` | 铁锅 FFT v2 |
| `debug_iron_goertzel.m` | 铁锅 Goertzel |
| `debug_iron_L.m` | 铁锅 L |
| `debug_iron_variance.m` | 铁锅方差 |
| `debug_irms_correct.m` | I_rms 修正 |
| `debug_outlier.m` | 异常值 |
| `debug_power_interp.m` | 功率插值 |
| `debug_power_verify.m` | 功率验证 |
| `debug_sinefit.m` | 正弦拟合 |
| `debug_valley.m` | 谷值检测 |
| `debug_vdc.m` | Vdc |
| `debug_vdc_bins.m` | Vdc 分箱 |

#### 对比/验证 (6 个)

| 文件 | 职责 |
|------|------|
| `batch_iron2.m` | 铁锅批量 |
| `batch_steel.m` | 钢锅批量 |
| `check_stability.m` | 稳定性检查 |
| `compare_3files.m` | 3 文件对比 |
| `compare_waveform.m` | 波形对比 |
| `verify_L_reference.m` | L 参考验证 |

#### 卡尔曼/仿真 (3 个)

| 文件 | 职责 |
|------|------|
| `sim_kalman_l.m` | L 卡尔曼仿真 |
| `sim_kalman_l2.m` | L 卡尔曼 v2 |
| `run_orig_pipeline.m` | 原始管道 |
| `test_valley_flat.m` | 谷值平坦测试 |
| `list_all_frames.m` | 帧列表 |

### 3.2 C DLL 验证工具 (`sim/dll_test/`)

| 路径 | 职责 |
|------|------|
| [test/ctypes_bridge.py](test/ctypes_bridge.py) | ctypes 封装 (零公式逻辑) |
| [test/csv_loader.py](test/csv_loader.py) | CSV 解析 + 周期检测 (零业务逻辑) |
| `test/test_runner.py` | 单文件手动测试 |
| `*.dll` | C 编译产物 (power_calculator.dll) |

### 3.3 KEIL 移植参考 (`sim/matlab_half_bridge/keil_port/`)

| 文件 | 职责 | 状态 |
|------|------|------|
| `ih_params.h` | 数据结构 + 硬件标定 | CORDIC 地址待确认 |
| `ih_calculate.h` | 函数声明 | ✅ |
| `ih_calculate.c` | 核心算法 (CORDIC + M4F FPU) | 待验证 |
| `main.c` | HRTIM+ADC+DMA 配置模板 | 参考用 |

---

## 四、PC 端工具 (`tools/ekf_tuner/`)

### 4.1 GUI/通讯

| 文件 | 职责 |
|------|------|
| [m4_gui.py](m4_gui.py) | MODBUS GUI (PyQt) |
| [m4_modbus_tool.py](m4_modbus_tool.py) | MODBUS 通讯栈 |
| `m4_gui.bat` | 启动脚本 |

### 4.2 验证脚本

| 文件 | 职责 |
|------|------|
| `ekf_q_model.py` | EKF Q 值建模 |
| `eval_L_from_adc.py` | ADC → L 评估 |
| `eval_L_from_adc.m` | ADC → L 评估 (MATLAB) |
| `plot_utils.py` | 绘图工具 |

### 4.3 测试/MATLAB

| 文件 | 职责 |
|------|------|
| `run_ekf_tests.py` | EKF 测试 |
| `calc_physical_params.m` | 物理参数计算 |
| `verify_f0_q_l.m` | f0/Q/L 验证 |
| `verify_L_consistency.m` | L 一致性验证 |
| `run_verify_L.m` | L 验证入口 |

### 4.4 数据文件 (47 个)

| 模式 | 文件数 | 命名模式 |
|------|--------|---------|
| 功率扫描 | 47 | `capture_20260602_HHMMSS.csv` |

### 4.5 Web 仿真 (`sim/`)

| 路径 | 职责 |
|------|------|
| `index.html` | MODBUS 模拟器 UI |
| `modbus_sim.c` | C 模拟器源码 |
| `modbus_sim.exe` | 编译产物 |
| `run_tests_node.js` | Node.js 测试 |

---

## 五、应用层骨架 (`app/`)

| 路径 | 状态 |
|------|------|
| [app/ekf/FIRMWARE_PATCH.md](app/ekf/FIRMWARE_PATCH.md) | EKF 固件补丁说明 |
| [app/ekf/modbus_ekf_regs.c](app/ekf/modbus_ekf_regs.c) | EKF MODBUS 寄存器 |
| [app/ekf/modbus_ekf_regs.h](app/ekf/modbus_ekf_regs.h) | EKF MODBUS 声明 |
| `app/data_logger/` | 空目录 |
| `app/pot_detect/` | 空目录 |
| `app/var_gain_pid/` | 空目录 |

---

## 六、待归档/删除

| 路径 | 原因 |
|------|------|
| `Projects/LIB/API/API_hrtim_master_sync.c` | 不编译的重复文件 |
| `Projects/LIB/APP/APP_ADC - 副本.C` | 备份副本 |
| `src/base_class/` | 从 `Projects/BaseClass/` 迁移后的残留 (如存在) |

---

## 七、项目间文件

以下文件属于 `m4_ekf_observer` 但影响跨项目工作流：

| 路径 | 说明 |
|------|------|
| [sim/matlab_half_bridge/AUDIT_BRIEF.md](AUDIT_BRIEF.md) | 第三方审计说明书 |
| [sim/matlab_half_bridge/AUDIT_REPORT_CLAUDE.md](AUDIT_REPORT_CLAUDE.md) | 审计报告 (李工 + AI) |
| [sim/matlab_half_bridge/TEST_REPORT.md](TEST_REPORT.md) | 测试报告 |
| [sim/matlab_half_bridge/batch_validate_report.txt](batch_validate_report.txt) | 批量验证报告 (47文件) |
| `sim/matlab_half_bridge/golden_batch.json` | MATLAB golden (47文件/16字段) |
| `sim/matlab_half_bridge/golden.json` | MATLAB golden (3文件/16字段) |

---

*索引生成: 2026-06-03 — 不含 Drvr/Startup/HAL 内部厂商文件*
