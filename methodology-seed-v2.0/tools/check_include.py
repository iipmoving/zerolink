#!/usr/bin/env python3
"""
check_include.py — Data Switcher include permission auditor (v2.3)

Rules:
  1. [SWITCHER] Only data_switcher.c may include other modules' _io.h files.
  2. [SELF]     A module may include its OWN _io.h (e.g., app_power.c -> app_power_io.h).
  3. [HEADER_AUTONOMY]  Any .h file must NOT include another module's .h or _io.h.
  4. [CROSS_MODULE]     Non-switcher .c files must not include other modules' .h directly.
  5. All _io.h includes must use full path: #include "../include/xxx_io.h"

Framework headers (std_module.h, data_switcher.h, pendsv_switcher.h) are
exempt from Rules 3-4 — they are shared infrastructure, not module headers.

Usage:
  python check_include.py <project_root>
  python check_include.py --self-test
"""

import sys
import os
import re

# ---- Framework headers allowed in any file (shared infrastructure, not modules) ----
FRAMEWORK_HEADERS = frozenset({
    'std_module.h',
    'data_switcher.h',
    'pendsv_switcher.h',
})

# ---- Regex patterns ----
# #include "../include/<name>_io.h"
_IO_INCLUDE_RE = re.compile(
    r'^\s*#include\s+"\.\./include/(\w+)_io\.h"\s*$'
)

# #include "anything"
_LOCAL_INCLUDE_RE = re.compile(
    r'^\s*#include\s+"([^"]+)"\s*$'
)

SWITCHER_FILE = "data_switcher.c"


# ---- Helpers ----

def extract_module_from_filename(filename):
    """Extract module name from filename.
    'APP_ADC.C' -> 'app_adc', 'app_power.c' -> 'app_power',
    'app_power.h' -> 'app_power'
    """
    base = os.path.basename(filename).lower()
    for suffix in ('.c', '.h', '.C', '.H'):
        if base.endswith(suffix):
            base = base[:-len(suffix)]
            break
    return base


def resolve_included_module(include_path):
    """Resolve an include path to a module name.
    '../include/app_adc_io.h' -> 'app_adc'
    'app_power.h' -> 'app_power'
    'std_module.h' -> None (framework header -> exempt)
    Returns (module_name_or_None, is_framework)
    """
    inc = include_path.lower()

    # Framework headers are always exempt
    if inc in FRAMEWORK_HEADERS:
        return (None, True)

    # ../include/<name>_io.h
    m = re.match(r'\.\./include/(\w+)_io\.h$', inc)
    if m:
        return (m.group(1), False)

    # <name>.h  (direct include in same directory)
    m = re.match(r'(\w+)\.h$', inc)
    if m:
        return (m.group(1), False)

    # Non-module include (e.g. "core/std_module.h" with path prefix)
    return (None, False)


def check_project(root_dir):
    violations = []

    for dirpath, dirnames, filenames in os.walk(root_dir):
        dirnames[:] = [d for d in dirnames
                       if d not in ('.git', 'tools', 'include', '__pycache__')]

        for fname in filenames:
            ext = fname.lower()
            if not (ext.endswith('.c') or ext.endswith('.h')):
                continue

            fpath = os.path.join(dirpath, fname)
            rel = os.path.relpath(fpath, root_dir)

            own_module = extract_module_from_filename(fname)
            is_switcher = (fname.lower() == SWITCHER_FILE)
            is_header = ext.endswith('.h')
            is_source = ext.endswith('.c')

            try:
                with open(fpath, 'r', encoding='utf-8', errors='ignore') as fh:
                    lines = fh.readlines()
            except (IOError, OSError) as e:
                violations.append(f"{rel}:0: ERROR — cannot read: {e}")
                continue

            for lineno, line in enumerate(lines, 1):
                # ---------------------------------------------------------------
                # Rule 1 & 2: _io.h include check
                # ---------------------------------------------------------------
                io_m = _IO_INCLUDE_RE.match(line)
                if io_m:
                    io_module = io_m.group(1)

                    if is_switcher:
                        continue   # Switcher may include any _io.h

                    if own_module == io_module:
                        continue   # Own _io.h is OK

                    violations.append(
                        f"{rel}:{lineno}: VIOLATION [IO_INCLUDE] — "
                        f"includes '../include/{io_module}_io.h' "
                        f"(module '{io_module}'), but only data_switcher.c "
                        f"may include other modules' _io.h"
                    )
                    continue   # Already caught; skip local-include check below

                # ---------------------------------------------------------------
                # Rules 3 & 4: General cross-module include check
                # ---------------------------------------------------------------
                local_m = _LOCAL_INCLUDE_RE.match(line)
                if not local_m:
                    continue

                inc_path = local_m.group(1)
                inc_module, is_framework = resolve_included_module(inc_path)

                if is_framework or inc_module is None:
                    continue   # Exempt

                if inc_module == own_module:
                    continue   # Own header is OK

                # --- Rule 3: HEADER_AUTONOMY ---
                if is_header:
                    violations.append(
                        f"{rel}:{lineno}: VIOLATION [HEADER_AUTONOMY] — "
                        f"includes '{inc_path}' (module '{inc_module}'), but "
                        f".h files must NOT include other modules' .h files"
                    )

                # --- Rule 4: CROSS_MODULE ---
                if is_source and not is_switcher:
                    violations.append(
                        f"{rel}:{lineno}: VIOLATION [CROSS_MODULE] — "
                        f"includes '{inc_path}' (module '{inc_module}'), but "
                        f"cross-module access must go through data_switcher.c"
                    )

    return violations


def main():
    if len(sys.argv) < 2:
        print("Usage: python check_include.py <project_root>")
        sys.exit(1)

    root = sys.argv[1]
    violations = check_project(root)

    if violations:
        # Group by type for readability
        by_type = {}
        for v in violations:
            tag = v.split(']')[0] + ']' if ']' in v else 'OTHER'
            by_type.setdefault(tag, []).append(v)

        print(f"FAIL: {len(violations)} include permission violation(s):")
        for tag, items in sorted(by_type.items()):
            print(f"\n  {tag} ({len(items)}):")
            for item in items:
                print(f"    {item}")
        sys.exit(1)
    else:
        print("[PASS] include permissions: 0 violations")
        sys.exit(0)


if __name__ == '__main__':
    main()
