#!/usr/bin/env python3
"""
check_structs.py — 结构体一致性验证工具 (pre-commit hook)

验证生成文件与 structs.json 的一致性:
  1. structs.json 格式 + schema 校验
  2. serial 单调递增检查
  3. deprecated 字段引用检查
  4. 类型白名单检查
  5. generate_structs.py --check (生成对diff)

用法:
  python check_structs.py <project_dir>                            自动检测
  python check_structs.py <project_dir> --project four_head        内置预设
  python check_structs.py <project_dir> --config deps_config.json  自定义配置
  python check_structs.py --print-template                         输出 JSON 模板
  python check_structs.py --self-test                              自检

退出码: 0=通过, 1=发现违规, 2=用法错误
"""

import sys
import os
import re
import json
import subprocess
import tempfile


# ============================================================
# 内置预设 (与 generate_structs.py 保持一致)
# ============================================================
BUILTIN_PRESETS = {
    'four_head': {
        'project': 'four_head',
        'structs_source': 'cfg/structs.json',
        'module_search_paths': ['src/app', 'src/drv', 'src/proto', 'src/core', 'src/cfg', '.'],
        'type_whitelist': ['uint8_t', 'uint16_t', 'uint32_t', 'int8_t', 'int16_t', 'int32_t', 'char'],
    },
    'm4-ekf': {
        'project': 'm4-ekf',
        'structs_source': 'cfg/structs.json',
        'module_search_paths': ['app', 'base_class', 'proto', 'core', '.'],
        'type_whitelist': ['uint8_t', 'uint16_t', 'uint32_t', 'int8_t', 'int16_t', 'int32_t', 'char'],
    },
}

TYPE_SIZES = {
    'uint8_t': 1, 'uint16_t': 2, 'uint32_t': 4,
    'int8_t': 1, 'int16_t': 2, 'int32_t': 4,
    'char': 1, 'float': 4,
}


# ============================================================
# 配置加载
# ============================================================

def load_config_file(path):
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)


def resolve_config(project_dir, preset_name, config_path):
    if config_path:
        cfg = load_config_file(config_path)
        if 'structs_generation' in cfg:
            return cfg['structs_generation'], 'config:' + config_path
        if 'structs_source' in cfg:
            return cfg, 'config:' + config_path
        print("错误: 配置文件缺少 'structs_source' 字段")
        sys.exit(2)

    if preset_name:
        if preset_name not in BUILTIN_PRESETS:
            print(f"错误: 未知预设 '{preset_name}'.")
            print(f"可用: {list(BUILTIN_PRESETS.keys())}")
            sys.exit(2)
        return BUILTIN_PRESETS[preset_name], 'preset:' + preset_name

    deps_cfg = os.path.join(project_dir, 'deps_config.json')
    if os.path.isfile(deps_cfg):
        cfg = load_config_file(deps_cfg)
        if 'structs_generation' in cfg:
            return cfg['structs_generation'], 'file:deps_config.json#structs_generation'

    local_cfg = os.path.join(project_dir, 'structs_config.json')
    if os.path.isfile(local_cfg):
        return load_config_file(local_cfg), 'file:structs_config.json'

    if os.path.isdir(os.path.join(project_dir, 'drv')):
        return BUILTIN_PRESETS['four_head'], 'auto:four_head'
    if os.path.isdir(os.path.join(project_dir, 'base_class')):
        return BUILTIN_PRESETS['m4-ekf'], 'auto:m4-ekf'

    print("错误: 无法自动检测项目类型, 请使用 --project 或 --config")
    print(f"可用预设: {list(BUILTIN_PRESETS.keys())}")
    sys.exit(2)


# ============================================================
# structs.json 加载
# ============================================================

def load_structs_json(filepath):
    if not os.path.exists(filepath):
        print(f"错误: structs.json 不存在: {filepath}")
        sys.exit(2)
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            return json.load(f)
    except json.JSONDecodeError as e:
        print(f"错误: structs.json JSON 解析失败: {e}")
        sys.exit(2)


# ============================================================
# 检查项 1: JSON format + schema
# ============================================================

