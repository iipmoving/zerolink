#!/usr/bin/env python3
"""
generate_structs.py — 跨模块结构体代码生成器 (多项目可配置)

从 cfg/structs.json (单一数据源) 读取结构体定义,
为每个模块生成 types.h (owner 全字段 + consumer 子集+padding).

用法:
  python generate_structs.py <project_dir>                            自动检测
  python generate_structs.py <project_dir> --project four_head        内置预设
  python generate_structs.py <project_dir> --config deps_config.json  自定义配置
  python generate_structs.py <project_dir> --check                    检查模式 (diff)
  python generate_structs.py <project_dir> --struct StateGlobal       单结构体
  python generate_structs.py <project_dir> --source cfg/structs.json  指定 JSON 路径
  python generate_structs.py --print-template                         输出 JSON 模板
  python generate_structs.py --self-test                              自检

退出码: 0=成功, 1=检查模式发现差异, 2=用法错误
"""

import sys
import os
import re
import json
import tempfile
import datetime
from collections import OrderedDict


# ============================================================
# 内置预设
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

# 类型→大小映射 (V1.0 硬编码, 嵌入式兼容)
TYPE_SIZES = {
    'uint8_t': 1, 'uint16_t': 2, 'uint32_t': 4,
    'int8_t': 1, 'int16_t': 2, 'int32_t': 4,
    'char': 1, 'float': 4,
}

# AUTO-GENERATED 区块标记
AUTO_GEN_START = '/* === AUTO-GENERATED from structs.json'
AUTO_GEN_END = '/* === END AUTO-GENERATED === */'
MANUAL_START = '/* === 手写区域 === */'


# ============================================================
# 配置加载
# ============================================================

def load_config_file(path):
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)


def resolve_config(project_dir, preset_name, config_path):
    if config_path:
        cfg = load_config_file(config_path)
        if 'structs' in cfg:
            # 可能误传了 structs.json 本身, 提取 structs_source 相关
            pass
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

    # 尝试从 project_dir/deps_config.json 读取 structs_generation 段
    deps_cfg = os.path.join(project_dir, 'deps_config.json')
    if os.path.isfile(deps_cfg):
        cfg = load_config_file(deps_cfg)
        if 'structs_generation' in cfg:
            return cfg['structs_generation'], 'file:deps_config.json#structs_generation'

    # 本地 structs_config.json
    local_cfg = os.path.join(project_dir, 'structs_config.json')
    if os.path.isfile(local_cfg):
        return load_config_file(local_cfg), 'file:structs_config.json'

    # 自动检测
    if os.path.isdir(os.path.join(project_dir, 'drv')):
        return BUILTIN_PRESETS['four_head'], 'auto:four_head'
    if os.path.isdir(os.path.join(project_dir, 'base_class')):
        return BUILTIN_PRESETS['m4-ekf'], 'auto:m4-ekf'

    print("错误: 无法自动检测项目类型, 请使用 --project 或 --config")
    print(f"可用预设: {list(BUILTIN_PRESETS.keys())}")
    sys.exit(2)


# ============================================================
# structs.json 加载与校验
# ============================================================

def load_structs_json(filepath):
    """加载并校验 structs.json。返回 dict 或报错退出。"""
    if not os.path.exists(filepath):
        print(f"错误: structs.json 不存在: {filepath}")
        print("请先创建 cfg/structs.json (运行 --print-template 查看格式)")
        sys.exit(2)

    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            data = json.load(f)
    except json.JSONDecodeError as e:
        print(f"错误: structs.json JSON 解析失败: {e}")
        sys.exit(2)

    errors = validate_schema(data)
    if errors:
        print(f"错误: structs.json 校验失败 ({len(errors)} 项):")
        for e in errors:
            print(f"  - {e}")
        sys.exit(2)

    return data


