#!/usr/bin/env python3
"""
check_weak_pairs.py — __weak 回调配对一致性检查 (多项目可配置)

扫描 interface_map.h 中记录的配对，验证:
  1. 发送方文件存在对应的 __weak 声明
  2. 接收方文件存在对应的强符号实现
  3. 函数签名一致（参数类型/顺序/返回值）
  4. 孤儿 __weak: 存在 __weak 声明但 interface_map.h 无记录
  5. 孤儿强符号: 存在回调实现但无 __weak 发送方

用法:
  python check_weak_pairs.py <project_dir>                            自动检测
  python check_weak_pairs.py <project_dir> --project four_head        内置预设
  python check_weak_pairs.py <project_dir> --config deps_config.json  自定义配置
  python check_weak_pairs.py --print-template                         输出 JSON 模板
  python check_weak_pairs.py --self-test                              自检

退出码: 0=通过, 1=发现违规, 2=用法错误

配置来源优先级: --config > --project > 自动检测(drv/或base_class/目录)
如果 project_dir 下有 weak_pairs_config.json 自动加载。
"""

import sys
import os
import re
import json
import tempfile
from collections import defaultdict

# ============================================================
# 内置预设
# ============================================================
BUILTIN_PRESETS = {
    'four_head': {
        'interface_map': 'core/interface_map.h',
        'scan_dirs': ['app', 'drv', 'hal', 'proto', 'core', 'cfg'],
        'public_api_excludes': [
            'engine_.*',
            '.*_OnTick', '.*_OnSecondTick', '.*_OnInit',
            '.*_Inject',
        ],
        'callback_name_pattern': r'^\w+_On\w+$',
    },
    'm4-ekf': {
        'interface_map': 'core/interface_map.h',
        'scan_dirs': ['app', 'base_class', 'proto', 'core'],
        'public_api_excludes': [
            '.*_OnTick', '.*_OnInit',
        ],
        'callback_name_pattern': r'^\w+_On\w+$',
    },
}


# ============================================================
# 正则表达式 (编译一次)
# ============================================================

PAIR_START_RE = re.compile(r'/[*]\s*Pair\s+(\S+)\s*:\s*(.+?)\s*[*]/')
SENDER_RE = re.compile(
    r'发送方\s*:\s*(\S+\.[cC])\s+WEAK\s+'
    r'(\w+(?:\s*\*)?)\s+'           # return type
    r'(\w+)\s*[(]'                    # function name
    r'([^)]*)[)]\s*[{]'               # params
)
RECEIVER_RE = re.compile(
    r'接收方\s*:\s*(\S+\.[cC])\s+'
    r'(\w+(?:\s*\*)?)\s+'            # return type
    r'(\w+)\s*[(]'                     # function name
    r'([^)]*)[)]'                      # params
)
NOTE_RE = re.compile(r'注\s*:\s*(.+?)\s*[*]/')

# 搜索 WEAK 声明: WEAK void FuncName(params);
WEAK_DECL_H_RE = re.compile(
    r'(?:WEAK|__weak|__attribute__\(\(weak\)\))\s+'
    r'(\w+(?:\s*\*)?)\s+'            # return type
    r'(\w+)\s*[(]'                     # function name
    r'([^)]*)[)]'                      # params
)


# ============================================================
# 配置加载
# ============================================================

def load_config_file(path):
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)


