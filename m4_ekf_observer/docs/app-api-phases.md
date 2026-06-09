# APP/API 层分相文档

## APP层架构

### 1. 电源管理模块 (`APP/POWER`)
- **核心文件**: `app_debug.h`
- **功能**: 
  - 提供调试接口（`DEBUG_PRINT()`宏）
  - 系统状态日志输出
  - 内存占用监控
- **调用链**: 
  ```c
  app_debug_init() -> sys_log_configure() -> uart_dma_transmit()
  ```
- **关键数据结构**: 
  ```c
  typedef struct {
    uint32_t log_level;
    uint8_t  buffer[LOG_BUFFER_SIZE];
    uint16_t write_ptr;
  } DebugContext;
  ```

### 2. 多核适配层
- **M0+实现**: 使用 `__ASM` 内联汇编处理异常
- **M3/M33差异**: 
  | 特性 | M0+ | M3 | M33 |
  |---|---|---|---|
  | MPU支持 | ❌ | ✅ | ✅ |
  | FPU集成 | ❌ | ❌ | ✅ |
  | TrustZone | ❌ | ❌ | ✅ |

## API层实现

### 1. CMSIS接口规范
- **启动流程**: 
  ```assembly
  Reset_Handler
    B    startup_code
    LDR    R0,=SystemInit
    BLX    R0
    LDR    R0,=main
    BX     R0
  ```
- **系统初始化**: 
  ```c
  void SystemInit(void) {
    SCB->CPACR |= 0x00F00000;
    SystemCoreClockUpdate();
    InitializeRegions();
  }
  ```

### 2. 内存分区管理
- **典型.sct配置**: 
  ```ld
  LR_IROM1 0x08000000 0x00080000 {
    ER_IROM1 0x08000000 0x00080000 {
      *(.isr_vector)
      *(InRoot$$Sections)
      .ANY (RESET, +First)
    }
    RW_IRAM1 0x20000000 0x00020000 {
      .ANY (.data, .bss)
    }
  }
  ```

### 3. 异常处理机制
- **HardFault处理**: 
  ```assembly
  HardFault_Handler
    TST    LR, #4
    ITE    EQ
    MRSEQ  R0, MSP
    MRSNE  R0, PSP
    B      Fault_Handler
  ```
- **NMI处理**: 
  ```c
  void NMI_Handler(void) {
    while(1);
  }
  ```

## 编译器适配层

| 编译器 | 特殊配置 | 输出格式 |
|---|---|---|
| ARM AC5 | `--cpu Cortex-M0+` | ELF32 |
| ARM AC6 | `--target Cortex-M0+` | ELF32 |
| GCC | `-mthumb -mcpu=cortex-m0plus` | ELF |
| IAR | `--cpu Cortex-M0+` | ICF |

**验证方法**: 
```bash
fromelf -c project.elf  # 查看编译器特性
arm-none-eabi-objdump -d project.elf > disasm.txt
```

**注意**: 当前文档聚焦APP层业务逻辑与CMSIS接口实现，未包含HAL层内容。是否需要补充其他模块？