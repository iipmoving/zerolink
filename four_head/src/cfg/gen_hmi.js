#!/usr/bin/env node
/**
 * gen_hmi.js — JSON → C 配置表生成器
 *
 * 读取 sim/test_hmi/logic/four_head_v4.json，
 * 输出 Claude/cfg/hmi_data.c + hmi_data.h。
 *
 * 用法: node cfg/gen_hmi.js
 *
 * 所有 JSON 字符串键→C枚举的映射在此文件集中维护。
 * 修改 JSON 后运行此脚本即可更新 C 配置表。
 */

'use strict';

const fs = require('fs');
const path = require('path');

const ROOT = path.resolve(__dirname, '..', '..');
const JSON_PATH = path.join(ROOT, 'sim', 'test_hmi', 'logic', 'four_head_v4.json');
const OUT_C = path.join(__dirname, 'hmi_data.c');
const OUT_H = path.join(__dirname, 'hmi_data.h');

/* ================================================================
 * 一、映射表（JSON 字符串 → C 标识符）
 * ================================================================ */

/* 按键名 → KeyCode_t 枚举值 */
const KEY_MAP = {
    'POWER':       'KEY_ONOFF',
    'TIMER':       'KEY_TIME_SET',
    'PAUSE':       'KEY_STOP',
    'CHILD_LOCK':  'KEY_LOCK',
    'HEAD_1':      'KEY_LEFT_P_SET_UP',
    'HEAD_2':      'KEY_LEFT_P_SET',
    'HEAD_3':      'KEY_RIGHT_P_SET',
    'HEAD_4':      'KEY_RIGHT_P_SET_UP',
    'HEAD_SELF':   'HMI_KEY_HEAD_SELF',
    'HEAD_OTHER':  'HMI_KEY_HEAD_OTHER',
    'ZONE':        'KEY_BIND',
    'PLUS':        'KEY_ADD',
    'MINUS':       'KEY_SUB',
    '0': 'KEY_POWER_0', '1': 'KEY_POWER_1', '2': 'KEY_POWER_2',
    '3': 'KEY_POWER_3', '4': 'KEY_POWER_4', '5': 'KEY_POWER_5',
    '6': 'KEY_POWER_6', '7': 'KEY_POWER_7', '8': 'KEY_POWER_8',
    '9': 'KEY_POWER_9',
};

/* 事件名 → 常量 */
const EVENT_MAP = { 'tap': 'HMI_EVT_TAP', 'long': 'HMI_EVT_LONG' };

/* 动作名 → HmiAction_t 枚举 */
const ACTION_MAP = {
    'go_working':                'HMI_ACT_GO_WORKING',
    'go_powered_off':            'HMI_ACT_GO_POWERED_OFF',
    'go_deep_sleep':             'HMI_ACT_GO_DEEP_SLEEP',
    'toggle_pause':              'HMI_ACT_TOGGLE_PAUSE',
    'toggle_child_lock':         'HMI_ACT_TOGGLE_CHILD_LOCK',
    'select_head':               'HMI_ACT_SELECT_HEAD',
    'confirm_select':            'HMI_ACT_CONFIRM_SELECT',
    'confirm_select_immediate':  'HMI_ACT_CONFIRM_SELECT_IMMEDIATE',
    'set_power':                 'HMI_ACT_SET_POWER',
    'enter_boost':               'HMI_ACT_ENTER_BOOST',
    'exit_boost':                'HMI_ACT_EXIT_BOOST',
    'exit_boost_set_power':      'HMI_ACT_EXIT_BOOST_SET_POWER',
    'enter_timer_setting':       'HMI_ACT_ENTER_TIMER_SETTING',
    'confirm_timer':             'HMI_ACT_CONFIRM_TIMER',
    'cancel_timer_setting':      'HMI_ACT_CANCEL_TIMER_SETTING',
    'cancel_timer_active':       'HMI_ACT_CANCEL_TIMER_ACTIVE',
    'timer_adjust':              'HMI_ACT_TIMER_ADJUST',
    'show_dash':                 'HMI_ACT_SHOW_DASH',
    'show_pa':                   'HMI_ACT_SHOW_PA',
    'show_power_mode':           'HMI_ACT_SHOW_POWER_MODE',
    'display_all_off':           'HMI_ACT_DISPLAY_ALL_OFF',
    'clear_all_heads':           'HMI_ACT_CLEAR_ALL_HEADS',
    'hothead_clear':             'HMI_ACT_HOTHEAD_CLEAR',
    'clear_hothead':             'HMI_ACT_CLEAR_HOTHEAD',
    'led_power_on':              'HMI_ACT_LED_POWER_ON',
    'led_power_off':             'HMI_ACT_LED_POWER_OFF',
    'led_power_blink':           'HMI_ACT_LED_POWER_ON',   /* blink → on */
    'led_all_off':               'HMI_ACT_LED_ALL_OFF',
    'led_pause_on':              'HMI_ACT_LED_PAUSE_ON',
    'led_pause_off':             'HMI_ACT_LED_PAUSE_OFF',
    'led_timer_on':              'HMI_ACT_LED_TIMER_ON',
    'led_timer_off':             'HMI_ACT_LED_TIMER_OFF',
    'led_child_lock_on':         'HMI_ACT_LED_CHILD_LOCK_ON',
    'led_child_lock_off':        'HMI_ACT_LED_CHILD_LOCK_OFF',
    'reset_idle_timer':          'HMI_ACT_RESET_IDLE_TIMER',
    'reset_off_timer':           'HMI_ACT_RESET_OFF_TIMER',
    'beep_valid':                'HMI_ACT_BEEP_VALID',
    'beep_invalid':              'HMI_ACT_BEEP_INVALID',
    'wake_from_sleep':           'HMI_ACT_GO_WORKING',     /* 同义 */
};

