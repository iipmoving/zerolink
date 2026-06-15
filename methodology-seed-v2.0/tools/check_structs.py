#!/usr/bin/env python3
"""
check_structs.py — 结构体一致性验证工具 (pre-commit hook)

支持两种模式:
  A. structs.json 模式 (V1.0): JSON schema + serial + deprecated + 白名单 + 生成一致性
  B. .h @STRUCT 模式 (V2.0): 解析 .h 接口结构体段, 双向维护一致性验证

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
    'char': 1, 'float': 4, 'bool': 1,
}


# ============================================================
# @STRUCT .h 模式 — 正则表达式
# ============================================================

# @STRUCT tag: /* @STRUCT Name  owner=mod  suffix=OUT  [source=OwnerStruct] */
STRUCT_TAG_RE = re.compile(
    r'@STRUCT\s+(\w+)\s+owner=(\S+)\s+suffix=(\w+)'
)
SOURCE_RE = re.compile(r'source=(\S+)')

# Field: type name[array]; /* offset=N, size=N [...comment...] */
FIELD_RE = re.compile(
    r'(\w+(?:\s*\*)?)\s+(\w+)(?:\[(\d+)\])?;\s*/\*\s*offset=(\d+),\s*size=(\d+)[^*]*\*/'
)

# sizeof comment: } Name; /* sizeof=N [...comment...] */
SIZEOF_RE = re.compile(r'\}\s*(\w+);\s*/\*\s*sizeof=(\d+)[^*]*\*/')

# Naming convention: } Prefix_Input_t / Prefix_Output_t / Prefix_Link_t
TYPEDEF_NAME_RE = re.compile(r'\}\s*(\w+?)_(Output_Link|Input|Output)\b')
SAME_AS_RE = re.compile(r'/\*\s*same as\s+\S+\s*\*/')

# Section markers
IFACE_BEGIN_RE = re.compile(r'INTERFACE STRUCTS')
IFACE_END_RE = re.compile(r'END INTERFACE STRUCTS')


# ============================================================
# .h @STRUCT 解析
# ============================================================

def parse_h_interface_structs(project_dir, search_paths):
    """解析所有 .h 文件中的 INTERFACE STRUCTS 段, 提取 @STRUCT 定义.

    返回: dict keyed by struct name, 每个 value:
        { owner, suffix, file, sizeof, fields: [{name, type, offset, size, count}] }
    """
    h_structs = {}

    for sp in search_paths:
        base = os.path.join(project_dir, sp)
        if not os.path.isdir(base):
            continue
        for root, dirs, files in os.walk(base):
            for fname in files:
                if not fname.endswith('.h'):
                    continue
                filepath = os.path.join(root, fname)
                relpath = os.path.relpath(filepath, project_dir)

                with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                    lines = f.readlines()

                _parse_one_h(lines, relpath, h_structs)

    return h_structs


def _parse_one_h(lines, relpath, h_structs):
    """从单个 .h 文件的各行中提取 @STRUCT 定义。"""
    in_section = False

    for idx, line in enumerate(lines):
        stripped = line.strip()

        # track INTERFACE STRUCTS section
        if IFACE_BEGIN_RE.search(stripped):
            in_section = True
            continue
        if IFACE_END_RE.search(stripped):
            in_section = False
            continue

        if not in_section:
            continue

        # match @STRUCT tag
        tag_m = STRUCT_TAG_RE.search(stripped)
        if not tag_m:
            continue

        struct_name = tag_m.group(1)
        owner = tag_m.group(2)
        suffix = tag_m.group(3)

        # optional source= (consumer references owner struct)
        src_m = SOURCE_RE.search(stripped)
        source = src_m.group(1) if src_m else struct_name

        h_structs[struct_name] = {
            '_name': struct_name,
            'owner': owner,
            'suffix': suffix,
            'source': source,
            'file': relpath,
            'sizeof': 0,
            'fields': [],
        }

        # scan forward to collect fields + sizeof
        for j in range(idx + 1, min(idx + 50, len(lines))):
            fline = lines[j].strip()

            # check for sizeof comment
            sz_m = SIZEOF_RE.search(fline)
            if sz_m and sz_m.group(1) == struct_name:
                h_structs[struct_name]['sizeof'] = int(sz_m.group(2))
                break

            # check for field
            f_m = FIELD_RE.search(fline)
            if f_m:
                ftype = f_m.group(1)
                fname = f_m.group(2)
                fcount = f_m.group(3)
                foffset = int(f_m.group(4))
                fsize = int(f_m.group(5))
                h_structs[struct_name]['fields'].append({
                    'name': fname,
                    'type': ftype,
                    'offset': foffset,
                    'size': fsize,
                    'count': int(fcount) if fcount else 1,
                })


# ============================================================
# .h 双向结构体一致性检查
# ============================================================

def _is_nested_struct_match(consumer_type, owner_type, h_structs):
    """检查两个类型名是否引用相同语义的结构体 (嵌套结构体的 source 链匹配).

    consumer_type 是 consumer 侧的嵌套结构体名 (如 Adc_HrtimState_IN_t),
    owner_type 是 owner 侧的嵌套结构体名 (如 IH_HrtimState).
    如果 consumer_type 的 source 指向 owner_type, 则认为匹配.
    """
    if consumer_type not in h_structs or owner_type not in h_structs:
        return False
    cinfo = h_structs[consumer_type]
    oinfo = h_structs[owner_type]
    # consumer's source should equal owner's name or owner's source
    return (cinfo['source'] == owner_type or
            cinfo['source'] == oinfo['source'])


# ============================================================
# 命名约定检查 — Input_Link / Output_Link 配对
# ============================================================

def check_naming_convention(project_dir, search_paths):
    """检查 .h 文件中的 I/O 结构体命名约定.

    规则:
      - _Input / _Output 必须配对出现 (同模块前缀)
      - _Output_Link 是数据列定义, 不参与配对
    """
    violations = []
    modules = {}

    for sp in search_paths:
        base = os.path.join(project_dir, sp)
        if not os.path.isdir(base):
            continue
        for root, dirs, files in os.walk(base):
            for fname in files:
                if not fname.endswith('.h'):
                    continue
                filepath = os.path.join(root, fname)
                relpath = os.path.relpath(filepath, project_dir)

                with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()

                for m in TYPEDEF_NAME_RE.finditer(content):
                    prefix = m.group(1)
                    suffix = m.group(2)
                    if prefix not in modules:
                        modules[prefix] = {}
                    modules[prefix][suffix] = relpath

    # Check: Input/Output pairing
    for prefix, pair in modules.items():
        if 'Input' not in pair:
            violations.append({
                'name': prefix,
                'desc': (f'结构体 "{prefix}" 缺少 _Input 配对 '
                         f'(仅 {pair.get("Output", "?")})'),
            })
        elif 'Output' not in pair:
            violations.append({
                'name': prefix,
                'desc': (f'结构体 "{prefix}" 缺少 _Output 配对 '
                         f'(仅 {pair.get("Input", "?")})'),
            })

    return violations


def check_h_consistency(h_structs):
    """验证 consumer 结构体与 owner (source) 结构体的字段一致性.

    规则:
      - consumer 字段必须在 owner 中存在
      - 同名字段的 offset, type, size 必须一致
      - consumer 字段顺序与 owner 保持一致 (按 offset 排序)
    """
    violations = []

    # 分离 owner (source == struct_name) 和 consumer (source != struct_name)
    owners = {}
    consumers = []

    for name, info in h_structs.items():
        if info['source'] == name:
            owners[name] = info
        else:
            consumers.append(info)

    for cinfo in consumers:
        source_name = cinfo['source']
        if source_name not in owners:
            violations.append({
                'consumer': cinfo.get('file', '?') + ':' + cinfo.get('_name', '?'),
                'source': source_name,
                'desc': (
                    f'consumer 引用的 source "{source_name}" 未找到 owner 声明'
                ),
            })
            continue

        oinfo = owners[source_name]
        owner_fields = {f['name']: f for f in oinfo['fields']}

        # consumer fields must be a subset of owner fields, same order
        prev_offset = -1
        for cf in cinfo['fields']:
            cname = cf['name']
            if cname not in owner_fields:
                violations.append({
                    'consumer': _cinfo_id(cinfo),
                    'source': source_name,
                    'desc': (
                        f'consumer 字段 "{cname}" 不在 owner "{source_name}" 中'
                    ),
                })
                continue

            of = owner_fields[cname]

            # offset check
            if cf['offset'] != of['offset']:
                violations.append({
                    'consumer': _cinfo_id(cinfo),
                    'source': source_name,
                    'desc': (
                        f'字段 "{cname}" offset 不一致: '
                        f'consumer={cf["offset"]}, owner={of["offset"]}'
                    ),
                })

            # type check — 嵌套结构体通过 source 链比对, 不按字面类型名
            if cf['type'] != of['type']:
                if not _is_nested_struct_match(cf['type'], of['type'], h_structs):
                    violations.append({
                        'consumer': _cinfo_id(cinfo),
                        'source': source_name,
                        'desc': (
                            f'字段 "{cname}" 类型不一致: '
                            f'consumer={cf["type"]}, owner={of["type"]}'
                        ),
                    })

            # size check
            if cf['size'] != of['size']:
                violations.append({
                    'consumer': _cinfo_id(cinfo),
                    'source': source_name,
                    'desc': (
                        f'字段 "{cname}" size 不一致: '
                        f'consumer={cf["size"]}, owner={of["size"]}'
                    ),
                })

            # order check: consumer fields must follow owner order
            if cf['offset'] < prev_offset:
                violations.append({
                    'consumer': _cinfo_id(cinfo),
                    'source': source_name,
                    'desc': (
                        f'consumer 字段顺序与 owner 不一致 '
                        f'(字段 "{cname}" offset={cf["offset"]} '
                        f'< 前字段 offset={prev_offset})'
                    ),
                })
            prev_offset = cf['offset']

        # sizeof check: consumer sizeof should match owner sizeof
        if cinfo['sizeof'] != 0 and oinfo['sizeof'] != 0:
            if cinfo['sizeof'] != oinfo['sizeof']:
                violations.append({
                    'consumer': _cinfo_id(cinfo),
                    'source': source_name,
                    'desc': (
                        f'sizeof 不一致: '
                        f'consumer={cinfo["sizeof"]}, owner={oinfo["sizeof"]}'
                    ),
                })

    return violations


def _cinfo_id(cinfo):
    """返回 consumer 的可读标识符."""
    name = cinfo.get('_name', '?')
    f = cinfo.get('file', '?')
    return f'{f} ({name})' if name != '?' else f


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
        return None  # 不再直接退出 — 允许回退到 .h 模式
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
    script_dir = os.path.dirname(os.path.abspath(__file__))
    gen_script = os.path.join(script_dir, 'generate_structs.py')

    if not os.path.exists(gen_script):
        return -1, '', f'generate_structs.py 未找到: {gen_script}'

    tmpfd, tmpcfg = tempfile.mkstemp(suffix='.json', prefix='structs_cfg_')
    try:
        with os.fdopen(tmpfd, 'w', encoding='utf-8') as f:
            json.dump(config, f)

        cmd = [
            sys.executable, gen_script,
            project_dir,
            '--config', tmpcfg,
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
    finally:
        os.unlink(tmpcfg)


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
        assert len(errs) >= 1
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
            assert len(v) == 0
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
            assert len(v) == 1
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

    # 12) @STRUCT 解析 — 完整 .h 段
    def test_parse_h_structs():
        with tempfile.TemporaryDirectory() as tmpdir:
            mod_dir = os.path.join(tmpdir, 'base_class', 'test_mod')
            os.makedirs(mod_dir)
            h_file = os.path.join(mod_dir, 'test.h')
            with open(h_file, 'w', encoding='utf-8') as f:
                f.write('''/* ============================================================
 *  INTERFACE STRUCTS
 * ============================================================ */

/* @STRUCT TestResult  owner=test_mod  suffix=OUT */
typedef struct {
    float value_a;                     /* offset=0,  size=4 */
    uint16_t flags;                    /* offset=4,  size=2 */
    bool ok;                           /* offset=6,  size=1 */
} TestResult;                          /* sizeof=7 */

/* === END INTERFACE STRUCTS === */
''')
            result = parse_h_interface_structs(tmpdir, ['base_class'])
            assert 'TestResult' in result
            r = result['TestResult']
            assert r['owner'] == 'test_mod'
            assert r['suffix'] == 'OUT'
            assert r['sizeof'] == 7
            assert len(r['fields']) == 3
            assert r['fields'][0]['name'] == 'value_a'
            assert r['fields'][0]['type'] == 'float'
            assert r['fields'][0]['offset'] == 0
            assert r['fields'][0]['size'] == 4
        return True
    t("@STRUCT .h 解析 (完整段落)", test_parse_h_structs)

    # 13) @STRUCT 一致性 — 匹配
    def test_h_consistency_match():
        structs = {
            'OwnerOut': {
                'owner': 'owner_mod', 'suffix': 'OUT', 'source': 'OwnerOut',
                'file': 'owner/owner.h', 'sizeof': 8,
                'fields': [
                    {'name': 'a', 'type': 'float', 'offset': 0, 'size': 4, 'count': 1},
                    {'name': 'b', 'type': 'uint16_t', 'offset': 4, 'size': 2, 'count': 1},
                ],
                '_name': 'OwnerOut',
            },
            'ConsumerIn': {
                'owner': 'consumer_mod', 'suffix': 'IN', 'source': 'OwnerOut',
                'file': 'consumer/consumer.h', 'sizeof': 8,
                'fields': [
                    {'name': 'a', 'type': 'float', 'offset': 0, 'size': 4, 'count': 1},
                    {'name': 'b', 'type': 'uint16_t', 'offset': 4, 'size': 2, 'count': 1},
                ],
                '_name': 'ConsumerIn',
            },
        }
        v = check_h_consistency(structs)
        assert len(v) == 0, f"Expected 0 violations, got: {v}"
        return True
    t("@STRUCT 一致性 (完全匹配)", test_h_consistency_match)

    # 14) @STRUCT 一致性 — offset 不匹配
    def test_h_consistency_offset_mismatch():
        structs = {
            'OwnerOut': {
                'owner': 'owner_mod', 'suffix': 'OUT', 'source': 'OwnerOut',
                'file': 'owner/owner.h', 'sizeof': 8,
                'fields': [
                    {'name': 'a', 'type': 'float', 'offset': 0, 'size': 4, 'count': 1},
                ],
                '_name': 'OwnerOut',
            },
            'ConsumerIn': {
                'owner': 'consumer_mod', 'suffix': 'IN', 'source': 'OwnerOut',
                'file': 'consumer/consumer.h', 'sizeof': 8,
                'fields': [
                    {'name': 'a', 'type': 'float', 'offset': 4, 'size': 4, 'count': 1},
                ],
                '_name': 'ConsumerIn',
            },
        }
        v = check_h_consistency(structs)
        assert len(v) == 1
        assert 'offset' in v[0]['desc']
        return True
    t("@STRUCT 一致性 (offset 不匹配)", test_h_consistency_offset_mismatch)

    # 15) @STRUCT 解析 — 嵌套结构体
    def test_parse_nested_struct():
        with tempfile.TemporaryDirectory() as tmpdir:
            mod_dir = os.path.join(tmpdir, 'base_class', 'test_mod')
            os.makedirs(mod_dir)
            h_file = os.path.join(mod_dir, 'test_nested.h')
            with open(h_file, 'w', encoding='utf-8') as f:
                f.write('''/* ============================================================
 *  INTERFACE STRUCTS
 * ============================================================ */

/* @STRUCT Inner  owner=test_mod  suffix=OUT */
typedef struct {
    uint16_t x;                        /* offset=0,  size=2 */
    uint16_t y;                        /* offset=2,  size=2 */
} Inner;                               /* sizeof=4 */

/* @STRUCT Outer  owner=test_mod  suffix=OUT */
typedef struct {
    Inner inner;                       /* offset=0,  size=4 — nested */
    float z;                           /* offset=4,  size=4 */
} Outer;                               /* sizeof=8 */

/* === END INTERFACE STRUCTS === */
''')
            result = parse_h_interface_structs(tmpdir, ['base_class'])
            assert 'Inner' in result
            assert 'Outer' in result
            assert result['Inner']['sizeof'] == 4
            assert result['Outer']['sizeof'] == 8
            assert result['Outer']['fields'][0]['type'] == 'Inner'
            assert result['Outer']['fields'][0]['size'] == 4
        return True
    t("@STRUCT .h 解析 (嵌套结构体)", test_parse_nested_struct)

    # 16) 命名约定检查 — Input_Link / Output_Link 配对
    def test_naming_convention_pairing():
        with tempfile.TemporaryDirectory() as tmpdir:
            mod_dir = os.path.join(tmpdir, 'src', 'app', 'test_mod')
            os.makedirs(mod_dir)
            h_file = os.path.join(mod_dir, 'test_mod.h')
            with open(h_file, 'w', encoding='utf-8') as f:
                f.write('typedef struct { uint16_t val; } Power_Input;\n')
                # no Power_Output — should trigger violation
            violations = check_naming_convention(tmpdir, ['src/app'])
            assert len(violations) == 1
            assert '缺少 _Output' in violations[0]['desc']
        return True
    t("命名约定 (缺少 _Output 配对)", test_naming_convention_pairing)

    # 17) 命名约定检查 — 完整配对
    def test_naming_convention_ok():
        with tempfile.TemporaryDirectory() as tmpdir:
            mod_dir = os.path.join(tmpdir, 'src', 'app', 'test_mod')
            os.makedirs(mod_dir)
            h_file = os.path.join(mod_dir, 'test_mod.h')
            with open(h_file, 'w', encoding='utf-8') as f:
                f.write('typedef struct { uint16_t val; } Power_Input;\n')
                f.write('typedef struct { uint32_t result; } Power_Output;\n')
            violations = check_naming_convention(tmpdir, ['src/app'])
            assert len(violations) == 0, f"Expected 0 violations, got: {violations}"
        return True
    t("命名约定 (完整配对)", test_naming_convention_ok)

    if errors:
        print(f"\n[FAIL] --self-test: {len(errors)}/17 项失败")
        sys.exit(1)

    print(f"\n[PASS] --self-test 全部通过 (17/17)")
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
    search_paths = config.get('module_search_paths', ['.'])
    has_structs_json = os.path.exists(json_path)

    print(f"check_structs.py — 结构体一致性验证")
    print(f"项目目录: {project_dir}")
    print(f"规则来源: {cfg_source}")
    print()

    total_violations = 0

    # ================================================================
    # 模式 A: structs.json 存在 → 走 JSON 六件套
    # ================================================================
    if has_structs_json:
        print(f"数据源:   {json_path}")
        print()

        # ---- 检查 A1: JSON format + schema ----
        print("--- 1. structs.json 格式校验 ---")
        structs_data = load_structs_json(json_path)
        whitelist = config.get('type_whitelist',
                               ['uint8_t', 'uint16_t', 'uint32_t', 'int8_t',
                                'int16_t', 'int32_t', 'char'])
        schema_errors = check_json_schema(structs_data, whitelist)
        total_violations += len(schema_errors)
        if schema_errors:
            print(f"  [FAIL] {len(schema_errors)} 项错误:")
            for e in schema_errors:
                print(f"    - {e}")
        else:
            print(f"  [PASS] schema 有效 (serial={structs_data.get('serial', '?')}, "
                  f"{len(structs_data.get('structs', {}))} struct(s))")
        print()

        # ---- 检查 A2: serial 单调递增 ----
        print("--- 2. serial 单调递增检查 ---")
        serial_violations = check_serial_monotonic(
            project_dir, structs_data.get('serial', 0), search_paths)
        total_violations += len(serial_violations)
        if serial_violations:
            print(f"  [FAIL] {len(serial_violations)} 项 serial 回退:")
            for v in serial_violations:
                print(f"    {v['file']}: JSON serial={v['json_serial']} < "
                      f"文件 serial={v['file_serial']}")
        else:
            print(f"  [PASS] 所有生成文件 serial <= JSON serial")
        print()

        # ---- 检查 A3: deprecated 引用 ----
        print("--- 3. deprecated 字段引用检查 ---")
        dep_violations = check_deprecated_refs(structs_data)
        total_violations += len(dep_violations)
        if dep_violations:
            print(f"  [FAIL] {len(dep_violations)} 项废弃字段引用:")
            for v in dep_violations:
                print(f"    {v['desc']}")
        else:
            print(f"  [PASS] 无 consumer 引用废弃字段")
        print()

        # ---- 检查 A4: 类型白名单 ----
        print("--- 4. 类型白名单检查 ---")
        type_violations = check_type_whitelist(structs_data, whitelist)
        total_violations += len(type_violations)
        if type_violations:
            print(f"  [FAIL] {len(type_violations)} 项非白名单类型:")
            for v in type_violations:
                print(f"    {v['desc']}")
        else:
            print(f"  [PASS] 所有字段类型在白名单内")
        print()

        # ---- 检查 A5: 模块存在性 ----
        print("--- 5. 模块目录存在性 ---")
        module_violations = check_module_existence(structs_data, project_dir, search_paths)
        if module_violations:
            print(f"  [WARN] {len(module_violations)} 个模块目录未找到 (将在首次生成时创建):")
            for v in module_violations:
                print(f"    - {v['desc']}")
        else:
            print(f"  [PASS] 所有模块目录存在")
        print()

        # ---- 检查 A6: 生成一致性 ----
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
                total_violations += 1
            else:
                print(f"  [FAIL] generate_structs.py --check 执行异常 (rc={rc})")
                total_violations += 1
        print()

    # ================================================================
    # 模式 B: structs.json 不存在 → .h @STRUCT 解析
    # ================================================================
    else:
        print(f"数据源:   .h @STRUCT 标记 (structs.json 不存在, 使用双向维护模式)")
        print()

        # ---- 检查 B1: 解析 .h INTERFACE STRUCTS 段 ----
        print("--- 1. .h @STRUCT 解析 ---")
        h_structs = parse_h_interface_structs(project_dir, search_paths)

        if not h_structs:
            print("  [INFO] 未找到 INTERFACE STRUCTS 段 — 没有需要检查的跨模块结构体")
            print()
        else:
            owner_count = sum(1 for s in h_structs.values() if s['source'] == s.get('_name', ''))
            consumer_count = sum(1 for s in h_structs.values()
                                if s['source'] != s.get('_name', ''))
            # Re-count properly: owner = source equals struct name
            owner_count = 0
            consumer_count = 0
            for name, info in h_structs.items():
                if info['source'] == name:
                    owner_count += 1
                else:
                    consumer_count += 1

            print(f"  [PASS] 解析完成: {len(h_structs)} 结构体 "
                  f"({owner_count} owner, {consumer_count} consumer)")
            for name, info in sorted(h_structs.items()):
                tag = 'OWNER' if info['source'] == name else f'CONSUMER of {info["source"]}'
                print(f"    {name}: {info['sizeof']} bytes, "
                      f"{len(info['fields'])} fields, "
                      f"owner={info['owner']}, {tag} "
                      f"({info['file']})")
            print()

            # ---- 检查 B2: consumer vs owner 一致性 ----
            print("--- 2. 双向结构体一致性 ---")
            h_violations = check_h_consistency(h_structs)
            total_violations += len(h_violations)
            if h_violations:
                print(f"  [FAIL] {len(h_violations)} 项不一致:")
                for v in h_violations:
                    print(f"    {v['consumer']}: {v['desc']}")
            else:
                print(f"  [PASS] 所有 consumer 结构体与 owner 一致")
            print()

    # ================================================================
    # 通用检查: I/O 结构体命名约定 — Input/Output 配对 + Link 注解
    # 在两种模式中都运行, 扫描所有 .h 文件
    # ================================================================
    print("--- 命名约定: I/O 结构体 ---")
    naming_violations = check_naming_convention(project_dir, search_paths)
    total_violations += len(naming_violations)
    if naming_violations:
        print(f"  [FAIL] {len(naming_violations)} 项命名约定违规:")
        for v in naming_violations:
            print(f"    - {v['desc']}")
    else:
        print("  [PASS] _Input / _Output 配对完整")
    print()

    # ---- 汇总 ----
    if total_violations > 0:
        print(f"[FAIL] 发现 {total_violations} 项问题, 请在提交前修复")
        sys.exit(1)

    print(f"[PASS] 全部通过 — 0 违规")
    sys.exit(0)


if __name__ == '__main__':
    main()
