# m4_ekf_observer 方法论渐进迁移方案

> **目的**: 在不破坏现有固件的前提下，将零耦合方法论逐步导入 m4_ekf_observer 项目。新模块全量采用，旧模块渐进过渡。

---

## 一、现状

**已就绪**:
- `CLAUDE.md` 已绑定方法论规则（分层、SOP、四件套、三段式范式）
- `.claude/skills/check/` 和 `.claude/skills/new-module/` SKILL 已安装
- `../methodology-seed/tools/` 工具链可用（check_deps.py / check_weak_pairs.py / generate_structs.py / check_structs.py）
- vendor HAL 已隔离为 No-Go 区域

**缺失**（需逐步补齐）:
- `cfg/structs.json` — 跨模块结构体注册表
- `core/interface_map.h` — __weak 配对注册表
- `deps_config.json` — 层依赖规则配置文件
- `core/` — 消息调度器等基础设施（目录存在，内容为空）
- `proto/` — 协议编解码模块（目录存在，内容为空）

---

## 二、模块清单与层归属

### APP 层（业务逻辑）

| 模块 | 路径 | 状态 | 说明 |
|------|------|------|------|
| `ekf` | `app/ekf/` | **已实现** | EKF 遥测 + MODBUS 寄存器块 0x1020-0x1029 |
| `pot_detect` | `app/pot_detect/` | 空壳 | 锅具检测，待开发 |
| `var_gain_pid` | `app/var_gain_pid/` | 空壳 | 变增益 PID，待开发 |
| `data_logger` | `app/data_logger/` | 空壳 | 数据记录，待开发 |

### BASE_CLASS 层（基础类 / 纯算法）

| 模块 | 文件 | 说明 |
|------|------|------|
| `power_calculator` | `base_class/src/power_calculator.c` | 功率计算（上下管电流→功率） |
| `resonant_f0` | `base_class/src/resonant_f0.c` | 谐振频率 f0 估算 |
| `phase` | `base_class/src/phase.c` | 相位计算 |
| `dma_hrtim` | `base_class/src/dma_hrtim.c` | DMA+HRTIM 数据处理 |
| `commclass` | `base_class/src/commclass.c` | 通信基础类 |
| `pluse` | `base_class/src/pluse.c` | 脉冲处理 |
| `printMessage` | `base_class/src/printMessage.c` | 调试打印 |
| `queue` | `base_class/src/queue.c` | 队列数据结构 |

### VENDOR 层（原厂固件，不可迁移）

| 路径 | 说明 |
|------|------|
| `src/RX32G410_FW_HAL_V1.3N/` | RX32G410 原厂 HAL + 项目基线（150+ 文件） |

**VENDOR 内部结构**（仅列出与模块有交互的部分）:

| 子路径 | 说明 |
|--------|------|
| `Projects/APP/POWER/` | 电源控制（状态机、保护、任务调度），与 app/ 层模块通过 __weak 交互 |
| `Projects/BaseClass/` | 原厂基础类（与 base_class/ 有重复，需确认权威来源） |
| `Projects/modbus/` | MODBUS RTU 从站协议栈 |
| `Projects/API/` | API 封装（HRTIM, ADC, COMP, GPIO, I2C） |
| `Drivers/RX32G4xx_HAL_Driver/` | 外设 HAL 驱动 |
| `Projects/LIB/APP/` | 应用库实现（app_power.c 定义 __weak 回调供 ekf 模块使用） |

---

## 三、迁移优先级

### P0 — 当前开发中的新模块（全量方法论）

| 模块 | 说明 |
|------|------|
| `pot_detect` | 锅具检测 — 下一个待开发模块，从零开始按方法论 |
| `var_gain_pid` | 变增益 PID |
| `data_logger` | 数据记录 |

**策略**: P0 模块**从第一行代码就按方法论**。使用 `/new-module` SKILL 创建，通过 `/check` 验证。

### P1 — 边界模块（与其他模块交互多）

| 模块 | 说明 |
|------|------|
| `ekf` | 已使用 `__weak` 模式（`API_POWER_EKF_GetTelemetry`），需规范化 |
| `power_calculator` | 功率计算，将成为 APP 层模块的算法依赖 |
| `resonant_f0` | f0 估算，将成为多个模块的输入源 |
| `phase` | 相位计算 |

**策略**: P1 模块优先迁移，因为它们影响新模块的接口设计。迁移主要是规范化现有的 `__weak` 模式并注册到 `interface_map.h`。

### P2 — 内部模块（相对独立）

