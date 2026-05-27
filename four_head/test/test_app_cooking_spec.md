# app_cooking.c 独立自动化测试规格书

> 目标: 不依赖任何其他模块，独立编译、独立运行、自动验证
> 方式: 重定义 __weak 回调捕获输出 + 直接调用模块入口函数 + assert 断言

---

## 一、模块边界

```
输入 (__weak 接收)                    输出 (__weak 发出)
─────────────────────                ─────────────────────
AppCooking_OnKey(param, data)        AppPower_OnPowerCtrl(param, data)
AppCooking_OnRegData(param, data)    DrvDisplay_OnRefresh(param, data)
AppCooking_OnTimer1s(param, data)
```

**内部状态**: `s_ctx[4]` (4炉头 × CookCtx_t)，每个包含 state/menu_id/step_index/step_timer/current_temp/user_power_lv/boil_samples/boil_temp_acc
**内部函数**: `detect_boil()` / `temp_hysteresis()` / `post_power_cmd()` / `post_display()` / `on_cooking_ctrl()`

---

## 二、测试骨架结构

```c
// test_app_cooking.h
#ifndef TEST_APP_COOKING_H
#define TEST_APP_COOKING_H

// 测试模块内部状态的可选扩展（通过 app_cooking.c 的 static 消除后）
// 当前无额外声明需求

#endif
```

```c
// test_app_cooking.c
#include "app_cooking.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ========== 测试捕获全局变量 ========== */
static int      g_power_ctrl_count = 0;
static uint8_t  g_power_head       = 0xFF;
static uint8_t  g_power_onoff      = 0;
static uint16_t g_power_target     = 0;
static uint8_t  g_power_level      = 0;
static uint8_t  g_power_mode       = 0;

static int      g_disp_count       = 0;
static uint8_t  g_disp_head        = 0xFF;
static uint8_t  g_disp_cmd         = 0;

/* 测试辅助: 清除捕获状态 */
static void reset_captures(void)
{
    g_power_ctrl_count = 0;
    g_power_head       = 0xFF;
    g_power_onoff      = 0;
    g_power_target     = 0;
    g_power_level      = 0;
    g_power_mode       = 0;

    g_disp_count       = 0;
    g_disp_head        = 0xFF;
    g_disp_cmd         = 0;
}

/* ========== __weak 回调: 被测试库调用的捕获 ========== */
void AppPower_OnPowerCtrl(uint16_t param, void *data_ptr)
{
    /* data_ptr → LocalPowerCtrl_t (app_cooking.c:31) */
    typedef struct {
        uint8_t  head_index;
        uint8_t  onoff;
        uint16_t target_power;
        uint8_t  power_level;
        uint8_t  work_mode;
        uint16_t target_temp;
    } PowerCtrl_t;

    PowerCtrl_t *p = (PowerCtrl_t *)data_ptr;
    g_power_ctrl_count++;
    if (p) {
        g_power_head   = p->head_index;
        g_power_onoff  = p->onoff;
        g_power_target = p->target_power;
        g_power_level  = p->power_level;
        g_power_mode   = p->work_mode;
    }
}

void DrvDisplay_OnRefresh(uint16_t param, void *data_ptr)
{
    (void)data_ptr;
    g_disp_count++;
    g_disp_head = (uint8_t)((param >> 8) & 0xFF);
    g_disp_cmd  = (uint8_t)(param & 0xFF);
}
```

---

## 三、测试用例（12个，按优先级排列）

### P0: 基础功能 (6个)

| # | 用例 | 步骤 | 预期 |
|---|------|------|------|
| T1 | 初始化后所有炉头OFF | `App_Cooking_Init()` | s_ctx[0..3].state == COOK_STA_OFF |
| T2 | 启动保温菜单(炉头0) | KEY_MENU按下(0x04/PRESS) → 1s定时器触发 | `AppPower_OnPowerCtrl` 收到: head=0, power=200W(档位3对应s_power_table[3]=400W... 实际由temp_hysteresis决定初始值) |
| T3 | 停止运行中的炉头 | KEY_STOP按下(0x02/PRESS) | `AppPower_OnPowerCtrl` 收到: head=X, onoff=0 |
| T4 | 暂停→恢复 | COOK_CMD_PAUSE → 1s后 → COOK_CMD_RESUME → 1s后 | 暂停期间不发功率命令; 恢复后恢复 |
| T5 | 用户调速(启动火锅后按档位5→3) | KEY_MENU→火锅 → KEY_POWER_5 → KEY_POWER_3 → 1s后 | 功率从档位5切换为档位3 |
| T6 | 煮沸检测(温度上升并在目标附近稳定) | 注入 reg_data 使温度从25→95°C逐秒上升 → 稳定在97°C | `detect_boil()` 返回1 → 推进到下一步 |

### P1: 边界/异常 (4个)

