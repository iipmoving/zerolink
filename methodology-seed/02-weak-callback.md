# 02 — __weak 回调零依赖通信模式

---

## 一、原理

```c
/* === 发送方模块 === */

/* 定义空壳: 如果没人接收，静默丢弃 */
__weak void AppHmi_OnKey(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }

/* 发送时直接调用 (不经过任何中间层) */
static void Key_PostEvent(uint8_t key_code, uint8_t key_state)
{
    uint16_t param = (uint16_t)key_code | ((uint16_t)key_state << 8);
    AppHmi_OnKey(param, NULL);       /* 调用 __weak 声明 */
    AppCooking_OnKey(param, NULL);   /* 多接收方: 逐一调用 */
}
```

```c
/* === 接收方模块 === */

/* 强符号实现: 链接器自动覆盖发送方的 __weak 空壳 */
void AppHmi_OnKey(uint16_t param, void *data_ptr)
{
    uint8_t key_code  = (uint8_t)(param & 0xFFu);
    uint8_t key_state = (uint8_t)((param >> 8) & 0xFFu);
    (void)data_ptr;
    /* ... 业务处理 ... */
}
```

### 链接器行为

```
发送方定义: __weak void AppHmi_OnKey(...) { }   ← 总是存在(空壳)
接收方定义:        void AppHmi_OnKey(...) { }   ← 如果存在则覆盖

结果:
  有接收方 → 链接器选强符号 → 接收方函数被调用
  无接收方 → 链接器选弱符号 → 空壳运行, 静默丢弃, 无运行时错误
```

---

## 二、与消息队列方案对比

| 方面 | 消息队列方案 | __weak 直调 |
|------|-------------|-------------|
| 发送代码 | `Msg_Post(ID, param, &data)` | `Receiver_OnXxx(param, &data)` |
| 接收注册 | `Register(ID, handler)` | 强符号同名函数 |
| 消息 ID | 需要，全局唯一 | 不需要 |
| 跨模块 include | `msg_scheduler.h` | 无 |
| 队列 | 环形队列 | 无，直接调 |
| 异步性 | 消费端异步 | 同步直调 |
| 运行时内存 | 队列缓冲+消息体 | 0 |
| 多接收方 | 多次 Msg_Post | 多次函数调用 |
| 独立编译 | 需 msg_scheduler.o | 零外部依赖 |

---

## 三、实施步骤

### Step 1: 发送方添加 __weak 声明
```c
__weak void Receiver_OnXxx(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }
```

### Step 2: 发送方调用
```c
/* 替换原有的 Msg_Post(ID, ...) */
Receiver_OnXxx(param, &data);
```

### Step 3: 接收方强实现
```c
/* 去掉 MsgId_t 参数，改用约定函数名 */
void Receiver_OnXxx(uint16_t param, void *data_ptr) {
    /* 业务逻辑不变 */
}
```

### Step 4: 多接收方广播
```c
/* 为每个接收方声明独立的 __weak 空壳，逐一调用 */
Receiver1_OnData(param, &data);
Receiver2_OnData(param, &data);
Receiver3_OnData(param, &data);
```

### Step 5: 更新 interface_map.h
```c
/* Pair X: module_a → module_b */
/* 发送方: module_a.c  WEAK void ModuleB_OnEvent(uint16_t param, void *data_ptr) {} */
/* 接收方: module_b.c  void ModuleB_OnEvent(uint16_t param, void *data_ptr) */
```

---

## 四、WEAK 宏（跨编译器兼容）

```c
/* weak_macro.h */
#if defined(__ARMCC_VERSION)
  #define WEAK __weak
#elif defined(__EMSCRIPTEN__)
  #define WEAK __attribute__((weak))
#elif defined(__GNUC__)
  #define WEAK __attribute__((weak))
#else
  #error "Unsupported compiler: define WEAK macro manually"
#endif
```

---

## 五、命名约定

```
格式: {ModulePrefix}_On{Event}(uint16_t param, void *data_ptr)

APP 模块: App{Name}_On{Event}
  例: AppHmi_OnKey, AppPower_OnPowerCtrl

DRV 模块: Drv{Name}_On{Event}
  例: DrvDisplay_OnRefresh, DrvBuzzer_OnCtrl

所有 __weak 回调和强符号实现必须使用相同签名。
param:  轻量数据(键码、索引、布尔值), 直接传值
data_ptr: 复杂数据指针, NULL 表示无数据
返回值: 统一 void
```

---

## 六、限制与注意

### 同步性
__weak 直调是同步的——调用即执行。ISR 中不能直接调用耗时函数。应所有调用在主循环中，ISR 只设标志。

### 单可执行文件
__weak 依赖链接器在编译时解析。所有模块必须链接到同一个可执行文件。不适用于动态加载插件。

### 多接收方
每个接收方需要独立的 __weak 空壳声明和独立的调用语句。发送方显式知道所有接收方（通过函数名），但不知道接收方是谁、做什么。
