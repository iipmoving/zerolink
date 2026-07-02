#!/usr/bin/env python
"""全参数逐帧交叉验证: C DLL vs MATLAB Golden — 纯调用, 零公式逻辑"""
import sys, os, json, math

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'dll_test', 'test'))
from ctypes_bridge import calculate_elec_params_20ms, elec_to_dict
from csv_loader import load_cycles

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
CSV_DIR = BASE_DIR
GOLDEN_PATH = os.path.join(BASE_DIR, 'golden.json')
CSV_FILES = ['r104850.csv', 'r104849.csv', 'r104852.csv']

TOL = {
    "P_W":       2.0,
    "I_rms":     2.0,
    "I_peak":    2.0,
    "Vdc_mean":  2.0,
    "phi_deg":   1.0,
    "f_sw_kHz":  1.0,
    "f_res_kHz": 2.0,
    "D_U_pct":   2.0,
    "DT1_us":    5.0,
    "DT2_us":    5.0,
    "cos_phi":   0.02,
    "L_uH":      5.0,
    "Q_factor":  5.0,
    "R_ohm":     5.0,
    "Z_mag_ohm": 5.0,
    "X_ohm":     10.0,
}


def load_golden():
    with open(GOLDEN_PATH, 'r') as f:
        return json.load(f)


def run_c_cycle(cur, hrt, vlt, inp):
    """Call C DLL CalculateElecParams_20ms — single head"""
    current_bufs = [cur, None, None, None]
    hrtim_bufs   = [hrt, None, None, None]
    voltage_bufs = [vlt, None, None, None]
    inputs       = [inp, None, None, None]

    elec = calculate_elec_params_20ms(current_bufs, hrtim_bufs, voltage_bufs, inputs)
    return elec_to_dict(elec[0])


def error_pct(m, c):
    if abs(m) < 1e-9:
        return abs(c - m) * 100.0
    return abs((c - m) / m) * 100.0


def error_abs(m, c):
    return abs(c - m)


def test_file(csv_name, golden_cycles):
    csv_path = os.path.join(CSV_DIR, csv_name)
    if not os.path.exists(csv_path):
        print(f"  CSV not found: {csv_path}")
        return [], False

    cycles, data = load_cycles(csv_path, min_len=15)
    if len(cycles) != len(golden_cycles):
        print(f"  WARNING: CSV has {len(cycles)} cycles, golden has {len(golden_cycles)}")

    n = min(len(cycles), len(golden_cycles))
    all_errors = []
    all_passed = True

    for i in range(n):
        cur, hrt, vlt, inp, s, e = cycles[i]
        gc = golden_cycles[i]

        m_all = gc['golden']                     # MATLAB golden
        c_all = run_c_cycle(cur, hrt, vlt, inp)  # C DLL

        cycle_errors = {}
        cycle_failed = False
        for param, tol in TOL.items():
            m_val = m_all.get(param, None)
            c_val = c_all.get(param, None)
            if m_val is None or c_val is None:
                continue

            if param in ('phi_deg', 'cos_phi'):
                err = error_abs(m_val, c_val)
            else:
                err = error_pct(m_val, c_val)

            cycle_errors[param] = err
            if err > tol:
                cycle_failed = True
                all_passed = False

        all_errors.append(cycle_errors)

        status = "PASS" if not cycle_failed else "FAIL"
        worst = max(cycle_errors.items(), key=lambda x: x[1])
        print(f"  C{i+1:2d} {status}: P_W={c_all['P_W']:6.1f}W  "
              f"I_rms={c_all['I_rms']:5.2f}A  I_pk={c_all['I_peak']:5.1f}A  "
              f"phi={c_all['phi_deg']:5.1f}deg  "
              f"L={c_all['L_uH']:5.1f}uH  f_res={c_all['f_res_kHz']:5.1f}kHz  "
              f"Q={c_all['Q_factor']:4.2f}  R={c_all['R_ohm']:5.2f}ohm  "
              f"worst: {worst[0]}={worst[1]:.2f}")

        if cycle_failed:
            for param, err in cycle_errors.items():
                if err > TOL[param]:
                    m_val = m_all.get(param)
                    c_val = c_all.get(param)
                    print(f"         {param}: M={m_val:.4f} C={c_val:.4f} err={err:.2f} > tol={TOL[param]}")

    return all_errors, all_passed


def print_param_summary(all_errors, file_label):
    if not all_errors:
        return
    params = TOL.keys()
    print(f"\n  {'Param':>12s} | {'MeanErr':>8s} | {'MaxErr':>8s} | {'Tol':>8s} | {'Status':>6s}")
    print(f"  {'-'*12}-+-{'-'*8}-+-{'-'*8}-+-{'-'*8}-+-{'-'*6}")
    for param in params:
        vals = [e[param] for e in all_errors if param in e]
        if not vals:
            continue
        mean_e = sum(vals) / len(vals)
        max_e = max(vals)
        tol = TOL[param]
        status = "PASS" if max_e <= tol else "FAIL"
        print(f"  {param:>12s} | {mean_e:7.2f} | {max_e:7.2f} | {tol:7.2f} | {status:>6s}")


if __name__ == "__main__":
    print("=" * 70)
    print("Cross-Validation: C DLL (CalculateElecParams_20ms) vs MATLAB Golden")
    print("=" * 70)

    gj = load_golden()
    all_file_results = {}

    for csv_name in CSV_FILES:
        golden_cycles = None
        for fd in gj.get('files', []):
            if fd['name'] == csv_name:
                golden_cycles = fd.get('cycles', [])
                break

        if golden_cycles is None:
            print(f"\n[{csv_name}] SKIP: no golden data")
            continue

        print(f"\n{'='*70}")
        print(f"[{csv_name}] {len(golden_cycles)} cycles")
        print(f"{'='*70}")

        errors, passed = test_file(csv_name, golden_cycles)
        all_file_results[csv_name] = (errors, passed)
        print_param_summary(errors, csv_name)

    print(f"\n{'='*70}")
    print("Summary")
    print(f"{'='*70}")

    total_cycles = 0
    total_passed = 0
    for csv_name, (errors, passed) in all_file_results.items():
        n = len(errors)
        total_cycles += n
        if passed:
            total_passed += 1
        status = "PASS" if passed else "FAIL"
        print(f"  {csv_name}: {n} cycles — {status}")

    print(f"\n  Files: {total_passed}/{len(all_file_results)} all cycles PASS")

    all_errors = []
    for errors, _ in all_file_results.values():
        all_errors.extend(errors)

    if all_errors:
        print(f"\n  Global ({total_cycles} cycles):")
        print_param_summary(all_errors, "ALL")

    failures = [n for n, (_, p) in all_file_results.items() if not p]
    if failures:
        print(f"\n  FAIL: {failures}")
    else:
        print(f"\n  ALL PASSED")

    print(f"\n{'='*70}")
    print("Done")
    print(f"{'='*70}")
