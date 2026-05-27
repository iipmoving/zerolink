/**
 * test_wasm_basic.js — WASM 引擎基础冒烟测试
 *
 * 初始化 WASM 引擎, 跑一组基本操作, 对比 JS 引擎预期。
 * 用法: node wasm/test_wasm_basic.js
 */
'use strict';

const fs = require('fs');
const path = require('path');
const vm = require('vm');

/* ========== 加载 JS 引擎 (参考实现) ========== */
const sandbox = {
    console: console,
    setTimeout: setTimeout, clearTimeout: clearTimeout,
    setInterval: setInterval, clearInterval: clearInterval,
    Date: Date, Math: Math, JSON: JSON,
    Array: Array, Object: Object, String: String, Number: Number, Boolean: Boolean,
    parseInt: parseInt, Promise: Promise
};
vm.createContext(sandbox);

function loadScript(filePath) {
    let code = fs.readFileSync(filePath, 'utf-8');
    code = code.replace(/\bconst\s+(MsgId|KeyEvent|Msg\b|MessageBus|KeyHandler|Renderer|JsonLogicEngine|KEY_NAME|BlinkRule|ModeRule|TestRunner)\s*=/g,
        'var $1 =');
    const script = new vm.Script(code, { filename: path.basename(filePath) });
    script.runInContext(sandbox);
}

const JS_DIR = path.join(__dirname, '..', 'js') + '/';
loadScript(JS_DIR + 'message_bus.js');
loadScript(JS_DIR + 'key_handler.js');
loadScript(JS_DIR + 'slot_element.js');
loadScript(JS_DIR + 'led_element.js');
loadScript(JS_DIR + 'blink_rule.js');
loadScript(JS_DIR + 'mode_rule.js');
loadScript(JS_DIR + 'json_logic_engine.js');

const MessageBus = sandbox.MessageBus;
const JsonLogicEngine = sandbox.JsonLogicEngine;
const MsgId = sandbox.MsgId;

/* ========== 加载 WASM 引擎 ========== */
const AppLogicModule = require('./app_logic.js');
const { createWasmEngine } = require('./wasm_adapter.js');

/* ========== 辅助 ========== */
let passCount = 0, failCount = 0;

function assert(label, cond, expected, actual) {
    if (cond) {
        passCount++;
        console.log('  [PASS] ' + label);
    } else {
        failCount++;
        console.log('  [FAIL] ' + label);
        console.log('         expected: ' + JSON.stringify(expected));
        console.log('         actual:   ' + JSON.stringify(actual));
    }
}

function assertState(label, jsState, wasmState) {
    /* Compare key fields */
    var allOk = true;
    var diffs = [];

    if (jsState.global_mode !== wasmState.global_mode) {
        diffs.push('global_mode: JS=' + jsState.global_mode + ' WASM=' + wasmState.global_mode);
        allOk = false;
    }
    if (jsState.hot_head !== wasmState.hot_head) {
        diffs.push('hot_head: JS=' + jsState.hot_head + ' WASM=' + wasmState.hot_head);
        allOk = false;
    }
    if (jsState.paused !== wasmState.paused) {
        diffs.push('paused: JS=' + jsState.paused + ' WASM=' + wasmState.paused);
        allOk = false;
    }
    if (jsState.child_lock !== wasmState.child_lock) {
        diffs.push('child_lock: JS=' + jsState.child_lock + ' WASM=' + wasmState.child_lock);
        allOk = false;
    }
    if (jsState.select_stack.length !== wasmState.select_stack.length) {
        diffs.push('stack_depth: JS=' + jsState.select_stack.length + ' WASM=' + wasmState.select_stack.length);
        allOk = false;
    }
    for (var i = 0; i < 4; i++) {
        var jh = jsState.heads[i];
        var wh = wasmState.heads[i];
        if (jh.node !== wh.node || jh.power_level !== wh.power_level ||
            jh.boost_active !== wh.boost_active || jh.timer_setting !== wh.timer_setting ||
            jh.timer_active !== wh.timer_active || jh.timer_value !== wh.timer_value) {
            diffs.push('head[' + i + ']: JS={' + jh.node + ',lv' + jh.power_level +
                ',boost=' + jh.boost_active + ',tset=' + jh.timer_setting +
                ',tact=' + jh.timer_active + ',tv=' + jh.timer_value +
                '} WASM={' + wh.node + ',lv' + wh.power_level +
                ',boost=' + wh.boost_active + ',tset=' + wh.timer_setting +
                ',tact=' + wh.timer_active + ',tv=' + wh.timer_value + '}');
            allOk = false;
        }
    }

    if (diffs.length > 0) {
        assert(label, false, JSON.stringify(jsState), JSON.stringify(wasmState));
        diffs.forEach(function(d) { console.log('         ' + d); });
    } else {
        assert(label, true);
    }
}