def validate_schema(data):
    """校验 structs.json 顶层结构。返回错误列表。"""
    errors = []

    if not isinstance(data, dict):
        return ['根元素必须是 JSON object']

    if 'version' not in data:
        errors.append('缺少 "version" 字段')
    if 'serial' not in data:
        errors.append('缺少 "serial" 字段')
    elif not isinstance(data['serial'], int):
        errors.append('"serial" 必须是整数')
    if 'structs' not in data:
        errors.append('缺少 "structs" 字段')
        return errors
    if not isinstance(data['structs'], dict):
        errors.append('"structs" 必须是 object')
        return errors

    for key, sdef in data['structs'].items():
        errs = validate_struct_def(key, sdef)
        errors.extend(errs)

    return errors


def validate_struct_def(key, sdef):
    """校验单个结构体定义。返回错误列表。"""
    errors = []

    if not isinstance(sdef, dict):
        errors.append(f'structs["{key}"]: 必须是 object')
        return errors

    for required in ['description', 'owner', 'suffix', 'fields']:
        if required not in sdef:
            errors.append(f'structs["{key}"]: 缺少 "{required}" 字段')

    if 'fields' in sdef and isinstance(sdef['fields'], list):
        field_names = set()
        for i, f in enumerate(sdef['fields']):
            if not isinstance(f, dict):
                errors.append(f'structs["{key}"].fields[{i}]: 必须是 object')
                continue
            if 'name' not in f:
                errors.append(f'structs["{key}"].fields[{i}]: 缺少 "name"')
            else:
                if f['name'] in field_names:
                    errors.append(f'structs["{key}"].fields[{i}]: 重复字段名 "{f["name"]}"')
                field_names.add(f['name'])
            if 'type' not in f:
                errors.append(f'structs["{key}"].fields[{i}]: 缺少 "type"')

    if 'consumers' in sdef and isinstance(sdef['consumers'], dict):
        for cname, cdef in sdef['consumers'].items():
            if not isinstance(cdef, dict):
                errors.append(f'structs["{key}"].consumers["{cname}"]: 必须是 object')
                continue
            if 'fields' not in cdef:
                errors.append(
                    f'structs["{key}"].consumers["{cname}"]: 缺少 "fields"')
            elif isinstance(cdef['fields'], list):
                # 检查 consumer fields 都是 owner fields 的子集
                owner_fields = {f['name'] for f in sdef.get('fields', [])}
                for cf in cdef['fields']:
                    if cf not in owner_fields:
                        errors.append(
                            f'structs["{key}"].consumers["{cname}"].fields: '
                            f'"{cf}" 不在 owner fields 中')

    if 'deprecated' in sdef and isinstance(sdef['deprecated'], dict):
        for dfname, dinfo in sdef['deprecated'].items():
            owner_fields = {f['name'] for f in sdef.get('fields', [])}
            if dfname not in owner_fields:
                errors.append(
                    f'structs["{key}"].deprecated["{dfname}"]: '
                    f'字段不在 owner fields 中')

    return errors


# ============================================================
# 模块目录查找
# ============================================================

def find_module_dir(project_dir, module_name, search_paths):
    """在项目目录下查找模块目录。返回路径或 None。"""
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
                return entry_path
    return None


def ensure_module_dir(project_dir, module_name, search_paths):
    """查找或创建模块目录。"""
    existing = find_module_dir(project_dir, module_name, search_paths)
    if existing:
        return existing
    # 在第一个有效搜索路径下创建
    for sp in search_paths:
        base = os.path.join(project_dir, sp)
        if os.path.isdir(base):
            module_dir = os.path.join(base, module_name)
            os.makedirs(module_dir, exist_ok=True)
            return module_dir
    # 兜底: 项目根目录
    module_dir = os.path.join(project_dir, module_name)
    os.makedirs(module_dir, exist_ok=True)
    return module_dir


# ============================================================
# 字段大小计算
# ============================================================

def compute_field_size(field, type_sizes=None):
    """计算字段的字节大小。"""
    if type_sizes is None:
        type_sizes = TYPE_SIZES
    ftype = field.get('type', 'uint8_t')
    size = type_sizes.get(ftype, 1)
    count = field.get('count', 1)
    if count is None:
        count = 1
    return size * count


def to_pascal_case(snake_str):
    """将 snake_case 转为 PascalCase。"""
    return ''.join(word.capitalize() for word in snake_str.split('_'))


# ============================================================
# 代码生成
# ============================================================