/* Guard 检查类型 */
const GUARD_MAP = {
    'always':   'HMI_GUARD_ALWAYS',
    'all_idle': 'HMI_GUARD_ALL_IDLE',
};

/* 超时键名 → HmiTimeoutId_t */
const TIMEOUT_KEY_ENUM = {
    'select_confirm_ms':  'HMI_TO_SELECT_CONFIRM_MS',
    'timer_confirm_ms':   'HMI_TO_TIMER_CONFIRM_MS',
    'boost_max_ms':       'HMI_TO_BOOST_MAX_MS',
    'idle_to_standby_ms': 'HMI_TO_IDLE_TO_STANDBY_MS',
    'idle_to_off_ms':     'HMI_TO_IDLE_TO_OFF_MS',
    'off_to_sleep_ms':    'HMI_TO_OFF_TO_SLEEP_MS',
    'default_timer_min':  'HMI_TO_DEFAULT_TIMER_MIN',
    'power_on_all_on_ms': 'HMI_TO_POWER_ON_ALL_ON_MS',
    'version_show_ms':    'HMI_TO_VERSION_SHOW_MS',
    'power_key_long_ms':  'HMI_TO_POWER_KEY_LONG_MS',
};

/* seg_mode 值 */
const SEG_MODE_MAP = {
    'power':          'HMI_SEG_MODE_POWER',
    'dash':           'HMI_SEG_MODE_DASH',
    'off':            'HMI_SEG_MODE_OFF',
    'ascii':          'HMI_SEG_MODE_ASCII',
    'timer_setting':  'HMI_SEG_MODE_TIMER_SETTING',
};

/* ModeRule condition */
const MODE_COND_MAP = {
    'powered_off':       'HMI_MODE_COND_POWERED_OFF',
    'deep_sleep':        'HMI_MODE_COND_DEEP_SLEEP',
    'paused':            'HMI_MODE_COND_PAUSED',
    'any_timer_setting': 'HMI_MODE_COND_ANY_TIMER_SETTING',
};

/* 全局模式 JSON键 → HmiGlobalNode_t */
const GLOBAL_NODE_MAP = {
    'powered_off': 'HMI_NODE_POWERED_OFF',
    'deep_sleep':  'HMI_NODE_DEEP_SLEEP',
    'working':     'HMI_NODE_WORKING',
    'paused':      'HMI_NODE_PAUSED',
};

/* Zone 状态键 → HmiZoneNode_t */
const ZONE_NODE_MAP = {
    'idle':      'HMI_ZONE_IDLE',
    'selecting': 'HMI_ZONE_SELECTING',
    'cooking':   'HMI_ZONE_COOKING',
};

/* 进程键 → HmiProcessNode_t */
const PROC_NODE_MAP = {
    'timer_setting': 'HMI_PROC_TIMER_SETTING',
    'timer_active':  'HMI_PROC_TIMER_ACTIVE',
    'boost_active':  'HMI_PROC_BOOST_ACTIVE',
};

/* ================================================================
 * 二、辅助函数
 * ================================================================ */