| 模块 | 说明 |
|------|------|
| `queue` | 纯数据结构，无外部依赖 |
| `commclass` | 通信基础类 |
| `dma_hrtim` | DMA 数据处理 |
| `pluse` | 脉冲处理 |
| `printMessage` | 调试工具 |

**策略**: P2 模块在 P1 完成后逐批迁移。迁移成本低（独立性强），优先级低。

### NEVER — 不可迁移

| 路径 | 原因 |
|------|------|
| `src/RX32G410_FW_HAL_V1.3N/` 全部 | 原厂固件，不可修改 |
| `Drivers/RX32G4xx_HAL_Driver/` | 外设 HAL，原厂维护 |
| `Projects/modbus/` | MODBUS 协议栈，已验证稳定 |
| `Projects/APP/POWER/` 中与硬件保护相关的部分 | 安全关键逻辑 |

---

## 四、每模块迁移检查清单

### P0: pot_detect（新建）

- [ ] 使用 `/new-module` SKILL 创建，层 = `app`
- [ ] 三段式骨架：输入段（__weak 获取数据）→ 计算段 → 输出段（结果回调）
- [ ] `.h` include guard 注释 `#define`（APP 层规则）
- [ ] `__weak` 空壳声明在 .c 顶部
- [ ] 如需跨模块结构体 → 编辑 `cfg/structs.json` → 运行 `generate_structs.py`
- [ ] `__weak` 配对注册到 `core/interface_map.h`
- [ ] `/check` 四件套全 PASS
- [ ] armcc 编译 0 error / 0 warning

### P0: var_gain_pid（新建）

- [ ] 同上 SOP，层 = `app` 或 `base_class`（取决于是否纯算法）
- [ ] 纯算法 → `base_class`，有 I/O 交互 → `app`

### P0: data_logger（新建）

- [ ] 同上，层 = `app`

### P1: ekf（规范化）

- [ ] 将 `API_POWER_EKF_GetTelemetry` __weak 配对注册到 `interface_map.h`
- [ ] 检查 `modbus_ekf_regs.c` 中是否有违反层规则的 `#include`
- [ ] 如有跨模块结构体（如 `EKF_Telemetry_t`）→ 注册到 `cfg/structs.json`
- [ ] 运行 `check_deps.py --project m4-ekf` → 清理违规
- [ ] 运行 `check_weak_pairs.py --project m4-ekf` → 确认通过

### P1: power_calculator（规范化）

- [ ] 确认 `base_class/src/power_calculator.c` 与 vendor `Projects/BaseClass/src/power_calculator.c` 的权威关系
- [ ] 如 base_class/ 版本是权威 → vendor 版本标记 deprecated
- [ ] 提取公有接口，添加 `__weak` 供 APP 层调用
- [ ] 注册到 `interface_map.h` 和 `structs.json`

### P1: resonant_f0（规范化）

- [ ] 同上：确认权威来源 → 提取接口 → 注册

### P2: queue, commclass, phase, dma_hrtim, pluse, printMessage

- [ ] 确认是否仍在使用
- [ ] 未使用的模块 → 归档或移入 `src/RX32G410_FW_HAL_V1.3N/`（vendor 区域）
- [ ] 仍使用的模块 → 规范化 `#include` → 通过 `check_deps.py`

---

## 五、双轨策略

迁移期间，**新 __weak 通道与旧通信通道可以并存**：

```
新模块 (方法论)                    旧模块 (原厂)
─────────────                    ─────────
app/pot_detect                   Projects/APP/POWER/
  ├── __weak → base_class/*        ├── 直接调用 → Projects/APP/sys/
  ├── Msg_Post → core/msg (未来)    ├── 直接调用 → Projects/BaseClass/
  └── 不 include vendor 头文件       └── 深依赖 vendor HAL
```

**共存规则**:
1. 新模块**不得** `#include` vendor 目录下的任何文件
2. 需要通过 vendor 提供的数据 → 用 `__weak` 桥接（vendor 侧放一个强符号适配器）
3. vendor 代码调用新模块 → 在 vendor 代码中声明 `extern`（不在新模块侧改）
4. 逐步将 vendor `Projects/APP/POWER/` 中的逻辑迁移到 `app/` 层

**适配器模式**（新模块需要 vendor 数据）:

```c
// 新模块 app/pot_detect/pot_detect.c
__weak void Vendor_GetADCReadings(uint16_t *vdc, uint16_t *irms) {
    *vdc = 0; *irms = 0;  // 空壳默认
}

void PotDetect_Run(void) {
    uint16_t vdc, irms;
    Vendor_GetADCReadings(&vdc, &irms);
    /* 计算逻辑... */
}

// 适配器放在 vendor 区域 (或 base_class 桥接模块)
// vendor_adapter.c (在 base_class/)
#include "../src/RX32G410_FW_HAL_V1.3N/Projects/API/API_adc.h"
void Vendor_GetADCReadings(uint16_t *vdc, uint16_t *irms) {
    *vdc = API_ADC_GetVDC();
    *irms = API_ADC_GetIrms();
}
```

链接器自动选择强符号（适配器），新模块不感知 vendor 路径。

---

## 六、首次启动清单

在创建第一个 P0 新模块之前，需要先完成基础设施：

### 6.1 创建配置骨架

```bash
# 在 m4_ekf_observer/ 目录下
mkdir -p cfg core

# 创建 structs.json（空但有 serial）
cat > cfg/structs.json << 'EOF'
{
  "version": "1.0",
  "serial": 0,
  "structs": {}
}
EOF

# 创建 deps_config.json
cat > deps_config.json << 'EOF'
{
  "layer_dirs": ["app", "base_class", "proto", "core"],
  "scan_dirs": ["app", "base_class", "proto", "core", "src"],
  "rules": {
    "app": {
      "allowed": ["app/", "core/", "proto/", "<"],
      "forbidden": ["base_class/", "src/"]
    },
    "base_class": {
      "allowed": ["base_class/", "core/", "<"],
      "forbidden": ["app/", "proto/"]
    },
    "proto": {
      "allowed": ["proto/", "core/", "<"],
      "forbidden": ["app/", "base_class/"]
    },
    "core": {
      "allowed": ["core/", "<"],
      "forbidden": ["app/", "base_class/", "proto/"]
    }
  }
}
EOF

# 创建 interface_map.h
cat > core/interface_map.h << 'EOF'
/* ============================================================
   interface_map.h — __weak 配对注册表
   
   DO NOT #include this file in C code.
   This is for check_weak_pairs.py validation only.
   ============================================================ */

/* Format:
 * Pair: {sender_module} → {receiver_module}
 * weak:  __weak void {Receiver}_On{Event}(params);
 * strong: implemented in {receiver}.c
 */

/* === REGISTERED PAIRS === */

#endif /* INTERFACE_MAP_H */
EOF
```

### 6.2 验证工具链

```bash
python ../methodology-seed/tools/check_deps.py . --project m4-ekf
python ../methodology-seed/tools/check_weak_pairs.py . --project m4-ekf
python ../methodology-seed/tools/check_structs.py . --project m4-ekf
```

（首次运行预期会有一些 vendor 相关告警，逐步清理）

### 6.3 创建第一个 P0 模块

```bash
# 使用 /new-module SKILL 创建
```

---

## 七、不迁移清单

以下**永久保留**在原位置，不纳入方法论管理：

| 路径 | 原因 |
|------|------|
| `src/RX32G410_FW_HAL_V1.3N/` 全部 | 原厂固件库，不可修改 |
| `src/RX32G410_FW_HAL_V1.3N/Drivers/` | CMSIS + HAL 驱动 |
| `src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/` 中的安全保护逻辑 | 硬件安全关键路径 |
| `src/RX32G410_FW_HAL_V1.3N/Projects/modbus/` | MODBUS 协议栈，已验证 |
| `src/RX32G410_FW_HAL_V1.3N/Middlewares/` | BSP/系统初始化 |
| `sim/` | MATLAB 仿真（不同语言/工具链） |
| `tools/ekf_tuner/` | Python PC 工具（Python 调用者规则适用） |
| `hard/` | 硬件设计文件 |

---

## 八、进度追踪

| 阶段 | 内容 | 预计 | 状态 |
|------|------|------|------|
| **Phase 0** | 创建 cfg/structs.json + deps_config.json + core/interface_map.h | 一次性 | 待执行 |
| **Phase 1** | 创建第一个 P0 模块（pot_detect）按完整方法论 | 首个模块 | 待执行 |
| **Phase 2** | P1 模块规范化（ekf / power_calculator / resonant_f0） | 逐个模块 | 待排期 |
| **Phase 3** | P2 模块清理（queue / commclass / phase / dma_hrtim 等） | 批量 | 待排期 |
| **Phase 4** | core/ 消息调度器实现 | 大任务 | 待评估 |

---

*文档版本: v1.0, 2026-06-05*
*对应 CLAUDE.md: §12 方法论文档*