def resolve_config(project_dir, preset_name, config_path):
    if config_path:
        cfg = load_config_file(config_path)
        # 支持从 deps_config.json 读取 weak_pairs 子段
        if 'weak_pairs' in cfg:
            return cfg['weak_pairs'], 'config:' + config_path
        if 'interface_map' in cfg:
            return cfg, 'config:' + config_path
        print("错误: 配置文件缺少 'interface_map' 字段")
        sys.exit(2)

    if preset_name:
        if preset_name not in BUILTIN_PRESETS:
            print(f"错误: 未知预设 '{preset_name}'.")
            print(f"可用: {list(BUILTIN_PRESETS.keys())}")
            sys.exit(2)
        return BUILTIN_PRESETS[preset_name], 'preset:' + preset_name

    # 尝试从 project_dir/weak_pairs_config.json 自动加载
    local_cfg = os.path.join(project_dir, 'weak_pairs_config.json')
    if os.path.isfile(local_cfg):
        return load_config_file(local_cfg), 'file:weak_pairs_config.json'

    # 也尝试从 deps_config.json 读取 weak_pairs 段
    deps_cfg = os.path.join(project_dir, 'deps_config.json')
    if os.path.isfile(deps_cfg):
        cfg = load_config_file(deps_cfg)
        if 'weak_pairs' in cfg:
            return cfg['weak_pairs'], 'file:deps_config.json#weak_pairs'

    # 自动检测
    if os.path.isdir(os.path.join(project_dir, 'drv')):
        return BUILTIN_PRESETS['four_head'], 'auto:four_head'
    if os.path.isdir(os.path.join(project_dir, 'base_class')):
        return BUILTIN_PRESETS['m4-ekf'], 'auto:m4-ekf'

    print("错误: 无法自动检测项目类型，请使用 --project 或 --config")
    print(f"可用预设: {list(BUILTIN_PRESETS.keys())}")
    sys.exit(2)


# ============================================================
# 接口: 从 interface_map.h 解析配对
# ============================================================

def parse_interface_map(filepath):
    """从 interface_map.h 提取所有配对记录。"""
    pairs = []
    if not os.path.exists(filepath):
        return pairs

    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    lines = content.split('\n')

    i = 0
    while i < len(lines):
        line = lines[i].strip()
        pm = PAIR_START_RE.match(line)
        if not pm:
            i += 1
            continue

        pair_id = pm.group(1)
        description = pm.group(2)

        sender_info = None
        receiver_info = None
        note = None

        j = i + 1
        while j < len(lines) and j < i + 10:
            cline = lines[j].strip()
            if not cline.startswith('*') and not cline.startswith('/*'):
                break

            sm = SENDER_RE.search(cline)
            if sm:
                sender_info = (sm.group(1), sm.group(2).strip(),
                               sm.group(3), sm.group(4))

            rm = RECEIVER_RE.search(cline)
            if rm:
                receiver_info = (rm.group(1), rm.group(2).strip(),
                                 rm.group(3), rm.group(4))

            nm = NOTE_RE.search(cline)
            if nm:
                note = nm.group(1).strip()

            j += 1

        if sender_info and receiver_info:
            pairs.append((pair_id, description,
                          sender_info[0], sender_info[1], sender_info[2], sender_info[3],
                          receiver_info[0], receiver_info[1], receiver_info[2], receiver_info[3],
                          note))

        i = j

    return pairs


# ============================================================
# 函数签名查找与比较
# ============================================================

TYPE_KEYWORDS = (
    'const', 'volatile', 'void', 'struct', 'union', 'enum',
    'unsigned', 'signed', 'char', 'int', 'short', 'long', 'float', 'double',
)


def _is_type_token(tok):
    """判断 token 是否是类型关键字或指针（非参数名）。"""
    return tok in TYPE_KEYWORDS or tok.endswith('*') or tok == '*'


def normalize_params(params_str):
    """规范化参数字符串用于比较: 去除空格差异、参数名，保留类型信息。"""
    s = params_str.strip()
    if s == 'void' or s == '':
        return 'void'
    s = re.sub(r'\s+', ' ', s)
    parts = s.split(',')
    normalized = []
    for p in parts:
        p = p.strip()
        tokens = p.split()
        # 合并指针: "void * data_ptr" → "void* data_ptr"
        #          "void *data_ptr"  → "void* data_ptr"
        merged = []
        for tok in tokens:
            if tok == '*':
                if merged:
                    merged[-1] += '*'
            elif tok.startswith('*'):
                # *data_ptr → merge * to previous, keep data_ptr
                if merged:
                    merged[-1] += '*'
                else:
                    merged.append('*')
                name = tok[1:]  # remove leading *
                if name:
                    merged.append(name)
            else:
                merged.append(tok)
        # 去掉最后的参数名
        if merged and not _is_type_token(merged[-1]):
            merged.pop()
        normalized.append(' '.join(merged))
    return ', '.join(normalized).strip() if normalized else 'void'


def find_file(source_dir, filename):
    """在 source_dir 下搜索文件名。"""
    for root, dirs, files in os.walk(source_dir):
        for f in files:
            if f == filename or f == os.path.basename(filename):
                return os.path.join(root, f)
    return None


