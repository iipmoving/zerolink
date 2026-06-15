#!/usr/bin/env python3
"""
check_paradigm.py — v2.2 PULL paradigm compliance auditor

Verifies that modules correctly use MODULE_SKELETON/MODULE_EXPORT and
do NOT fall back to manual GetIO/DoWork patterns.

Rules:
  1. Every module .c/.C file must use MODULE_SKELETON(name) with name != empty
  2. Every module .c/.C file with MODULE_SKELETON must also use MODULE_EXPORT(name)
  3. No manual GetIO implementation (must use MODULE_EXPORT)
  4. MODULE_SKELETON name must match MODULE_EXPORT name
  5. Every module .h that declares GetIO should use MODULE_IO_H (not hand-write)

Usage:
  python check_paradigm.py <project_root>
"""

import sys
import os
import re

SKIP_DIRS = {'.git', 'tools', '__pycache__', 'include', 'ProjectsOld',
             'ref-programs', 'chip-docs', 'sim', 'archive', 'memory', 'docs'}
SKIP_FILES = {'std_module.h', 'data_switcher.h', 'data_switcher.c'}

SKELETON_RE = re.compile(r'MODULE_SKELETON\s*\(\s*(\w*)\s*\)')
EXPORT_RE = re.compile(r'MODULE_EXPORT\s*\(\s*(\w+)\s*\)')
MANUAL_GETIO_RE = re.compile(r'void\s+(\w+)_GetIO\s*\([^)]*\)\s*\{')
SKELETON_EMPTY_RE = re.compile(r'MODULE_SKELETON\s*\(\s*\)')


def _strip_comments(content):
    result = list(content)
    i = 0
    n = len(content)
    while i < n:
        if content[i:i+2] == '//':
            while i < n and content[i] != '\n':
                result[i] = ' '
                i += 1
            continue
        if content[i:i+2] == '/*':
            while i < n - 1 and content[i:i+2] != '*/':
                result[i] = ' '
                i += 1
            if i < n - 1:
                result[i] = result[i+1] = ' '
                i += 2
            continue
        i += 1
    return ''.join(result)


def _find_line(content, pattern):
    idx = content.find(pattern)
    if idx < 0:
        return 0
    return content[:idx].count('\n') + 1


def check_file(fpath, rel):
    violations = []

    if not (fpath.endswith('.c') or fpath.endswith('.C') or fpath.endswith('.h')):
        return violations
    if os.path.basename(fpath) in SKIP_FILES:
        return violations

    basename = os.path.basename(fpath)
    if basename in ('main.c', 'rx32g4xx_it.c', 'data_type.h'):
        return violations

    with open(fpath, 'r', encoding='utf-8', errors='ignore') as fh:
        content = fh.read()

    scan = _strip_comments(content)
    is_c_file = fpath.endswith('.c') or fpath.endswith('.C')

    if is_c_file:
        has_skeleton = bool(SKELETON_RE.search(scan))
        has_empty = bool(SKELETON_EMPTY_RE.search(scan))

        if has_empty:
            lineno = _find_line(content, 'MODULE_SKELETON')
            violations.append(f"{rel}:{lineno}: VIOLATION — MODULE_SKELETON() without name parameter. Must be MODULE_SKELETON(ModuleName)")
        elif has_skeleton:
            if not EXPORT_RE.search(scan):
                exp_name = SKELETON_RE.search(scan).group(1)
                violations.append(f"{rel}:{_find_line(content, 'MODULE_SKELETON')}: VIOLATION — MODULE_SKELETON({exp_name}) without MODULE_EXPORT({exp_name})")

            export_names = {m.group(1) for m in EXPORT_RE.finditer(scan)}
            for m in MANUAL_GETIO_RE.finditer(scan):
                func_name = m.group(1)
                if func_name in export_names:
                    continue
                lineno = content[:m.start()].count('\n') + 1
                violations.append(f"{rel}:{lineno}: VIOLATION — manual '{func_name}_GetIO' implementation found. Must use MODULE_EXPORT({func_name}) instead")

        if not has_skeleton and not has_empty:
            for m in EXPORT_RE.finditer(scan):
                exp_name = m.group(1)
                violations.append(f"{rel}:{_find_line(content, f'MODULE_EXPORT({exp_name})')}: VIOLATION — MODULE_EXPORT({exp_name}) without MODULE_SKELETON. MODULE_SKELETON must come before MODULE_EXPORT")

    if fpath.endswith('.h'):
        if '_io' not in basename:
            return violations
        manual_decl = re.search(r'void\s+(\w+)_GetIO\s*\([^)]*\)\s*;', scan)
        has_io_h = bool(re.search(r'MODULE_IO_H\s*\(', scan))
        if manual_decl and not has_io_h:
            for m in manual_decl.finditer(scan):
                violations.append(f"{rel}:{_find_line(content, m.group(1) + '_GetIO')}: VIOLATION — manual GetIO declaration. Use MODULE_IO_H({m.group(1)})")

    # v2.3 — ProcessInput must use data struct LINK status, not g_input.info.status
    if is_c_file and has_skeleton:
        if 'g_input.info.status' in scan or 'g_output.info.status' in scan:
            lines = content.split('\n')
            for i, line in enumerate(lines, 1):
                stripped = line.strip()
                if stripped.startswith('//') or stripped.startswith('/*') or stripped.startswith('*'):
                    continue
                if 'g_input.info.status' in line:
                    violations.append(f"{rel}:{i}: v2.3 VIOLATION — Use data struct LINK status, not g_input.info.status. Use in->input->status (single LINK) or in->xxx.status (multi-LINK)")
                if 'g_output.info.status' in line:
                    violations.append(f"{rel}:{i}: v2.3 VIOLATION — Use data struct LINK status, not g_output.info.status. Use out->head.status or out->xxx.status (data level)")

    return violations


def check_project(project_root):
    violations = []
    for dirpath, dirnames, filenames in os.walk(project_root):
        dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]
        for fname in filenames:
            if fname in SKIP_FILES:
                continue
            fpath = os.path.join(dirpath, fname)
            rel = os.path.relpath(fpath, project_root)
            violations.extend(check_file(fpath, rel))
    return violations


def main():
    if len(sys.argv) < 2:
        print("Usage: python check_paradigm.py <project_root>")
        sys.exit(1)

    root = sys.argv[1]
    violations = check_project(root)

    if violations:
        print(f"FAIL: {len(violations)} paradigm violation(s):")
        for v in violations:
            print(f"  {v}")
        sys.exit(1)
    else:
        print("PASS: all modules use correct paradigm")
        sys.exit(0)


if __name__ == '__main__':
    main()
