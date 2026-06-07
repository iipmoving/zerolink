#!/usr/bin/env python3
"""
check_deps.py — 层依赖规则合规检查 (多项目可配置)

扫描项目目录下所有 .c 和 .h 文件的 #include 语句,
验证是否符合模块分层架构的依赖规则.

用法:
  python check_deps.py <project_dir>                            自动检测
  python check_deps.py <project_dir> --project four_head        内置预设
  python check_deps.py <project_dir> --project m4-ekf           内置预设
  python check_deps.py <project_dir> --config deps_config.json  自定义配置
  python check_deps.py --print-template                         输出 JSON 模板
  python check_deps.py --self-test                              自检

退出码: 0=通过, 1=发现违规, 2=用法错误
"""

import sys
import os
import re
import json
import tempfile

# ============================================================
# 内置预设
# ============================================================
BUILTIN_PRESETS = {
    'four_head': {
        'layer_dirs': ['app', 'drv', 'hal', 'proto', 'core', 'cfg'],
        'scan_dirs': ['app', 'drv', 'hal', 'proto', 'core', 'cfg', 'src'],
        'weak_whitelist': [],  # APP→APP __weak 例外白名单
        'rules': {
            'app': {
                'allowed': ['app/', 'core/', 'proto/', 'cfg/', '<'],
                'forbidden': ['drv/', 'hal/'],
            },
            'drv': {
                'allowed': ['drv/', 'core/', 'hal/', 'cfg/', '<'],
                'forbidden': ['app/', 'proto/'],
            },
            'hal': {
                'allowed': [
                    'hal/', '<',
                    'sc32f1xxx_', 'sc32L14xx', 'system_',
                    'lib/', 'SMG_Disp_', 'SC_TK_Scan', 'TKDriver',
                ],
                'forbidden': ['core/', 'app/', 'drv/', 'proto/', 'cfg/'],
            },
            'proto': {
                'allowed': ['proto/', 'core/', '<'],
                'forbidden': ['app/', 'drv/', 'hal/', 'cfg/'],
            },
            'core': {
                'allowed': ['core/', '<'],
                'forbidden': ['app/', 'drv/', 'hal/', 'proto/', 'cfg/'],
            },
            'cfg': {
                'allowed': ['cfg/', 'app/', '<'],
                'forbidden': ['drv/', 'hal/', 'proto/', 'core/'],
            },
        },
    },
    'm4-ekf': {
        'layer_dirs': ['app', 'base_class', 'proto', 'core'],
        'scan_dirs': ['app', 'base_class', 'proto', 'core', 'src'],
        'weak_whitelist': [],  # APP→APP __weak 例外白名单
        'rules': {
            'app': {
                'allowed': ['app/', 'core/', 'proto/', '<'],
                'forbidden': ['base_class/', 'src/'],
            },
            'base_class': {
                'allowed': ['base_class/', 'core/', '<'],
                'forbidden': ['app/', 'proto/'],
            },
            'proto': {
                'allowed': ['proto/', 'core/', '<'],
                'forbidden': ['app/', 'base_class/'],
            },
            'core': {
                'allowed': ['core/', '<'],
                'forbidden': ['app/', 'base_class/', 'proto/'],
            },
        },
    },
}

INCLUDE_RE = re.compile(r'^\s*#include\s+[<"]([^>"]+)[>"]')
EXTERN_RE = re.compile(r'^\s*extern\s+')

SYSTEM_HEADERS = {
    'stdint.h', 'stddef.h', 'stdarg.h', 'stdbool.h', 'string.h',
    'stdio.h', 'stdlib.h', 'math.h', 'assert.h', 'limits.h',
    'ctype.h', 'errno.h', 'float.h', 'inttypes.h',
}


def load_config_file(path):
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)


