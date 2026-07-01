#!/usr/bin/env python3
import xml.etree.ElementTree as ET
import os

proj = r'D:\OBSIDIAN\MOVING IH\ZEROLINK\m4_ekf_observer\src\RX32G410_FW_HAL_V1.3N\Projects\Projects\Keil\halfLib.uvprojx'
proj_dir = os.path.dirname(proj)

tree = ET.parse(proj)
root = tree.getroot()

for group_elem in root.iter('Group'):
    gn = group_elem.find('GroupName')
    group_name = gn.text.strip() if gn is not None and gn.text else ''
    files_elem = group_elem.find('Files')
    if files_elem is None:
        continue
    for file_elem in files_elem.iter('File'):
        fn = file_elem.find('FileName')
        fp = file_elem.find('FilePath')
        file_name = fn.text.strip() if fn is not None and fn.text else ''
        file_path = fp.text.strip() if fp is not None and fp.text else ''
        if not file_name or not file_path:
            continue
        ext = os.path.splitext(file_name)[1].lower()
        if ext not in ('.c', '.h', '.cpp'):
            continue
        abs_path = os.path.normpath(os.path.join(proj_dir, file_path))
        rel_path = os.path.relpath(abs_path, proj_dir).replace('\\', '/')
        print("  [%s] %s" % (group_name, rel_path))