def _find_weak_in_content(filepath, func_name):
    """在文件内容中搜索 WEAK 声明 (多行感知)。"""
    if not os.path.exists(filepath):
        return None
    # 跳过 interface_map.h (纯文档, 不是实际声明)
    if os.path.basename(filepath) == 'interface_map.h':
        return None

    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    weak_positions = []
    for m in re.finditer(r'\b(?:WEAK|__weak)\b', content):
        weak_positions.append(m.end())
    for m in re.finditer(r'__attribute__\(\(weak\)\)', content):
        weak_positions.append(m.end())
    weak_positions.sort()

    for wpos in weak_positions:
        window = content[wpos:wpos + 500]
        fn_pattern = re.compile(
            r'(\w+(?:\s*\*)?)\s+'            # return type
            + re.escape(func_name) +
            r'\s*[(]([^)]*)[)]'              # params
        )
        m = fn_pattern.search(window)
        if m:
            lineno = content[:wpos].count('\n') + 1
            return (m.group(1).strip(), m.group(2), lineno)

    return None


def _find_strong_in_content(filepath, func_name):
    """搜索强符号定义 — 排除 WEAK/__weak/extern/typedef 行。"""
    if not os.path.exists(filepath):
        return None

    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    # 逐行匹配，排除以 WEAK/__weak/extern/typedef 开头的行
    pattern = re.compile(
        r'^(?!\s*(?:WEAK|__weak|extern|typedef)\b)'
        r'(?:inline\s+)?(?:static\s+)?'
        r'(\w+(?:\s*\*)?)\s+'
        + re.escape(func_name) +
        r'\s*[(]'
        r'([^)]*)'
        r'[)]',
        re.MULTILINE
    )

    m = pattern.search(content)
    if m:
        lineno = content[:m.start()].count('\n') + 1
        return (m.group(1).strip(), m.group(2), lineno)
    return None


def find_function_in_file(filepath, func_name, is_weak):
    """搜索指定函数。

    is_weak=True:  搜索 WEAK 声明 (先查 .h 再查 .c)
    is_weak=False: 搜索强符号定义 (仅 .c)
    """
    if not os.path.exists(filepath):
        return None

    if is_weak:
        base = os.path.splitext(filepath)[0]
        search_files = [filepath]
        h_path = base + '.h'
        if os.path.exists(h_path):
            search_files.insert(0, h_path)

        for sf in search_files:
            result = _find_weak_in_content(sf, func_name)
            if result:
                return result
        return None
    else:
        return _find_strong_in_content(filepath, func_name)


def compare_signatures(sender_sig, receiver_sig):
    """比较发送方和接收方的函数签名。返回: (is_match, diff_description)。"""
    s_ret, s_params = sender_sig
    r_ret, r_params = receiver_sig

    diffs = []
    if s_ret != r_ret:
        diffs.append(f"返回类型: '{s_ret}' vs '{r_ret}'")

    s_norm = normalize_params(s_params)
    r_norm = normalize_params(r_params)
    if s_norm != r_norm:
        diffs.append(f"参数: '{s_params}' vs '{r_params}'")

    if diffs:
        return False, '; '.join(diffs)
    return True, None


# ============================================================
# 模式 A: 从 interface_map.h 验证
# ============================================================