def resolve_rules(project_dir, preset_name, config_path):
    if config_path:
        return load_config_file(config_path), 'config:' + config_path
    if preset_name:
        if preset_name not in BUILTIN_PRESETS:
            print(f"错误: 未知预设 '{preset_name}'.")
            print(f"可用: {list(BUILTIN_PRESETS.keys())}")
            sys.exit(2)
        return BUILTIN_PRESETS[preset_name], 'preset:' + preset_name

    # 尝试从 project_dir/deps_config.json 自动加载
    local_config = os.path.join(project_dir, 'deps_config.json')
    if os.path.isfile(local_config):
        return load_config_file(local_config), 'file:deps_config.json'

    # 自动检测
    if os.path.isdir(os.path.join(project_dir, 'drv')):
        return BUILTIN_PRESETS['four_head'], 'auto:four_head'
    if os.path.isdir(os.path.join(project_dir, 'base_class')):
        return BUILTIN_PRESETS['m4-ekf'], 'auto:m4-ekf'

    print("错误: 无法自动检测项目类型, 请使用 --project 或 --config")
    print(f"可用预设: {list(BUILTIN_PRESETS.keys())}")
    sys.exit(2)


def classify_file(relpath, layer_dirs):
    """根据路径判断文件属于哪个层。"""
    # 规范化路径
    parts = relpath.replace('\\', '/').split('/')
    for layer in layer_dirs:
        try:
            idx = parts.index(layer)
        except ValueError:
            continue
        # 层目录必须是顶层 (idx==0) 或在 src/ 下 (idx==1 且 parts[0]=='src')
        if idx == 0:
            return layer
        if idx == 1 and parts[0] in ('src',):
            return layer
    return None


def parse_includes(filepath):
    includes = []
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            for line in f:
                m = INCLUDE_RE.match(line)
                if m:
                    includes.append(m.group(1))
    except Exception:
        pass
    return includes


def normalize_include(inc_path):
    while inc_path.startswith('../'):
        inc_path = inc_path[3:]
    if inc_path.startswith('./'):
        inc_path = inc_path[2:]
    return inc_path


WEAK_APP_RE = re.compile(r'__attribute__\(\(weak\)\)\s+(void|uint8_t|uint16_t|int8_t|int16_t)\s+(\w+)')
APP_PREFIXES = ('App', 'Proto')

def check_weak_callbacks(project_dir, whitelist):
    """检查 APP→APP __weak 输出回调，非白名单则违规。"""
    violations = []
    app_dir = os.path.join(project_dir, 'app')
    if not os.path.isdir(app_dir):
        return violations

    for fname in sorted(os.listdir(app_dir)):
        if not fname.endswith('.c'):
            continue
        fpath = os.path.join(app_dir, fname)
        with open(fpath, 'r', encoding='utf-8', errors='ignore') as f:
            for line_no, line in enumerate(f, 1):
                m = WEAK_APP_RE.search(line)
                if not m:
                    continue
                func_name = m.group(2)
                # 检查接收方是否在 APP 层（函数名以 App/Proto 开头）
                is_app_dest = any(func_name.startswith(p) for p in APP_PREFIXES)
                if not is_app_dest:
                    continue  # DRV/HAL 方向合法
                if func_name in whitelist:
                    continue  # 白名单例外
                violations.append({
                    'file': f'app/{fname}:{line_no}',
                    'func': func_name,
                    'desc': f'APP→APP __weak 输出回调 ({func_name})，应改 g_output+ST_OUT'
                })
    return violations


def is_system_header(inc_path):
    if inc_path.startswith('<'):
        return True
    name = inc_path.strip('"')
    return name in SYSTEM_HEADERS


def check_include(inc_path, layer, rules, relpath):
    """检查单个 include 是否合规。返回 (is_ok, violation_desc)。"""
    norm = normalize_include(inc_path)
    rule = rules[layer]

    if is_system_header(inc_path):
        return True, None

    # 同目录 include (不含路径分隔符) — 始终允许
    if '/' not in norm:
        return True, None

    for pat in rule['forbidden']:
        if norm.startswith(pat):
            return False, f"禁止 include '{pat}' 层: {inc_path}"

    for pat in rule['allowed']:
        if norm.startswith(pat):
            return True, None

    return False, f"不在 {layer} 层允许列表中: {inc_path}"