def check_json_schema(data, whitelist):
    """校验 structs.json schema。返回错误列表。"""
    errors = []

    if not isinstance(data, dict):
        return ['根元素必须是 JSON object']

    for key in ['version', 'serial', 'structs']:
        if key not in data:
            errors.append(f'缺少 "{key}" 字段')

    if 'serial' in data and not isinstance(data['serial'], int):
        errors.append('"serial" 必须是整数')

    if 'structs' not in data:
        return errors

    structs = data['structs']
    if not isinstance(structs, dict):
        errors.append('"structs" 必须是 object')
        return errors

    for skey, sdef in structs.items():
        if not isinstance(sdef, dict):
            errors.append(f'structs["{skey}"]: 必须是 object')
            continue

        for required in ['description', 'owner', 'suffix', 'fields']:
            if required not in sdef:
                errors.append(f'structs["{skey}"]: 缺少 "{required}"')

        if 'fields' in sdef and isinstance(sdef['fields'], list):
            field_names = set()
            for i, f in enumerate(sdef['fields']):
                if not isinstance(f, dict):
                    errors.append(f'structs["{skey}"].fields[{i}]: 必须是 object')
                    continue
                if 'name' not in f:
                    errors.append(f'structs["{skey}"].fields[{i}]: 缺少 "name"')
                else:
                    if f['name'] in field_names:
                        errors.append(
                            f'structs["{skey}"].fields[{i}]: '
                            f'重复字段名 "{f["name"]}"')
                    field_names.add(f['name'])
                if 'type' not in f:
                    errors.append(f'structs["{skey}"].fields[{i}]: 缺少 "type"')

        if 'consumers' in sdef and isinstance(sdef['consumers'], dict):
            for cname, cdef in sdef['consumers'].items():
                if not isinstance(cdef, dict):
                    errors.append(
                        f'structs["{skey}"].consumers["{cname}"]: 必须是 object')
                    continue
                if 'fields' not in cdef:
                    errors.append(
                        f'structs["{skey}"].consumers["{cname}"]: 缺少 "fields"')
                elif isinstance(cdef['fields'], list):
                    owner_fields = {f['name'] for f in sdef.get('fields', [])}
                    for cf in cdef['fields']:
                        if cf not in owner_fields:
                            errors.append(
                                f'structs["{skey}"].consumers["{cname}"].fields: '
                                f'"{cf}" 不在 owner fields 中')

        if 'deprecated' in sdef and isinstance(sdef['deprecated'], dict):
            owner_fields = {f['name'] for f in sdef.get('fields', [])}
            for dfname in sdef['deprecated']:
                if dfname not in owner_fields:
                    errors.append(
                        f'structs["{skey}"].deprecated["{dfname}"]: '
                        f'字段不在 owner fields 中')

    return errors


# ============================================================
# 检查项 2: serial 单调递增
# ============================================================

SERIAL_RE = re.compile(
    r'/\* === AUTO-GENERATED from structs\.json serial=(\d+) === \*/')


def check_serial_monotonic(project_dir, json_serial, search_paths):
    """扫描所有 types.h, 检查内嵌 serial <= json_serial。返回违规列表。"""
    violations = []

    for sp in search_paths:
        base = os.path.join(project_dir, sp)
        if not os.path.isdir(base):
            continue
        for root, dirs, files in os.walk(base):
            for fname in files:
                if fname != 'types.h':
                    continue
                filepath = os.path.join(root, fname)
                relpath = os.path.relpath(filepath, project_dir)
                try:
                    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                except Exception:
                    continue

                m = SERIAL_RE.search(content)
                if not m:
                    continue

                file_serial = int(m.group(1))
                if json_serial < file_serial:
                    violations.append({
                        'file': relpath,
                        'json_serial': json_serial,
                        'file_serial': file_serial,
                        'desc': (
                            f'serial 回退: JSON serial={json_serial} '
                            f'< 文件 serial={file_serial}'
                        ),
                    })

    return violations


# ============================================================
# 检查项 3: deprecated 字段引用
# ============================================================

