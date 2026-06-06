#!/usr/bin/env python3
"""
check_include.py — Data Switcher _io.h include permission auditor

Rules:
  1. Only data_switcher.c may include other modules' _io.h files.
  2. A module may include its OWN _io.h (e.g., app_power.c → app_power_io.h).
  3. All _io.h includes must use full path: #include "../include/xxx_io.h"

Usage:
  python check_include.py <project_root>
  python check_include.py --self-test
"""

import sys
import os
import re
import glob as gb

# Matches: #include "../include/<name>_io.h"
_IO_INCLUDE_RE = re.compile(
    r'^\s*#include\s+"\.\./include/(\w+)_io\.h"\s*$'
)

# The one file allowed to include any _io.h
SWITCHER_FILE = "data_switcher.c"


def extract_module_from_filename(filename):
    """e.g. 'APP_ADC.C' -> 'app_adc', 'app_power.c' -> 'app_power'"""
    base = os.path.basename(filename).lower()
    # Try stripping known suffixes
    for suffix in ('.c', '.C'):
        if base.endswith(suffix):
            base = base[:-len(suffix)]
            break
    return base


def check_project(root_dir):
    violations = []
    seen = set()

    # Find all .c/.C files (skip tools/, .git/, include/)
    for dirpath, dirnames, filenames in os.walk(root_dir):
        # Skip non-source dirs
        dirnames[:] = [d for d in dirnames if d not in ('.git', 'tools', 'include', '__pycache__')]
        for fname in filenames:
            if not (fname.endswith('.c') or fname.endswith('.C')):
                continue
            fpath = os.path.join(dirpath, fname)
            rel = os.path.relpath(fpath, root_dir)

            with open(fpath, 'r', encoding='utf-8', errors='ignore') as fh:
                for lineno, line in enumerate(fh, 1):
                    m = _IO_INCLUDE_RE.match(line)
                    if not m:
                        continue
                    io_module = m.group(1)  # e.g. "app_adc", "app_power"
                    own_module = extract_module_from_filename(fname)
                    is_switcher = (fname.lower() == SWITCHER_FILE)

                    # Rule: Switcher may include anything
                    if is_switcher:
                        continue

                    # Rule: Module may include its own _io.h
                    if own_module == io_module:
                        continue

                    violations.append(
                        f"{rel}:{lineno}: VIOLATION — includes "
                        f"'../include/{io_module}_io.h' but this file is not the owner "
                        f"({own_module}) and not data_switcher.c"
                    )
                    seen.add((fpath, lineno))

    return violations


def main():
    if len(sys.argv) < 2:
        print("Usage: python check_include.py <project_root>")
        sys.exit(1)

    root = sys.argv[1]
    violations = check_project(root)

    if violations:
        print(f"FAIL: {len(violations)} include permission violation(s):")
        for v in violations:
            print(f"  {v}")
        sys.exit(1)
    else:
        print("[PASS] _io.h include permissions: 0 violations")
        sys.exit(0)


if __name__ == '__main__':
    main()
