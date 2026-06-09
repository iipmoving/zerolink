#!/usr/bin/env python3
"""
check_output_callback.py — v2.2 PULL paradigm auditor (InputCallback + OutputCallback)

Scans all .c/.C source files for v1.x legacy patterns that must be eliminated.

Rules:
  1. '_onOutput' identifier — BLOCK (v1.x name, renamed to OutputCallback in v2.2)
  2. ST_OUT in non-MODULE_SKELETON files — BLOCK (v2.2 paradigm uses ST_OUT internally)
  3. __weak OnOutput function definition — BLOCK (removed in v2.2)
  4. void* param in __weak callback (non-MODULE_SKELETON file) — BLOCK
     → v1.x implicit struct pointer; must migrate to Para_Grp_t *pOut
  5. APP→DRV __weak multi-param or void* — BLOCK
     → APP→DRV 方向仅允许单标量参数或无参数
  6. '@OUTPUT_CALLBACK' marker — whitelist (user-confirmed real-time exception)
  7. '@V1_VOIDPTR' marker — whitelist (transitional v1.x void* callback)
  8. __attribute__((weak)) function name not ending in 'Callback' — WARN
     → All weak functions must use Callback suffix

Usage:
  python check_output_callback.py <project_root>
"""

import sys
import os
import re

SKIP_FILES = {'std_module.h', 'API_comp.c', 'cmsis_armcc.h', 'cmsis_compiler.h',
              'cmsis_gcc.h', 'cmsis_armclang.h', 'cmsis_armclang_ltm.h', 'cmsis_iccarm.h'}

FORBIDDEN = [
    ('_onOutput', '_onOutput identifier — renamed to OutputCallback in v2.2'),
]

# ST_OUT — only BLOCK in non-MODULE_SKELETON files
STOUT_RE = re.compile(r'\bST_OUT\b')

# __weak OnOutput function definition
WEAK_ONOUTPUT_RE = re.compile(
    r'__(?:attribute__\s*\(\s*\(\s*weak\s*\)\s*\)|weak)\s+'
    r'void\s+\w+_OnOutput\s*\('
)

# v1.x void* callback: __weak void Func(uint16_t param, void *data_ptr)
# These hide the actual struct type — must migrate to Para_Grp_t *pOut
VOIDPTR_CALLBACK_RE = re.compile(
    r'__(?:attribute__\s*\(\s*\(\s*weak\s*\)\s*\)|weak)\s+'
    r'(?:void|uint\w+_t|int\w+_t)\s+\w+\s*\([^)]*void\s*\*\s*\w+[^)]*\)'
)

# Rule 5: APP→DRV __weak callbacks — single scalar param or no param only
APP_DRV_WEAK_RE = re.compile(
    r'__(?:attribute__\s*\(\s*\(\s*weak\s*\)\s*\)|weak)\s+'
    r'(?:\w+\s+)??'
    r'(Drv\w*)\s*\(([^)]*)\)'
)

# @OUTPUT_CALLBACK / @V1_VOIDPTR / @ALLOW_NON_CALLBACK_WEAK whitelist markers
WHITELIST_MARKER_RE = re.compile(
    r'@(?:OUTPUT_CALLBACK|V1_VOIDPTR|ALLOW_NON_CALLBACK_WEAK):\s*(.+?)(?:\s*\*/|\s*\n|$)'
)

# Rule 8: weak function name must end with Callback
# Extracts function name after __attribute__((weak))
WEAK_NON_CALLBACK_RE = re.compile(
    r'__attribute__\s*\(\s*\(\s*weak\s*\)\s*\)\s+'
    r'(?:\w+(?:\s*\*)?\s+)*?'       # return type (optional pointer)
    r'(\w+)\s*\('                   # function name
)

# Does a file use MODULE_SKELETON?
HAS_SKELETON_RE = re.compile(r'\bMODULE_SKELETON\s*\(')


