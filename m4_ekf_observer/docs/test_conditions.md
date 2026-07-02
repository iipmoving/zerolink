# M4 电磁炉 EKF 数据采集 — 测试工况准备清单

## 硬件准备

| 项目 | 要求 |
|------|------|
| M4 半桥控制板 | 已烧录含 EKF 遥测固件 (0x1020-0x1029) |
| USB-RS485 转换器 | 支持 115200 8N1 |
| 电源 | 单相 220V/50Hz，额定 ≥ 3.5kW |
| 锅具 | 见下表 |
| 水量 | 每个测试前锅中加入适量水，防止干烧 |
| PC | Windows，Python 3.9+，`pip install -r tools/ekf_tuner/requirements.txt` |

## 锅具需求

| 编号 | 材质 | 口径 | 用途 | 必选 |
|------|------|------|------|------|
| A | 430 不锈钢 | 18cm | 钢锅基准 (有 Curie 拐点) | **必须** |
| B | 铁锅 (珐琅/生铁) | 22cm | 铁锅基准 (无 Curie 拐点) | **必须** |
| C | 430 不锈钢 | 16cm | 小尺寸对比 | 建议 |
| D | 铁锅 | 26cm | 大尺寸对比 | 建议 |
| E | 劣质锅 (薄底/复合底异常) | 任意 | 恶劣锅具限功 | 可选 |

**至少准备 A + B 两种锅。** 如果只有一种材质的锅，power_sweep 和 freq_step 仍可执行，但 pot_material 测试无法做对比分析。

## 连接步骤

```
PC USB → USB-RS485 → A+/B- → M4 控制板 UART2 (MODBUS)
```

1. M4 控制板断电
2. 连接 RS485 A+ 到板子 A+，B- 到板子 B-
3. 控制板上电
4. PC 端确认串口号 (设备管理器 → 端口)
5. 验证通讯: `python tools/ekf_tuner/m4_modbus_tool.py COMx --read`
   - 应看到 r_power_w=0, r_work_sta=0 等数据
6. 验证 EKF 遥测: `python tools/ekf_tuner/m4_modbus_tool.py COMx --read-ekf`
   - 应看到 e_freq_hz=0, e_phase_deg=0 (未工作时)

---

## 测试序列

### 测试 0: 通讯验证 (5 分钟)

**目的**: 确认 MODBUS 通讯稳定，EKF 遥测寄存器有数据

```bash
python tools/ekf_tuner/m4_modbus_tool.py COMx --monitor --csv
```

- 锅具: 任意
- 操作: 不放锅或放锅均可，不做任何操作
- 成功标准: 连续 30s 无通讯超时，CSV 有 `e_freq_hz` / `e_phase_deg` 字段

### 测试 1: 功率扫描 (约 6 分钟)

**目的**: 获取 P-f-φ-ξ 关系，标定 P_max 和 K_f

```bash
python tools/ekf_tuner/run_ekf_tests.py COMx --test power_sweep
```

- 锅具: **锅具 A (430 不锈钢 18cm)**，装半锅水
- 操作: 全程不碰锅，不抬锅
- 说明: 脚本会自动设功率 500→800→1000→1200→1500→1800→2000W，每段 30s
- 注意: 水可能在中途沸腾，正常现象，继续测试即可

### 测试 2: 锅具材质对比 (约 3 分钟)

**目的**: 观察钢锅 Curie 拐点 vs 铁锅线性 ξ-P 关系

```bash
# 第一轮: 钢锅
python tools/ekf_tuner/run_ekf_tests.py COMx --test pot_material

# 第二轮: 铁锅 (手动换锅后执行)
python tools/ekf_tuner/run_ekf_tests.py COMx --test pot_material --csv ekf_pot_material_iron_20260526.csv
```

- 第一轮: **锅具 A (430 不锈钢)**，装半锅水
- 第二轮: **锅具 B (铁锅)**，装半锅水
- 操作: 全程不碰锅
- 说明: 1500W 持续 2 分钟，观察 430 不锈钢在 Curie 温度附近 ξ 的转折点

### 测试 3: 频率步进响应 (约 4 分钟)

