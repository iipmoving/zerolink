#!/usr/bin/env python3
"""
v4json_to_c.py — 四头电磁炉 HMI JSON → C数据源文件 转换工具

工作流:
  开发阶段:  JS模拟器 (four_head_v4.json) ←→ 对客确认
  确认后:    python v4json_to_c.py four_head_v4.json
             → hmi_logic_data.c  (常量数组, 直接加入KEIL工程)
             → hmi_logic_defs.h  (枚举定义)

  零运行时JSON解析器, 零malloc, 零运行时开销。
"""

import json
import sys
import os
from collections import OrderedDict
import datetime

# ============================================================
# 1. 枚举定义（与 v4 引擎一致）
# ============================================================

KEY_ENUM = OrderedDict([
    ("KEY_TIMER", 1), ("KEY_PAUSE", 2), ("KEY_CHILD_LOCK", 3), ("KEY_POWER", 4),
    ("KEY_HEAD_1", 5), ("KEY_HEAD_2", 6), ("KEY_HEAD_3", 7), ("KEY_HEAD_4", 8),
    ("KEY_ZONE", 9), ("KEY_MINUS", 10), ("KEY_PLUS", 11),
    ("KEY_0", 12), ("KEY_1", 13), ("KEY_2", 14), ("KEY_3", 15),
    ("KEY_4", 16), ("KEY_5", 17), ("KEY_6", 18), ("KEY_7", 19),
    ("KEY_8", 20), ("KEY_9", 21)
])

KEY_SHORT_TO_FULL = {
    "TIMER": "KEY_TIMER", "PAUSE": "KEY_PAUSE", "CHILD_LOCK": "KEY_CHILD_LOCK",
    "POWER": "KEY_POWER",
    "HEAD_1": "KEY_HEAD_1", "HEAD_2": "KEY_HEAD_2", "HEAD_3": "KEY_HEAD_3",
    "HEAD_4": "KEY_HEAD_4",
    "ZONE": "KEY_ZONE", "MINUS": "KEY_MINUS", "PLUS": "KEY_PLUS",
    "0": "KEY_0", "1": "KEY_1", "2": "KEY_2", "3": "KEY_3", "4": "KEY_4",
    "5": "KEY_5", "6": "KEY_6", "7": "KEY_7", "8": "KEY_8", "9": "KEY_9",
    "HEAD_SELF": "HEAD_SELF", "HEAD_OTHER": "HEAD_OTHER",
}

EVT_ENUM = {"tap": 0, "long": 1, "release": 2}

GLOBAL_MODE_ENUM = OrderedDict([
    ("powering_up", 0), ("version_show", 1), ("powered_off", 2),
    ("working", 3), ("paused", 4), ("deep_sleep", 5)
])

ZONE_NODE_ENUM = {"idle": 0, "selecting": 1, "cooking": 2}

PROCESS_ENUM = {"timer_setting": 0, "timer_active": 1, "boost_active": 2}

ACTION_ENUM = OrderedDict([
    ("go_working", 1), ("go_powered_off", 2), ("go_deep_sleep", 3),
    ("toggle_pause", 4), ("toggle_child_lock", 5),
    ("select_head", 10), ("confirm_select", 11), ("confirm_select_immediate", 12),
    ("set_power", 13),
    ("enter_boost", 20), ("exit_boost", 21), ("exit_boost_set_power", 22),
    ("enter_timer_setting", 30), ("confirm_timer", 31),
    ("cancel_timer_setting", 32), ("cancel_timer_active", 33),
    ("timer_adjust", 34),
    ("show_dash", 40), ("show_pa", 41), ("show_power_mode", 42),
    ("display_all_off", 43), ("clear_all_heads", 44), ("hothead_clear", 45),
    ("reset_idle_timer", 46), ("reset_off_timer", 47),
    ("led_power_on", 50), ("led_power_off", 51), ("led_power_blink", 52),
    ("led_timer_on", 53), ("led_timer_off", 54),
    ("led_pause_on", 55), ("led_pause_off", 56),
    ("led_child_lock_on", 57), ("led_child_lock_off", 58),
    ("led_all_on", 59), ("led_all_off", 60),
])