function parseRouteKey(routeKey /* "POWER long" */) {
    var idx = routeKey.lastIndexOf(' ');
    var name = routeKey.substring(0, idx);
    var evt  = routeKey.substring(idx + 1);
    return { name: name, event: evt };
}

function parseActionStr(actionStr) {
    var parts = actionStr.split(' ');
    var action = parts[0];
    var param = 0;
    if (parts.length > 1) {
        param = (parts[1] === '%head') ? -1 : parseInt(parts[1]);
    }
    return { action: action, param: param };
}

function emitRoutes(routesObj) {
    /* 返回 C 初始化列表字符串 */
    var lines = [];
    if (!routesObj) return { body: '    { 0, 0, 0, 0 } /* empty terminator */', count: 0 };
    var keys = Object.keys(routesObj);
    for (var i = 0; i < keys.length; i++) {
        var rk = parseRouteKey(keys[i]);
        var keyEnum = KEY_MAP[rk.name];
        if (!keyEnum) {
            console.error('Unknown key: ' + rk.name);
            process.exit(1);
        }
        var evtConst = EVENT_MAP[rk.event];
        if (!evtConst) {
            console.error('Unknown event: ' + rk.event);
            process.exit(1);
        }
        var act = parseActionStr(routesObj[keys[i]]);
        var actEnum = ACTION_MAP[act.action];
        if (!actEnum) {
            console.error('Unknown action: ' + act.action);
            process.exit(1);
        }
        lines.push('    { ' + keyEnum + ', ' + evtConst + ', ' + actEnum + ', ' + act.param + ' }');
    }
    return { body: lines.join(',\n'), count: keys.length };
}

function emitActionList(actionList) {
    if (!actionList || actionList.length === 0) return { body: '    (HmiAction_t)0 /* empty terminator */', count: 0 };
    var items = [];
    for (var i = 0; i < actionList.length; i++) {
        var a = ACTION_MAP[actionList[i]];
        if (!a) {
            /* 可能是带参数的动作 */
            var parsed = parseActionStr(actionList[i]);
            a = ACTION_MAP[parsed.action];
            if (!a) { console.error('Unknown action: ' + actionList[i]); process.exit(1); }
            items.push(a + '  /* param=' + parsed.param + ' */');
        } else {
            items.push(a);
        }
    }
    return { body: items.join(',\n    '), count: actionList.length };
}

function emitGuards(guardsArray) {
    if (!guardsArray || guardsArray.length === 0) return { body: '    { 0, 0, 0, 0 } /* empty terminator */', count: 0 };
    var lines = [];
    for (var i = 0; i < guardsArray.length; i++) {
        var g = guardsArray[i];
        var check = GUARD_MAP[g.check] || 'HMI_GUARD_ALWAYS';
        var msKey = TIMEOUT_KEY_ENUM[g.ms_key];
        var act = parseActionStr(g.action);
        var actEnum = ACTION_MAP[act.action];
        lines.push('    { ' + check + ', ' + msKey + ', ' + actEnum + ', ' + act.param + ' }');
    }
    return { body: lines.join(',\n'), count: guardsArray.length };
}

function emitTimeoutEntry(cfg) {
    if (!cfg || !cfg.timeout) return '0xFF, 0, 0';
    var t = cfg.timeout;
    var msKey = TIMEOUT_KEY_ENUM[t.ms_key] || 'HMI_TO_SELECT_CONFIRM_MS';
    var act = parseActionStr(t.action);
    return msKey + ', ' + ACTION_MAP[act.action] + ', ' + act.param;
}

/* ================================================================
 * 三、主生成逻辑
 * ================================================================ */

