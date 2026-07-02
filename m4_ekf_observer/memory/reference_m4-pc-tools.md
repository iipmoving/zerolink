---
name: m4-pc-tools
description: M4 PC 端工具链 — MODBUS 通讯、EKF 整定、自动化测试的位置和用法
metadata:
  type: reference
---

# M4 PC 工具链

## 工具清单

| 工具 | 路径 | 用途 |
|------|------|------|
| MODBUS 通讯 | `tools/ekf_tuner/m4_modbus_tool.py` | 读取遥测/下发控制/数据采集 |
| EKF 整定 GUI | `tools/ekf_tuner/m4_gui.py` | EKF 参数可视化整定 |
| 自动化测试 | `tools/ekf_tuner/run_ekf_tests.py` | 批量测试用例 |
| 绘图工具 | `tools/ekf_tuner/plot_utils.py` | 数据可视化 |
| 依赖 | `tools/ekf_tuner/requirements.txt` | pymodbus, pyserial |

## 常用命令

```bash
pip install pymodbus pyserial

# 交互模式
python m4_modbus_tool.py COM3

# 单次读取
python m4_modbus_tool.py COM3 --read

# 记录30秒
python m4_modbus_tool.py COM3 --log 30

# 设定1000W并启动
python m4_modbus_tool.py COM3 --power 1000 --on
```

## GUI 启动

```bash
python m4_gui.py
# 或 Windows: 双击 m4_gui.bat
```