def verify_from_interface_map(pairs, source_dir):
    """根据 interface_map.h 中的记录验证代码。"""
    violations = []

    for (pair_id, desc,
         s_file, s_ret, s_name, s_params,
         r_file, r_ret, r_name, r_params,
         note) in pairs:

        # 检查发送方文件
        s_path = find_file(source_dir, s_file)
        if s_path is None:
            violations.append({
                'pair': pair_id,
                'type': 'sender_file_missing',
                'desc': f"Pair {pair_id} ({desc}): 发送方文件 '{s_file}' 不存在",
            })
            continue

        s_func = find_function_in_file(s_path, s_name, is_weak=True)
        if s_func is None:
            violations.append({
                'pair': pair_id,
                'type': 'sender_weak_missing',
                'desc': f"Pair {pair_id} ({desc}): {s_file} 中未找到 __weak {s_name}() 声明",
                'file': s_path,
            })
            continue

        s_actual_ret, s_actual_params, s_line = s_func

        ok, diff = compare_signatures((s_ret, s_params), (s_actual_ret, s_actual_params))
        if not ok:
            violations.append({
                'pair': pair_id,
                'type': 'sender_signature_mismatch',
                'desc': f"Pair {pair_id} ({desc}): {s_file}:{s_line} __weak {s_name}() 签名不匹配 — {diff}",
                'file': s_path,
                'line': s_line,
            })

        # 检查接收方文件
        r_path = find_file(source_dir, r_file)
        if r_path is None:
            violations.append({
                'pair': pair_id,
                'type': 'receiver_file_missing',
                'desc': f"Pair {pair_id} ({desc}): 接收方文件 '{r_file}' 不存在",
            })
            continue

        r_func = find_function_in_file(r_path, r_name, is_weak=False)
        if r_func is None:
            violations.append({
                'pair': pair_id,
                'type': 'receiver_strong_missing',
                'desc': f"Pair {pair_id} ({desc}): {r_file} 中未找到强符号 {r_name}() 实现",
                'file': r_path,
            })
            continue

        r_actual_ret, r_actual_params, r_line = r_func

        ok, diff = compare_signatures((r_ret, r_params), (r_actual_ret, r_actual_params))
        if not ok:
            violations.append({
                'pair': pair_id,
                'type': 'receiver_signature_mismatch',
                'desc': f"Pair {pair_id} ({desc}): {r_file}:{r_line} {r_name}() 签名不匹配 — {diff}",
                'file': r_path,
                'line': r_line,
            })

        # 收发双方签名一致性
        ok, diff = compare_signatures((s_ret, s_params), (r_ret, r_params))
        if not ok:
            violations.append({
                'pair': pair_id,
                'type': 'cross_signature_mismatch',
                'desc': f"Pair {pair_id} ({desc}): 收发双方签名不匹配 — {diff}",
            })

    return violations


# ============================================================
# 模式 B: 孤儿扫描
# ============================================================

def scan_orphans(source_dir, known_pairs, config):
    """扫描所有 .c/.h 文件中的 __weak 声明和强符号回调，检测孤儿。"""
    violations = []

    known_sender_funcs = set(p[4] for p in known_pairs)
    known_receiver_funcs = set(p[8] for p in known_pairs)

    # 编译公共 API 排除模式
    exclude_patterns = []
    for pat in config.get('public_api_excludes', []):
        try:
            exclude_patterns.append(re.compile('^' + pat + '$' if '$' not in pat else pat))
        except re.error:
            pass

    cb_pattern_str = config.get('callback_name_pattern', r'^\w+_On\w+$')
    try:
        callback_name_re = re.compile(cb_pattern_str)
    except re.error:
        callback_name_re = re.compile(r'^\w+_On\w+$')

    scan_dirs = config.get('scan_dirs', ['app', 'drv', 'hal', 'proto', 'core', 'cfg'])

    all_files = []
    for scan_dir in scan_dirs:
        dir_path = os.path.join(source_dir, scan_dir)
        if not os.path.isdir(dir_path):
            continue
        for root, dirs, files in os.walk(dir_path):
            for f in files:
                if f.endswith('.c') or f.endswith('.h'):
                    all_files.append(os.path.join(root, f))

    found_weaks = []      # (file, func_name, return_type, params, line, fullpath)
    found_callbacks = []  # (file, func_name, return_type, params, line, fullpath)

    for cfile in all_files:
        # 跳过 interface_map.h (纯文档)
        if os.path.basename(cfile) == 'interface_map.h':
            continue

        try:
            with open(cfile, 'r', encoding='utf-8', errors='ignore') as f:
                for lineno, line in enumerate(f, 1):
                    wm = WEAK_DECL_H_RE.search(line)
                    if wm:
                        fname = wm.group(2)
                        if '_On' in fname or callback_name_re.match(fname):
                            found_weaks.append(
                                (os.path.basename(cfile), fname,
                                 wm.group(1), wm.group(3), lineno, cfile))

                    # 检查 .c 中的强符号回调定义
                    if cfile.endswith('.c'):
                        sm = re.match(
                            r'^(?!.*(?:WEAK|__weak|extern|typedef))\s*'
                            r'(?:inline\s+)?(?:static\s+)?'
                            r'(\w+(?:\s*\*)?)\s+'       # return type
                            r'(\w+)\s*[(]'                # function name
                            r'([^)]*)[)]',                 # params
                            line.strip()
                        )
                        if sm:
                            fname = sm.group(2)
                            if callback_name_re.match(fname):
                                # 排除公共 API
                                excluded = False
                                for pat in exclude_patterns:
                                    if pat.match(fname):
                                        excluded = True
                                        break
                                if not excluded:
                                    if 'WEAK' not in line and '__weak' not in line and '__attribute__((weak))' not in line:
                                        found_callbacks.append(
                                            (os.path.basename(cfile), fname,
                                             sm.group(1), sm.group(3), lineno, cfile))
        except Exception:
            continue

    # 检测孤儿 __weak
    for wf, wn, wr, wp, wl, wpath in found_weaks:
        if wn not in known_sender_funcs:
            violations.append({
                'pair': 'ORPHAN',
                'type': 'orphan_weak',
                'desc': f"孤儿 __weak: {wf}:{wl} WEAK {wn}() — interface_map.h 中未记录此发送方",
                'file': wpath,
                'line': wl,
            })

    # 检测孤儿强符号
    weak_names = set(w[1] for w in found_weaks)
    for cf, cn, cr, cp, cl, cpath in found_callbacks:
        if cn not in known_receiver_funcs:
            if cn not in weak_names:
                violations.append({
                    'pair': 'ORPHAN',
                    'type': 'orphan_strong',
                    'desc': f"孤儿强符号: {cf}:{cl} {cn}() — 无对应 __weak 声明且 interface_map.h 中未记录",
                    'file': cpath,
                    'line': cl,
                })

    return violations


