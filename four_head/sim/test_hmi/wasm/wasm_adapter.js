/**
 * wasm_adapter.js — WASM 引擎适配器
 *
 * 加载 emcc 生成的 app_logic.js (MODULARIZE 格式),
 * 暴露与 JS 引擎一致的测试接口。
 *
 * 模式映射:
 *   C HmiGlobalNode_t → JS: 0=powered_off, 1=deep_sleep, 2=working, 3=paused
 *   C HmiZoneNode_t   → JS: 0=idle, 1=selecting, 2=cooking
 */
'use strict';

const GLOBAL_MODE_C2JS = ['powered_off', 'deep_sleep', 'working', 'paused'];
const ZONE_NODE_C2JS   = ['idle', 'selecting', 'cooking'];
const SEG_MODE_C2JS    = ['power', 'dash', 'off', 'ascii', 'timer_setting'];

/* key name → KeyCode_t enum value */
/* 键码映射 — 与 src/drv/drv_key.h 和 src/app/app_hmi.h 的 HMIKeyCode_t 一致 */
const KEY_CODE = {
    POWER: 1,       /* KEY_ONOFF / HMI_KEY_ONOFF */
    MINUS: 2,       /* KEY_SUB / HMI_KEY_SUB */
    PLUS: 3,        /* KEY_ADD / HMI_KEY_ADD */
    TIMER: 5,       /* KEY_TIME_SET / HMI_KEY_TIME_SET */
    PAUSE: 4,       /* KEY_STOP / HMI_KEY_STOP */
    ZONE: 7,        /* KEY_BIND / HMI_KEY_BIND */
    CHILD_LOCK: 7,  /* KEY_LOCK / HMI_KEY_LOCK */
    HEAD_4: 8,      /* KEY_RIGHT_P_SET / HMI_KEY_RIGHT_P_SET */
    HEAD_3: 9,      /* KEY_LEFT_P_SET / HMI_KEY_LEFT_P_SET */
    HEAD_2: 10,     /* KEY_RIGHT_P_SET_UP / HMI_KEY_RIGHT_P_SET_UP */
    HEAD_1: 11,     /* KEY_LEFT_P_SET_UP / HMI_KEY_LEFT_P_SET_UP */
    '0': 12, '1': 13, '2': 14, '3': 15, '4': 16,
    '5': 17, '6': 18, '7': 19, '8': 20, '9': 21
};

/* event: 'tap'→0x10, 'long'→0x02 */
const EVT_CODE = { tap: 0x10, long: 0x02 };

function createWasmEngine(wasmModule) {
    var _mod = wasmModule;

    function init() {
        _mod._engine_init();
    }

    function postKey(keyName, evtName) {
        var key = KEY_CODE[keyName];
        var evt = EVT_CODE[evtName];
        if (key === undefined || evt === undefined) {
            throw new Error('Unknown key/evt: ' + keyName + '/' + evtName);
        }
        _mod._engine_post_key(key, evt);
    }

    function tick100ms() { _mod._engine_tick_100ms(); }
    function tick1s()    { _mod._engine_tick_1s(); }

    function getState() {
        var heads = [];
        for (var i = 0; i < 4; i++) {
            heads.push({
                node: ZONE_NODE_C2JS[_mod._engine_get_zone_node(i)] || 'idle',
                power_level: _mod._engine_get_zone_power(i),
                boost_active: !!_mod._engine_get_zone_boost(i),
                timer_setting: !!_mod._engine_get_zone_timer_setting(i),
                timer_active: !!_mod._engine_get_zone_timer_active(i),
                timer_value: _mod._engine_get_zone_timer_value(i)
            });
        }
        var stack = [];
        var depth = _mod._engine_get_stack_depth();
        for (var j = 0; j < depth; j++) {
            stack.push(_mod._engine_get_stack_at(j));
        }
        return {
            global_mode: GLOBAL_MODE_C2JS[_mod._engine_get_global_mode()] || 'powered_off',
            child_lock: !!_mod._engine_is_child_lock(),
            paused: !!_mod._engine_is_paused(),
            hot_head: _mod._engine_get_hot_head(),
            select_stack: stack,
            heads: heads
        };
    }

    function getDisplay() {
        var seg_chars = [];
        for (var i = 0; i < 8; i++) {
            seg_chars.push(String.fromCharCode(_mod._engine_get_seg_char(i)));
        }
        var seg_blink = [];
        for (var j = 0; j < 4; j++) {
            seg_blink.push(!!_mod._engine_get_seg_blink(j));
        }
        var leds = {
            power: !!_mod._engine_get_led_power(),
            timer: !!_mod._engine_get_led_timer(),
            pause_btn: !!_mod._engine_get_led_pause(),
            child_lock: !!_mod._engine_get_led_child_lock(),
            head_select: [],
            power_level: []
        };
        for (var k = 0; k < 4; k++) {
            leds.head_select.push(!!_mod._engine_get_led_head_select(k));
        }
        for (var m = 0; m < 10; m++) {
            leds.power_level.push(!!_mod._engine_get_led_power_level(m));
        }
        return {
            seg_chars: seg_chars,
            seg_blink: seg_blink,
            seg_mode: SEG_MODE_C2JS[_mod._engine_get_seg_mode()] || 'power',
            leds: leds
        };
    }

    function forceSelectTimeout(idx) {
        /* 直接调用 C 侧 confirm_select, 不推进时间 */
        _mod._engine_force_select_confirm(idx);
    }

    function forceBoostTimeout(idx) {
        /* 直接调用 C 侧 exit_boost, 不推进时间 */
        _mod._engine_force_boost_exit(idx);
    }

    function forceTimerExpire(idx) {
        _mod._engine_force_timer_expire(idx);
    }

    return {
        _mod: _mod,  /* raw emscripten module for direct C function access */
        init: init,
        postKey: postKey,
        tick100ms: tick100ms,
        tick1s: tick1s,
        getState: getState,
        getDisplay: getDisplay,
        forceSelectTimeout: forceSelectTimeout,
        forceBoostTimeout: forceBoostTimeout,
        forceTimerExpire: forceTimerExpire
    };
}

/* Node.js 导出 */
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { createWasmEngine, GLOBAL_MODE_C2JS, ZONE_NODE_C2JS, SEG_MODE_C2JS, KEY_CODE, EVT_CODE };
}