def _strip_comments(content):
    """Replace C comments with spaces, preserving character positions."""
    result = list(content)
    i = 0
    n = len(content)
    while i < n:
        # Skip string literals
        if i < n - 1 and content[i] == '"':
            i += 1
            while i < n and content[i] != '"':
                if content[i] == '\\':
                    i += 1
                i += 1
            i += 1
            continue
        # // line comment → replace with spaces until newline
        if i < n - 1 and content[i] == '/' and content[i+1] == '/':
            while i < n and content[i] != '\n':
                result[i] = ' '
                i += 1
            continue
        # /* block comment */ → replace with spaces
        if i < n - 1 and content[i] == '/' and content[i+1] == '*':
            result[i] = ' '
            result[i+1] = ' '
            i += 2
            while i < n - 1 and not (content[i] == '*' and content[i+1] == '/'):
                result[i] = ' '
                i += 1
            if i < n - 1:
                result[i] = ' '
                result[i+1] = ' '
                i += 2
            continue
        i += 1
    return ''.join(result)


def _has_marker_near(content, pos, search_radius=10):
    """Check if a whitelist marker exists within `search_radius` lines of `pos`."""
    line_start = content.rfind('\n', 0, pos) + 1
    search_start = line_start
    for _ in range(search_radius):
        prev = content.rfind('\n', 0, search_start - 1)
        if prev == -1:
            search_start = 0
            break
        search_start = prev + 1
    line_end = content.find('\n', pos)
    if line_end == -1:
        line_end = len(content)
    snippet = content[search_start:line_end]
    return bool(WHITELIST_MARKER_RE.search(snippet))


def extract_whitelist(project_root):
    """Extract approved whitelist entries from source files."""
    approved = []
    for dirpath, dirnames, filenames in os.walk(project_root):
        dirnames[:] = [d for d in dirnames
                       if d not in ('.git', 'tools', '__pycache__', 'include')]
        for fname in filenames:
            if not (fname.endswith('.c') or fname.endswith('.C')):
                continue
            if fname in SKIP_FILES:
                continue
            fpath = os.path.join(dirpath, fname)
            rel = os.path.relpath(fpath, project_root)
            with open(fpath, 'r', encoding='utf-8', errors='ignore') as fh:
                for lineno, line in enumerate(fh, 1):
                    m = WHITELIST_MARKER_RE.search(line)
                    if m:
                        approved.append({
                            'file': rel, 'line': lineno,
                            'reason': m.group(1).strip(),
                        })
    return approved


def _count_params(params_str):
    """Count real parameters, treating 'void' as 0."""
    stripped = params_str.strip()
    if not stripped or stripped.lower() == 'void':
        return 0
    parts = [p.strip() for p in stripped.split(',') if p.strip()]
    return len(parts)


def _has_voidptr(params_str):
    """Check if params string contains void*."""
    return bool(re.search(r'void\s*\*', params_str))


