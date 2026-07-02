#!/usr/bin/env python3
"""
ih_elec_params.py — Python 参考实现 + C DLL 交叉验证
======================================================
纯 Python 版 IH_CalculateParams() 与 C 版公式逐行一致,
用于交叉验证 C 编译结果.

用法:
  # 纯 Python 计算 (无需编译)
  from ih_elec_params import ih_calc
  r = ih_calc(I_peak=20.0, Vdc=302, phi=60, f_sw=30)

  # C DLL 交叉验证 (需先编译 ih_elec_params.c → .dll/.so)
  python ih_elec_params.py --dll ih_elec_params.dll
  python ih_elec_params.py --dll ih_elec_params.dll --golden golden_batch.json
"""

import math, json, sys, os, ctypes

# ========== 标定常数 ==========
C_FARAD = 0.9e-6
C_UF    = 0.9

# ========== 纯 Python 参考实现 ==========

def ih_calc(I_peak, Vdc=0, phi=0, f_sw=30):
    """与 MATLAB export_golden.m 公式链逐行对齐

    Args:
        I_peak: 谐振电流峰值 (A)
        Vdc:    母线电压均值 (V)
        phi:    相位角 (度)
        f_sw:   开关频率 (kHz)

    Returns:
        dict: 全部电参数
    """
    omega_sw = 2 * math.pi * f_sw * 1e3

    V_C_peak = I_peak / (omega_sw * C_FARAD)

    numerator   = Vdc / 2 + V_C_peak
    denominator = I_peak * omega_sw
    L_uH = (numerator / denominator) * 1e6 if denominator > 0 else 0

    if L_uH <= 0:
        return {'L_uH': 0, 'valid': False}

    LC = (L_uH * 1e-6) * C_FARAD
    f_res_kHz = (1 / (2 * math.pi * math.sqrt(LC))) / 1e3 if LC > 0 else f_sw
    omega_res = 2 * math.pi * f_res_kHz * 1e3

    phi_rad = math.radians(phi)
    tan_phi = math.tan(phi_rad)
    ratio = f_sw / f_res_kHz
    denom = ratio - 1 / ratio
    Q = tan_phi / denom if abs(denom) > 1e-6 else 0

    R = omega_res * (L_uH * 1e-6) / Q if Q > 1e-6 else 0

    I_rms = I_peak / math.sqrt(2)
    P = I_rms ** 2 * R

    X_L = omega_sw * (L_uH * 1e-6)
    X_C = 1 / (omega_sw * C_FARAD)
    X = X_L - X_C
    Z = math.sqrt(R ** 2 + X ** 2)

    return {
        'I_peak':   I_peak,
        'Vdc_mean': Vdc,
        'phi_deg':  phi,
        'f_sw_kHz': f_sw,
        'omega_sw': omega_sw,
        'V_C_peak': V_C_peak,
        'L_uH':     L_uH,
        'f_res_kHz': f_res_kHz,
        'omega_res': omega_res,
        'Q_factor': Q,
        'R_ohm':    R,
        'I_rms':    I_rms,
        'P_W':      P,
        'X_L_ohm':  X_L,
        'X_C_ohm':  X_C,
        'X_ohm':    X,
        'Z_mag_ohm': Z,
        'valid':    L_uH > 1 and f_res_kHz > 1,
    }


# ========== C DLL 封装 ==========

class C_ElecInput(ctypes.Structure):
    _fields_ = [
        ('I_peak_A', ctypes.c_float),
        ('Vdc_mean', ctypes.c_float),
        ('phi_deg',  ctypes.c_float),
        ('f_sw_kHz', ctypes.c_float),
    ]

class C_ElecResult(ctypes.Structure):
    _fields_ = [
        ('I_peak_A',   ctypes.c_float),
        ('Vdc_mean',   ctypes.c_float),
        ('phi_deg',    ctypes.c_float),
        ('f_sw_kHz',   ctypes.c_float),
        ('omega_sw',   ctypes.c_float),
        ('V_C_peak',   ctypes.c_float),
        ('L_uH',       ctypes.c_float),
        ('f_res_kHz',  ctypes.c_float),
        ('omega_res',  ctypes.c_float),
        ('Q_factor',   ctypes.c_float),
        ('R_ohm',      ctypes.c_float),
        ('I_rms',      ctypes.c_float),
        ('P_W',        ctypes.c_float),
        ('X_L_ohm',    ctypes.c_float),
        ('X_C_ohm',    ctypes.c_float),
        ('X_ohm',      ctypes.c_float),
        ('Z_mag_ohm',  ctypes.c_float),
        ('valid',      ctypes.c_bool),
    ]

class CDLLWrapper:
    def __init__(self, dll_path):
        self.dll = ctypes.CDLL(dll_path)
        self.dll.IH_CalculateParams.argtypes = [
            ctypes.POINTER(C_ElecInput),
            ctypes.POINTER(C_ElecResult),
        ]
        self.dll.IH_CalculateParams.restype = None

    def calc(self, I_peak, Vdc=0, phi=0, f_sw=30):
        cin = C_ElecInput(I_peak, Vdc, phi, f_sw)
        cout = C_ElecResult()
        self.dll.IH_CalculateParams(ctypes.byref(cin), ctypes.byref(cout))
        return {
            'I_peak':   cout.I_peak_A,
            'Vdc_mean': cout.Vdc_mean,
            'phi_deg':  cout.phi_deg,
            'f_sw_kHz': cout.f_sw_kHz,
            'omega_sw': cout.omega_sw,
            'V_C_peak': cout.V_C_peak,
            'L_uH':     cout.L_uH,
            'f_res_kHz': cout.f_res_kHz,
            'omega_res': cout.omega_res,
            'Q_factor': cout.Q_factor,
            'R_ohm':    cout.R_ohm,
            'I_rms':    cout.I_rms,
            'P_W':      cout.P_W,
            'X_L_ohm':  cout.X_L_ohm,
            'X_C_ohm':  cout.X_C_ohm,
            'X_ohm':    cout.X_ohm,
            'Z_mag_ohm':cout.Z_mag_ohm,
            'valid':    bool(cout.valid),
        }