def format_field_decl(field):
    """格式化字段声明。返回 C 声明字符串。"""
    ftype = field['type']
    fname = field['name']
    count = field.get('count')
    if count:
        return f"{ftype} {fname}[{count}]"
    return f"{ftype} {fname}"


def generate_owner_struct(struct_key, struct_def, whitelist, type_sizes):
    """生成 owner 模块的完整结构体。返回 C 代码字符串。"""
    fields = struct_def.get('fields', [])
    suffix = struct_def.get('suffix', '')
    deprecated = struct_def.get('deprecated', {})

    struct_name = f"{struct_key}_{suffix}_t" if suffix else f"{struct_key}_t"

    lines = []
    lines.append(f"// {struct_def.get('description', struct_key)}")
    lines.append(f"typedef struct {{")

    # 对齐列宽
    max_type_len = 0
    for f in fields:
        decl = format_field_decl(f)
        max_type_len = max(max_type_len, len(decl))

    for f in fields:
        decl = format_field_decl(f)
        comment_parts = []
        if f.get('note'):
            comment_parts.append(f['note'])
        if f['name'] in deprecated:
            dinfo = deprecated[f['name']]
            comment_parts.append(
                f"@deprecated: use {dinfo.get('replaced_by', '?')}")
        comment = f"  /* {'; '.join(comment_parts)} */" if comment_parts else ""
        lines.append(f"    {decl:<{max_type_len}}{comment}")

    lines.append(f"}} {struct_name};")
    return '\n'.join(lines)


def generate_consumer_struct(struct_key, struct_def, consumer_name,
                             consumer_def, whitelist, type_sizes):
    """生成 consumer 模块的结构体 (子集字段 + padding)。返回 C 代码字符串。"""
    owner_fields = struct_def.get('fields', [])
    consumer_field_names = set(consumer_def.get('fields', []))
    deprecated = struct_def.get('deprecated', {})

    custom_name = consumer_def.get('struct_name')
    if custom_name:
        struct_name = custom_name
    else:
        pascal_name = to_pascal_case(consumer_name)
        struct_name = f"{pascal_name}_{struct_key}_IN_t"

    lines = []
    lines.append(f"// {struct_def.get('description', struct_key)}")
    lines.append(f"// consumer: {consumer_name}  ← owner: {struct_def.get('owner', '?')}")
    lines.append(f"typedef struct {{")

    pad_idx = 0
    decls = []

    for f in owner_fields:
        fname = f['name']

        # 检查 deprecated 引用
        if fname in consumer_field_names and fname in deprecated:
            print(
                f"错误: consumer '{consumer_name}' 引用了废弃字段 '{fname}' "
                f"({deprecated[fname].get('replaced_by', '?')})")
            print(f"  在 struct '{struct_key}' 中, 请更新 consumer fields 列表")
            sys.exit(2)

        if fname in consumer_field_names:
            decl = format_field_decl(f)
            decls.append((decl, f"  /* -> {struct_def.get('owner', '?')} */", False))
        else:
            size = compute_field_size(f, type_sizes)
            pad_decl = f"uint8_t __pad_{pad_idx}[{size}]"
            comment = f"  /* PADDING: {fname} ({size} bytes) */"
            decls.append((pad_decl, comment, True))
            pad_idx += 1

    # 对齐
    max_decl_len = max(len(d[0]) for d in decls) if decls else 0
    for decl, comment, _ in decls:
        lines.append(f"    {decl:<{max_decl_len}}{comment}")

    lines.append(f"}} {struct_name};")
    return '\n'.join(lines)


def make_auto_header(serial, version="1.0"):
    """生成 AUTO-GENERATED 文件头。"""
    now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    return (
        f"/* ================================================================\n"
        f"   AUTO-GENERATED from cfg/structs.json  serial={serial}  schema={version}\n"
        f"   Generated: {now}\n"
        f"   DO NOT EDIT THIS FILE.\n"
        f"\n"
        f"   修改跨模块结构体请编辑: cfg/structs.json\n"
        f"   然后运行:              python tools/generate_structs.py\n"
        f"   ================================================================ */"
    )


