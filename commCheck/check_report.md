# 双模块连接管道检查报告 (v2.0)

> 检查范围：DrvKey → AppHmi + AppPower → AppHmi 指针直穿管道
> 生成时间：2026-06-12
> 检查目录：D:\OBSIDIAN\MOVING IH\ZEROLINK\four_head\src

---

## 1. 文件结构检查

| 文件 (全路径) | 状态 | 说明 |
|---------------|------|------|
| [std_module.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/core/std_module.h) | ✅ 存在 | v2.3 骨架宏定义 |
| [data_switcher.c](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/core/data_switcher.c) | ✅ 存在 | PULL 路由调度器 + 多通道并行 InputCallback |
| [drv_key_io.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/include/drv_key_io.h) | ✅ 存在 | DrvKey IO 接口定义 |
| [app_hmi_io.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/include/app_hmi_io.h) | ✅ 存在 | AppHmi IO 接口定义 |
| [app_power_io.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/include/app_power_io.h) | ✅ 存在 | AppPower IO 接口定义 |
| [drv_key.c](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/drv/drv_key.c) | ✅ 存在 | DrvKey 实现 |
| [app_hmi.c](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/app/app_hmi.c) | ✅ 存在 | AppHmi 实现 |

---

## 2. 多通道并行处理检查

### 2.1 通道架构

```
┌─────────────────────────────────────────────────────────────┐
│              AppHmi_InputCallback()                        │
│                      │                                     │
│         ┌────────────┼────────────┐                        │
│         ▼            ▼            ▼                        │
│   ┌──────────┐  ┌──────────┐  ┌──────────┐                │
│   │  通道1   │  │  通道2   │  │  通道N   │                │
│   │ DrvKey   │  │AppPower  │  │   ...    │                │
│   │  →Hmi    │  │  →Hmi    │  │          │                │
│   └────┬─────┘  └────┬─────┘  └────┬─────┘                │
│        │             │             │                       │
│        ▼             ▼             ▼                       │
│   独立检查      独立检查      独立检查                      │
│        │             │             │                       │
│        └─────────────┴─────────────┘                       │
│                      │                                     │
│                      ▼                                     │
│            并行更新 AppHmi_Input                           │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 通道独立性验证

| 通道 | Producer | Consumer | 独立检查 | 并行处理 |
|------|----------|----------|----------|----------|
| 通道1 | DrvKey | AppHmi.key | ✅ | ✅ |
| 通道2 | AppPower | AppHmi.power | ✅ | ✅ |

---

## 3. InputCallback 实现检查

### 3.1 函数定义

**文件**：[data_switcher.c](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/core/data_switcher.c#L98-L132)

```c
void AppHmi_InputCallback(void)
{
    Para_Grp_t *pIn = s_slots[SLOT_HMI].pIn;
    if (!pIn || !pIn->para) return;

    AppHmi_Input *hmi_in = (AppHmi_Input *)pIn->para;

    /* ========== 通道1: DrvKey → AppHmi (按键事件) ========== */
    {
        Para_Grp_t *pOut = s_slots[SLOT_KEY].pOut;
        if (pOut && pOut->para) {
            DrvKey_Output *key_out = (DrvKey_Output *)pOut->para;
            if (key_out->to_hmi && (key_out->to_hmi->status & ST_NEW)) {
                hmi_in->key = (Key_to_Hmi_Input_Link *)key_out->to_hmi;
                hmi_in->key->status |= ST_NEW;
            }
        }
    }

    /* ========== 通道2: AppPower → AppHmi (功率状态) ========== */
    {
        Para_Grp_t *pOut = s_slots[SLOT_POWER].pOut;
        if (pOut && pOut->para) {
            AppPower_Output *power_out = (AppPower_Output *)pOut->para;
            if (power_out->to_hmi && (power_out->to_hmi->status & ST_NEW)) {
                hmi_in->power = (Power_to_Hmi_Input_Link *)power_out->to_hmi;
                hmi_in->power->status |= ST_NEW;
            }
        }
    }
}
```

### 3.2 检查项

| 检查点 | 通道1 | 通道2 | 状态 |
|--------|-------|-------|------|
| 独立代码块 | ✅ | ✅ | PASS |
| 空指针检查 (pOut) | ✅ | ✅ | PASS |
| 空指针检查 (pOut->para) | ✅ | ✅ | PASS |
| 空指针检查 (to_hmi) | ✅ | ✅ | PASS |
| ST_NEW 检查 | ✅ | ✅ | PASS |
| 指针直穿赋值 | ✅ | ✅ | PASS |
| 置 ST_NEW 标志 | ✅ | ✅ | PASS |

---

## 4. 接口定义一致性检查

### 4.1 DrvKey Output vs AppHmi Input

**文件**：[drv_key_io.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/include/drv_key_io.h) vs [app_hmi_io.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/include/app_hmi_io.h)

| 字段 | DrvKey OUTPUT_PARAMS | AppHmi INPUT_PARAMS | 一致性 |
|------|----------------------|---------------------|--------|
| key_code | uint8_t | uint8_t | ✅ |
| key_state | uint8_t | uint8_t | ✅ |
| head_index | uint8_t | uint8_t | ✅ |
| res[1] | uint8_t[1] | uint8_t[1] | ✅ |

### 4.2 AppPower Output vs AppHmi Input

**文件**：[app_power_io.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/include/app_power_io.h) vs [app_hmi_io.h](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/include/app_hmi_io.h)

| 字段 | AppPower OUTPUT_PARAMS | AppHmi INPUT_PARAMS | 一致性 |
|------|------------------------|---------------------|--------|
| head_index | uint8_t | uint8_t | ✅ |
| power_on | uint8_t | uint8_t | ✅ |
| power_level | uint8_t | uint8_t | ✅ |
| res[1] | uint8_t[1] | uint8_t[1] | ✅ |
| actual_power | uint16_t | uint16_t | ✅ |

---

## 5. LINK 结构一致性检查

### 5.1 Key_to_Hmi LINK

| 字段 | DrvKey OUTPUT_LINK | AppHmi INPUT_LINK | 一致性 |
|------|--------------------|--------------------|--------|
| status | uint8_t | uint8_t | ✅ |
| max_count | uint8_t | uint8_t | ✅ |
| count | uint8_t | uint8_t | ✅ |
| res[1] | uint8_t[1] | uint8_t[1] | ✅ |
| *params | Key_to_Hmi_Params* | Key_to_Hmi_Params* | ✅ |

### 5.2 Power_to_Hmi LINK

| 字段 | AppPower OUTPUT_LINK | AppHmi INPUT_LINK | 一致性 |
|------|----------------------|--------------------|--------|
| status | uint8_t | uint8_t | ✅ |
| max_count | uint8_t | uint8_t | ✅ |
| count | uint8_t | uint8_t | ✅ |
| res[1] | uint8_t[1] | uint8_t[1] | ✅ |
| *params | Power_to_Hmi_Params* | Power_to_Hmi_Params* | ✅ |

---

## 6. 数据流完整性

### 6.1 通道1: DrvKey → AppHmi

```
[drv_key.c](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/drv/drv_key.c)
    └─> Key_PostEvent() 写 PARAMS + 置 ST_NEW
              │
              ▼
