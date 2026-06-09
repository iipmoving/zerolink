#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
check_deps.py - Layer dependency auditor for v2.0 Data Switcher methodology
"""

import os
import sys
import re
import argparse

# Layer rules for four_head
LAYER_RULES = {
    'app': {'allowed': ['app', 'drv', 'proto', 'core'], 'forbidden': ['hal']},
    'drv': {'allowed': ['drv', 'hal', 'core'], 'forbidden': ['app', 'proto']},
    'hal': {'allowed': ['hal', 'core'], 'forbidden': ['app', 'drv', 'proto']},
    'proto': {'allowed': ['proto', 'core'], 'forbidden': ['app', 'drv', 'hal']},
    'core': {'allowed': ['core'], 'forbidden': ['app', 'drv', 'hal', 'proto']}
}

# _io.h files are special - only data_switcher.c can include them
IO_PATTERN = re.compile(r'#include\s*[<"]include/(\w+_io\.h)[>"]')
WEAK_PATTERN = re.compile(r'__weak\s+void\s+(\w+_On\w+)\s*\(')

def find_c_files(base_path):
    """Find all .c files in src/ directory"""
    c_files = []
    for root, dirs, files in os.walk(os.path.join(base_path, 'src')):
        for f in files:
            if f.endswith('.c'):
                c_files.append(os.path.join(root, f))
    return c_files

def get_layer(filepath):
    """Extract layer from file path"""
    parts = filepath.split(os.sep)
    if 'src' in parts:
        idx = parts.index('src') + 1
        if idx < len(parts):
            return parts[idx]
    return None

def check_include_guard(filepath):
    """Check if header uses //#define pattern (L0 blocking)"""
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
        # Check for //#define pattern
        if '//#define' in content:
            return True
    return False

def check_io_h_access(filepath, content):
    """Check if _io.h is accessed correctly"""
    violations = []
    io_matches = IO_PATTERN.findall(content)
    
    for io_file in io_matches:
        # Only data_switcher.c can include _io.h files
        if 'data_switcher.c' not in filepath:
            violations.append(f"  - {os.path.basename(filepath)} includes {io_file} (only data_switcher.c allowed)")
    
    return violations

def check_layer_dependencies(filepath, content):
    """Check if includes violate layer rules"""
    violations = []
    layer = get_layer(filepath)
    
    if layer not in LAYER_RULES:
        return violations
    
    rules = LAYER_RULES[layer]
    include_pattern = re.compile(r'#include\s*[<"](\w+)/')
    includes = include_pattern.findall(content)
    
    for incl in includes:
        # Skip standard includes
        if incl in ['stdint', 'stdbool', 'string', 'stdio']:
            continue
        
        # Check forbidden includes
        if incl in rules['forbidden']:
            violations.append(f"  - {os.path.basename(filepath)} includes {incl}/ (forbidden in {layer} layer)")
    
    return violations

def main():
    parser = argparse.ArgumentParser(description='Layer dependency auditor')
    parser.add_argument('path', help='Project path')
    parser.add_argument('--project', required=True, help='Project name')
    parser.add_argument('--v2', action='store_true', help='Enable v2.0 checks')
    args = parser.parse_args()
    
    print(f"=== Layer Dependency Audit ({args.project}) ===")
    
    c_files = find_c_files(args.path)
    all_violations = []
    
    for filepath in c_files:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        # Check _io.h access (v2.0 only)
        if args.v2:
            io_violations = check_io_h_access(filepath, content)
            all_violations.extend(io_violations)
        
        # Check layer dependencies
        layer_violations = check_layer_dependencies(filepath, content)
        all_violations.extend(layer_violations)
    
    # Check header files for L0 blocking
    for root, dirs, files in os.walk(os.path.join(args.path, 'src')):
        for f in files:
            if f.endswith('.h') and not f.endswith('_io.h'):
                filepath = os.path.join(root, f)
                if not check_include_guard(filepath):
                    all_violations.append(f"  - {filepath} does not use //#define (L0 blocking required)")
    
    if all_violations:
        print("\n[FAIL] Found violations:")
        for v in all_violations:
            print(v)
        print(f"\nTotal violations: {len(all_violations)}")
        sys.exit(1)
    else:
        print("[PASS] No dependency violations found")
        sys.exit(0)

if __name__ == '__main__':
    main()