def make_auto_block(structs_content, serial, version="1.0"):
    """生成完整的 AUTO-GENERATED 区块 (含标记)。"""
    header = make_auto_header(serial, version)
    return (
        f"{header}\n"
        f"\n"
        f"/* === AUTO-GENERATED from structs.json serial={serial} === */\n"
        f"/* DO NOT EDIT BELOW. Run: python tools/generate_structs.py */\n"
        f"\n"
        f"{structs_content}\n"
        f"\n"
        f"/* === END AUTO-GENERATED === */"
    )


def parse_existing_types_h(filepath):
    """解析已有 types.h, 提取手写区域和 AUTO-GENERATED 区域。返回 (manual_parts, auto_parts, has_auto_block)。"""
    if not os.path.exists(filepath):
        return [], None, False

    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    # 检查是否有 AUTO-GENERATED 区块
    auto_start_re = re.compile(r'/\* === AUTO-GENERATED from structs\.json serial=(\d+) === \*/')
    auto_end_re = re.compile(r'/\* === END AUTO-GENERATED === \*/')

    start_m = auto_start_re.search(content)
    end_m = auto_end_re.search(content)

    if start_m and end_m:
        embedded_serial = int(start_m.group(1))
        before = content[:start_m.start()].rstrip()
        after = content[end_m.end():].lstrip()
        manual_parts = []
        if before:
            manual_parts.append(before)
        if after:
            manual_parts.append(after)
        return manual_parts, embedded_serial, True
    else:
        # 没有 AUTO-GENERATED 区块, 整个文件视为手写区域
        return [content.rstrip()], None, False


def merge_types_h(filepath, structs_content, serial, version="1.0"):
    """将生成的 structs 内容合并到 types.h 中。"""
    manual_parts, embedded_serial, has_auto = parse_existing_types_h(filepath)

    # 检查 serial 回退
    if embedded_serial is not None and serial < embedded_serial:
        print(f"警告: {filepath}")
        print(f"  structs.json serial={serial} < 文件内嵌 serial={embedded_serial}")
        print(f"  JSON 可能被回退了 — 请确认这是有意操作, 然后重新生成")
        # 仍然允许生成, 但发出警告

    auto_block = make_auto_block(structs_content, serial, version)

    if has_auto:
        # 只替换 AUTO-GENERATED 区块
        # manual_parts[0] = before, manual_parts[1:] = after (可能有多个)
        before = manual_parts[0] if len(manual_parts) > 0 else ''
        after_parts = manual_parts[1:] if len(manual_parts) > 1 else []
        after = '\n\n'.join(after_parts)
        parts = [p for p in [before, auto_block, after] if p]
        return '\n\n'.join(parts) + '\n'
    else:
        # 文件全新 或 无 AUTO-GENERATED 区块
        # 将现有内容包裹为手写区域
        if manual_parts and manual_parts[0].strip():
            return f"{MANUAL_START}\n{manual_parts[0]}\n\n{auto_block}\n"
        else:
            return f"{auto_block}\n"


# ============================================================
# 主生成逻辑
# ============================================================