def check_project(project_root):
    violations = []
    warnings = []

    approved = extract_whitelist(project_root)

    for dirpath, dirnames, filenames in os.walk(project_root):
        dirnames[:] = [d for d in dirnames
                       if d not in ('.git', 'tools', '__pycache__', 'include')]
        for fname in filenames:
            if not (fname.endswith('.c') or fname.endswith('.C')):
                continue
            if fname in SKIP_FILES:
                continue
            fpath = os.path.join(dirpath, fname)
            rel = os.path.relpath(fpath, project_root)

            with open(fpath, 'r', encoding='utf-8', errors='ignore') as fh:
                content = fh.read()

            scan_content = _strip_comments(content)
            uses_skeleton = bool(HAS_SKELETON_RE.search(scan_content))
            is_app_file = rel.replace('\\', '/').startswith('app/')

            # Rule 1: '_onOutput' identifier — v1.x name
            for pattern, desc in FORBIDDEN:
                for m in re.finditer(r'\b' + re.escape(pattern) + r'\b', scan_content):
                    if _has_marker_near(content, m.start()):
                        continue
                    lineno = content[:m.start()].count('\n') + 1
                    violations.append(
                        f"{rel}:{lineno}: VIOLATION — '{pattern}' ({desc})"
                    )

            # Rule 2: ST_OUT — BLOCK in non-MODULE_SKELETON files only
            # (MODULE_SKELETON files use ST_OUT internally in the DoWork macro)
            if not uses_skeleton:
                for m in STOUT_RE.finditer(scan_content):
                    if _has_marker_near(content, m.start()):
                        continue
                    lineno = content[:m.start()].count('\n') + 1
                    violations.append(
                        f"{rel}:{lineno}: VIOLATION — 'ST_OUT' in non-MODULE_SKELETON file. "
                        f"v2.2 paradigm: ST_OUT is only valid inside MODULE_SKELETON DoWork"
                    )

            # Rule 3: __weak OnOutput definitions
            for m in WEAK_ONOUTPUT_RE.finditer(scan_content):
                if _has_marker_near(content, m.start()):
                    continue
                lineno = content[:m.start()].count('\n') + 1
                violations.append(
                    f"{rel}:{lineno}: VIOLATION — __weak OnOutput definition "
                    f"(removed in v2.2: use InputCallback PULL routing)"
                )

            # Rule 4: void* callback — v1.x implicit struct pointer
            for m in VOIDPTR_CALLBACK_RE.finditer(scan_content):
                if _has_marker_near(content, m.start()):
                    continue
                lineno = content[:m.start()].count('\n') + 1
                if uses_skeleton:
                    violations.append(
                        f"{rel}:{lineno}: VIOLATION — void* callback in "
                        f"MODULE_SKELETON file (v1.x implicit struct pointer. "
                        f"Migrate to InputCallback pattern: "
                        f"middle layer reads from s_slot[].pOut, writes to s_slot[].pIn"
                    )
                else:
                    warnings.append(
                        f"{rel}:{lineno}: WARN — void* callback "
                        f"(v1.x implicit struct pointer. "
                        f"Plan migration to InputCallback PULL routing)"
                    )

            # Rule 5: APP→DRV __weak callbacks
            if is_app_file:
                for m in APP_DRV_WEAK_RE.finditer(scan_content):
                    if _has_marker_near(content, m.start()):
                        continue
                    func_name = m.group(1)
                    params_str = m.group(2)
                    n_params = _count_params(params_str)
                    has_voidptr = _has_voidptr(params_str)
                    lineno = content[:m.start()].count('\n') + 1
                    if has_voidptr:
                        violations.append(
                            f"{rel}:{lineno}: VIOLATION — APP→DRV __weak "
                            f"'{func_name}' has void* struct pointer. "
                            f"APP→DRV 禁止传结构体指针, 仅允许标量参数. "
                            f"Add '@V1_VOIDPTR: <reason>' for transitional exception"
                        )
                    elif n_params > 1:
                        warnings.append(
                            f"{rel}:{lineno}: WARN — APP→DRV __weak "
                            f"'{func_name}' has {n_params} params. "
                            f"APP→DRV 建议单标量参数或无参数, 考虑合并为结构体通过标准 PULL 路由"
                        )

            # Rule 8: all __attribute__((weak)) functions must end with 'Callback'
            for m in WEAK_NON_CALLBACK_RE.finditer(scan_content):
                func_name = m.group(1)
                if func_name.endswith('Callback') or func_name.endswith('CallBack'):
                    continue  # already has Callback suffix
                if func_name == 'Error_Handler':
                    continue  # CMSIS standard, not our convention
                if _has_marker_near(content, m.start()):
                    continue
                lineno = content[:m.start()].count('\n') + 1
                warnings.append(
                    f"{rel}:{lineno}: WARN — __weak '{func_name}' missing 'Callback' suffix. "
                    f"All weak functions must end with 'Callback'. "
                    f"Add '@ALLOW_NON_CALLBACK_WEAK: <reason>' to whitelist"
                )

    return violations, warnings, approved


def main():
    if len(sys.argv) < 2:
        print("Usage: python check_output_callback.py <project_root>")
        sys.exit(1)

    root = sys.argv[1]
    violations, warnings, approved = check_project(root)

    if approved:
        print(f"[INFO] @OUTPUT_CALLBACK whitelist ({len(approved)}):")
        for a in approved:
            print(f"  {a['file']}:{a['line']}: {a['reason']}")

    if warnings:
        print(f"\n[WARN] {len(warnings)} warning(s):")
        for w in warnings:
            print(f"  {w}")

    if violations:
        print(f"\nFAIL: {len(violations)} output callback violation(s):")
        for v in violations:
            print(f"  {v}")
        sys.exit(1)
    else:
        print(f"\n[PASS] output callback audit: 0 violations")
        sys.exit(0)


if __name__ == '__main__':
    main()