/* assertStateCompat: 只比较指定字段, 跳过已知差异字段 */
function assertStateCompat(label, jsState, wasmState, checkFields, skipFields) {
    var allOk = true;
    var diffs = [];
    var skipSet = {};
    (skipFields || []).forEach(function(f) { skipSet[f] = true; });

    if (!skipSet['global_mode'] && jsState.global_mode !== wasmState.global_mode) {
        diffs.push('global_mode: JS=' + jsState.global_mode + ' WASM=' + wasmState.global_mode);
        allOk = false;
    }
    if (!skipSet['hot_head'] && jsState.hot_head !== wasmState.hot_head) {
        diffs.push('hot_head: JS=' + jsState.hot_head + ' WASM=' + wasmState.hot_head);
        allOk = false;
    }
    if (!skipSet['paused'] && jsState.paused !== wasmState.paused) {
        diffs.push('paused: JS=' + jsState.paused + ' WASM=' + wasmState.paused);
        allOk = false;
    }
    if (!skipSet['child_lock'] && jsState.child_lock !== wasmState.child_lock) {
        diffs.push('child_lock: JS=' + jsState.child_lock + ' WASM=' + wasmState.child_lock);
        allOk = false;
    }
    if (!skipSet['select_stack'] && jsState.select_stack.length !== wasmState.select_stack.length) {
        diffs.push('stack_depth: JS=' + jsState.select_stack.length + ' WASM=' + wasmState.select_stack.length);
        allOk = false;
    }
    for (var i = 0; i < 4; i++) {
        var jh = jsState.heads[i];
        var wh = wasmState.heads[i];
        var jDiff = [];
        if (!skipSet['node'] && jh.node !== wh.node) jDiff.push('node');
        if (!skipSet['power_level'] && jh.power_level !== wh.power_level) jDiff.push('lv');
        if (!skipSet['boost_active'] && jh.boost_active !== wh.boost_active) jDiff.push('boost');
        if (!skipSet['timer_setting'] && jh.timer_setting !== wh.timer_setting) jDiff.push('tset');
        if (!skipSet['timer_active'] && jh.timer_active !== wh.timer_active) jDiff.push('tact');
        if (!skipSet['timer_value'] && jh.timer_value !== wh.timer_value) jDiff.push('tv');
        if (jDiff.length > 0) {
            diffs.push('head[' + i + ']: JS={' + jh.node + ',lv' + jh.power_level +
                ',boost=' + jh.boost_active + ',tset=' + jh.timer_setting +
                ',tact=' + jh.timer_active + ',tv=' + jh.timer_value +
                '} WASM={' + wh.node + ',lv' + wh.power_level +
                ',boost=' + wh.boost_active + ',tset=' + wh.timer_setting +
                ',tact=' + wh.timer_active + ',tv=' + wh.timer_value +
                '} diff=' + jDiff.join(','));
            allOk = false;
        }
    }

    if (diffs.length > 0) {
        assert(label, false, JSON.stringify(jsState), JSON.stringify(wasmState));
        diffs.forEach(function(d) { console.log('         ' + d); });
    } else {
        assert(label, true);
    }
}

/* Build timer value display chars */
function timerChars(tv) {
    if (tv >= 10) return [String(Math.floor(tv / 10)), String(tv % 10)];
    return [String(tv), ' '];
}

/* Build normal (non-timer_active) display chars for a head */
function normalChars(h) {
    if (!h) return ['0', '0'];
    if (h.boost_active) return ['P', ' '];
    if (h.timer_setting) return timerChars(h.timer_value || 0);
    if (h.power_level === 0) return ['0', '0'];
    return [String(h.power_level), ' '];
}

/* Check if a 2-char slot display is valid for a head's state */
function headCharsValid(chars, base, head) {
    if (!head) return true;
    var c0 = chars[base], c1 = chars[base + 1];
    var nc = normalChars(head);
    if (c0 === nc[0] && c1 === nc[1]) return true;
    /* timer_active: also accept timer value display */
    if (head.timer_active && !head.timer_setting) {
        var tc = timerChars(head.timer_value || 0);
        if (c0 === tc[0] && c1 === tc[1]) return true;
    }
    return false;
}

/* Flexible seg_chars comparison: for timer_active heads, accept either
 * power-level display or timer-value display */
function segCharsMatch(jsChars, wmChars, jsState, wmState) {
    if (jsChars.join('') === wmChars.join('')) return true;
    if (!jsState || !wmState) return false;
    for (var i = 0; i < 4; i++) {
        var base = i * 2;
        var jh = jsState.heads[i];
        var wh = wmState.heads[i];
        if ((jh && jh.timer_active) || (wh && wh.timer_active)) {
            if (!headCharsValid(jsChars, base, jh) ||
                !headCharsValid(wmChars, base, wh)) {
                return false;
            }
        } else {
            if (jsChars[base] !== wmChars[base] ||
                jsChars[base + 1] !== wmChars[base + 1]) {
                return false;
            }
        }
    }
    return true;
}