def check_deprecated_refs(structs_data):
    """检查所有 consumer 是否引用了 deprecated 字段。返回违规列表。"""
    violations = []

    for skey, sdef in structs_data.get('structs', {}).items():
        deprecated = sdef.get('deprecated', {})
        if not deprecated:
            continue

        consumers = sdef.get('consumers', {})
        for cname, cdef in consumers.items():
            for cf in cdef.get('fields', []):
                if cf in deprecated:
                    dinfo = deprecated[cf]
                    violations.append({
                        'struct': skey,
                        'consumer': cname,
                        'field': cf,
                        'replaced_by': dinfo.get('replaced_by', '?'),
                        'desc': (
                            f'structs["{skey}"].consumers["{cname}"] '
                            f'引用了废弃字段 "{cf}" '
                            f'(替代: {dinfo.get("replaced_by", "?")})'
                        ),
                    })
    return violations


# ============================================================
# 检查项 4: 类型白名单
# ============================================================

def check_type_whitelist(structs_data, whitelist):
    """检查所有字段类型是否在白名单中。返回违规列表。"""
    violations = []

    for skey, sdef in structs_data.get('structs', {}).items():
        for f in sdef.get('fields', []):
            ftype = f.get('type', '')
            if ftype and ftype not in whitelist:
                violations.append({
                    'struct': skey,
                    'field': f.get('name', '?'),
                    'type': ftype,
                    'desc': (
                        f'structs["{skey}"].{f.get("name", "?")}: '
                        f'类型 "{ftype}" 不在白名单中'
                    ),
                })
    return violations


# ============================================================
# 检查项 5: 模块存在性
# ============================================================

def check_module_existence(structs_data, project_dir, search_paths):
    """检查 owner 和 consumer 模块目录是否存在。返回违规列表。"""
    violations = []
    all_modules = set()

    for skey, sdef in structs_data.get('structs', {}).items():
        all_modules.add(sdef.get('owner', ''))
        for cname in sdef.get('consumers', {}):
            all_modules.add(cname)

    for module_name in all_modules:
        if not module_name:
            continue
        found = False
        norm_name = module_name.lower().replace('-', '_').replace(' ', '_')
        for sp in search_paths:
            base = os.path.join(project_dir, sp)
            if not os.path.isdir(base):
                continue
            for entry in os.listdir(base):
                entry_path = os.path.join(base, entry)
                if not os.path.isdir(entry_path):
                    continue
                entry_norm = entry.lower().replace('-', '_').replace(' ', '_')
                if entry_norm == norm_name:
                    found = True
                    break
            if found:
                break

        if not found:
            violations.append({
                'module': module_name,
                'desc': f'模块 "{module_name}" 的目录不存在 (搜索路径: {search_paths})',
                'is_warning': True,
            })

    return violations


# ============================================================
# 检查项 6: 生成一致性 (调用 generate_structs.py --check)
# ============================================================

def run_generate_check(project_dir, config, json_path):
    """调用 generate_structs.py --check。返回 (exit_code, stdout, stderr)。"""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    gen_script = os.path.join(script_dir, 'generate_structs.py')

    if not os.path.exists(gen_script):
        return -1, '', f'generate_structs.py 未找到: {gen_script}'

    cmd = [
        sys.executable, gen_script,
        project_dir,
        '--source', json_path,
        '--check',
    ]

    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=30,
        )
        return result.returncode, result.stdout, result.stderr
    except subprocess.TimeoutExpired:
        return -1, '', 'generate_structs.py --check 超时 (30s)'
    except Exception as e:
        return -1, '', f'调用 generate_structs.py 失败: {e}'


# ============================================================
# --print-template
# ============================================================