| # | 用例 | 步骤 | 预期 |
|---|------|------|------|
| T7 | 无效head_idx被忽略 | `AppCooking_OnKey(0x1F01)` (head=15) | 无崩溃, 无功率命令发出 |
| T8 | 无效菜单ID | COOK_PARAM(0, START, 99) → `on_cooking_ctrl()` | 不启动, state保持OFF |
| T9 | 温度滞后控制边界: 低于目标-3°C | 设current_temp=50, target=65 | 全功率(`>0`) |
| T10 | 温度滞后控制边界: 高于目标 | 设current_temp=70, target=65 | 停止加热(power=0) |

### P2: 菜单流程 (2个)

| # | 用例 | 步骤 | 预期 |
|---|------|------|------|
| T11 | 煮沸→慢炖完整流程启动 | 启动菜单3 → 检测到沸腾 → 应切换到慢炖步骤 | COOK_STEP_BOIL完成后自动推进到COOK_STEP_POWER(档位2,300W) |
| T12 | 菜单完成自动关机 | 启动菜单2无计时步骤 → 应一步完成 → state变为COMPLETE | state == COOK_STA_COMPLETE |

---

## 四、测试编译与运行

```bash
# 仅需 app_cooking.c + app_cooking.h，零其他模块依赖
armcc -c --cpu Cortex-M0+ -DSC32L14xx --c99 \
    -I "Claude/app" \
    -o "test_output/test_app_cooking.o" \
    "test_app_cooking.c"

armcc -c --cpu Cortex-M0+ -DSC32L14xx --c99 \
    -I "Claude/app" \
    -o "test_output/app_cooking.o" \
    "Claude/app/app_cooking.c"

# 链接成独立测试可执行文件(需libc支持printf/assert)
armlink --elf --no_merge \
    "test_output/test_app_cooking.o" \
    "test_output/app_cooking.o" \
    -o "test_output/test_cooking.axf"

# 运行 (如在模拟器/开发板上)
# 或交叉编译到x86运行:
# gcc test_app_cooking.c ../Claude/app/app_cooking.c -I ../Claude/app -o test_cooking
# ./test_cooking
```

---

## 五、断言示例

```c
/* T2: 启动保温菜单 */
static void test_start_keepwarm(void)
{
    App_Cooking_Init();
    reset_captures();

    /* 模拟KEY_MENU按下 */
    AppCooking_OnKey(((uint16_t)0x01 << 8) | 0x04, NULL);  /* KEY_MENU, PRESS */
    AppCooking_OnTimer1s(0, NULL);  /* 触发1s节拍处理 */

    /* 验证发出了功率命令 */
    assert(g_power_ctrl_count >= 1);
    assert(g_power_head == 0);
    assert(g_power_onoff == 1);  /* 开启 */
    assert(g_power_level == 3);  /* 保温默认档位3 */

    printf("[PASS] T2: 启动保温菜单\n");
}

/* T3: 停止炉头 */
static void test_stop_head(void)
{
    App_Cooking_Init();
    reset_captures();

    /* 先启动 */
    AppCooking_OnKey(((uint16_t)0x01 << 8) | 0x04, NULL);  /* MENU */
    reset_captures();

    /* 再停止 */
    AppCooking_OnKey(((uint16_t)0x01 << 8) | 0x02, NULL);  /* STOP */
    AppCooking_OnTimer1s(0, NULL);

    assert(g_power_ctrl_count >= 1);
    assert(g_power_onoff == 0);  /* 关闭 */

    printf("[PASS] T3: 停止炉头\n");
}

/* 主入口 */
int main(void)
{
    App_Cooking_Init();

    test_start_keepwarm();   /* T2 */
    test_stop_head();        /* T3 */
    /* ... 其余用例 */

    printf("\n=== ALL TESTS PASSED ===\n");
    return 0;
}
```

---

## 六、当前app_cooking.c对测试的阻碍

| 阻碍 | 位置 | 建议 |
|------|------|------|
| `s_ctx[]`, `s_power_cmd`, `s_selected_head` 全是 `static` | 模块级 | 测试时可通过断言外部输出来间接验证，不需要直接访问内部状态（黑盒测试） |
| `on_cooking_ctrl()` 是 `static` | 无法直接调用 | 通过公共入口 `AppCooking_OnKey()` 和 `AppCooking_OnTimer1s()` 间接触发（已足够） |
| `detect_boil()`, `temp_hysteresis()` 是 `static` | 无法单独测试 | 通过输入已知温度值调用1s定时器来间接测试（已包含在T6/T9/T10） |
| `s_power_table[]` 是 `static` | 无法验证具体值 | 通过捕获 `AppPower_OnPowerCtrl` 的 `target_power` 来验证 |

**结论**: 不需要修改 app_cooking.c 一行代码。测试骨架在外部通过 __weak 重定义 + 入口函数直调 + 输出捕获 完成全黑盒测试。
