---
name: led-test-mode
description: "HMI_DEBUG_KEYS is the only LED test entry, no separate DRV-level test function needed"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 8468b018-0dc2-4dea-925a-f289e7ef9c0b
---

唯一 LED 测试入口: `#define HMI_DEBUG_KEYS` → 长按开关进入 → `test_led_set`/`test_led_show` (app_hmi.c)
- 上屏 L+序号, 下屏 灯名, +/- 切灯
- HMI 测试序号: 0=电源 1=定时 2=暂停 3=童锁 4-7=炉头选中 8-17=档位L0-L9
- 映射走正常路径: test_led_set → leds_* → post_display → sync_hmi_display → s_mode_map/s_lvl_map → IO

**Why:** 用户用此模式验证按键-显示-灯对齐。不可再创建第二套测试(如 Drv_Display_TestSingleLED)，两套序号体系不同会导致混乱。
**How to apply:** 所有 LED 映射修改后，通过此 HMI 测试模式验证，不用改 drv_display.c 的测试函数。