# ============================================================
# --print-template
# ============================================================

def print_template():
    template = {
        "_comment": "__weak 回调配对检查配置",
        "interface_map": "core/interface_map.h",
        "scan_dirs": ["app", "drv", "hal", "proto", "core", "cfg"],
        "public_api_excludes": [
            "engine_.*",
            ".*_OnTick",
            ".*_OnSecondTick",
            ".*_OnInit",
            ".*_Inject"
        ],
        "callback_name_pattern": "^\\w+_On\\w+$",
    }
    print(json.dumps(template, indent=2, ensure_ascii=False))


# ============================================================
# --self-test
# ============================================================

def run_self_test():
    errors = []

    def t(name, condition_fn):
        try:
            ok = condition_fn()
            if ok:
                print(f"  [PASS] {name}")
            else:
                errors.append(name)
                print(f"  [FAIL] {name}")
        except Exception as e:
            errors.append(name)
            print(f"  [FAIL] {name}: {e}")

    # 1) 内置预设结构正确
    def test_builtin_structure():
        for name in ['four_head', 'm4-ekf']:
            p = BUILTIN_PRESETS[name]
            assert 'interface_map' in p
            assert 'scan_dirs' in p
            assert 'public_api_excludes' in p
            assert isinstance(p['scan_dirs'], list)
            assert len(p['scan_dirs']) >= 3
        return True
    t("内置预设结构 (four_head + m4-ekf 配置完整)", test_builtin_structure)

    # 2) JSON 序列化/反序列化
    def test_json_roundtrip():
        cfg = BUILTIN_PRESETS['four_head']
        s = json.dumps(cfg, ensure_ascii=False)
        parsed = json.loads(s)
        assert parsed == cfg
        return True
    t("JSON 序列化/反序列化", test_json_roundtrip)

    # 3) normalize_params
    def test_normalize_params():
        assert normalize_params('uint16_t param, void *data_ptr') == 'uint16_t, void*'
        assert normalize_params('void') == 'void'
        assert normalize_params('') == 'void'
        assert normalize_params('int a, float b, char *c') == 'int, float, char*'
        return True
    t("normalize_params 规范化", test_normalize_params)

    # 4) compare_signatures
    def test_compare_signatures():
        ok, _ = compare_signatures(('void', 'uint16_t p'), ('void', 'uint16_t param'))
        assert ok
        ok, diff = compare_signatures(('void', 'uint16_t p'), ('int', 'uint16_t p'))
        assert not ok
        assert 'void' in diff
        return True
    t("compare_signatures (类型匹配+类型不匹配)", test_compare_signatures)

    # 5) parse_interface_map
    def test_parse_interface_map():
        test_map = os.path.join(tempfile.gettempdir(), 'test_interface_map.h')
        with open(test_map, 'w', encoding='utf-8') as f:
            f.write('/* Pair A: test pair */\n')
            f.write(' * 发送方: sender.c  WEAK void Target_OnEvent(uint16_t p, void *d) {}\n')
            f.write(' * 接收方: target.c  void Target_OnEvent(uint16_t p, void *d)\n')
        try:
            pairs = parse_interface_map(test_map)
            assert len(pairs) == 1
            assert pairs[0][0] == 'A'
            assert pairs[0][4] == 'Target_OnEvent'
            assert pairs[0][8] == 'Target_OnEvent'
        finally:
            os.unlink(test_map)
        return True
    t("parse_interface_map (配对格式解析)", test_parse_interface_map)

    # 6) parse_interface_map 文件不存在
    def test_parse_interface_map_missing():
        pairs = parse_interface_map('/nonexistent/path.h')
        assert pairs == []
        return True
    t("parse_interface_map (文件不存在→空列表)", test_parse_interface_map_missing)

    # 7) WEAK_DECL_H_RE
    def test_weak_decl_re():
        m = WEAK_DECL_H_RE.search('WEAK void Func(uint16_t p, void *d);')
        assert m is not None
        assert m.group(2) == 'Func'
        m = WEAK_DECL_H_RE.search('__weak int Handler(int x);')
        assert m is not None
        assert m.group(2) == 'Handler'
        return True
    t("WEAK_DECL_H_RE (WEAK + __weak)", test_weak_decl_re)

    # 8) callback_name_re 可配置
    def test_callback_re_config():
        re1 = re.compile(r'^\w+_On\w+$')
        assert re1.match('AppHmi_OnKey')
        assert re1.match('DrvLed_OnRefresh')
        assert not re1.match('engine_tick')
        assert re1.match('EKF_OnUpdate')
        return True
    t("callback_name_pattern 可配置", test_callback_re_config)

    # 9) config 文件加载
    def test_config_load():
        with tempfile.NamedTemporaryFile(
            mode='w', suffix='.json', delete=False, encoding='utf-8'
        ) as f:
            json.dump(BUILTIN_PRESETS['m4-ekf'], f)
            tmp_path = f.name
        try:
            loaded = load_config_file(tmp_path)
            assert loaded['interface_map'] == 'core/interface_map.h'
            assert 'scan_dirs' in loaded
        finally:
            os.unlink(tmp_path)
        return True
    t("JSON 配置文件加载", test_config_load)

    # 10) 强符号检测排除 WEAK 前缀
    def test_strong_excludes_weak():
        with tempfile.NamedTemporaryFile(
            mode='w', suffix='.c', delete=False, encoding='utf-8'
        ) as f:
            f.write('WEAK void Test_OnEvent(void) {}\n')
            f.write('void Test_OnEvent(void)    {}\n')
            tmp_path = f.name
        try:
            r = _find_strong_in_content(tmp_path, 'Test_OnEvent')
            assert r is not None
            assert r[2] == 2  # 第2行
        finally:
            os.unlink(tmp_path)
        return True
    t("强符号检测排除 WEAK 行", test_strong_excludes_weak)

    # 11) _find_weak 跨行匹配
    def test_weak_multiline():
        with tempfile.NamedTemporaryFile(
            mode='w', suffix='.h', delete=False, encoding='utf-8'
        ) as f:
            f.write('WEAK\n')
            f.write('void MultiLine_OnFire(uint8_t x);\n')
            tmp_path = f.name
        try:
            r = _find_weak_in_content(tmp_path, 'MultiLine_OnFire')
            assert r is not None
            assert r[0] == 'void'
        finally:
            os.unlink(tmp_path)
        return True
    t("_find_weak 跨行匹配", test_weak_multiline)

    if errors:
        print(f"\n[FAIL] --self-test: {len(errors)}/11 项失败")
        sys.exit(1)

    print(f"\n[PASS] --self-test 全部通过 (11/11)")
    return True