**目的**: 观察小功率变化时的 Δf→ΔP→Δφ 动态响应

```bash
python tools/ekf_tuner/run_ekf_tests.py COMx --test freq_step
```

- 锅具: **锅具 A (430 不锈钢)**，装半锅水
- 操作: 全程不碰锅
- 说明: 500→600→700→...→1000W 逐步递增，每步 20s

### 测试 4: 移锅检测 (约 2 分钟)

**目的**: 标定 dξ/dt 移锅阈值

```bash
python tools/ekf_tuner/run_ekf_tests.py COMx --test pot_lift
```

- 锅具: **锅具 A 或 B**，装半锅水
- 操作: **需要人的配合**。脚本以 800W 持续记录 60s。在这 60s 内：
  - 第 15s 左右：抬锅 3cm，保持 2s 后放回
  - 第 30s 左右：抬锅 3cm，保持 2s 后放回
  - 第 45s 左右：快速颠锅 (抬起即放)
- 重要: 每次抬锅/颠锅后用笔记下大致时间，方便标注数据

### 测试 5: 锅具大小对比 (可选，约 2 分钟 × N)

**目的**: 建立 ξ_steady 与锅径的映射

```bash
# 每个锅执行一次
python tools/ekf_tuner/run_ekf_tests.py COMx --test pot_size --csv ekf_pot_16cm.csv
python tools/ekf_tuner/run_ekf_tests.py COMx --test pot_size --csv ekf_pot_18cm.csv
python tools/ekf_tuner/run_ekf_tests.py COMx --test pot_size --csv ekf_pot_22cm.csv
python tools/ekf_tuner/run_ekf_tests.py COMx --test pot_size --csv ekf_pot_26cm.csv
```

- 锅具: 不同尺寸锅轮流测试
- 操作: 全程不碰锅
- 说明: 1500W 持续 60s，对比不同尺寸锅的 ξ_steady 值

### 测试 6: 恶劣锅具限功 (可选，约 2 分钟)

```bash
python tools/ekf_tuner/run_ekf_tests.py COMx --test power_sweep --csv ekf_bad_pot.csv
```

- 锅具: **锅具 E (劣质锅)**，装半锅水
- 操作: 全程不碰锅
- 注意: 如果 IGBT 温度异常或电流过大，脚本不会自动停机，**需要人工监控温度读数并决定是否终止**

---

## 安全注意事项

1. **全程有人值守**，不要离开
2. 锅具加水防干烧，水位 ≥ 锅深 1/3
3. 监控 `r_igbt_temp` 读数，如超过 **60°C** 立即 `Ctrl+C` 终止
4. 测试环境通风良好，远离易燃物
5. 抬锅测试 (测试4) 戴隔热手套
6. 测试完成后脚本会自动关机 (set_work_sta=False)
7. 如果脚本异常退出，手动执行: `python tools/ekf_tuner/m4_modbus_tool.py COMx --off`

## 数据文件命名建议

为避免覆盖数据，建议给 CSV 文件名加材质/尺寸后缀：

```bash
python tools/ekf_tuner/run_ekf_tests.py COMx --test power_sweep --csv ekf_sweep_ss430_18cm_20260526.csv
python tools/ekf_tuner/run_ekf_tests.py COMx --test pot_material --csv ekf_material_iron_22cm_20260526.csv
python tools/ekf_tuner/run_ekf_tests.py COMx --test freq_step --csv ekf_step_ss430_18cm_20260526.csv
python tools/ekf_tuner/run_ekf_tests.py COMx --test pot_lift --csv ekf_lift_iron_22cm_20260526.csv
```

## 预期数据量

| 测试 | 时长 | 采样率 | 预计条数 |
|------|------|--------|----------|
| power_sweep | 260s | 10Hz | ~2600 |
| pot_material | 130s × N | 10Hz | ~1300 × N |
| freq_step | 175s | 10Hz | ~1750 |
| pot_lift | 65s | 10Hz | ~650 |
| pot_size | 70s × N | 10Hz | ~700 × N |

## 数据回传

测试完成后，将生成的 CSV 文件放在:
```
m4_ekf_observer/data/raw/
```

然后通知技术负责人开始离线 EKF 参数整定。