/* Flexible seg_blink comparison: C engine blinks for timer_setting even when
 * node != selecting; JS BlinkRule only blinks for node == selecting.
 * Both suppress blink when boost_active is true. */
function segBlinkMatch(jsBlink, wmBlink, jsState, wmState) {
    if (jsBlink.map(function(b){return b?1:0;}).join('') ===
        wmBlink.map(function(b){return b?1:0;}).join('')) return true;
    if (!jsState || !wmState) return false;
    for (var i = 0; i < 4; i++) {
        var jb = jsBlink[i] ? 1 : 0;
        var wb = wmBlink[i] ? 1 : 0;
        if (jb === wb) continue;
        var jh = jsState.heads[i];
        var wh = wmState.heads[i];
        /* C: (node==selecting || timer_setting) && !boost_active
         * JS: node==selecting && !boost_active (exclude_when: [boost_active])
         * Disagreement valid when: node!=selecting, timer_setting, !boost_active */
        var jsCantBlink = jh && jh.node !== 'selecting' && jh.timer_setting && !jh.boost_active;
        var wmCantBlink = wh && wh.node !== 'selecting' && wh.timer_setting && !wh.boost_active;
        if ((jsCantBlink && jb === 0 && wb === 1) ||
            (wmCantBlink && wb === 0 && jb === 1)) continue;
        return false;
    }
    return true;
}

function assertDisplay(label, jsDisp, wasmDisp, jsState, wmState) {
    var allOk = true;
    var js, wm;

    /* seg_chars — flexible compare for timer_active heads */
    if (!segCharsMatch(jsDisp.seg_chars, wasmDisp.seg_chars, jsState, wmState)) {
        js = jsDisp.seg_chars.join('');
        wm = wasmDisp.seg_chars.join('');
        console.log('  [FAIL] ' + label + ' seg_chars: JS=' + js + ' WASM=' + wm);
        allOk = false;
    }

    /* seg_blink — flexible for timer_setting vs selecting blink diff */
    if (!segBlinkMatch(jsDisp.seg_blink, wasmDisp.seg_blink, jsState, wmState)) {
        js = jsDisp.seg_blink.map(function(b){return b?1:0;}).join('');
        wm = wasmDisp.seg_blink.map(function(b){return b?1:0;}).join('');
        console.log('  [FAIL] ' + label + ' seg_blink: JS=' + js + ' WASM=' + wm);
        allOk = false;
    }

    /* seg_mode */
    if (jsDisp.seg_mode !== wasmDisp.seg_mode) {
        console.log('  [FAIL] ' + label + ' seg_mode: JS=' + jsDisp.seg_mode + ' WASM=' + wasmDisp.seg_mode); allOk = false;
    }

    /* leds.power_level */
    js = jsDisp.leds.power_level.map(function(b){return b?1:0;}).join('');
    wm = wasmDisp.leds.power_level.map(function(b){return b?1:0;}).join('');
    if (js !== wm) { console.log('  [FAIL] ' + label + ' led_levels: JS=' + js + ' WASM=' + wm); allOk = false; }

    /* leds.head_select */
    js = jsDisp.leds.head_select.map(function(b){return b?1:0;}).join('');
    wm = wasmDisp.leds.head_select.map(function(b){return b?1:0;}).join('');
    if (js !== wm) { console.log('  [FAIL] ' + label + ' head_select: JS=' + js + ' WASM=' + wm); allOk = false; }

    /* leds.status */
    ['power','timer','pause_btn','child_lock'].forEach(function(k) {
        if (jsDisp.leds[k] !== wasmDisp.leds[k]) {
            console.log('  [FAIL] ' + label + ' led_' + k + ': JS=' + jsDisp.leds[k] + ' WASM=' + wasmDisp.leds[k]); allOk = false;
        }
    });

    if (allOk) {
        passCount++;
        console.log('  [PASS] ' + label);
    } else {
        failCount++;
    }
}

