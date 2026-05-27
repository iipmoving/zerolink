## AI嵌入式编程约束（完整最终版）

### 一、分层架构

text

APP层（纯消息驱动，只收发消息）
     ↑↓ 消息（唯一共享头文件 msg_def.h）
API层（订阅消息→调用Drv→回复消息）
     ↑↓ 函数调用
Drv层（纯硬件操作，不知道消息机制）
     ↑↓ 函数调用
HAL层（寄存器操作）

**头文件隔离：**

- APP层：只能包含 `msg_def.h`
    
- API层：可包含 `msg_def.h` + `drv_*.h`
    
- Drv层：只能包含 `hal_*.h`，**禁止**包含 `msg_def.h`
    
- 任何层不能反向包含上层头文件
    

**API调用Drv命名规则：**

c

int api_xxx_from_uart1_i2c2(void);  // 注明调用的Drv来源

---

### 二、类型与内存

- 用 `stdint.h` 类型，禁用 `int`/`char`
    
- 按范围选最小类型：0-255用`uint8_t`，256-65535用`uint16_t`
    
- **STM32**：可用32位（`uint32_t`），效率高
    
- **8051**：优先用16位（`uint16_t`），**不用32位**（除非显式声明 `__attribute__((xx))` 或 `xdata`）。原因：32位运算需要多个指令周期，RAM占用大，KEIL C51会插入大量库函数，显著降低性能
    
- 布尔值用位域
    
- 常量字符串强制放ROM：STM32用 `const` + 属性，8051用 `code`
    

---

### 三、浮点与定点

- **禁止浮点**（不用 `float`/`double`）
    
- 全部用**定点数**：
    

c

// 例如：温度范围 -40.00 ~ 125.00
// 用 int16_t，约定单位 0.01°C
int16_t temperature = 2530;  // 表示 25.30°C

- 定点数在注释中注明单位和精度
    

---

### 四、对齐规则

- STM32内部结构体：`#pragma pack(4)`
    
- 通信消息结构体：`#pragma pack(1)`
    
- 8051：通信消息用 `#pragma pack(1)`，内部自然对齐
    
- 必须加 `_Static_assert` 验证关键结构体大小和偏移
    

---

### 五、位域

- 所有bool标志集中定义在位域结构体中
    
- 位域必须用 `uint8_t` 类型
    
- 未使用位显式命名 `reserved`
    
- 提供 `flags ↔ uint8_t` 转换函数用于跨语言
    
- 多任务/中断中修改位域必须关中断
    

---

### 六、阻塞与等待

- 禁止无退出条件的循环
    
- 所有循环必须有：超时退出 + 最大重试次数
    
- 用状态机替代阻塞式延时
    

---

### 七、状态确认

- 外部输入连续采样 ≥3次，2次一致算有效
    
- 数据变化后才开始确认
    

---

### 八、异常处理

- 所有错误必须有明确处理路径（重试/降级/复位/报警）
    
- 禁止吞掉错误
    

---

### 九、数据有效性检查

每次使用数据前检查：指针非空、范围合法、CRC正确、时间戳未过期

---

### 十、编译期验证清单

c

_Static_assert(sizeof(struct_t) == SIZE, "size mismatch");
_Static_assert(offsetof(struct_t, field) == OFF, "offset mismatch");
_Static_assert(sizeof(flags_t) == 1, "flags size error");
// 分层检查
#ifdef MSG_DEF_H
#error "Drv层不能包含消息定义头文件"
#endif

---

### 十一、AI生成代码自检清单

- 分层正确（APP只含msg_def.h，Drv不含msg_def.h）
    
- 类型按范围选最小
    
- 8051没用32位（除非显式声明）
    
- 没有浮点，全用定点
    
- 对齐正确，有static_assert
    
- 位域用uint8_t，有reserved位
    
- 循环有超时退出
    
- 输入有多次确认
    
- 错误有处理路径
    
- 数据使用前有有效性检查