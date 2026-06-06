# Makefile — WASM 双引擎测试 (直接编译 src/ 源码)
# 用法: make build   (编译C→WASM)
#       make test    (运行测试, 需先 build)
#       make clean   (删除编译产物)

EMCC = D:/emsdk/upstream/emscripten/emcc.bat
CFLAGS = -O1 -s MODULARIZE=1 -s EXPORT_NAME="AppLogicModule"
CFLAGS += -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap']"
CFLAGS += -I. -I../../src -DWASM_BUILD

EXPORTED_FUNCS = _engine_init,_engine_post_key,_engine_tick_100ms,_engine_tick_1s,\
	_engine_get_global_mode,_engine_is_child_lock,_engine_is_paused,\
	_engine_get_hot_head,_engine_get_stack_depth,_engine_get_stack_at,\
	_engine_get_zone_node,_engine_get_zone_power,_engine_get_zone_boost,\
	_engine_get_zone_timer_setting,_engine_get_zone_timer_active,\
	_engine_get_zone_timer_value,\
	_engine_get_seg_char,_engine_get_seg_blink,_engine_get_seg_mode,\
	_engine_get_led_power,_engine_get_led_timer,_engine_get_led_pause,\
	_engine_get_led_child_lock,_engine_get_led_head_select,\
	_engine_get_led_power_level,\
	_engine_force_select_confirm,_engine_force_boost_exit,_engine_force_timer_expire,\
		_Key_HeadKeyToIndex,_Key_HeadIndexToKey,_Key_DigitKeyToLevel,_Key_IsHeadKey

# 直接编译 src/ 源码 + wasm_bridge 桩
SRCS = wasm_bridge.c \
       ../../src/app/app_hmi.c \
       ../../src/cfg/hmi_data.c

OUTPUT = app_logic.js

.PHONY: all build test clean wasm_test

all: build test

build: $(OUTPUT)

$(OUTPUT): $(SRCS)
	$(EMCC) $(CFLAGS) -s EXPORTED_FUNCTIONS="[$(EXPORTED_FUNCS)]" $(SRCS) -o $@

test: build
	node ../wasm/test_wasm_basic.js

clean:
	rm -f app_logic.js app_logic.wasm
