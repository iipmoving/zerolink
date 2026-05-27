# 技术栈模板

> 用法: 复制到 `.claude/specs/tech-stack.md`，填入实际值。

## 编程语言
- **语言**: {C / C++ / ...}
- **标准**: {C89 / C99 / C11 / ...}
- **编译约束**: {如: 无 _Static_assert, 用 CT_ASSERT 替代}

## 编译器/IDE
- **编译器**: {如: Keil MDK ARMCC V5.06 / GCC 12 / Clang 16}
- **IDE**: {如: Keil uVision 5 / VS Code / CLion}
- **调试器**: {如: JLink / ST-Link / OpenOCD}

## 硬件平台
- **MCU**: {型号}
- **架构**: {Cortex-M0+ / M4 / RISC-V / ...}
- **主频**: {48MHz / 168MHz / ...}
- **时钟源**: {内部RC / 外部晶振}
- **Flash**: {256K / 512K / ...}
- **SRAM**: {16K / 64K / ...}
- **堆栈**: {Heap X B / Stack Y B}

### 外设
- {TIM0: 系统时基 / 125us / 8kHz}
- {UART0: 通讯 / DMA / 57600}
- {GPIO: 显示 / 按键 / ...}

## 依赖库
- {库名 版本 — 用途}
- {库名 版本 — 用途}

## 版本管理
- {Git / SVN / ...}

---

*最后更新：{日期}，技术负责人*
