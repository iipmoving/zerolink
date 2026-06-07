#!/usr/bin/env python3
"""
check_output_callback.py — v2.2 PULL paradigm auditor

Scans all .c/.C source files for v1.x legacy patterns that must be eliminated.

Rules:
  1. '_onOutput' identifier — BLOCK (restricted: requires @OUTPUT_CALLBACK whitelist)
  2. 'ST_OUT' constant — BLOCK (restricted: requires @OUTPUT_CALLBACK whitelist)
  3. __weak OnOutput function definition — BLOCK (removed in v2.2)
  4. void* param in __weak callback (non-MODULE_SKELETON file) — BLOCK
     → v1.x implicit struct pointer; must migrate to Para_Grp_t *pOut
  5. APP→DRV __weak multi-param or void* — BLOCK
     → APP→DRV 方向仅允许单标量参数或无参数
  6. '@OUTPUT_CALLBACK' marker — whitelist (user-confirmed real-time exception)
  7. '@V1_VOIDPTR' marker — whitelist (transitional v1.x void* callback)

Usage:
  python check_output_callback.py <project_root>
"""

import sys
import os
import re

SKIP_FILES = {'std_module.h'}

FORBIDDEN = [
    ('_onOutput', '_onOutput identifier — restricted to @OUTPUT_CALLBACK whitelist only'),
    ('ST_OUT',    'ST_OUT (0x04) state bit — restricted to @OUTPUT_CALLBACK whitelist only'),
]

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
# APP 层定义的 __weak Drv* 函数: 只能 0 或 1 个标量参数, 禁止 void* 或多参数
APP_DRV_WEAK_RE = re.compile(
    r'__(?:attribute__\s*\(\s*\(\s*weak\s*\)\s*\)|weak)\s+'
    r'(?:\w+\s+)??'
    r'(Drv\w*)\s*\(([^)]*)\)'
)

# @OUTPUT_CALLBACK / @V1_VOIDPTR whitelist markers
WHITELIST_MARKER_RE = re.compile(
    r'@(?:OUTPUT_CALLBACK|V1_VOIDPTR):\s*(.+?)(?:\s*\*/|\s*\n|$)'
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
    # Search backward a few lines for marker
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

            # Scan stripped content (no comments), but check markers
            # against original content (markers live in comments)
            scan_content = _strip_comments(content)
            uses_skeleton = bool(HAS_SKELETON_RE.search(scan_content))
            is_app_file = rel.replace('\\', '/').startswith('app/')

            # Rule 1+2: Forbidden patterns
            for pattern, desc in FORBIDDEN:
                for m in re.finditer(r'\b' + re.escape(pattern) + r'\b', scan_content):
                    if _has_marker_near(content, m.start()):
                        continue
                    lineno = content[:m.start()].count('\n') + 1
                    violations.append(
                        f"{rel}:{lineno}: VIOLATION — '{pattern}' ({desc})"
                    )

            # Rule 3: __weak OnOutput definitions
            for m in WEAK_ONOUTPUT_RE.finditer(scan_content):
                if _has_marker_near(content, m.start()):
                    continue
                lineno = content[:m.start()].count('\n') + 1
                violations.append(
                    f"{rel}:{lineno}: VIOLATION — __weak OnOutput definition "
                    f"(removed in v2.2: use Switcher PULL routing)"
                )

            # Rule 4: void* callback — v1.x implicit struct pointer
            # BLOCK in MODULE_SKELETON files; WARN in legacy files
            for m in VOIDPTR_CALLBACK_RE.finditer(scan_content):
                if _has_marker_near(content, m.start()):
                    continue
                lineno = content[:m.start()].count('\n') + 1
                if uses_skeleton:
                    violations.append(
                        f"{rel}:{lineno}: VIOLATION — void* callback in "
                        f"MODULE_SKELETON file (v1.x implicit struct pointer. "
                        f"Migrate to 'void {{Consumer}}_On{{Producer}}Data(Para_Grp_t *pOut)' "
                        f"or add '@V1_VOIDPTR: <reason>' marker for transitional exception)"
                    )
                else:
                    warnings.append(
                        f"{rel}:{lineno}: WARN — void* callback "
                        f"(v1.x implicit struct pointer. "
                        f"Plan migration to Para_Grp_t *pOut)"
                    )

            # Rule 5: APP→DRV __weak callbacks
            # void* struct pointer → VIOLATION (结构体指针禁止)
            # multi scalar params → WARN (不鼓励但非一刀切)
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