def generate_all(structs_data, config, project_dir, struct_filter=None):
    """遍历 structs.json, 为每个模块生成 types.h。返回生成的文件列表。"""
    serial = structs_data.get('serial', 0)
    version = structs_data.get('version', '1.0')
    structs = structs_data.get('structs', {})

    whitelist = config.get('type_whitelist',
                           ['uint8_t', 'uint16_t', 'uint32_t', 'int8_t',
                            'int16_t', 'int32_t', 'char'])
    type_sizes = config.get('type_sizes', TYPE_SIZES)
    search_paths = config.get('module_search_paths', ['.'])

    # 按模块聚合生成内容
    # module_contents[module_name] = [] of (struct block string)
    module_contents = OrderedDict()

    for skey, sdef in structs.items():
        if struct_filter and skey != struct_filter:
            continue

        # 校验字段类型
        for f in sdef.get('fields', []):
            ftype = f.get('type', '')
            if ftype not in whitelist:
                print(f"错误: struct '{skey}' 字段 '{f['name']}' 类型 '{ftype}' 不在白名单中")
                print(f"  允许的类型: {whitelist}")
                sys.exit(2)

        # Owner struct
        owner = sdef['owner']
        owner_code = generate_owner_struct(skey, sdef, whitelist, type_sizes)
        module_contents.setdefault(owner, []).append(owner_code)

        # Consumer structs
        consumers = sdef.get('consumers', {})
        for cname, cdef in consumers.items():
            # 检查 deprecated 引用
            deprecated = sdef.get('deprecated', {})
            for cf in cdef.get('fields', []):
                if cf in deprecated:
                    print(
                        f"错误: struct '{skey}' consumer '{cname}' "
                        f"引用了废弃字段 '{cf}'")
                    print(f"  替代字段: {deprecated[cf].get('replaced_by', '?')}")
                    sys.exit(2)

            consumer_code = generate_consumer_struct(
                skey, sdef, cname, cdef, whitelist, type_sizes)
            module_contents.setdefault(cname, []).append(consumer_code)

    # 写入文件
    generated_files = []
    for module_name, struct_blocks in module_contents.items():
        structs_content = '\n\n'.join(struct_blocks)
        module_dir = ensure_module_dir(project_dir, module_name, search_paths)
        output_path = os.path.join(module_dir, 'types.h')

        merged = merge_types_h(output_path, structs_content, serial, version)

        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(merged)

        generated_files.append(output_path)
        print(f"  GENERATED: {os.path.relpath(output_path, project_dir)}")

    return generated_files


# ============================================================
# --check 模式
# ============================================================

AUTO_START_RE = re.compile(
    r'/\* === AUTO-GENERATED from structs\.json serial=\d+ === \*/')
AUTO_END_RE = re.compile(r'/\* === END AUTO-GENERATED === \*/')


def extract_struct_content(file_content):
    """从 types.h 中提取 AUTO-GENERATED 区块内的结构体内容。
    返回 (struct_text, has_auto_block)。"""
    m_start = AUTO_START_RE.search(file_content)
    m_end = AUTO_END_RE.search(file_content)
    if m_start and m_end:
        inner = file_content[m_start.end():m_end.start()].strip()
        # 去掉可能的 "/* DO NOT EDIT BELOW..." 注释行
        lines = inner.split('\n')
        cleaned = []
        skip_next = True  # skip the first "DO NOT EDIT" line
        for line in lines:
            stripped = line.strip()
            if skip_next and (stripped.startswith('/* DO NOT EDIT') or stripped == ''):
                if stripped == '':
                    continue
                skip_next = False
                continue
            skip_next = False
            cleaned.append(line)
        return '\n'.join(cleaned).strip(), True
    return file_content.strip(), False


def normalize_struct_text(text):
    """规范化结构体文本用于比较 — 去除空白差异。"""
    lines = [line.rstrip() for line in text.split('\n')]
    # 去掉首尾空行
    while lines and not lines[0].strip():
        lines.pop(0)
    while lines and not lines[-1].strip():
        lines.pop()
    return '\n'.join(lines)