# ========== 交叉验证 ==========

def cross_validate(py_result, c_result, tol_pct=0.01):
    """比较 Python 与 C 结果, tol_pct=0.01=1% """
    params = ['L_uH', 'f_res_kHz', 'Q_factor', 'R_ohm', 'I_rms', 'P_W',
              'X_L_ohm', 'X_C_ohm', 'X_ohm', 'Z_mag_ohm', 'V_C_peak']
    max_err = 0
    worst = ''
    for p in params:
        a = py_result.get(p, 0)
        b = c_result.get(p, 0)
        if abs(a) < 1e-12 and abs(b) < 1e-12:
            continue
        denom = max(abs(a), abs(b), 1e-12)
        err = abs(a - b) / denom
        if err > max_err:
            max_err = err
            worst = f'{p}: py={a:.6f} c={b:.6f} err={err*100:.4f}%'
    return max_err, worst


# ========== 测试入口 ==========

def self_test():
    """内置测试用例"""
    cases = [
        {'I_peak': 20.04, 'Vdc': 302.3, 'phi': 59.7, 'f_sw': 29.85,
         'label': '104849 典型'},
        {'I_peak': 10.72, 'Vdc': 50.0,  'phi': 73.6, 'f_sw': 32.96,
         'label': '103925 轻载'},
        {'I_peak': 37.70, 'Vdc': 240.0, 'phi': 48.7, 'f_sw': 23.30,
         'label': '145711 大功率'},
        {'I_peak': 5.0,   'Vdc': 30.0,  'phi': 70.0, 'f_sw': 30.0,
         'label': '极轻载'},
    ]
    print('纯 Python 参考计算结果:')
    print()
    for c in cases:
        r = ih_calc(**{k: c[k] for k in ['I_peak','Vdc','phi','f_sw']})
        print('  %s: L=%6.2fμH  f_res=%6.2fkHz  Q=%5.2f  R=%7.4fΩ  P=%7.1fW' % (
            c['label'], r['L_uH'], r['f_res_kHz'], r['Q_factor'], r['R_ohm'], r['P_W']))


def cross_validate_with_dll(dll_path):
    """与 C DLL 交叉验证"""
    c = CDLLWrapper(dll_path)
    cases = [
        {'I_peak': 20.04, 'Vdc': 302.3, 'phi': 59.7, 'f_sw': 29.85},
        {'I_peak': 10.72, 'Vdc': 50.0,  'phi': 73.6, 'f_sw': 32.96},
        {'I_peak': 37.70, 'Vdc': 240.0, 'phi': 48.7, 'f_sw': 23.30},
    ]
    print('C DLL vs Python 交叉验证:')
    print()
    max_err = 0
    for case in cases:
        py = ih_calc(**case)
        cd = c.calc(**case)
        err, worst = cross_validate(py, cd)
        max_err = max(max_err, err)
        print('  I_peak=%6.2fA: 最大偏差=%7.4f%%  %s' % (
            case['I_peak'], err*100, worst if worst else ''))
    print()
    if max_err < 0.01:
        print('✅ 全部 PASS (最大偏差 %.4f%%)' % (max_err*100))
    else:
        print('❌ 存在偏差 %.4f%%' % (max_err*100))


def cross_validate_with_golden(dll_path, golden_path):
    """与 golden_batch.json 交叉验证"""
    c = CDLLWrapper(dll_path)
    with open(golden_path) as f:
        data = json.load(f)

    total = 0
    passed = 0
    max_err = 0
    worst_case = ''

    for fi in data['files']:
        for cy in fi['cycles']:
            g = cy['golden']
            inp = cy['input']

            py = ih_calc(g['I_peak'], g['Vdc_mean'], g['phi_deg'], g['f_sw_kHz'])
            cd = c.calc(g['I_peak'], g['Vdc_mean'], g['phi_deg'], g['f_sw_kHz'])
            err, worst = cross_validate(py, cd)

            total += 1
            if err < 0.01:
                passed += 1
            if err > max_err:
                max_err = err
                worst_case = f'{fi["name"]} C{cy["idx"]}: {worst}'

    print('C DLL vs Golden 交叉验证 (%d 周期):' % total)
    print('  通过: %d/%d' % (passed, total))
    print('  最大偏差: %.4f%%' % (max_err*100))
    if worst_case:
        print('  最差: %s' % worst_case)
    return passed == total


# ========== 命令行入口 ==========

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='IH 电参数计算 — 交叉验证')
    parser.add_argument('--dll', type=str, default=None,
                        help='C DLL 路径 (交叉验证用)')
    parser.add_argument('--golden', type=str, default=None,
                        help='golden_batch.json 路径 (批量验证用)')
    args = parser.parse_args()

    self_test()
    print()

    if args.dll:
        cross_validate_with_dll(args.dll)
        print()

    if args.golden and args.dll:
        ok = cross_validate_with_golden(args.dll, args.golden)
        sys.exit(0 if ok else 1)
