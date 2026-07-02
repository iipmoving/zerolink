# 项目结构文档

## 项目概述

该项目位于 `D:/OBSIDIAN/MOVING IH/低耦合程序架构/m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects`，主要包含以下核心内容：

- **APP层**: 业务逻辑实现（如 `app_debug.h`）
- **CMSIS设备支持**: ARM Cortex-M系列核心外设驱动（排除HAL层）
- **构建配置**: Keil项目配置文件（.sct, .icf等）

## 目录结构

```text
Projects/
├── APP/
│   └── POWER/
│       └── inc/
│           └── app_debug.h  # 调试模块接口
├── .cmsis/
│   └── device/
│       └── ARM/
│           ├── ARMCM0plus/  # Cortex-M0+支持
│           ├── ARMCM3/      # Cortex-M3支持
│           └── ARMCM33/     # Cortex-M33支持
└── Projects/Keil/  # Keil项目配置
    └── .cmsis/
        └── device/
            └── ARM/  # 核心设备支持文件
```

## 关键模块说明

### 1. 调试模块 (`app_debug.h`)
- **职责**: 提供调试接口和日志功能
- **依赖**: 无直接硬件依赖，通过消息队列与底层通信
- **入口点**: `APP/POWER/inc/app_debug.h`

### 2. CMSIS设备支持
排除标准HAL层后，保留以下核心功能：
- **启动代码**: `startup_ARM*.s`/`.c`（不同核心版本）
- **系统初始化**: `system_ARM*.c` 实现核心时钟配置
- **内存布局**: `.sct`/`.icf` 文件定义内存分区

### 3. 构建系统集成
- **Keil项目配置**: 通过 `.sct` 文件管理内存映射
- **多核支持**: 提供 Cortex-M0+/M3/M33 的差异化实现

## 项目运行方式
1. 使用 Keil uVision 打开项目
2. 配置目标设备核心类型（M0+/M3/M33）
3. 编译并下载到对应 MCU
4. 通过 `app_debug.h` 接口验证功能

**注意**: 用户明确要求排除 CMSIS HAL 层分析，文档聚焦于应用层和核心支持文件

## 多核支持差异

| 核心类型 | 启动文件 | 内存配置 | 特殊功能 |
|----------|----------|----------|----------|
| Cortex-M0+ | `startup_ARMCM0plus.s` | `ARMCM0plus.sct` | 低功耗模式支持 |
| Cortex-M3 | `startup_ARMCM3.s` | `ARMCM3.sct` | MPU支持 |
| Cortex-M33 | `startup_ARMCM33.s` | `ARMCM33.sct` | TrustZone安全扩展 |

## 内存布局配置

内存配置文件（`.sct`/`.icf`）定义了以下关键分区：
```c
/* 示例：ARMCM3.sct 内存定义 */
LR_IROM1 0x08000000 0x00080000  {    ; load region size_region
  ER_IROM1 0x08000000 0x00080000  {  ; load address = execution address
   *.o (RESET, +First)
   *(InRoot$$Sections)
   *.o (RESET, +First)
   *(InRoot$$Sections)
  }
  RW_IRAM1 0x20000000 0x00020000  {
   *(.data)
   *(.bss)
  }
}
```

## 异常处理机制

启动文件中实现了以下异常处理：
- **硬故障处理**: `HardFault_Handler()` 在 `startup_ARM*.s` 中定义
- **NMI处理**: `NMI_Handler()` 提供非屏蔽中断支持
- **系统滴答**: `SysTick_Handler()` 用于系统定时

## 构建验证流程

1. 使用 Keil 编译检查内存占用
2. 通过 `fromelf -z` 查看映像文件组成
3. 使用 `Debug -> Start/Stop Debug Session` 验证硬件交互
4. 通过 `app_debug.h` 接口输出运行状态