def parse_route_key(route_key):
    parts = route_key.strip().split()
    if len(parts) < 2:
        return None, None
    key_short = parts[0]
    evt_str = parts[1].lower()
    evt_code = EVT_ENUM.get(evt_str)
    if key_short in ("HEAD_SELF", "HEAD_OTHER"):
        return ("SPECIAL_" + key_short, evt_code)
    full_key = KEY_SHORT_TO_FULL.get(key_short)
    if full_key is None:
        return None, None
    key_code = KEY_ENUM.get(full_key)
    return (key_code, evt_code)


def parse_action(action_str):
    parts = action_str.strip().split()
    action_name = parts[0]
    action_code = ACTION_ENUM.get(action_name, 0)
    param = 0
    if len(parts) > 1:
        try:
            param = int(parts[1])
        except ValueError:
            param = 0  # %head 等动态参数在C侧运行时解析
    return (action_code, param)


def c_arr_name(name):
    """JSON段名 → C数组名"""
    return name.replace("-", "_").replace(".", "_")


# ============================================================
# 2. 主转换器
# ============================================================

def convert(input_path, output_dir=None):
    if output_dir is None:
        output_dir = os.path.dirname(input_path)

    with open(input_path, 'r', encoding='utf-8') as f:
        v4 = json.load(f)

    raw_timeouts = v4.get("timeouts", {})
    timeout_keys = list(raw_timeouts.keys())
    tmo_values = [raw_timeouts[k] for k in timeout_keys]

    def tmo_val(key):
        return raw_timeouts.get(key, 0)

    ts = datetime.datetime.now().strftime('%Y-%m-%d %H:%M')
    src_name = os.path.basename(input_path)

    # ============================================================
    # 2.1 解析路由表
    # ============================================================
    KEY_CNT = 22
    EVT_CNT = 2
    KEYxEVT = KEY_CNT * EVT_CNT

    GLOBAL_MODE_CNT = len(GLOBAL_MODE_ENUM)
    ZONE_CNT = len(ZONE_NODE_ENUM)
    PROC_CNT = len(PROCESS_ENUM)

    def make_route_table(rows):
        return [[-1] * KEYxEVT for _ in range(rows)]

    global_rt = make_route_table(GLOBAL_MODE_CNT)
    zone_rt = make_route_table(ZONE_CNT)
    proc_rt = make_route_table(PROC_CNT)

    def fill_route(table, mode_idx, routes):
        for rk, av in routes.items():
            kc, ec = parse_route_key(rk)
            if kc is None:
                continue
            if isinstance(kc, str) and kc.startswith("SPECIAL_"):
                continue  # HEAD_SELF/HEAD_OTHER 在C侧单独处理
            ac, ap = parse_action(av)
            slot = kc * EVT_CNT + ec
            if 0 <= slot < KEYxEVT:
                table[mode_idx][slot] = (ac << 8) | ap

    # --- 全局路由 ---
    gmodes = v4.get("global_routes", {})
    for mn, mc in gmodes.items():
        if mn.startswith("_") or mn == "child_lock":
            continue
        mi = GLOBAL_MODE_ENUM.get(mn, -1)
        if mi >= 0:
            fill_route(global_rt, mi, mc.get("routes", {}))

    # --- Zone 路由 ---
    zmodes = v4.get("zone_routes", {})
    for zn, zc in zmodes.items():
        if zn.startswith("_"):
            continue
        zi = ZONE_NODE_ENUM.get(zn, -1)
        if zi >= 0:
            fill_route(zone_rt, zi, zc.get("routes", {}))

    # --- 进程路由 ---
    pmodes = v4.get("process_routes", {})
    for pn, pc in pmodes.items():
        if pn.startswith("_"):
            continue
        pi = PROCESS_ENUM.get(pn, -1)
        if pi >= 0:
            fill_route(proc_rt, pi, pc.get("routes", {}))

    # --- HEAD_SELF / HEAD_OTHER 特殊路由（Zone内）---
    self_other = {}
    for zn, zc in zmodes.items():
        if zn.startswith("_"):
            continue
        zi = ZONE_NODE_ENUM.get(zn, -1)
        if zi < 0:
            continue
        for rk, av in zc.get("routes", {}).items():
            kc, ec = parse_route_key(rk)
            if not (isinstance(kc, str) and kc.startswith("SPECIAL_")):
                continue
            is_self = 1 if kc == "SPECIAL_HEAD_SELF" else 0
            ac, ap = parse_action(av)
            self_other.setdefault(zi, []).append((is_self, ec, ac, ap))

    # --- 上电序列 ---
    seq = v4.get("power_on_sequence", [])
    seq_c = []
    for s in seq:
        d = s.get("delay_ms_key", s.get("delay_ms", 0))
        if isinstance(d, str):
            d = tmo_val(d) if d in raw_timeouts else 3000
        leds_str = s.get("leds", "")
        leds_val = 0xFFFF if leds_str == "all_on" else (0 if leds_str == "all_off" else 0)
        goto_mode = GLOBAL_MODE_ENUM.get(s.get("goto", ""), -1)
        seq_c.append((s.get("step", 0), d, s.get("seg_chars", ""), leds_val, goto_mode))

    # --- Guards ---
    guards = []
    for mn, mc in gmodes.items():
        if mn.startswith("_") or mn == "child_lock":
            continue
        mi = GLOBAL_MODE_ENUM.get(mn, -1)
        for g in mc.get("guards", []):
            guards.append((mi, tmo_val(g.get("ms_key", 0)), ACTION_ENUM.get(g.get("action"), 0)))

    # --- State timeouts (模式内超时) ---
    state_tmos = []
    zone_tmos = []
    proc_tmos = []
    for mn, mc in gmodes.items():
        if mn.startswith("_") or mn == "child_lock":
            continue
        mi = GLOBAL_MODE_ENUM.get(mn, -1)
        t = mc.get("timeout")
        if t:
            state_tmos.append((mi, tmo_val(t.get("ms_key", 0)), ACTION_ENUM.get(t.get("action"), 0)))
    for zn, zc in zmodes.items():
        if zn.startswith("_"):
            continue
        zi = ZONE_NODE_ENUM.get(zn, -1)
        t = zc.get("timeout")
        if t:
            zone_tmos.append((zi, tmo_val(t.get("ms_key", 0)), ACTION_ENUM.get(t.get("action"), 0)))
    for pn, pc in pmodes.items():
        if pn.startswith("_"):
            continue
        pi = PROCESS_ENUM.get(pn, -1)
        t = pc.get("timeout")
        if t:
            proc_tmos.append((pi, tmo_val(t.get("ms_key", 0)), ACTION_ENUM.get(t.get("action"), 0)))

    # --- Enter / Exit actions ---
    enter_acts = []
    exit_acts = []
    for mn, mc in gmodes.items():
        if mn.startswith("_") or mn == "child_lock":
            continue
        mi = GLOBAL_MODE_ENUM.get(mn, -1)
        ea = mc.get("enter_actions", [])
        if ea:
            enter_acts.append((mi, [ACTION_ENUM.get(a, 0) for a in ea]))
        xa = mc.get("exit_actions", [])
        if xa:
            exit_acts.append((mi, [ACTION_ENUM.get(a, 0) for a in xa]))

    # --- Child lock whitelist ---
    whitelist = []
    cl = gmodes.get("child_lock", {})
    for k in cl.get("block_all_except", []):
        fk = KEY_SHORT_TO_FULL.get(k)
        if fk and fk in KEY_ENUM:
            whitelist.append(KEY_ENUM[fk])

    # ============================================================
    # 2.2 Elements 解析
    # ============================================================
    elems = v4.get("elements", {})
    ss = elems.get("segment_slot", {})
    seg_slot = (ss.get("digit_count", 2), ord(str(ss.get("boost_char", "P"))[0]))

    be = elems.get("boost", {})
    boost_lv = be.get("power_level", 9)

    te = elems.get("timer", {})
    timer_cfg = (te.get("adjust_min", 1), te.get("adjust_max", 99), te.get("adjust_step", 1))

    ds = elems.get("stack", {})
    stack_depth = ds.get("max_depth", 4)

    blink = elems.get("blink", {})
    blink_phase = blink.get("phase_ms", 300)
    blink_excl = [{"timer_setting": 0, "boost_active": 1}.get(e, 0) for e in blink.get("exclude_when", [])]

    # ============================================================
    # 2.3 Display 规则
    # ============================================================
    disp = v4.get("display", {})
    priority_chain = []
    mode_map = {"timer_setting": 0, "boost_active": 1, "power_level": 2}
    for item in disp.get("priority_chain", []):
        priority_chain.append(mode_map.get(item, 0))

    blk = disp.get("blink_rule", {})
    alt = disp.get("alternate_rule", {})
    alt_phases = alt.get("phases", [])

    # ============================================================
    # 3. 写 C 头文件
    # ============================================================
    os.makedirs(output_dir, exist_ok=True)
    base = os.path.join(output_dir, "hmi_logic")

    with open(base + "_defs.h", 'w', encoding='utf-8') as f:
        f.write(f"""/**
 * hmi_logic_defs.h — HMI 逻辑数据定义（自动生成）
 * 源文件: {src_name}
 * 生成时间: {ts}
 *
 * 使用方式:
 *   1. #include "hmi_logic_defs.h"
 *   2. #include "hmi_logic_data.c"  (或在工程中加入该.c文件)
 *   3. 用枚举值查表
 */
#ifndef __HMI_LOGIC_DEFS_H__
#define __HMI_LOGIC_DEFS_H__

#include <stdint.h>

/* ============================================================
 * 按键编码
 * ============================================================ */
typedef enum {{
    {chr(10).join(f"    {k.upper()} = {v}," for k, v in KEY_ENUM.items())}
}} key_code_t;

/* ============================================================
 * 按键事件类型
 * ============================================================ */
typedef enum {{
    EVT_TAP     = 0,
    EVT_LONG    = 1,
    EVT_RELEASE = 2
}} event_type_t;

/* ============================================================
 * 全局模式
 * ============================================================ */
typedef enum {{
    {chr(10).join(f"    GLOBAL_{k.upper()} = {v}," for k, v in GLOBAL_MODE_ENUM.items())}
}} global_mode_t;

/* ============================================================
 * Zone 节点状态
 * ============================================================ */
typedef enum {{
    ZONE_IDLE       = 0,
    ZONE_SELECTING  = 1,
    ZONE_COOKING    = 2
}} zone_node_t;

/* ============================================================
 * 正交进程类型
 * ============================================================ */
typedef enum {{
    PROC_TIMER_SETTING  = 0,
    PROC_TIMER_ACTIVE   = 1,
    PROC_BOOST_ACTIVE   = 2
}} process_type_t;

/* ============================================================
 * 动作枚举（路由目标）
 * ============================================================ */
typedef enum {{
    ACT_NONE                = 0,
    {chr(10).join(f"    ACT_{k.upper()} = {v}," for k, v in ACTION_ENUM.items())}
}} action_t;

/* ============================================================
 * 超时索引
 * ============================================================ */
typedef enum {{
{chr(10).join(f"    TMO_{k.upper().replace('_MS','')}_MS = {i}," for i, k in enumerate(timeout_keys))}
    TMO_COUNT = {len(timeout_keys)}
}} timeout_idx_t;

/* ============================================================
 * 路由表编码宏
 * ============================================================ */
#define KEY_COUNT           {KEY_CNT}
#define EVT_COUNT           2
#define GLOBAL_MODE_COUNT   {GLOBAL_MODE_CNT}
#define ZONE_COUNT          {ZONE_CNT}
#define PROCESS_COUNT       {PROC_CNT}

#define ROUTE_SLOT(key, evt)    ((key) * EVT_COUNT + (evt))
#define ROUTE_ENCODE(act, param) (((int16_t)(act) << 8) | (uint8_t)(param))
#define ROUTE_ACTION(val)       ((int16_t)((val) >> 8))
#define ROUTE_PARAM(val)        ((uint8_t)(val))

#define ROUTE_NONE              (-1)

/* ============================================================
 * 上电序列步进结构
 * ============================================================ */
typedef struct {{
    uint16_t delay_ms;
    const char seg_chars[9];
    uint16_t leds_mask;
    int8_t   goto_mode;      /* -1 = 不跳转 */
}} power_on_step_t;

/* ============================================================
 * 外部数据声明（定义在 hmi_logic_data.c）
 * ============================================================ */
extern const uint16_t timeout_values[TMO_COUNT];
extern const int16_t  global_route_table[GLOBAL_MODE_COUNT][KEY_COUNT * EVT_COUNT];
extern const int16_t  zone_route_table[ZONE_COUNT][KEY_COUNT * EVT_COUNT];
extern const int16_t  process_route_table[PROCESS_COUNT][KEY_COUNT * EVT_COUNT];
extern const power_on_step_t power_on_sequence[{len(seq_c)}];
extern const uint8_t  power_on_sequence_count;

extern const uint8_t  child_lock_whitelist[{len(whitelist)}];
extern const uint8_t  child_lock_whitelist_count;

extern const uint8_t  self_other_route_count;
extern const uint8_t  self_other_route_data[][4];  /* zone, is_self, evt, action */

extern const uint8_t  enter_action_count;
extern const uint8_t  enter_action_data[][9];      /* mode, act0..act7 */

extern const uint8_t  exit_action_count;
extern const uint8_t  exit_action_data[][9];

extern const int16_t  guarded_actions[][3];        /* mode, ms, action */
extern const uint8_t  guarded_action_count;

extern const int16_t  state_timeouts[][3];          /* mode, ms, action */
extern const uint8_t  state_timeout_count;

extern const int16_t  zone_timeouts[][3];
extern const uint8_t  zone_timeout_count;

extern const int16_t  process_timeouts[][3];
extern const uint8_t  process_timeout_count;

extern const uint8_t  priority_chain[3];
extern const uint8_t  blink_phase_ms;
extern const uint8_t  blink_exclude[];

extern const uint8_t  segment_digit_count;
extern const uint8_t  segment_boost_char;
extern const uint8_t  boost_power_level;
extern const uint8_t  timer_adjust_min;
extern const uint8_t  timer_adjust_max;
extern const uint8_t  timer_adjust_step;
extern const uint8_t  stack_max_depth;

extern const uint8_t  alternate_enabled;
extern const uint8_t  alternate_phases[2][2];       /* [phase][source/duration_ms] */

#endif /* __HMI_LOGIC_DEFS_H__ */
""")
    print(f"[OK] C头文件: {base}_defs.h")

    # ============================================================
    # 4. 写 C 数据源文件
    # ============================================================
    with open(base + "_data.c", 'w', encoding='utf-8') as f:
        f.write(f"""/**
 * hmi_logic_data.c — HMI 逻辑常量数据（自动生成）
 * 源文件: {src_name}
 * 生成时间: {ts}
 *
 * 直接加入KEIL工程编译, 零运行时JSON解析开销。
 */
#include "hmi_logic_defs.h"

/* ============================================================
 * 1. 超时值表
 * ============================================================ */
const uint16_t timeout_values[TMO_COUNT] = {{
    {', '.join(str(v) for v in tmo_values)}
}};

/* ============================================================
 * 2. 全局路由表 [GLOBAL_MODE_COUNT][KEY_COUNT * 2]
 *    槽值 = ROUTE_ENCODE(action, param), -1 = 无路由
 * ============================================================ */
const int16_t global_route_table[GLOBAL_MODE_COUNT][KEY_COUNT * EVT_COUNT] = {{
""")
        for mi in range(GLOBAL_MODE_CNT):
            row = global_rt[mi]
            # 只输出非 -1 的槽, 用 designated initializer
            parts = []
            for slot, val in enumerate(row):
                if val >= 0:
                    key_idx = slot // EVT_CNT
                    evt_idx = slot % EVT_CNT
                    parts.append(f"    [ROUTE_SLOT({key_idx}, {evt_idx})] = {val}")
            if parts:
                f.write(f"    [{mi}] = {{\n")
                f.write(",\n".join(parts) + "\n")
                f.write("    },\n")
            else:
                f.write(f"    [{mi}] = {{ -1 }},\n")
        f.write("};\n\n")

        # --- Zone 路由 ---
        f.write("/* ============================================================\n")
        f.write(" * 3. Zone 路由表 [ZONE_COUNT][KEY_COUNT * 2]\n")
        f.write(" * ============================================================ */\n")
        f.write("const int16_t zone_route_table[ZONE_COUNT][KEY_COUNT * EVT_COUNT] = {\n")
        for zi in range(ZONE_CNT):
            row = zone_rt[zi]
            parts = []
            for slot, val in enumerate(row):
                if val >= 0:
                    key_idx = slot // EVT_CNT
                    evt_idx = slot % EVT_CNT
                    parts.append(f"    [ROUTE_SLOT({key_idx}, {evt_idx})] = {val}")
            if parts:
                f.write(f"    [{zi}] = {{\n")
                f.write(",\n".join(parts) + "\n")
                f.write("    },\n")
            else:
                f.write(f"    [{zi}] = {{ -1 }},\n")
        f.write("};\n\n")

        # --- 进程路由 ---
        f.write("/* ============================================================\n")
        f.write(" * 4. 进程路由表 [PROCESS_COUNT][KEY_COUNT * 2]\n")
        f.write(" * ============================================================ */\n")
        f.write("const int16_t process_route_table[PROCESS_COUNT][KEY_COUNT * EVT_COUNT] = {\n")
        for pi in range(PROC_CNT):
            row = proc_rt[pi]
            parts = []
            for slot, val in enumerate(row):
                if val >= 0:
                    key_idx = slot // EVT_CNT
                    evt_idx = slot % EVT_CNT
                    parts.append(f"    [ROUTE_SLOT({key_idx}, {evt_idx})] = {val}")
            if parts:
                f.write(f"    [{pi}] = {{\n")
                f.write(",\n".join(parts) + "\n")
                f.write("    },\n")
            else:
                f.write(f"    [{pi}] = {{ -1 }},\n")
        f.write("};\n\n")

        # --- 上电序列 ---
        f.write("/* ============================================================\n")
        f.write(" * 5. 上电序列\n")
        f.write(" * ============================================================ */\n")
        f.write(f"const power_on_step_t power_on_sequence[{len(seq_c)}] = {{\n")
        for st in seq_c:
            chars = st[2] if st[2] else '""'
            f.write(f"    {{ {st[1]}, \"{chars}\", {st[3]}, {st[4]} }},\n")
        f.write("};\n")
        f.write(f"const uint8_t power_on_sequence_count = {len(seq_c)};\n\n")

        # --- 童锁白名单 ---
        f.write("/* ============================================================\n")
        f.write(" * 6. 童锁白名单\n")
        f.write(" * ============================================================ */\n")
        f.write(f"const uint8_t child_lock_whitelist[{len(whitelist)}] = {{ {', '.join(str(w) for w in whitelist)} }};\n")
        f.write(f"const uint8_t child_lock_whitelist_count = {len(whitelist)};\n\n")

        # --- HEAD_SELF/HEAD_OTHER ---
        flat_so = []
        for zi, rules in self_other.items():
            for is_self, ec, ac, ap in rules:
                flat_so.append((zi, is_self, ec, ac, ap))
        f.write("/* ============================================================\n")
        f.write(" * 7. HEAD_SELF / HEAD_OTHER 特殊路由\n")
        f.write(" *    [zone, is_self, evt, action]\n")
        f.write(" * ============================================================ */\n")
        f.write(f"const uint8_t self_other_route_data[{len(flat_so)}][4] = {{\n")
        for so in flat_so:
            f.write(f"    {{ {so[0]}, {so[1]}, {so[2]}, {so[3]} }},\n")
        f.write("};\n")
        f.write(f"const uint8_t self_other_route_count = {len(flat_so)};\n\n")

        # --- Enter actions ---
        f.write("/* ============================================================\n")
        f.write(" * 8. 全局模式进入动作\n")
        f.write(" *    [mode, act0..act7] (act=0 = end)\n")
        f.write(" * ============================================================ */\n")
        f.write(f"const uint8_t enter_action_data[{len(enter_acts)}][9] = {{\n")
        for mi, acts in enter_acts:
            padded = acts + [0] * (8 - len(acts))
            f.write(f"    {{ {mi}, {', '.join(str(a) for a in padded)} }},\n")
        f.write("};\n")
        f.write(f"const uint8_t enter_action_count = {len(enter_acts)};\n\n")

        # --- Exit actions ---
        f.write("/* ============================================================\n")
        f.write(" * 9. 全局模式退出动作\n")
        f.write(" * ============================================================ */\n")
        f.write(f"const uint8_t exit_action_data[{len(exit_acts)}][9] = {{\n")
        for mi, acts in exit_acts:
            padded = acts + [0] * (8 - len(acts))
            f.write(f"    {{ {mi}, {', '.join(str(a) for a in padded)} }},\n")
        f.write("};\n")
        f.write(f"const uint8_t exit_action_count = {len(exit_acts)};\n\n")

        # --- Guards ---
        f.write("/* ============================================================\n")
        f.write(" * 10. 守卫 (全idle→关机 等)\n")
        f.write(" *     [mode, ms, action]\n")
        f.write(" * ============================================================ */\n")
        f.write(f"const int16_t guarded_actions[{len(guards)}][3] = {{\n")
        for g in guards:
            f.write(f"    {{ {g[0]}, {g[1]}, {g[2]} }},\n")
        f.write("};\n")
        f.write(f"const uint8_t guarded_action_count = {len(guards)};\n\n")

        # --- State timeouts ---
        f.write(f"const int16_t state_timeouts[{len(state_tmos)}][3] = {{\n")
        for t in state_tmos:
            f.write(f"    {{ {t[0]}, {t[1]}, {t[2]} }},\n")
        f.write("};\n")
        f.write(f"const uint8_t state_timeout_count = {len(state_tmos)};\n\n")

        # --- Zone timeouts ---
        f.write(f"const int16_t zone_timeouts[{len(zone_tmos)}][3] = {{\n")
        for t in zone_tmos:
            f.write(f"    {{ {t[0]}, {t[1]}, {t[2]} }},\n")
        f.write("};\n")
        f.write(f"const uint8_t zone_timeout_count = {len(zone_tmos)};\n\n")

        # --- Process timeouts ---
        f.write(f"const int16_t process_timeouts[{len(proc_tmos)}][3] = {{\n")
        for t in proc_tmos:
            f.write(f"    {{ {t[0]}, {t[1]}, {t[2]} }},\n")
        f.write("};\n")
        f.write(f"const uint8_t process_timeout_count = {len(proc_tmos)};\n\n")

        # --- Display / Elements ---
        f.write("/* ============================================================\n")
        f.write(" * 11. 显示规则 & 元素属性\n")
        f.write(" * ============================================================ */\n")
        f.write(f"const uint8_t priority_chain[{len(priority_chain)}] = {{ {', '.join(str(p) for p in priority_chain)} }};\n")
        f.write(f"const uint8_t blink_phase_ms = {blink_phase};\n")
        f.write(f"const uint8_t blink_exclude[] = {{ {', '.join(str(x) for x in blink_excl)} }};\n")
        f.write(f"const uint8_t segment_digit_count = {seg_slot[0]};\n")
        f.write(f"const uint8_t segment_boost_char = {seg_slot[1]};\n")
        f.write(f"const uint8_t boost_power_level = {boost_lv};\n")
        f.write(f"const uint8_t timer_adjust_min = {timer_cfg[0]};\n")
        f.write(f"const uint8_t timer_adjust_max = {timer_cfg[1]};\n")
        f.write(f"const uint8_t timer_adjust_step = {timer_cfg[2]};\n")
        f.write(f"const uint8_t stack_max_depth = {stack_depth};\n")

        alt_en = 1 if alt_phases else 0
        f.write(f"const uint8_t alternate_enabled = {alt_en};\n")
        if alt_phases:
            f.write(f"const uint8_t alternate_phases[2][2] = {{\n")
            for p in alt_phases:
                src = {"power_level": 0, "timer_value": 1}.get(p.get("source", ""), 0)
                dur = p.get("duration_ms", 5000)
                f.write(f"    {{ {src}, {dur} }},\n")
            f.write("};\n")

    print(f"[OK] C数据源: {base}_data.c")

    # ============================================================
    # 5. 统计
    # ============================================================
    grc = sum(1 for r in global_rt for v in r if v >= 0)
    zrc = sum(1 for r in zone_rt for v in r if v >= 0)
    prc = sum(1 for r in proc_rt for v in r if v >= 0)
    print(f"""
转换完成:
  源文件:     {src_name}
  输出目录:   {output_dir}

  路由表:
    全局:     {grc} 条
    Zone:     {zrc} 条
    进程:     {prc} 条

  配置:
    超时:     {len(timeout_keys)} 项
    元素属性: {len(v4.get('elements', {}))} 类
    守卫:     {len(guards)} 条
    上电步骤: {len(seq_c)} 步

  枚举:
    动作:     {len(ACTION_ENUM)} 个
    按键:     {len(KEY_ENUM)} 个

  文件:
    {base}_defs.h  — 枚举定义 (加入include路径)
    {base}_data.c  — 常量数据 (加入KEIL工程)

  使用: JS模拟器调好逻辑 → 跑此工具 → KEIL编译 → 烧录
""")
    return base + "_data.c", base + "_defs.h"


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("用法: python v4json_to_c.py <four_head_v4.json路径> [输出目录]")
        print("示例: python v4json_to_c.py ../sim/test_hmi/logic/four_head_v4.json ../sim/test_hmi/logic/")
        sys.exit(1)
    input_path = sys.argv[1]
    output_dir = sys.argv[2] if len(sys.argv) > 2 else os.path.dirname(input_path)
    convert(input_path, output_dir)