def check_extern(filepath, relpath):
    violations = []
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            for lineno, line in enumerate(f, 1):
                stripped = line.strip()
                if stripped.startswith('//') or stripped.startswith('/*'):
                    continue
                if EXTERN_RE.match(line):
                    violations.append({
                        'file': relpath,
                        'line': lineno,
                        'text': line.strip(),
                        'desc': '.c 文件禁止 extern — 跨模块引用破坏零耦合, 改用 __weak 回调',
                    })
    except Exception:
        pass
    return violations


def scan_directory(project_dir, rules):
    """扫描项目目录下所有源文件。"""
    violations = []
    extern_violations = []
    file_count = 0

    layer_dirs = rules['layer_dirs']
    layer_rules = rules['rules']
    scan_dirs = rules.get('scan_dirs', layer_dirs + ['src'])

    for scan_dir in scan_dirs:
        dir_path = os.path.join(project_dir, scan_dir)
        if not os.path.isdir(dir_path):
            continue

        for root, dirs, files in os.walk(dir_path):
            for fname in files:
                if not (fname.endswith('.c') or fname.endswith('.h')):
                    continue

                filepath = os.path.join(root, fname)
                relpath = os.path.relpath(filepath, project_dir).replace('\\', '/')

                layer = classify_file(relpath, layer_dirs)
                if layer is None or layer not in layer_rules:
                    continue

                file_count += 1

                if fname.endswith('.c'):
                    extern_violations.extend(check_extern(filepath, relpath))

                for inc in parse_includes(filepath):
                    ok, desc = check_include(inc, layer, layer_rules, relpath)
                    if not ok:
                        violations.append({
                            'file': relpath,
                            'layer': layer,
                            'include': inc,
                            'desc': desc,
                        })

    return file_count, violations, extern_violations


# ============================================================
# --print-template
# ============================================================
def print_template():
    template = {
        "_comment": "层依赖规则配置 — 定义项目各层的 #include 白名单/黑名单",
        "layer_dirs": ["app", "drv", "hal", "proto", "core"],
        "scan_dirs": ["app", "drv", "hal", "proto", "core", "src"],
        "rules": {
            "app": {
                "allowed": ["app/", "core/", "proto/", "<"],
                "forbidden": ["drv/", "hal/"],
            },
            "drv": {
                "allowed": ["drv/", "core/", "hal/", "<"],
                "forbidden": ["app/", "proto/"],
            },
            "hal": {
                "allowed": ["hal/", "<"],
                "forbidden": ["core/", "app/", "drv/", "proto/"],
            },
            "proto": {
                "allowed": ["proto/", "core/", "<"],
                "forbidden": ["app/", "drv/", "hal/"],
            },
            "core": {
                "allowed": ["core/", "<"],
                "forbidden": ["app/", "drv/", "hal/", "proto/"],
            },
        },
    }
    print(json.dumps(template, indent=2, ensure_ascii=False))