def check_mode(structs_data, config, project_dir, struct_filter=None):
    """生成结构体内容, 与实际文件中的 AUTO-GENERATED 区块比较。"""
    serial = structs_data.get('serial', 0)
    structs = structs_data.get('structs', {})

    whitelist = config.get('type_whitelist',
                           ['uint8_t', 'uint16_t', 'uint32_t', 'int8_t',
                            'int16_t', 'int32_t', 'char'])
    type_sizes = config.get('type_sizes', TYPE_SIZES)
    search_paths = config.get('module_search_paths', ['.'])

    module_contents = OrderedDict()

    for skey, sdef in structs.items():
        if struct_filter and skey != struct_filter:
            continue

        for f in sdef.get('fields', []):
            ftype = f.get('type', '')
            if ftype not in whitelist:
                print(f"错误: struct '{skey}' 字段 '{f['name']}' 类型 '{ftype}' 不在白名单中")
                sys.exit(2)

        owner = sdef['owner']
        owner_code = generate_owner_struct(skey, sdef, whitelist, type_sizes)
        module_contents.setdefault(owner, []).append(owner_code)

        consumers = sdef.get('consumers', {})
        for cname, cdef in consumers.items():
            deprecated = sdef.get('deprecated', {})
            for cf in cdef.get('fields', []):
                if cf in deprecated:
                    print(
                        f"错误: struct '{skey}' consumer '{cname}' "
                        f"引用了废弃字段 '{cf}'")
                    sys.exit(2)

            consumer_code = generate_consumer_struct(
                skey, sdef, cname, cdef, whitelist, type_sizes)
            module_contents.setdefault(cname, []).append(consumer_code)

    import difflib
    mismatches = 0

    for module_name, struct_blocks in module_contents.items():
        new_struct_content = '\n\n'.join(struct_blocks)

        existing_path = None
        existing_dir = find_module_dir(project_dir, module_name, search_paths)
        if existing_dir:
            candidate = os.path.join(existing_dir, 'types.h')
            if os.path.exists(candidate):
                existing_path = candidate

        if existing_path:
            with open(existing_path, 'r', encoding='utf-8', errors='ignore') as f:
                actual_full = f.read()

            existing_struct, has_auto = extract_struct_content(actual_full)
            new_norm = normalize_struct_text(new_struct_content)
            existing_norm = normalize_struct_text(existing_struct)

            if new_norm != existing_norm:
                mismatches += 1
                relpath = os.path.relpath(existing_path, project_dir)
                print(f"  CHECK: {relpath}  — MISMATCH")

                diff = difflib.unified_diff(
                    existing_norm.splitlines(True),
                    new_norm.splitlines(True),
                    fromfile=f"{relpath} (actual)",
                    tofile=f"{relpath} (expected)",
                )
                for line in diff:
                    sys.stdout.write(line)
                print()
            else:
                relpath = os.path.relpath(existing_path, project_dir)
                # 同时检查 serial
                auto_start_re = re.compile(
                    r'/\* === AUTO-GENERATED from structs\.json serial=(\d+) === \*/')
                sm = auto_start_re.search(actual_full)
                if sm and int(sm.group(1)) != serial:
                    print(f"  CHECK: {relpath}  — SERIAL UPDATE "
                          f"({sm.group(1)} -> {serial})")
                else:
                    print(f"  CHECK: {relpath}  — OK")
        else:
            mismatches += 1
            safe_name = module_name
            relpath = f"{safe_name}/types.h"
            print(f"  CHECK: {relpath}  — MISSING (文件不存在)")
            print(f"    请运行 python tools/generate_structs.py 生成")

    return mismatches


# ============================================================
# --print-template
# ============================================================

def print_template():
    template = {
        "_comment": "结构体代码生成器配置 — 定义 structs.json 路径和模块搜索规则",
        "structs_source": "cfg/structs.json",
        "module_search_paths": ["src/app", "src/drv", "src/proto", "src/core", "."],
        "type_whitelist": [
            "uint8_t", "uint16_t", "uint32_t",
            "int8_t", "int16_t", "int32_t",
            "char"
        ],
        "type_sizes": {
            "uint8_t": 1, "uint16_t": 2, "uint32_t": 4,
            "int8_t": 1, "int16_t": 2, "int32_t": 4,
            "char": 1, "float": 4
        },
    }
    print(json.dumps(template, indent=2, ensure_ascii=False))


