# ADR: 应用层架构 — MODBUS + 业务模块

**日期**: 2026-05-21
**状态**: 已批准

## 决策

分两批建设应用层，第一批打通 MODBUS 通信，第二批建设业务逻辑。

## 模块划分

### 第一批: 通信层

| 模块 | 路径 | 职责 |
|------|------|------|
| proto_modbus | `Claude/proto/proto_modbus.{h,c}` | MODBUS 帧编解码、CRC-16 校验 |
| app_comm_mgr | `Claude/app/app_comm_mgr.{h,c}` | 4 炉头轮询调度、寄存器缓存、重试/超时 |

### 第二批: 业务层

| 模块 | 路径 | 职责 |
|------|------|------|
| app_protect | `Claude/app/app_protect.{h,c}` | 过压/过流/过热/无锅检测 |
| app_power | `Claude/app/app_power.{h,c}` | 功率计算 + PID + 下发 |
| app_cooking | `Claude/app/app_cooking.{h,c}` | 4 炉头状态机，按键→烹饪动作 |

## 消息流

```
hal_comm (帧就绪)
  → proto_modbus (解析)
    → MSG_COMM_DATA_UPDATE
      → app_comm_mgr (缓存寄存器, 决定响应)
        → proto_modbus (组帧)
          → hal_comm (发送)

app_cooking (用户按键)
  → MSG_POWER_CTRL
    → app_power (功率计算)
      → app_comm_mgr (写寄存器)

app_protect (检测到异常)
  → MSG_SYSTEM_ERROR
    → app_cooking (紧急停机)
      + drv_buzzer (报警音)
```

## 新增消息 ID

在 `msg_def.h` 中新增:
- `MSG_COMM_POLL_TICK` — 通信轮询节拍（替代直接调用）
- `MSG_REG_DATA_READY` — 寄存器数据就绪（炉头数据更新）

## 红线

- proto_modbus 不依赖任何 app/ 模块
- app/ 之间零直接 include，只走消息
- 寄存器地址和站号全部 `#define`，不硬编码在逻辑中
- CRC 用查表法（`static const uint16_t crc_table[256]`），不运行时计算多项式
- 每炉头重试上限 3 次，超时标为离线，不阻塞其他炉头

## CRC-16 方案：软件查表法（非硬件 CRC）

芯片内置 CRC 外设**不支持输入位反转(REFIN)和输出位反转(REFOUT)**，而 MODBUS CRC-16 要求二者。若用硬件 CRC，需逐字节做位反转预处理 + 位反转输出，还需 malloc 临时缓冲（参考程序做法），在嵌入式上不可接受。

**决策**: 使用参考程序 `CRC_CalculatedArrays.c` 中的软件查表法 `CRC_16_Modebus_Look_Table()`：
- 预计算 `auchCRCHi[256]` + `auchCRCLo[256]` 两张表（参考程序已验证正确）
- 零堆内存，一次查表一个字节，O(n) 时间复杂度
- 表数据可直接从参考程序复制，CRC 验证值已确认