# ============================================================
# --self-test
# ============================================================
def run_self_test():
    """自检: 验证 JSON 解析 + 分类 + 违规检测 + 模板生成。"""
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
            assert 'layer_dirs' in p
            assert 'rules' in p
            assert len(p['layer_dirs']) >= 3
            for layer in p['layer_dirs']:
                assert layer in p['rules'], f"{name}: {layer} 缺少规则"
        return True
    t("内置预设结构 (four_head + m4-ekf 规则完整)", test_builtin_structure)

    # 2) JSON 序列化/反序列化
    def test_json_roundtrip():
        cfg = BUILTIN_PRESETS['four_head']
        s = json.dumps(cfg, ensure_ascii=False)
        parsed = json.loads(s)
        assert parsed == cfg
        return True
    t("JSON 序列化/反序列化", test_json_roundtrip)

    # 3) 模板生成包含必要字段
    def test_template_fields():
        tmpl = json.dumps(BUILTIN_PRESETS['four_head'], indent=2)
        assert '"layer_dirs"' in tmpl
        assert '"rules"' in tmpl
        assert '"app"' in tmpl
        return True
    t("模板包含必要字段", test_template_fields)

    # 4) 文件分类
    def test_classify():
        ld = ['app', 'drv', 'hal', 'proto', 'core', 'cfg']
        assert classify_file('app/app_hmi.c', ld) == 'app'
        assert classify_file('drv/drv_key.c', ld) == 'drv'
        assert classify_file('hal/hal_gpio.c', ld) == 'hal'
        assert classify_file('proto/proto_modbus.c', ld) == 'proto'
        assert classify_file('core/msg_scheduler.c', ld) == 'core'
        assert classify_file('cfg/hmi_data.c', ld) == 'cfg'
        assert classify_file('vendor/lib/foo.h', ld) is None
        assert classify_file('src/main.c', ld) is None
        return True
    t("文件分类 (6层 + 未知)", test_classify)

    # 5) m4-ekf 分类
    def test_classify_m4():
        ld = ['app', 'base_class', 'proto', 'core']
        assert classify_file('app/app_ekf.c', ld) == 'app'
        assert classify_file('base_class/bc_drv.c', ld) == 'base_class'
        assert classify_file('src/vendor/foo.h', ld) is None
        return True
    t("m4-ekf 文件分类", test_classify_m4)

    # 6) 路径规范化
    def test_normalize():
        assert normalize_include('../drv/foo.h') == 'drv/foo.h'
        assert normalize_include('./foo.h') == 'foo.h'
        assert normalize_include('app/foo.h') == 'app/foo.h'
        assert normalize_include('../../../hal/bar.h') == 'hal/bar.h'
        return True
    t("路径规范化 (../ ./ 多层)", test_normalize)

    # 7) 系统头文件识别
    def test_system_header():
        assert is_system_header('<stdint.h>')
        assert is_system_header('"stdint.h"')
        assert is_system_header('"stddef.h"')
        assert not is_system_header('"app_hmi.h"')
        assert not is_system_header('"drv/drv_led.h"')
        return True
    t("系统头文件识别", test_system_header)

    # 8) Include 违规判定 (four_head)
    def test_violations_fh():
        r = BUILTIN_PRESETS['four_head']['rules']
        ok, _ = check_include('drv/drv_led.h', 'app', r, 'app/app_hmi.c')
        assert not ok, "app should NOT include drv/"
        ok, _ = check_include('hal/hal_gpio.h', 'app', r, 'app/app_hmi.c')
        assert not ok, "app should NOT include hal/"
        ok, _ = check_include('core/msg_scheduler.h', 'app', r, 'app/app_hmi.c')
        assert ok, "app CAN include core/"
        ok, _ = check_include('app/power_ctrl.h', 'drv', r, 'drv/drv_key.c')
        assert not ok, "drv should NOT include app/"
        ok, _ = check_include('hal/hal_gpio.h', 'drv', r, 'drv/drv_key.c')
        assert ok, "drv CAN include hal/"
        ok, _ = check_include('core/msg_scheduler.h', 'hal', r, 'hal/hal_gpio.c')
        assert not ok, "hal should NOT include core/"
        return True
    t("Include 违规判定 (four_head 6项)", test_violations_fh)

    # 9) Include 违规判定 (m4-ekf)
    def test_violations_m4():
        r = BUILTIN_PRESETS['m4-ekf']['rules']
        ok, _ = check_include('base_class/bc_drv.h', 'app', r, 'app/app_ekf.c')
        assert not ok, "app should NOT include base_class/"
        ok, _ = check_include('core/msg_scheduler.h', 'app', r, 'app/app_ekf.c')
        assert ok, "app CAN include core/"
        ok, _ = check_include('app/app_hmi.h', 'base_class', r, 'base_class/bc.c')
        assert not ok, "base_class should NOT include app/"
        return True
    t("Include 违规判定 (m4-ekf 3项)", test_violations_m4)

    # 10) Config 文件加载
    def test_config_load():
        with tempfile.NamedTemporaryFile(
            mode='w', suffix='.json', delete=False, encoding='utf-8'
        ) as f:
            json.dump(BUILTIN_PRESETS['m4-ekf'], f)
            tmp_path = f.name
        try:
            loaded = load_config_file(tmp_path)
            assert loaded['layer_dirs'] == ['app', 'base_class', 'proto', 'core']
            assert 'app' in loaded['rules']
        finally:
            os.unlink(tmp_path)
        return True
    t("JSON 配置文件加载", test_config_load)

    # 11) extern 检测
    def test_extern_check():
        import tempfile as tf2
        with tf2.NamedTemporaryFile(
            mode='w', suffix='.c', delete=False, encoding='utf-8'
        ) as f:
            f.write('extern int foo;\n')
            f.write('// extern int bar;\n')
            f.write('static int baz;\n')
            tmp_path = f.name
        try:
            v = check_extern(tmp_path, 'test.c')
            assert len(v) == 1
            assert v[0]['line'] == 1
        finally:
            os.unlink(tmp_path)
        return True
    t("extern 违规检测", test_extern_check)

    if errors:
        print(f"\n[FAIL] --self-test: {len(errors)}/{11} 项失败")
        sys.exit(1)

    print(f"\n[PASS] --self-test 全部通过 (11/11)")
    return True