def print_template():
    template = {
        "_comment": "结构体一致性验证配置 — 与 generate_structs.py 共用",
        "structs_source": "cfg/structs.json",
        "module_search_paths": ["src/app", "src/drv", "src/proto", "src/core", "."],
        "type_whitelist": [
            "uint8_t", "uint16_t", "uint32_t",
            "int8_t", "int16_t", "int32_t",
            "char"
        ],
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

    # 1) 内置预设结构
    def test_builtin_structure():
        for name in ['four_head', 'm4-ekf']:
            p = BUILTIN_PRESETS[name]
            assert 'structs_source' in p
            assert 'module_search_paths' in p
            assert 'type_whitelist' in p
        return True
    t("内置预设结构 (four_head + m4-ekf)", test_builtin_structure)

    # 2) JSON 序列化/反序列化
    def test_json_roundtrip():
        cfg = BUILTIN_PRESETS['four_head']
        s = json.dumps(cfg, ensure_ascii=False)
        parsed = json.loads(s)
        assert parsed == cfg
        return True
    t("JSON 序列化/反序列化", test_json_roundtrip)

    # 3) Schema 校验 — 正确数据
    def test_schema_valid():
        data = {
            "version": "1.0",
            "serial": 1,
            "structs": {
                "Test": {
                    "description": "test",
                    "owner": "test_mod",
                    "suffix": "OUT",
                    "fields": [
                        {"name": "a", "type": "uint8_t"},
                        {"name": "b", "type": "uint16_t"}
                    ]
                }
            }
        }
        errs = check_json_schema(data, BUILTIN_PRESETS['four_head']['type_whitelist'])
        assert errs == [], f"Unexpected errors: {errs}"
        return True
    t("Schema 校验 (正确 JSON)", test_schema_valid)

    # 4) Schema 校验 — 缺少字段
    def test_schema_invalid():
        data = {"version": "1.0"}
        errs = check_json_schema(data, BUILTIN_PRESETS['four_head']['type_whitelist'])
        assert len(errs) >= 2  # 缺少 serial + structs
        return True
    t("Schema 校验 (缺少 serial/structs)", test_schema_invalid)

    # 5) Schema 校验 — 重复字段名
    def test_schema_duplicate_fields():
        data = {
            "version": "1.0",
            "serial": 1,
            "structs": {
                "Test": {
                    "description": "test",
                    "owner": "mod",
                    "suffix": "OUT",
                    "fields": [
                        {"name": "a", "type": "uint8_t"},
                        {"name": "a", "type": "uint16_t"}
                    ]
                }
            }
        }
        errs = check_json_schema(data, BUILTIN_PRESETS['four_head']['type_whitelist'])
        assert len(errs) >= 1  # 重复字段名
        assert any('重复' in e for e in errs)
        return True
    t("Schema 校验 (重复字段名)", test_schema_duplicate_fields)

    # 6) Deprecated 引用检查
    def test_deprecated_check():
        data = {
            "version": "1.0",
            "serial": 1,
            "structs": {
                "Test": {
                    "description": "test",
                    "owner": "mod",
                    "suffix": "OUT",
                    "fields": [
                        {"name": "a", "type": "uint8_t"},
                        {"name": "old_field", "type": "uint16_t"}
                    ],
                    "consumers": {
                        "consumer_mod": {
                            "fields": ["a", "old_field"]
                        }
                    },
                    "deprecated": {
                        "old_field": {
                            "replaced_by": "new_field",
                            "deadline": "2026-12-31"
                        }
                    }
                }
            }
        }
        violations = check_deprecated_refs(data)
        assert len(violations) == 1
        assert violations[0]['field'] == 'old_field'
        return True
    t("Deprecated 引用检查 (consumer引用废弃字段)", test_deprecated_check)

    # 7) 类型白名单检查
    def test_type_whitelist_check():
        data = {
            "version": "1.0",
            "serial": 1,
            "structs": {
                "Test": {
                    "description": "test",
                    "owner": "mod",
                    "suffix": "OUT",
                    "fields": [
                        {"name": "a", "type": "uint8_t"},
                        {"name": "b", "type": "double"}
                    ]
                }
            }
        }
        whitelist = ['uint8_t', 'uint16_t', 'uint32_t']
        violations = check_type_whitelist(data, whitelist)
        assert len(violations) == 1
        assert 'double' in violations[0]['desc']
        return True
    t("类型白名单检查 (double不在列表)", test_type_whitelist_check)

    # 8) Serial 单调递增 — 正常
    def test_serial_ok():
        with tempfile.TemporaryDirectory() as tmpdir:
            mod_dir = os.path.join(tmpdir, 'src', 'app', 'test_mod')
            os.makedirs(mod_dir)
            types_h = os.path.join(mod_dir, 'types.h')
            with open(types_h, 'w') as f:
                f.write('/* === AUTO-GENERATED from structs.json serial=3 === */\n')
                f.write('typedef struct { int x; } Test_t;\n')
                f.write('/* === END AUTO-GENERATED === */\n')

            v = check_serial_monotonic(tmpdir, 5, ['src/app'])
            assert len(v) == 0  # JSON serial=5 >= file serial=3
        return True
    t("Serial 单调递增 (JSON >= 文件 → OK)", test_serial_ok)

    # 9) Serial 单调递增 — 回退
    def test_serial_fail():
        with tempfile.TemporaryDirectory() as tmpdir:
            mod_dir = os.path.join(tmpdir, 'src', 'app', 'test_mod')
            os.makedirs(mod_dir)
            types_h = os.path.join(mod_dir, 'types.h')
            with open(types_h, 'w') as f:
                f.write('/* === AUTO-GENERATED from structs.json serial=7 === */\n')
                f.write('typedef struct { int x; } Test_t;\n')
                f.write('/* === END AUTO-GENERATED === */\n')

            v = check_serial_monotonic(tmpdir, 5, ['src/app'])
            assert len(v) == 1  # JSON serial=5 < file serial=7
        return True
    t("Serial 单调递增 (JSON < 文件 → 违规)", test_serial_fail)

    # 10) 配置加载
    def test_config_load():
        with tempfile.NamedTemporaryFile(
            mode='w', suffix='.json', delete=False, encoding='utf-8'
        ) as f:
            json.dump(BUILTIN_PRESETS['m4-ekf'], f)
            tmp_path = f.name
        try:
            loaded = load_config_file(tmp_path)
            assert loaded['structs_source'] == 'cfg/structs.json'
        finally:
            os.unlink(tmp_path)
        return True
    t("JSON 配置文件加载", test_config_load)

    # 11) SERIAL_RE 正则
    def test_serial_re():
        m = SERIAL_RE.search(
            '/* === AUTO-GENERATED from structs.json serial=42 === */')
        assert m is not None
        assert int(m.group(1)) == 42
        m2 = SERIAL_RE.search('// some other comment')
        assert m2 is None
        return True
    t("SERIAL_RE 正则匹配", test_serial_re)

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
        print("check_structs.py --self-test")
        run_self_test()
        sys.exit(0)

    if '--print-template' in sys.argv:
        print_template()
        sys.exit(0)

    args = sys.argv[1:]
    preset_name = None
    config_path = None
    project_dir = None
    skip_generate_check = False

    i = 0
    while i < len(args):
        if args[i] == '--project' and i + 1 < len(args):
            preset_name = args[i + 1]
            i += 2
        elif args[i] == '--config' and i + 1 < len(args):
            config_path = args[i + 1]
            i += 2
        elif args[i] == '--skip-generate-check':
            skip_generate_check = True
            i += 1
        elif args[i] in ('--self-test', '--print-template'):
            i += 1
        elif not args[i].startswith('--'):
            project_dir = args[i]
            i += 1
        else:
            i += 1

    if project_dir is None:
        print("用法: python check_structs.py <project_dir> [选项]")
        print("      python check_structs.py --print-template")
        print("      python check_structs.py --self-test")
        print()
        print("选项:")
        print("  --project <name>          内置预设: four_head, m4-ekf")
        print("  --config <file>           自定义 JSON 配置")
        print("  --skip-generate-check     跳过 generate_structs.py --check")
        print()
        print("示例:")
        print("  python check_structs.py . --project four_head")
        print("  python check_structs.py . --config deps_config.json")
        sys.exit(2)

    project_dir = os.path.abspath(project_dir)
    if not os.path.isdir(project_dir):
        print(f"错误: 目录不存在: {project_dir}")
        sys.exit(2)

    config, cfg_source = resolve_config(project_dir, preset_name, config_path)

    json_path = os.path.join(project_dir,
                             config.get('structs_source', 'cfg/structs.json'))

    print(f"check_structs.py — 结构体一致性验证")
    print(f"项目目录: {project_dir}")
    print(f"规则来源: {cfg_source}")
    print(f"数据源:   {json_path}")
    print()

    # ---- 检查 1: JSON format + schema ----
    print("--- 1. structs.json 格式校验 ---")
    structs_data = load_structs_json(json_path)
    whitelist = config.get('type_whitelist',
                           ['uint8_t', 'uint16_t', 'uint32_t', 'int8_t',
                            'int16_t', 'int32_t', 'char'])
    schema_errors = check_json_schema(structs_data, whitelist)
    if schema_errors:
        print(f"  [FAIL] {len(schema_errors)} 项错误:")
        for e in schema_errors:
            print(f"    - {e}")
    else:
        print(f"  [PASS] schema 有效 (serial={structs_data.get('serial', '?')}, "
              f"{len(structs_data.get('structs', {}))} struct(s))")
    print()

    # ---- 检查 2: serial 单调递增 ----
    print("--- 2. serial 单调递增检查 ---")
    search_paths = config.get('module_search_paths', ['.'])
    serial_violations = check_serial_monotonic(
        project_dir, structs_data.get('serial', 0), search_paths)
    if serial_violations:
        print(f"  [FAIL] {len(serial_violations)} 项 serial 回退:")
        for v in serial_violations:
            print(f"    {v['file']}: JSON serial={v['json_serial']} < "
                  f"文件 serial={v['file_serial']}")
    else:
        print(f"  [PASS] 所有生成文件 serial <= JSON serial")
    print()

    # ---- 检查 3: deprecated 引用 ----
    print("--- 3. deprecated 字段引用检查 ---")
    dep_violations = check_deprecated_refs(structs_data)
    if dep_violations:
        print(f"  [FAIL] {len(dep_violations)} 项废弃字段引用:")
        for v in dep_violations:
            print(f"    {v['desc']}")
    else:
        print(f"  [PASS] 无 consumer 引用废弃字段")
    print()

    # ---- 检查 4: 类型白名单 ----
    print("--- 4. 类型白名单检查 ---")
    type_violations = check_type_whitelist(structs_data, whitelist)
    if type_violations:
        print(f"  [FAIL] {len(type_violations)} 项非白名单类型:")
        for v in type_violations:
            print(f"    {v['desc']}")
    else:
        print(f"  [PASS] 所有字段类型在白名单内")
    print()

    # ---- 检查 5: 模块存在性 (warning only) ----
    print("--- 5. 模块目录存在性 ---")
    module_violations = check_module_existence(structs_data, project_dir, search_paths)
    if module_violations:
        print(f"  [WARN] {len(module_violations)} 个模块目录未找到 (将在首次生成时创建):")
        for v in module_violations:
            print(f"    - {v['desc']}")
    else:
        print(f"  [PASS] 所有模块目录存在")
    print()

    # ---- 检查 6: 生成一致性 ----
    print("--- 6. 生成一致性 (generate_structs.py --check) ---")
    if skip_generate_check:
        print("  [SKIP] --skip-generate-check")
    else:
        rc, stdout, stderr = run_generate_check(project_dir, config, json_path)
        if stderr:
            print(stderr)
        if stdout:
            print(stdout)
        if rc == 0:
            print("  [PASS] 所有生成文件与 structs.json 一致")
        elif rc == 1:
            print("  [FAIL] 发现差异 — 请运行 generate_structs.py 重新生成")
        else:
            print(f"  [FAIL] generate_structs.py --check 执行异常 (rc={rc})")
    print()

    # ---- 汇总 ----
    total = len(schema_errors) + len(serial_violations) + len(dep_violations) \
        + len(type_violations) + len(module_violations)

    if schema_errors or serial_violations or dep_violations or type_violations:
        print(f"[FAIL] 发现 {total} 项问题, 请在提交前修复")
        sys.exit(1)

    # 如果 generate check 失败也需要报
    if not skip_generate_check:
        rc, _, _ = run_generate_check(project_dir, config, json_path)
        if rc != 0:
            print(f"[FAIL] 生成一致性检查未通过")
            sys.exit(1)

    print(f"[PASS] 全部通过 — 0 违规")
    sys.exit(0)


if __name__ == '__main__':
    main()
