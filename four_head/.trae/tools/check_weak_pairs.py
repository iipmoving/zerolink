#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
check_weak_pairs.py - __weak callback pair consistency checker for v2.0
"""

import os
import sys
import re
import argparse

WEAK_PATTERN = re.compile(r'__weak\s+void\s+(\w+)\s*\(')
STRONG_PATTERN = re.compile(r'^(?!__weak)\s*void\s+(\w+_On\w+)\s*\(')

def find_all_files(base_path, ext):
    """Find all files with given extension"""
    files = []
    for root, dirs, files_in_dir in os.walk(base_path):
        for f in files_in_dir:
            if f.endswith(ext):
                files.append(os.path.join(root, f))
    return files

def find_symbols(filepath, pattern):
    """Find all symbols matching pattern in file"""
    symbols = []
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            for line_num, line in enumerate(f, 1):
                match = pattern.search(line)
                if match:
                    symbols.append((match.group(1), filepath, line_num))
    except:
        pass
    return symbols

def main():
    parser = argparse.ArgumentParser(description='__weak callback pair checker')
    parser.add_argument('path', help='Project path')
    parser.add_argument('--project', required=True, help='Project name')
    parser.add_argument('--v2', action='store_true', help='Enable v2.0 checks')
    args = parser.parse_args()
    
    print(f"=== Weak Pair Consistency Check ({args.project}) ===")
    
    c_files = find_all_files(os.path.join(args.path, 'src'), '.c')
    
    # Collect all weak and strong symbols
    weak_symbols = {}  # name -> [(file, line), ...]
    strong_symbols = {}  # name -> [(file, line), ...]
    
    for filepath in c_files:
        weak = find_symbols(filepath, WEAK_PATTERN)
        strong = find_symbols(filepath, STRONG_PATTERN)
        
        for name, f, line in weak:
            if name not in weak_symbols:
                weak_symbols[name] = []
            weak_symbols[name].append((f, line))
        
        for name, f, line in strong:
            if name not in strong_symbols:
                strong_symbols[name] = []
            strong_symbols[name].append((f, line))
    
    # Check for orphan weak symbols (no matching strong)
    orphan_weak = [name for name in weak_symbols if name not in strong_symbols]
    
    # Check for orphan strong symbols (no matching weak)
    orphan_strong = [name for name in strong_symbols if name not in weak_symbols]
    
    # Check for multiple weak/strong definitions
    multiple_weak = [name for name in weak_symbols if len(weak_symbols[name]) > 1]
    multiple_strong = [name for name in strong_symbols if len(strong_symbols[name]) > 1]
    
    # Collect violations
    violations = []
    
    if orphan_weak:
        violations.append("[ORPHAN WEAK] Symbols with no strong implementation:")
        for name in orphan_weak:
            f, line = weak_symbols[name][0]
            violations.append(f"  - {name} defined at {os.path.basename(f)}:{line}")
    
    if orphan_strong:
        violations.append("[ORPHAN STRONG] Symbols with no weak declaration:")
        for name in orphan_strong:
            f, line = strong_symbols[name][0]
            violations.append(f"  - {name} defined at {os.path.basename(f)}:{line}")
    
    if multiple_weak:
        violations.append("[MULTIPLE WEAK] Symbols defined as weak multiple times:")
        for name in multiple_weak:
            locations = [f"{os.path.basename(f)}:{line}" for f, line in weak_symbols[name]]
            violations.append(f"  - {name}: {', '.join(locations)}")
    
    if multiple_strong:
        violations.append("[MULTIPLE STRONG] Symbols defined as strong multiple times:")
        for name in multiple_strong:
            locations = [f"{os.path.basename(f)}:{line}" for f, line in strong_symbols[name]]
            violations.append(f"  - {name}: {', '.join(locations)}")
    
    # v2.0: Check _OnInput pattern
    if args.v2:
        oninput_weak = [name for name in weak_symbols if name.endswith('_OnInput')]
        oninput_strong = [name for name in strong_symbols if name.endswith('_OnInput')]
        
        if not oninput_weak and not oninput_strong:
            violations.append("[v2.0] No _OnInput callbacks found (expected for Switcher mode)")
    
    if violations:
        print("\n[FAIL] Found violations:")
        for v in violations:
            print(v)
        print(f"\nTotal violations: {len([v for v in violations if v.startswith('  -')])}")
        sys.exit(1)
    else:
        print("[PASS] All weak pairs are consistent")
        sys.exit(0)

if __name__ == '__main__':
    main()