# ============================================================
# 入口
# ============================================================
def main():
    if '--self-test' in sys.argv:
        print("check_deps.py --self-test")
        run_self_test()
        sys.exit(0)

    if '--print-template' in sys.argv:
        print_template()
        sys.exit(0)

    # 解析参数
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
        print("用法: python check_deps.py <project_dir> [--project <name>] [--config <file>]")
        print("      python check_deps.py --print-template")
        print("      python check_deps.py --self-test")
        print()
        print("内置预设: four_head, m4-ekf")
        print("示例:")
        print("  python check_deps.py ../src --project four_head")
        print("  python check_deps.py . --config deps_config.json")
        sys.exit(2)

    project_dir = os.path.abspath(project_dir)
    if not os.path.isdir(project_dir):
        print(f"错误: 目录不存在: {project_dir}")
        sys.exit(2)

    rules, source = resolve_rules(project_dir, preset_name, config_path)

    print(f"check_deps.py — 层依赖规则检查")
    print(f"项目目录: {project_dir}")
    print(f"规则来源: {source}")
    print()

    file_count, violations, extern_violations = scan_directory(project_dir, rules)
    weak_violations = check_weak_callbacks(project_dir, rules.get('weak_whitelist', []))
    total = len(violations) + len(extern_violations) + len(weak_violations)

    if total == 0:
        print(f"[PASS] 全部通过 — 扫描 {file_count} 个文件, 0 违规")
        sys.exit(0)

    print(f"[FAIL] 发现 {total} 项违规 (共扫描 {file_count} 个文件):")
    print()

    if violations:
        by_layer = {}
        for v in violations:
            by_layer.setdefault(v['layer'], []).append(v)

        for layer in rules.get('layer_dirs', []):
            if layer not in by_layer:
                continue
            print(f"  [{layer}/] — {len(by_layer[layer])} 项 include 违规:")
            for v in by_layer[layer]:
                print(f"    {v['file']}")
                print(f"      #include \"{v['include']}\"")
                print(f"      → {v['desc']}")
            print()

    if extern_violations:
        print(f"  [extern] — {len(extern_violations)} 项跨模块 extern 违规:")
        for v in extern_violations:
            print(f"    {v['file']}:{v['line']}")
            print(f"      {v['text']}")
            print(f"      → {v['desc']}")
        print()

    if weak_violations:
        print(f"  [weak] — {len(weak_violations)} 项 APP→APP __weak 输出回调违规:")
        for v in weak_violations:
            print(f"    {v['file']}")
            print(f"      → {v['desc']}")
        print()

    print("修复指引:")
    print("  include → 移除跨层引用, 改用消息/回调")
    print("  extern  → 改用 __weak 函数 (消费者写 weak 默认, 生产者强覆盖)")
    print("  weak    → 改用 g_output+ST_OUT, 由 Switcher 统一路由")
    print("           需要白名单例外 → 用户确认后加入 deps_config.json weak_whitelist")
    print()
    sys.exit(1)


if __name__ == '__main__':
    main()