function generate(json) {
    var c = [];
    var h = [];
    var nameIdx = 0;
    function uniqueName(prefix) { return prefix + '_' + (nameIdx++); }

    c.push('/* hmi_data.c — 由 gen_hmi.js 自动生成, 勿手动编辑 */');
    c.push('/* 数据来源: sim/test_hmi/logic/four_head_v4.json   */');
    c.push('');
    c.push('#include "../app/app_hmi.h"');
    c.push('#include "../drv/drv_key.h"');
    c.push('');

    h.push('/* hmi_data.h — 由 gen_hmi.js 自动生成, 勿手动编辑 */');
    h.push('#ifndef HMI_DATA_H');
    h.push('#define HMI_DATA_H');
    h.push('');
    h.push('#include "../app/app_hmi.h"');
    h.push('');

    /* ---- 3.1 timeouts ---- */
    c.push('/* ===== 超时配置 (按 HmiTimeoutId_t 索引) ===== */');
    c.push('const uint32_t hmi_timeouts[HMI_TO_COUNT] = {');
    var toKeys = Object.keys(TIMEOUT_KEY_ENUM);
    for (var tk = 0; tk < toKeys.length; tk++) {
        var toKey = toKeys[tk];
        var toEnum = TIMEOUT_KEY_ENUM[toKey];
        var toVal = (json.timeouts && json.timeouts[toKey] !== undefined) ? json.timeouts[toKey] : 15000;
        c.push('    [' + toEnum + '] = ' + toVal + ',');
    }
    c.push('};');
    c.push('');
    h.push('extern const uint32_t hmi_timeouts[HMI_TO_COUNT];');

    /* ---- 3.2 power_on_sequence ---- */
    c.push('/* ===== 上电序列 ===== */');
    c.push('const HmiPowerOnStep_t hmi_power_on_seq[] = {');
    if (json.power_on_sequence) {
        for (var si = 0; si < json.power_on_sequence.length; si++) {
            var s = json.power_on_sequence[si];
            if (s.goto) {
                var gn = GLOBAL_NODE_MAP[s.goto] || 'HMI_NODE_POWERED_OFF';
                c.push('    { 0, 0, "", 0, ' + gn + ' },  /* step=' + s.step + ' goto=' + s.goto + ' */');
            } else {
                var delayKey = s.delay_ms_key || 'power_on_all_on_ms';
                var delayMs = json.timeouts ? (json.timeouts[delayKey] || 300) : 300;
                var sm = SEG_MODE_MAP[s.seg_mode || 'power'] || 'HMI_SEG_MODE_POWER';
                var ch = (s.seg_chars || '88888884');
                if (ch.length < 8) ch = (ch + '        ').substring(0, 8);
                var leds = (s.leds === 'all_on') ? 'HMI_LEDS_ALL_ON' :
                           (s.leds === 'all_off') ? 'HMI_LEDS_ALL_OFF' : 'HMI_LEDS_NONE';
                c.push('    { ' + delayMs + ', ' + sm + ', "' + ch + '", ' + leds + ', -1 },');
            }
        }
    }
    c.push('};');
    c.push('');
    h.push('extern const HmiPowerOnStep_t hmi_power_on_seq[];');
    h.push('#define HMI_POWER_ON_SEQ_LEN ' + (json.power_on_sequence ? json.power_on_sequence.length : 0));

    /* ---- 3.3 global_routes ---- */
    c.push('/* ===== 全局路由 ===== */');
    var globalModes = ['powered_off', 'deep_sleep', 'working', 'paused'];
    var globalNodeCfgInit = [];
    for (var gm = 0; gm < globalModes.length; gm++) {
        var gmKey = globalModes[gm];
        var gmc = (json.global_routes && json.global_routes[gmKey]) ? json.global_routes[gmKey] : {};

        var gmEnum = GLOBAL_NODE_MAP[gmKey];

        /* enter_actions */
        var ea = emitActionList(gmc.enter_actions || []);
        var eaName = uniqueName('s_enter_' + gmKey);
        c.push('static const HmiAction_t ' + eaName + '[] = {');
        c.push('    ' + ea.body);
        c.push('};');

        /* exit_actions */
        var xa = emitActionList(gmc.exit_actions || []);
        var xaName = uniqueName('s_exit_' + gmKey);
        c.push('static const HmiAction_t ' + xaName + '[] = {');
        c.push('    ' + xa.body);
        c.push('};');

        /* routes */
        var rts = emitRoutes(gmc.routes || {});
        var rtName = uniqueName('s_routes_' + gmKey);
        c.push('static const HmiRoute_t ' + rtName + '[] = {');
        c.push(rts.body);
        c.push('};');

        /* guards */
        var gds = emitGuards(gmc.guards || []);
        var gdName = uniqueName('s_guards_' + gmKey);
        c.push('static const HmiGuard_t ' + gdName + '[] = {');
        c.push(gds.body);
        c.push('};');
        c.push('');

        globalNodeCfgInit.push(
            '    [' + gmEnum + '] = {\n' +
            '        ' + eaName + ', ' + ea.count + ',\n' +
            '        ' + xaName + ', ' + xa.count + ',\n' +
            '        ' + rtName + ', ' + rts.count + ',\n' +
            '        ' + gdName + ', ' + gds.count + ',\n' +
            '    }'
        );
    }
    c.push('const HmiGlobalNodeCfg_t hmi_global_nodes[HMI_NODE_COUNT] = {');
    c.push(globalNodeCfgInit.join(',\n'));
    c.push('};');
    c.push('');
    h.push('extern const HmiGlobalNodeCfg_t hmi_global_nodes[HMI_NODE_COUNT];');

    /* ---- 3.4 zone_routes ---- */
    c.push('/* ===== Zone 路由 ===== */');
    var zoneStates = ['idle', 'selecting', 'cooking'];
    var zoneCfgInit = [];
    for (var zs = 0; zs < zoneStates.length; zs++) {
        var zsKey = zoneStates[zs];
        var zc = (json.zone_routes && json.zone_routes[zsKey]) ? json.zone_routes[zsKey] : {};
        var zsEnum = ZONE_NODE_MAP[zsKey];

        var zrts = emitRoutes(zc.routes || {});
        var zrName = uniqueName('s_zone_routes_' + zsKey);
        c.push('static const HmiRoute_t ' + zrName + '[] = {');
        c.push(zrts.body);
        c.push('};');

        var toStr = emitTimeoutEntry(zc);
        zoneCfgInit.push(
            '    [' + zsEnum + '] = {\n' +
            '        ' + zrName + ', ' + zrts.count + ',\n' +
            '        ' + toStr + ',\n' +
            '    }'
        );
    }
    c.push('const HmiZoneCfg_t hmi_zone_nodes[HMI_ZONE_COUNT] = {');
    c.push(zoneCfgInit.join(',\n'));
    c.push('};');
    c.push('');
    h.push('extern const HmiZoneCfg_t hmi_zone_nodes[HMI_ZONE_COUNT];');

    /* ---- 3.5 process_routes ---- */
    c.push('/* ===== 进程路由 ===== */');
    var procKeys = ['timer_setting', 'timer_active', 'boost_active'];
    var procCfgInit = [];
    for (var pk = 0; pk < procKeys.length; pk++) {
        var pKey = procKeys[pk];
        var pc = (json.process_routes && json.process_routes[pKey]) ? json.process_routes[pKey] : {};
        var pEnum = PROC_NODE_MAP[pKey];

        var prts = emitRoutes(pc.routes || {});
        var prName = uniqueName('s_proc_routes_' + pKey);
        c.push('static const HmiRoute_t ' + prName + '[] = {');
        c.push(prts.body);
        c.push('};');

        var ptoStr = emitTimeoutEntry(pc);
        procCfgInit.push(
            '    [' + pEnum + '] = {\n' +
            '        ' + prName + ', ' + prts.count + ',\n' +
            '        ' + ptoStr + ',\n' +
            '    }'
        );
    }
    c.push('const HmiProcessCfg_t hmi_process_nodes[HMI_PROC_COUNT] = {');
    c.push(procCfgInit.join(',\n'));
    c.push('};');
    c.push('');
    h.push('extern const HmiProcessCfg_t hmi_process_nodes[HMI_PROC_COUNT];');

    /* ---- 3.6 elements ---- */
    c.push('/* ===== 元素配置 ===== */');
    var el = (json.elements) ? json.elements : {};

    var tCfg = el.timer || {};
    var bCfg = el.boost || {};
    var sCfg = el.stack || {};
    var blCfg = el.blink || {};
    var mCfg = el.mode || {};

    c.push('const HmiElementCfg_t hmi_elements = {');
    c.push('    .timer_adjust_min     = ' + (tCfg.adjust_min !== undefined ? tCfg.adjust_min : 1) + ',');
    c.push('    .timer_adjust_max     = ' + (tCfg.adjust_max !== undefined ? tCfg.adjust_max : 99) + ',');
    c.push('    .timer_adjust_step    = ' + (tCfg.adjust_step !== undefined ? tCfg.adjust_step : 1) + ',');
    c.push('    .boost_power_level    = ' + (bCfg.power_level !== undefined ? bCfg.power_level : 9) + ',');
    c.push('    .stack_max_depth      = ' + (sCfg.max_depth !== undefined ? sCfg.max_depth : 4) + ',');
    c.push('    .blink_phase_ms       = ' + (blCfg.phase_ms !== undefined ? blCfg.phase_ms : 300) + ',');
    c.push('    .blink_pause_override = ' + (blCfg.pause_override !== undefined && blCfg.pause_override ? 1 : 0) + ',');
    c.push('    .mode_default         = ' + (SEG_MODE_MAP[mCfg['default']] || 'HMI_SEG_MODE_POWER') + ',');
    c.push('};');
    c.push('');
    h.push('extern const HmiElementCfg_t hmi_elements;');

    /* ModeRule */
    c.push('const HmiModeRule_t hmi_mode_rules[] = {');
    var mr = mCfg.rules || [];
    for (var mri = 0; mri < mr.length; mri++) {
        var cond = MODE_COND_MAP[mr[mri]['when']] || 'HMI_MODE_COND_POWERED_OFF';
        var sm = SEG_MODE_MAP[mr[mri].value] || 'HMI_SEG_MODE_POWER';
        c.push('    { ' + cond + ', ' + sm + ' },');
    }
    c.push('};');
    c.push('');
    h.push('extern const HmiModeRule_t hmi_mode_rules[];');
    h.push('#define HMI_MODE_RULE_COUNT ' + mr.length);

    /* Display patterns */
    var dp = el.display_patterns || {};
    c.push('const HmiDisplayPatterns_t hmi_patterns = {');
    c.push('    .dash = "' + (dp.dash || '--------') + '",');
    c.push('    .pa   = "' + (dp.pa   || 'PAPAPAPA') + '",');
    c.push('    .off  = "' + (dp.off  || '        ') + '",');
    c.push('};');
    c.push('');
    h.push('extern const HmiDisplayPatterns_t hmi_patterns;');

    /* Child lock whitelist — 读取 block_all_except */
    var clCfg = (json.global_routes && json.global_routes.child_lock) ? json.global_routes.child_lock : {};
    var cwl = clCfg.block_all_except || ['POWER'];
    c.push('/* ===== 童锁白名单 ===== */');
    for (var wl = 0; wl < cwl.length; wl++) {
        var wlKey = cwl[wl];
        var wlEnum = KEY_MAP[wlKey];
        if (!wlEnum) { console.error('Unknown whitelist key: ' + wlKey); process.exit(1); }
        /* 单独声明每个白名单路由条目 */
    }
    var wlRoutes = emitRoutes(
        (function() {
            var obj = {};
            for (var wi = 0; wi < cwl.length; wi++) {
                obj[cwl[wi] + ' tap'] = 'beep_valid';
                obj[cwl[wi] + ' long'] = 'beep_valid';
            }
            return obj;
        })()
    );
    var wlName = 's_child_lock_whitelist';
    c.push('static const HmiRoute_t ' + wlName + '[] = {');
    c.push(wlRoutes.body);
    c.push('};');
    c.push('');
    h.push('#define HMI_CHILD_LOCK_WL_LEN ' + wlRoutes.count);

    /* ---- 3.7 顶级 HmiConfig_t ---- */
    c.push('/* ===== 顶级配置聚合 ===== */');
    c.push('const HmiConfig_t hmi_cfg = {');
    c.push('    .timeouts                = hmi_timeouts,');
    c.push('    .power_on_seq            = hmi_power_on_seq,');
    c.push('    .power_on_seq_len        = HMI_POWER_ON_SEQ_LEN,');
    c.push('    .global_nodes            = hmi_global_nodes,');
    c.push('    .zone_nodes              = hmi_zone_nodes,');
    c.push('    .process_nodes           = hmi_process_nodes,');
    c.push('    .elements                = &hmi_elements,');
    c.push('    .mode_rules              = hmi_mode_rules,');
    c.push('    .mode_rule_count         = HMI_MODE_RULE_COUNT,');
    c.push('    .patterns                = &hmi_patterns,');
    c.push('    .child_lock_whitelist    = ' + wlName + ',');
    c.push('    .child_lock_whitelist_len = HMI_CHILD_LOCK_WL_LEN,');
    c.push('};');
    h.push('');
    h.push('extern const HmiConfig_t hmi_cfg;');

    /* 写入文件 */
    c.push('');
    h.push('');
    h.push('#endif /* HMI_DATA_H */');
    h.push('');

    return { c: c.join('\n'), h: h.join('\n') };
}

/* ================================================================
 * 四、入口
 * ================================================================ */

var json = JSON.parse(fs.readFileSync(JSON_PATH, 'utf-8'));
console.log('[gen_hmi] 读取: ' + json.meta.name + ' v' + json.meta.version);

var result = generate(json);

fs.writeFileSync(OUT_C, result.c, 'utf-8');
console.log('[gen_hmi] 生成: ' + OUT_C + ' (' + result.c.length + ' bytes)');

fs.writeFileSync(OUT_H, result.h, 'utf-8');
console.log('[gen_hmi] 生成: ' + OUT_H + ' (' + result.h.length + ' bytes)');

console.log('[gen_hmi] 完成.');