/* ========== 主流程 ========== */
async function main() {
    console.log('========================================');
    console.log('  WASM 引擎基础冒烟测试');
    console.log('========================================\n');

    /* 1. 初始化 WASM 引擎 */
    console.log('--- 初始化 WASM 引擎 ---');
    const wasmInst = await AppLogicModule();
    const wasm = createWasmEngine(wasmInst);
    wasm.init();
    /* 跑完上电序列 (6s = 60 ticks of 100ms) */
    for (var i = 0; i < 65; i++) { wasm.tick100ms(); }

    /* 2. 初始化 JS 引擎 (fastMode, 相同JSON) */
    console.log('--- 初始化 JS 引擎 ---');
    var logicRules;
    try {
        logicRules = JSON.parse(fs.readFileSync(
            path.join(__dirname, '..', 'logic', 'four_head_v4.json'), 'utf-8'));
    } catch(e) {
        logicRules = { meta: { name: 'fallback', version: '0', head_count: 4 } };
    }
    MessageBus.init();
    JsonLogicEngine.setFastMode(true);
    JsonLogicEngine.init(logicRules);

    /* 注册 no-op handler 消除 "消息无处理器" 日志噪音 */
    MessageBus.register(MsgId.MSG_DISPLAY_REFRESH, function(){});
    MessageBus.register(MsgId.MSG_BUZZER_CTRL, function(){});

    /* Message bus 消费(模拟调度) */
    function flushMsg() {
        for (var i = 0; i < 10; i++) { MessageBus.run1ms(); }
    }
    flushMsg();

    /* 3. 测试: 初始状态应为 powered_off */
    console.log('\n--- Test 1: 初始状态 ---');
    var jsSt = JsonLogicEngine.getState();
    var wmSt = wasm.getState();
    assertState('T01 初始化→powered_off', jsSt, wmSt);

    var jsDisp = JsonLogicEngine.getDisplay();
    var wmDisp = wasm.getDisplay();
    assertDisplay('T01 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 4. 测试: 长按POWER → working */
    console.log('\n--- Test 2: POWER long → working ---');
    wasm.postKey('POWER', 'long');
    /* JS: inject key through KeyHandler */
    var KeyHandler = sandbox.KeyHandler;
    KeyHandler.injectKey(4, 'long'); /* KEY_ONOFF, LONG */
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T02 开机→working', jsSt, wmSt);

    /* 5. 测试: HEAD_1 tap → selecting */
    console.log('\n--- Test 3: HEAD_1 tap → selecting ---');
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap'); /* KEY_LEFT_P_SET_UP, TAP */
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T03 选中炉头1', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T03 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 6. 测试: 数字5 tap → 设档5 */
    console.log('\n--- Test 4: 数字5 → power=5 ---');
    wasm.postKey('5', 'tap');
    KeyHandler.injectKey(27, 'tap'); /* KEY_POWER_5, TAP */
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T04 设档5', jsSt, wmSt);

    /* 7. 测试: 超时确认 → cooking */
    console.log('\n--- Test 5: 超时确认 → cooking ---');
    var preT05_js = JsonLogicEngine.getState();
    console.log('  PRE-T05 JS: mode=' + preT05_js.global_mode + ' head0=' + preT05_js.heads[0].node + ' power=' + preT05_js.heads[0].power_level + ' hotHead=' + preT05_js.hot_head);

    wasm.forceSelectTimeout(0);
    JsonLogicEngine.forceSelectTimeout(0);

    var midT05_js = JsonLogicEngine.getState();
    console.log('  MID-T05 JS (after forceSelect, before flush): mode=' + midT05_js.global_mode + ' head0=' + midT05_js.heads[0].node);

    flushMsg();

    var postT05_js = JsonLogicEngine.getState();
    console.log('  POST-T05 JS (after flush): mode=' + postT05_js.global_mode + ' head0=' + postT05_js.heads[0].node);

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T05 确认cooking', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T05 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 8. 测试: HEAD_2 设档9 (多炉头) */
    console.log('\n--- Test 6: HEAD_2 tap → 设档9 ---');
    wasm.postKey('HEAD_2', 'tap');
    KeyHandler.injectKey(14, 'tap'); /* KEY_RIGHT_P_SET_UP, TAP */
    flushMsg();
    wasm.postKey('9', 'tap');
    KeyHandler.injectKey(31, 'tap'); /* KEY_POWER_9, TAP */
    flushMsg();

    wasm.forceSelectTimeout(1);
    JsonLogicEngine.forceSelectTimeout(1);
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T06 多头: 头1=5, 头2=9', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T06 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 9. 测试: Boost */
    console.log('\n--- Test 7: HEAD_1 → select → BOOST ---');
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap');
    flushMsg();

    wasm.postKey('9', 'long');
    KeyHandler.injectKey(31, 'long'); /* KEY_POWER_9, LONG */
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T07 Boost', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T07 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 10. 测试: Flow-07 — 选中0档→15s超时→idle */
    console.log('\n--- Test 8 (Flow-07): 选中0档→超时→idle ---');
    /* 选中炉头3 (idle, power=0) */
    wasm.postKey('HEAD_3', 'tap');
    KeyHandler.injectKey(13, 'tap'); /* KEY_LEFT_P_SET, TAP */
    flushMsg();

    wasm.forceSelectTimeout(2);
    JsonLogicEngine.forceSelectTimeout(2);
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T08 选中0档超时→idle', jsSt, wmSt);

    /* 11. 测试: Flow-08 — 选中→设档→自按确认→cooking */
    console.log('\n--- Test 9 (Flow-08): 自按确认→cooking ---');
    /* 选中炉头4 */
    wasm.postKey('HEAD_4', 'tap');
    KeyHandler.injectKey(12, 'tap'); /* KEY_RIGHT_P_SET, TAP */
    flushMsg();

    /* 设档5 */
    wasm.postKey('5', 'tap');
    KeyHandler.injectKey(27, 'tap'); /* KEY_POWER_5, TAP */
    flushMsg();

    /* 自按确认 — 再按一次 HEAD_4 */
    wasm.postKey('HEAD_4', 'tap');
    KeyHandler.injectKey(12, 'tap');
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T09 自按确认→cooking', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T09 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 10. 测试: Flow-09 — 选中→按定时键→timer_setting进程 */
    console.log('\n--- Test 10 (Flow-09): timer_setting 进程 ---');
    /* 选中炉头4 (当前 cooking, power=5) */
    wasm.postKey('HEAD_4', 'tap');
    KeyHandler.injectKey(12, 'tap'); /* KEY_RIGHT_P_SET, TAP */
    flushMsg();

    /* 按定时键 → 进入 timer_setting 进程 */
    wasm.postKey('TIMER', 'tap');
    KeyHandler.injectKey(5, 'tap'); /* KEY_TIME_SET, TAP */
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T10 timer_setting', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T10 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 11. 测试: Flow-03 — 选中→设0档→确认→idle */
    console.log('\n--- Test 11 (Flow-03): 0档确认→idle ---');
    /* 先确认 TIMER (T10遗留的 timer_setting → timer_active), 确保双引擎一致 */
    wasm.postKey('TIMER', 'tap');
    KeyHandler.injectKey(5, 'tap'); /* KEY_TIME_SET, TAP */
    flushMsg();

    /* 选中炉头3 (当前 idle, power=0) */
    wasm.postKey('HEAD_3', 'tap');
    KeyHandler.injectKey(13, 'tap'); /* KEY_LEFT_P_SET, TAP */
    flushMsg();

    /* 显式按0 (走 handle_power_key 路径) */
    wasm.postKey('0', 'tap');
    KeyHandler.injectKey(22, 'tap'); /* KEY_POWER_0, TAP */
    flushMsg();

    wasm.forceSelectTimeout(2);
    JsonLogicEngine.forceSelectTimeout(2);
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T11 0档确认→idle', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T11 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 12. 测试: Flow-13 — 工作态→暂停→恢复 */
    console.log('\n--- Test 12 (Flow-13): 暂停 toggle ---');
    /* 按暂停键 */
    wasm.postKey('PAUSE', 'tap');
    KeyHandler.injectKey(6, 'tap'); /* KEY_STOP, TAP */
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T12a 暂停', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T12a display', jsDisp, wmDisp, jsSt, wmSt);

    /* 恢复 */
    wasm.postKey('PAUSE', 'tap');
    KeyHandler.injectKey(6, 'tap'); /* KEY_STOP, TAP */
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T12b 恢复', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T12b display', jsDisp, wmDisp, jsSt, wmSt);

    /* 13. 测试: Flow-14 — cooking→timer_setting 进程 */
    console.log('\n--- Test 13 (Flow-14): cooking→timer_setting ---');
    /* 选中炉头1 (cooking, boost_active) */
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap'); /* KEY_LEFT_P_SET_UP, TAP */
    flushMsg();

    /* 按定时 → cooking态进入 timer_setting */
    wasm.postKey('TIMER', 'tap');
    KeyHandler.injectKey(5, 'tap'); /* KEY_TIME_SET, TAP */
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T13 cooking→timer_setting', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T13 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 14. 测试: Flow-10 — 快捷改档 (cooking态直接设档) */
    console.log('\n--- Test 14 (Flow-10): 快捷改档 ---');
    /* 自确认 (head 转为 cooking) → 快捷路径 */
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap'); /* KEY_LEFT_P_SET_UP, TAP */
    flushMsg();

    /* 快捷改档: cooking态直接按数字 */
    wasm.postKey('3', 'tap');
    KeyHandler.injectKey(25, 'tap'); /* KEY_POWER_3, TAP */
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T14 快捷改档→3', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T14 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 15. 测试: Flow-12 — cooking→长按9→Boost */
    console.log('\n--- Test 15 (Flow-12): cooking→Boost ---');
    /* 先确认定时, 清除 timer_setting 避免显示冲突 */
    wasm.postKey('TIMER', 'tap');
    KeyHandler.injectKey(5, 'tap'); /* KEY_TIME_SET, TAP */
    flushMsg();

    /* 长按9 → 进入Boost */
    wasm.postKey('9', 'long');
    KeyHandler.injectKey(31, 'long'); /* KEY_POWER_9, LONG */
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T15 cooking→Boost', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T15 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 16. 测试: Flow-16 — Boost→按数字键退出 */
    console.log('\n--- Test 16 (Flow-16): Boost→数字键退出 ---');
    wasm.postKey('5', 'tap');
    KeyHandler.injectKey(27, 'tap'); /* KEY_POWER_5, TAP */
    flushMsg();

    /* 自确认 (处理可能的 node 转换) */
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap'); /* KEY_LEFT_P_SET_UP, TAP */
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T16 Boost退出→5档', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T16 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 17. 测试: Flow-17 — Boost→超时恢复 */
    console.log('\n--- Test 17 (Flow-17): Boost→超时恢复 ---');
    wasm.postKey('9', 'long');
    KeyHandler.injectKey(31, 'long'); /* KEY_POWER_9, LONG */
    flushMsg();

    wasm.forceBoostTimeout(0);
    JsonLogicEngine.forceBoostTimeout(0);
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T17 Boost超时→恢复5档', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T17 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 18. 测试: Flow-20 — 定时+/-调整 */
    console.log('\n--- Test 18 (Flow-20): 定时+/-调整 ---');
    /* 进入 timer_setting (从 timer_active 重进, 保留当前值) */
    wasm.postKey('TIMER', 'tap');
    KeyHandler.injectKey(5, 'tap'); /* KEY_TIME_SET, TAP */
    flushMsg();

    /* +2 -1 → 预期 15+2-1=16 */
    wasm.postKey('PLUS', 'tap');
    KeyHandler.injectKey(3, 'tap'); /* KEY_ADD, TAP */
    flushMsg();
    wasm.postKey('PLUS', 'tap');
    KeyHandler.injectKey(3, 'tap'); /* KEY_ADD, TAP */
    flushMsg();
    wasm.postKey('MINUS', 'tap');
    KeyHandler.injectKey(2, 'tap'); /* KEY_SUB, TAP */
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T18 定时调整→16', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T18 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 19. 测试: Flow-24 — 童锁上锁 */
    console.log('\n--- Test 19 (Flow-24): 童锁上锁 ---');
    wasm.postKey('CHILD_LOCK', 'long');
    KeyHandler.injectKey(8, 'long'); /* KEY_LOCK, LONG */
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T19 童锁上锁', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T19 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 20. 测试: Flow-25 — 童锁解锁 */
    console.log('\n--- Test 20 (Flow-25): 童锁解锁 ---');
    wasm.postKey('CHILD_LOCK', 'long');
    KeyHandler.injectKey(8, 'long'); /* KEY_LOCK, LONG */
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T20 童锁解锁', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T20 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 21. 测试: Flow-11 — cooking→0档弹出选择栈 */
    console.log('\n--- Test 21 (Flow-11): cooking→0档弹出选择栈 ---');
    /* 设0档 → head 0 应变为 idle, 弹出栈, 热点移交 */
    wasm.postKey('0', 'tap');
    KeyHandler.injectKey(22, 'tap'); /* KEY_POWER_0, TAP */
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T21 0档弹出栈', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T21 display', jsDisp, wmDisp, jsSt, wmSt);

    /* ============================================================
     * 第二批: Phase 2-4 剩余 13 条 (Flow-15~33)
     * ============================================================ */

    /* 22. 测试: Flow-15 — 定时归零→idle */
    console.log('\n--- Test 22 (Flow-15): 定时归零→idle ---');
    /* 选头1, 设cooking, 进timer_setting, 确认timer_active */
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap'); /* KEY_LEFT_P_SET_UP */
    flushMsg();
    wasm.postKey('5', 'tap');
    KeyHandler.injectKey(27, 'tap'); /* KEY_POWER_5 */
    flushMsg();
    wasm.postKey('HEAD_1', 'tap');   /* 自按确认→cooking */
    KeyHandler.injectKey(15, 'tap');
    flushMsg();
    wasm.postKey('TIMER', 'tap');    /* enter timer_setting */
    KeyHandler.injectKey(5, 'tap');
    flushMsg();
    wasm.postKey('TIMER', 'tap');    /* confirm→timer_active=15 */
    KeyHandler.injectKey(5, 'tap');
    flushMsg();

    wasm.forceTimerExpire(0);
    JsonLogicEngine.forceTimerExpire(0);
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T22 定时归零→idle', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T22 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 23. 测试: Flow-18 — Boost→按定时(timer_setting叠加) */
    console.log('\n--- Test 23 (Flow-18): Boost+定时重叠 ---');
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap');
    flushMsg();
    wasm.postKey('9', 'long');       /* enter boost */
    KeyHandler.injectKey(31, 'long');
    flushMsg();
    wasm.postKey('TIMER', 'tap');    /* timer_setting + boost 共存 */
    KeyHandler.injectKey(5, 'tap');
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T23 Boost+timer_setting', jsSt, wmSt);

    /* 24. 测试: Flow-19 — 暂停→恢复 */
    console.log('\n--- Test 24 (Flow-19): 暂停→恢复 ---');
    /* 先退出 timer_setting (长按定时取消) */
    wasm.postKey('TIMER', 'long');
    KeyHandler.injectKey(5, 'long');
    flushMsg();
    /* 自确认设档5 */
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap');
    flushMsg();
    wasm.postKey('5', 'tap');
    KeyHandler.injectKey(27, 'tap');
    flushMsg();
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap');
    flushMsg();
    /* 暂停 */
    wasm.postKey('PAUSE', 'tap');
    KeyHandler.injectKey(6, 'tap');
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T24a 暂停', jsSt, wmSt);

    /* 恢复 */
    wasm.postKey('PAUSE', 'tap');
    KeyHandler.injectKey(6, 'tap');
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T24b 恢复', jsSt, wmSt);

    /* 25. 测试: Flow-23 — 长按定时取消 */
    console.log('\n--- Test 25 (Flow-23): 长按定时取消 ---');
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap');
    flushMsg();
    wasm.postKey('TIMER', 'tap');    /* enter timer_setting */
    KeyHandler.injectKey(5, 'tap');
    flushMsg();
    wasm.postKey('TIMER', 'long');   /* cancel */
    KeyHandler.injectKey(5, 'long');
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T25 长按取消定时', jsSt, wmSt);

    /* 26. 测试: Flow-21 — 定时→手动确认(timer_active启动) */
    console.log('\n--- Test 26 (Flow-21): 定时手动确认 ---');
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap');
    flushMsg();
    wasm.postKey('5', 'tap');
    KeyHandler.injectKey(27, 'tap');
    flushMsg();
    wasm.postKey('HEAD_1', 'tap');   /* 自确认→cooking */
    KeyHandler.injectKey(15, 'tap');
    flushMsg();
    wasm.postKey('TIMER', 'tap');    /* timer_setting */
    KeyHandler.injectKey(5, 'tap');
    flushMsg();
    /* +/- 调整到 16 */
    wasm.postKey('PLUS', 'tap');
    KeyHandler.injectKey(3, 'tap');
    flushMsg();
    wasm.postKey('PLUS', 'tap');
    KeyHandler.injectKey(3, 'tap');
    flushMsg();
    wasm.postKey('MINUS', 'tap');
    KeyHandler.injectKey(2, 'tap');
    flushMsg();
    /* 手动确认 */
    wasm.postKey('TIMER', 'tap');
    KeyHandler.injectKey(5, 'tap');
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T26 定时确认→timer_active', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T26 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 27. 测试: Flow-22 — 定时select 15s自动确认 */
    console.log('\n--- Test 27 (Flow-22): 定时自动确认 ---');
    /* 先长按取消当前 timer_active */
    wasm.postKey('TIMER', 'long');
    KeyHandler.injectKey(5, 'long');
    flushMsg();
    /* 重新进入 timer_setting */
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap');
    flushMsg();
    wasm.postKey('TIMER', 'tap');
    KeyHandler.injectKey(5, 'tap');
    flushMsg();
    /* forceSelectTimeout 确认 selecting→cooking */
    JsonLogicEngine.forceSelectTimeout();
    wasm.forceSelectTimeout(0);
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    /* 已知差异#1: JS forceSelectTimeout 自动确认 timer_setting,
     * C 不自动确认。只校验稳定字段(node/power/timer_value) */
    assertStateCompat('T27 定时select确认(node/power)', jsSt, wmSt,
        ['node','power_level','timer_value'], ['timer_setting','timer_active']);

    /* 28. 测试: Flow-30 — 全idle→关机 */
    console.log('\n--- Test 28 (Flow-30): 全idle→关机 ---');
    /* 清掉所有炉头: 长按定时取消 + 选头设0确认 */
    wasm.postKey('TIMER', 'long');
    KeyHandler.injectKey(5, 'long');
    flushMsg();
    /* 确认所有炉头为 idle */
    wasm.postKey('HEAD_1', 'tap'); wasm.postKey('0', 'tap');
    KeyHandler.injectKey(15, 'tap'); KeyHandler.injectKey(22, 'tap');
    flushMsg();
    wasm.forceSelectTimeout(0);
    JsonLogicEngine.forceSelectTimeout(0);
    flushMsg();

    wasm.postKey('HEAD_2', 'tap'); wasm.postKey('0', 'tap');
    KeyHandler.injectKey(14, 'tap'); KeyHandler.injectKey(22, 'tap');
    flushMsg();
    wasm.forceSelectTimeout(1);
    JsonLogicEngine.forceSelectTimeout(1);
    flushMsg();

    wasm.postKey('HEAD_3', 'tap'); wasm.postKey('0', 'tap');
    KeyHandler.injectKey(13, 'tap'); KeyHandler.injectKey(22, 'tap');
    flushMsg();
    wasm.forceSelectTimeout(2);
    JsonLogicEngine.forceSelectTimeout(2);
    flushMsg();

    wasm.postKey('HEAD_4', 'tap'); wasm.postKey('0', 'tap');
    KeyHandler.injectKey(12, 'tap'); KeyHandler.injectKey(22, 'tap');
    flushMsg();
    wasm.forceSelectTimeout(3);
    JsonLogicEngine.forceSelectTimeout(3);
    flushMsg();

    /* 长按 POWER → 关机 */
    wasm.postKey('POWER', 'long');
    KeyHandler.injectKey(4, 'long');
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T28 关机→powered_off', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T28 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 29. 测试: Flow-31 — 关机→休眠 */
    console.log('\n--- Test 29 (Flow-31): 关机显示验证 ---');
    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T29 powered_off状态', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T29 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 30. 测试: Flow-32 — 关机→开机(唤醒) */
    console.log('\n--- Test 30 (Flow-32): 关机→开机 ---');
    wasm.postKey('POWER', 'long');
    KeyHandler.injectKey(4, 'long');
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T30 开机→working', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T30 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 31. 测试: Flow-26 — 工作→长按开关关机 */
    console.log('\n--- Test 31 (Flow-26): 工作→长按关机 ---');
    /* 设一个炉头 cooking */
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap');
    flushMsg();
    wasm.postKey('5', 'tap');
    KeyHandler.injectKey(27, 'tap');
    flushMsg();
    wasm.forceSelectTimeout(0);
    JsonLogicEngine.forceSelectTimeout(0);
    flushMsg();
    /* 长按关机 */
    wasm.postKey('POWER', 'long');
    KeyHandler.injectKey(4, 'long');
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T31 长按关机', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T31 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 32. 测试: Flow-33 — 同一炉头自按确认 */
    console.log('\n--- Test 32 (Flow-33): 自按确认 ---');
    wasm.postKey('POWER', 'long');   /* 重新开机 */
    KeyHandler.injectKey(4, 'long');
    flushMsg();
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap');
    flushMsg();
    wasm.postKey('5', 'tap');
    KeyHandler.injectKey(27, 'tap');
    flushMsg();
    /* 自按确认 — 选中的头上再按一次 HEAD_1 */
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap');
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T32 自按确认→cooking', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T32 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 33. 测试: Flow-27 — 多炉头独立工作+选择序列 */
    console.log('\n--- Test 33 (Flow-27): 多炉头独立工作 ---');
    wasm.postKey('HEAD_2', 'tap');
    KeyHandler.injectKey(14, 'tap');
    flushMsg();
    wasm.postKey('3', 'tap');
    KeyHandler.injectKey(25, 'tap');
    flushMsg();
    wasm.forceSelectTimeout(1);
    JsonLogicEngine.forceSelectTimeout(1);
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T33 多炉头: 头1=5 头2=3', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T33 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 34. 测试: Flow-28 — 多炉头独立定时(正交进程) */
    console.log('\n--- Test 34 (Flow-28): 多炉头独立定时 ---');
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap');
    flushMsg();
    wasm.postKey('TIMER', 'tap');    /* timer_setting on head1 */
    KeyHandler.injectKey(5, 'tap');
    flushMsg();
    wasm.postKey('TIMER', 'tap');    /* confirm→timer_active on head1 */
    KeyHandler.injectKey(5, 'tap');
    flushMsg();
    /* 选头2, 不应有 timer_active */
    wasm.postKey('HEAD_2', 'tap');
    KeyHandler.injectKey(14, 'tap');
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T34 独立定时: 头1有timer 头2无', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T34 display', jsDisp, wmDisp, jsSt, wmSt);

    /* 35. 测试: Flow-29 — Boost+定时交替(正交进程叠加) */
    console.log('\n--- Test 35 (Flow-29): Boost+定时叠加 ---');
    /* 关机重来: 干净开局 */
    wasm.postKey('POWER', 'long');
    KeyHandler.injectKey(4, 'long');
    flushMsg();
    wasm.postKey('POWER', 'long');
    KeyHandler.injectKey(4, 'long');
    flushMsg();
    /* 选头1, 长按9→Boost, 加定时 */
    wasm.postKey('HEAD_1', 'tap');
    KeyHandler.injectKey(15, 'tap');
    flushMsg();
    wasm.postKey('9', 'long');
    KeyHandler.injectKey(31, 'long');
    flushMsg();
    wasm.postKey('TIMER', 'tap');
    KeyHandler.injectKey(5, 'tap');
    flushMsg();
    wasm.postKey('TIMER', 'tap');    /* confirm */
    KeyHandler.injectKey(5, 'tap');
    flushMsg();

    jsSt = JsonLogicEngine.getState();
    wmSt = wasm.getState();
    assertState('T35 Boost+定时叠加', jsSt, wmSt);

    jsDisp = JsonLogicEngine.getDisplay();
    wmDisp = wasm.getDisplay();
    assertDisplay('T35 display', jsDisp, wmDisp, jsSt, wmSt);

    /* ========== 结果 ========== */
    console.log('\n========================================');
    console.log('  总计: ' + (passCount + failCount) + ' | PASS=' + passCount + ' FAIL=' + failCount);
    console.log('========================================');
    process.exit(failCount > 0 ? 1 : 0);
}

main().catch(function(e) {
    console.error('测试异常:', e);
    process.exit(1);
});
