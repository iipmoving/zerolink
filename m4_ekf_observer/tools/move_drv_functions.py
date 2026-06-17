#!/usr/bin/env python3
"""
将 app_power_claude.c 中的 DRV 函数剪切到文件底部 #if 0 区域。
保留前向声明，函数体移至底部批量 #if 0。
"""

import re
import os

SRC_FILE = os.path.join(os.path.dirname(__file__),
    '../src/RX32G410_FW_HAL_V1.3N/Projects/LIB/APP/app_power_claude.c')
SRC_FILE = os.path.normpath(SRC_FILE)

# 需要移出的函数名 -> 目标模块
TARGETS = {
    # drv_isr.c
    'API_HRTIM1_TEST_CMP1_IRQHandlerCallback': 'drv_isr',
    'API_ADC_Current1AWD_IRQHandlerCallBack': 'drv_isr',
    'API_ADC_Current2AWD_IRQHandlerCallBack': 'drv_isr',

    # drv_pwm.c
    's_pwm_off': 'drv_pwm',
    's_pwm_on': 'drv_pwm',
    'PPGonOffCh1': 'drv_pwm',
    'PPGonOffCh2': 'drv_pwm',
    'PPGonOffCh3': 'drv_pwm',
    'PPGonOffCh4': 'drv_pwm',
    'PPGdeadTimeCh1': 'drv_pwm',
    'PPGdeadTimeCh2': 'drv_pwm',
    'PPGdeadTimeCh3': 'drv_pwm',
    'PPGdeadTimeCh4': 'drv_pwm',
    'PPGinit': 'drv_pwm',
    'PPGsetHalf': 'drv_pwm',
    'API_HRTIM_PanOffCallBack': 'drv_pwm',

    # drv_pan_pulse.c
    'StartPPG': 'drv_pan_pulse',
    'check_pot_pluse': 'drv_pan_pulse',
    'check_pot_in': 'drv_pan_pulse',

    # drv_pan_count.c
    'APP_POWER_PanCountInitChX': 'drv_pan_count',
    'APP_POWER_PanCountInitCh1': 'drv_pan_count',
    'APP_POWER_PanCountInitCh2': 'drv_pan_count',
    'APP_POWER_PanCountInitCh3': 'drv_pan_count',
    'APP_POWER_PanCountInitCh4': 'drv_pan_count',
    'APP_POWER_PanCountGetValue': 'drv_pan_count',
    'APP_POWER_PanCountSetValue': 'drv_pan_count',
    'API_DMA_PAN_IRQHandlerCallBack': 'drv_pan_count',

    # drv_fmac.c
    'APP_POWER_FmacSetPan': 'drv_fmac',
    'API_POWER_PanFmac': 'drv_fmac',
    'APP_POWER_PanPluseMessage': 'drv_fmac',
    'API_POWER_PanCheckPluse': 'drv_fmac',
    'API_FMAC_AppPowerOverCallBack': 'drv_fmac',

    # drv_protect.c
    'APP_POWER_CompSetValue': 'drv_protect',
    'getOvpValueAdj': 'drv_protect',
    'APP_POWER_SetTxaAwdValue': 'drv_protect',
    'APP_POWERR_SetTxaAwdValue': 'drv_protect',
    'APP_ADC_getOverAdcChannel': 'drv_protect',
    'getTxaDmaCircleValue': 'drv_protect',
}