# ============================================================
# 入口
# ============================================================

def main():
    if '--self-test' in sys.argv:
        print("check_weak_pairs.py --self-test")
        run_self_test()
        sys.exit(0)

    if '--print-template' in sys.argv:
        print_template()
        sys.exit(0)

    args = sys.argv[1:]
    preset_name = None
    config_path = None
    project_dir = None

    i = 0
    while i < len(args):
        if args[i] == '--project' and i + 1 < len(args):
            preset_name = args[i + 1]
            i += 2
        elif args[i] == '--config' and i + 1 < len(args):
            config_path = args[i + 1]
            i += 2
        elif args[i] in ('--self-test', '--print-template'):
            i += 1
        elif not args[i].startswith('--'):
            project_dir = args[i]
            i += 1
        else:
            i += 1

    if project_dir is None:
        print("用法: python check_weak_pairs.py <project_dir> [--project <name>] [--config <file>]")
        print("      python check_weak_pairs.py --print-template")
        print("      python check_weak_pairs.py --self-test")
        print()
        print("内置预设: four_head, m4-ekf")
        print("示例:")
        print("  python check_weak_pairs.py ../src --project four_head")
        print("  python check_weak_pairs.py . --config deps_config.json")
        sys.exit(2)

    project_dir = os.path.abspath(project_dir)
    if not os.path.isdir(project_dir):
        print(f"错误: 目录不存在: {project_dir}")
        sys.exit(2)

    config, source = resolve_config(project_dir, preset_name, config_path)

    # 解析 interface_map.h 路径
    imap_path = os.path.join(project_dir, config['interface_map'])
    if not os.path.exists(imap_path):
        # 尝试在 project_dir 内搜索
        for root, dirs, files in os.walk(project_dir):
            if 'interface_map.h' in files:
                imap_path = os.path.join(root, 'interface_map.h')
                break

    print(f"check_weak_pairs.py — __weak 回调配对一致性检查")
    print(f"项目目录: {project_dir}")
    print(f"规则来源: {source}")

    violations = []

    if os.path.exists(imap_path):
        print(f"接口映射表: {imap_path}")
        print()

        pairs = parse_interface_map(imap_path)
        if pairs:
            print(f"从 interface_map.h 解析到 {len(pairs)} 对 __weak 配对")
            for p in pairs:
                print(f"  Pair {p[0]}: {p[1]} — {p[4]}() [{p[2]} → {p[6]}]")

            v1 = verify_from_interface_map(pairs, project_dir)
            violations.extend(v1)
        else:
            print("  (未解析到任何配对，仅做孤儿扫描)")
            pairs = []

        v2 = scan_orphans(project_dir, pairs, config)
        violations.extend(v2)
    else:
        print(f"接口映射表: (未找到: {config['interface_map']})")
        print()
        violations = scan_orphans(project_dir, [], config)

    print()

    if not violations:
        print("[PASS] 全部通过 — __weak 配对一致，0 违规")
        sys.exit(0)

    by_type = defaultdict(list)
    for v in violations:
        by_type[v['type']].append(v)

    print(f"[FAIL] 发现 {len(violations)} 项违规:")
    print()

    type_labels = {
        'sender_file_missing': '发送方文件不存在',
        'sender_weak_missing': '发送方缺少 __weak 声明',
        'sender_signature_mismatch': '发送方签名不匹配',
        'receiver_file_missing': '接收方文件不存在',
        'receiver_strong_missing': '接收方缺少强符号实现',
        'receiver_signature_mismatch': '接收方签名不匹配',
        'cross_signature_mismatch': '收发双方签名不一致',
        'orphan_weak': '孤儿 __weak 声明 (interface_map.h 未记录)',
        'orphan_strong': '孤儿强符号 (无对应 __weak)',
    }

    for vtype, label in type_labels.items():
        if vtype in by_type:
            print(f"  [{label}] — {len(by_type[vtype])} 项:")
            for v in by_type[vtype]:
                print(f"    {v['desc']}")
            print()

    print("修复指引:")
    print("  → 新增配对: 在 interface_map.h 中注册，发送方加 __weak 空壳，接收方加强实现")
    print("  → 签名不匹配: 检查 interface_map.h 记录与实际代码的参数类型/顺序")
    print("  → 孤儿: 要么补充 interface_map.h 记录，要么删除无用的声明/实现")
    print()
    sys.exit(1)


if __name__ == '__main__':
    main()
