@echo off
D:\emsdk\upstream\emscripten\emcc.bat ^
  -O1 ^
  -s MODULARIZE=1 ^
  -s EXPORT_NAME="AppLogicModule" ^
  -s EXPORTED_FUNCTIONS="['_engine_init','_engine_post_key','_engine_tick_100ms','_engine_tick_1s','_engine_get_global_mode','_engine_is_child_lock','_engine_is_paused','_engine_get_hot_head','_engine_get_stack_depth','_engine_get_stack_at','_engine_get_zone_node','_engine_get_zone_power','_engine_get_zone_boost','_engine_get_zone_timer_setting','_engine_get_zone_timer_active','_engine_get_zone_timer_value','_engine_get_seg_char','_engine_get_seg_blink','_engine_get_seg_mode','_engine_get_led_power','_engine_get_led_timer','_engine_get_led_pause','_engine_get_led_child_lock','_engine_get_led_head_select','_engine_get_led_power_level','_engine_force_select_confirm','_engine_force_boost_exit','_engine_force_timer_expire','_Key_HeadKeyToIndex','_Key_HeadIndexToKey','_Key_DigitKeyToLevel','_Key_IsHeadKey']" ^
  -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap']" ^
  -I. ^
  modules/main.c modules/key_module.c modules/timer_module.c modules/display_module.c modules/state_module.c cfg/hmi_data.c ^
  -o app_logic.js