[data_switcher.c](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/core/data_switcher.c)
    └─> AppHmi_InputCallback() → hmi_in->key = key_out->to_hmi
              │
              ▼
[app_hmi.c](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/app/app_hmi.c)
    └─> ProcessInput() 消费数据
```

### 6.2 通道2: AppPower → AppHmi

```
[app_power.c](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/app/app_power.c)
    └─> ProcessInput() 写 PARAMS + 置 ST_NEW
              │
              ▼
[data_switcher.c](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/core/data_switcher.c)
    └─> AppHmi_InputCallback() → hmi_in->power = power_out->to_hmi
              │
              ▼
[app_hmi.c](file:///D:/OBSIDIAN/MOVING%20IH/ZEROLINK/four_head/src/app/app_hmi.c)
    └─> ProcessInput() 消费数据
```

---

## 7. 状态位协议检查

| 状态位 | 置位位置 | 清零位置 | 检查结果 |
|--------|----------|----------|----------|
| ST_INIT | Constructor() | 永不清零 | ✅ |
| ST_NEW | InputCallback() | ProcessInput() 消费后 | ✅ |
| ST_OUT | Producer ProcessInput | _route_xxx() 路由后 | ✅ |

---

## 8. 代码风格检查

| 检查项 | 状态 | 文件 |
|--------|------|------|
| 宏命名规范 | ✅ PASS | MODULE_OUTPUT_PARAMS/INPUT_PARAMS/LINK |
| 文件命名规范 | ✅ PASS | 模块名_io.h |
| Include 路径 | ✅ PASS | 使用全路径 #include "include/xxx_io.h" |
| 注释规范 | ✅ PASS | 关键函数和结构体有注释 |
| 缩进风格 | ✅ PASS | 统一使用 4 空格缩进 |

---

## 9. 总结

| 检查类别 | 通过 | 说明 |
|----------|------|------|
| 文件结构 | ✅ | 全部存在 (7个文件) |
| 多通道并行 | ✅ | 独立检查，互不影响 |
| 接口一致性 | ✅ | 布局完全匹配 |
| LINK 一致性 | ✅ | 指针类型匹配 |
| 指针直穿 | ✅ | 零拷贝实现 |
| 状态位协议 | ✅ | 正确实现 |
| 代码风格 | ✅ | 符合规范 |

**结论**：✅ **多通道并行处理实现正确**

---

*检查报告生成完毕*