def find_func_body(lines, start_idx):
    """找到函数体起始和结束行号。返回 (start, end_exclusive) 或 None"""
    # 找到第一个不在字符串/注释中的 {
    brace_line = None
    for i in range(start_idx, min(start_idx + 15, len(lines))):
        line = lines[i]
        if b'(' in line and b')' in line:
            # 检查是否前向声明 (行末是 ; 且没有 {)
            stripped = line.strip()
            if stripped.rstrip().endswith(b';'):
                if b'{' not in line:
                    continue  # 前向声明
            # 找到 {
            if b'{' in line:
                # 确保 { 在 ) 之后
                paren_idx = line.rfind(b')')
                brace_idx = line.find(b'{', paren_idx)
                if brace_idx > paren_idx:
                    brace_line = i
                    break

    if brace_line is None:
        # 可能 { 在下一行
        for i in range(start_idx, min(start_idx + 15, len(lines))):
            line = lines[i]
            if b'(' in line and b')' in line and not line.strip().rstrip().endswith(b';'):
                # 函数签名，检查下一行是否有 {
                if i + 1 < len(lines) and b'{' in lines[i + 1]:
                    brace_line = i + 1
                    break

    if brace_line is None:
        return None

    # 从 brace_line 开始匹配大括号
    depth = 0
    started = False
    for i in range(brace_line, len(lines)):
        line = lines[i]
        for ch in line:
            if ch == ord('{'):
                depth += 1
                started = True
            elif ch == ord('}'):
                depth -= 1
                if started and depth == 0:
                    return (start_idx, i + 1)
        # 安全限制：一页内必须闭合
        if i - brace_line > 200:
            break

    return None


def main():
    with open(SRC_FILE, 'rb') as f:
        lines = f.read().split(b'\r\n')

    print(f'File: {SRC_FILE}')
    print(f'Total lines: {len(lines)}')

    # 查找所有目标函数
    extracted = {}  # module -> [(func_name, start, end, text_lines)]

    for func_name, module in TARGETS.items():
        found = False
        for i in range(len(lines)):
            line = lines[i]
            # 匹配函数定义开头：void|INT8U|uint8_t|INT16U|static 等后跟 func_name(
            # 但不能是前向声明
            pattern = rb'(?:void|INT8U|INT16U|uint8_t|uint16_t|uint32_t|static|int16_t)\s+' + func_name.encode() + rb'\s*\('
            if re.search(pattern, line):
                result = find_func_body(lines, i)
                if result:
                    start, end = result
                    if module not in extracted:
                        extracted[module] = []
                    extracted[module].append((func_name, start, end))
                    print(f'  {func_name}: lines {start+1}-{end} -> {module}')
                    found = True
                    break
        if not found:
            print(f'  {func_name}: NOT FOUND!')

    total_funcs = sum(len(v) for v in extracted.values())
    print(f'\nFound {total_funcs} functions')

    # 收集要删除的行号
    remove_lines = set()
    for module, funcs in extracted.items():
        for func_name, start, end in funcs:
            for j in range(start, end):
                remove_lines.add(j)

    print(f'Lines to remove: {len(remove_lines)}')

    # 构建新文件
    new_lines = []
    for i, line in enumerate(lines):
        if i not in remove_lines:
            new_lines.append(line)

    # 在尾部添加 #if 0 区域 -- 每个函数独立包装，避免内部 #ifdef 嵌套冲突
    header = [
        b'',
        b'/* ============================================================' ,
        b' *  #if 0 section -- DRV functions moved to powerGroup/drv/' ,
        b' *  Each function individually wrapped to avoid nesting with' ,
        b' *  preprocessor directives inside the function body.' ,
        b' * ============================================================ */',
    ]
    new_lines.extend(header)

    for module in sorted(extracted.keys()):
        funcs = extracted[module]
        new_lines.append(b'')
        new_lines.append(b'/* ---- ' + module.encode() + b' ---- */')
        for func_name, start, end in funcs:
            new_lines.append(b'')
            new_lines.append(b'#if 0 /* Moved to powerGroup/drv/' + module.encode() + b'.c */')
            new_lines.append(lines[start])  # signature
            for j in range(start + 1, end - 1):
                new_lines.append(lines[j])
            new_lines.append(lines[end - 1])
            new_lines.append(b'#endif /* ' + func_name.encode() + b' */')

    new_lines.extend([b''])

    output = b'\r\n'.join(new_lines)

    with open(SRC_FILE, 'wb') as f:
        f.write(output)

    print(f'\nDone: {len(lines)} -> {len(new_lines)} lines')
    print(f'Moved {total_funcs} functions to #if 0 section at bottom')


if __name__ == '__main__':
    main()