def print_structs_template():
    """打印 structs.json 模板 (--print-structs-template)。"""
    template = {
        "version": "1.0",
        "serial": 0,
        "structs": {
            "_example_StateGlobal": {
                "description": "状态机模块输出的全局状态",
                "owner": "state_module",
                "suffix": "OUT",
                "fields": [
                    {"name": "head_power", "type": "uint8_t", "count": 4, "note": "4个炉头功率档位"},
                    {"name": "work_mode", "type": "uint8_t", "note": "系统工作模式"},
                    {"name": "timer_remaining", "type": "uint16_t", "note": "定时剩余秒数"},
                    {"name": "timer_active", "type": "uint8_t", "note": "定时激活标志"}
                ],
                "consumers": {
                    "display_module": {
                        "fields": ["head_power", "work_mode"]
                    },
                    "timer_module": {
                        "fields": ["timer_remaining", "timer_active"]
                    }
                }
            }
        }
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
            assert isinstance(p['module_search_paths'], list)
            assert len(p['module_search_paths']) >= 2
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
        errs = validate_schema(data)
        assert errs == [], f"Unexpected errors: {errs}"
        return True
    t("Schema 校验 (正确 JSON)", test_schema_valid)

    # 4) Schema 校验 — 缺少字段
    def test_schema_invalid():
        data = {"version": "1.0"}
        errs = validate_schema(data)
        assert len(errs) >= 2  # 缺少 serial + structs
        return True
    t("Schema 校验 (缺少 serial/structs)", test_schema_invalid)

    # 5) 字段大小计算
    def test_field_size():
        assert compute_field_size({'type': 'uint8_t'}) == 1
        assert compute_field_size({'type': 'uint16_t'}) == 2
        assert compute_field_size({'type': 'uint32_t'}) == 4
        assert compute_field_size({'type': 'uint8_t', 'count': 4}) == 4
        assert compute_field_size({'type': 'uint16_t', 'count': 3}) == 6
        assert compute_field_size({'type': 'char', 'count': 8}) == 8
        return True
    t("字段大小计算 (标量+数组)", test_field_size)

    # 6) Owner struct 生成
    def test_generate_owner():
        sdef = {
            "description": "test struct",
            "owner": "test_mod",
            "suffix": "OUT",
            "fields": [
                {"name": "a", "type": "uint8_t", "note": "field a"},
                {"name": "b", "type": "uint16_t"}
            ]
        }
        code = generate_owner_struct("Test", sdef, TYPE_SIZES.keys(), TYPE_SIZES)
        assert "Test_OUT_t" in code
        assert "uint8_t a" in code
        assert "uint16_t b" in code
        assert "/* field a */" in code
        return True
    t("Owner struct 生成", test_generate_owner)

    # 7) Consumer struct 生成 (含 padding)
    def test_generate_consumer():
        sdef = {
            "description": "test struct",
            "owner": "test_mod",
            "suffix": "OUT",
            "fields": [
                {"name": "a", "type": "uint8_t"},
                {"name": "b", "type": "uint16_t"},
                {"name": "c", "type": "uint32_t"}
            ]
        }
        cdef = {"fields": ["a", "c"]}
        code = generate_consumer_struct(
            "Test", sdef, "consumer_mod", cdef,
            TYPE_SIZES.keys(), TYPE_SIZES)
        assert "ConsumerMod_Test_IN_t" in code  # default name (PascalCase)
        assert "uint8_t a" in code
        assert "uint32_t c" in code
        assert "__pad_0[2]" in code  # PADDING for b (uint16_t = 2 bytes)
        assert "PADDING: b" in code
        return True
    t("Consumer struct 生成 (含 padding)", test_generate_consumer)

    # 8) Consumer 自定义 struct_name
    def test_consumer_custom_name():
        sdef = {
            "description": "test",
            "owner": "src_mod",
            "suffix": "OUT",
            "fields": [{"name": "x", "type": "uint8_t"}]
        }
        cdef = {"fields": ["x"], "struct_name": "Key_HeadState"}
        code = generate_consumer_struct(
            "HeadState", sdef, "key_module", cdef,
            TYPE_SIZES.keys(), TYPE_SIZES)
        assert "Key_HeadState" in code
        assert "Consumer_HeadState_IN_t" not in code
        return True
    t("Consumer 自定义 struct_name", test_consumer_custom_name)

    # 9) AUTO-GENERATED header 含 serial
    def test_auto_header():
        header = make_auto_header(5)
        assert "serial=5" in header
        assert "AUTO-GENERATED" in header
        assert "cfg/structs.json" in header
        return True
    t("AUTO-GENERATED header (serial嵌入)", test_auto_header)

    # 10) Auto block 含标记
    def test_auto_block():
        block = make_auto_block("// generated content", 3)
        assert "serial=3" in block
        assert "AUTO-GENERATED from structs.json" in block
        assert "END AUTO-GENERATED" in block
        return True
    t("AUTO-GENERATED block (含标记)", test_auto_block)

    # 11) 配置加载
    def test_config_load():
        with tempfile.NamedTemporaryFile(
            mode='w', suffix='.json', delete=False, encoding='utf-8'
        ) as f:
            json.dump(BUILTIN_PRESETS['m4-ekf'], f)
            tmp_path = f.name
        try:
            loaded = load_config_file(tmp_path)
            assert loaded['structs_source'] == 'cfg/structs.json'
            assert 'module_search_paths' in loaded
        finally:
            os.unlink(tmp_path)
        return True
    t("JSON 配置文件加载", test_config_load)

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
        print("generate_structs.py --self-test")
        run_self_test()
        sys.exit(0)

    if '--print-template' in sys.argv:
        print_template()
        sys.exit(0)

    if '--print-structs-template' in sys.argv:
        print_structs_template()
        sys.exit(0)

    args = sys.argv[1:]
    preset_name = None
    config_path = None
    project_dir = None
    check = False
    struct_filter = None
    source_path = None

    i = 0
    while i < len(args):
        if args[i] == '--project' and i + 1 < len(args):
            preset_name = args[i + 1]
            i += 2
        elif args[i] == '--config' and i + 1 < len(args):
            config_path = args[i + 1]
            i += 2
        elif args[i] == '--source' and i + 1 < len(args):
            source_path = args[i + 1]
            i += 2
        elif args[i] == '--struct' and i + 1 < len(args):
            struct_filter = args[i + 1]
            i += 2
        elif args[i] == '--check':
            check = True
            i += 1
        elif args[i] in ('--self-test', '--print-template', '--print-structs-template'):
            i += 1
        elif not args[i].startswith('--'):
            project_dir = args[i]
            i += 1
        else:
            i += 1

    if project_dir is None:
        print("用法: python generate_structs.py <project_dir> [选项]")
        print("      python generate_structs.py --print-template")
        print("      python generate_structs.py --print-structs-template")
        print("      python generate_structs.py --self-test")
        print()
        print("选项:")
        print("  --project <name>    内置预设: four_head, m4-ekf")
        print("  --config <file>     自定义 JSON 配置")
        print("  --source <file>     指定 structs.json 路径")
        print("  --struct <name>     只生成指定结构体")
        print("  --check             检查模式 (diff 现有文件)")
        print()
        print("示例:")
        print("  python generate_structs.py . --project four_head")
        print("  python generate_structs.py . --check")
        sys.exit(2)

    project_dir = os.path.abspath(project_dir)
    if not os.path.isdir(project_dir):
        print(f"错误: 目录不存在: {project_dir}")
        sys.exit(2)

    config, cfg_source = resolve_config(project_dir, preset_name, config_path)

    # 确定 structs.json 路径
    if source_path:
        json_path = os.path.join(project_dir, source_path)
    else:
        json_path = os.path.join(project_dir, config.get('structs_source', 'cfg/structs.json'))

    # 如果 structs.json 不存在且不是 --check 模式, 打印友好提示
    if not os.path.exists(json_path) and not check:
        print(f"注意: structs.json 不存在 ({json_path})")
        print(f"  运行 --print-structs-template 查看模板格式")
        print(f"  或创建 cfg/structs.json 后重新运行")
        sys.exit(0)

    if not check:
        mode_str = "生成"
    else:
        mode_str = "检查"

    print(f"generate_structs.py — 跨模块结构体代码{mode_str}")
    print(f"项目目录: {project_dir}")
    print(f"规则来源: {cfg_source}")
    print(f"数据源:   {os.path.relpath(json_path, project_dir) if os.path.exists(json_path) else json_path}")
    print()

    if not os.path.exists(json_path):
        print("错误: structs.json 不存在, 无法执行检查")
        sys.exit(2)

    structs_data = load_structs_json(json_path)

    if check:
        mismatches = check_mode(structs_data, config, project_dir, struct_filter)
        if mismatches == 0:
            print(f"\nRESULT: 全部一致 — 0 差异")
            sys.exit(0)
        else:
            print(f"\nRESULT: {mismatches} 个文件不一致.")
            print(f"  请运行 python tools/generate_structs.py 重新生成.")
            sys.exit(1)
    else:
        generated = generate_all(structs_data, config, project_dir, struct_filter)
        print(f"\n[PASS] 生成完成 — {len(generated)} 个模块的 types.h 已更新")
        sys.exit(0)


if __name__ == '__main__':
    main()
