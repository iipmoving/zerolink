#!/usr/bin/env python
"""批量交叉验证: C DLL vs MATLAB Golden — 纯调用, 零公式逻辑"""
import sys, os, json

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'dll_test', 'test'))
from ctypes_bridge import calculate_elec_params_20ms, elec_to_dict
from csv_loader import load_cycles

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
CSV_DIR  = r'D:\OBSIDIAN\MOVING IH\低耦合程序架构\m4_ekf_observer\tools\ekf_tuner/captures'
GOLDEN_PATH = os.path.join(BASE_DIR, 'golden_batch.json')
REPORT_PATH = os.path.join(BASE_DIR, 'batch_validate_report.txt')

# 允许误差 (% 或绝对)
TOL = {
    "P_W": 2.0, "I_rms": 2.0, "I_peak": 2.0, "Vdc_mean": 2.0,
    "phi_deg": 1.0, "f_sw_kHz": 1.0,
    "D_U_pct": 2.0, "DT1_us": 5.0, "DT2_us": 5.0,
    "cos_phi": 0.02, "L_uH": 5.0, "Q_factor": 5.0,
    "R_ohm": 5.0, "Z_mag_ohm": 5.0, "X_ohm": 10.0,
}


def load_golden():
    with open(GOLDEN_PATH, 'r') as f:
        return json.load(f)


def run_c_cycle(cur, hrt, vlt, inp):
    """Call C DLL CalculateElecParams_20ms — single head, zero Python logic"""
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
        return None, f"CSV not found: {csv_path}"

    cycles, _data = load_cycles(csv_path, min_len=15)
    n = min(len(cycles), len(golden_cycles))

    all_errors = []
    all_passed = True
    detail_lines = []
    fail_count = 0

    for i in range(n):
        cur, hrt, vlt, inp, s, e = cycles[i]
        gc = golden_cycles[i]
        m_all = gc['golden']           # MATLAB golden (from file)
        c_all = run_c_cycle(cur, hrt, vlt, inp)  # C DLL

        cycle_errors = {}
        cycle_failed = False
        for param, tol in TOL.items():
            m_val = m_all.get(param)
            c_val = c_all.get(param)
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

        worst = max(cycle_errors.items(), key=lambda x: x[1])
        status = "PASS" if not cycle_failed else "FAIL"

        line = (f"  C{i+1:3d} {status}: "
                f"P={c_all['P_W']:7.1f}W  I_rms={c_all['I_rms']:5.2f}A  "
                f"I_pk={c_all['I_peak']:5.1f}A  phi={c_all['phi_deg']:6.1f}deg  "
                f"f_sw={c_all['f_sw_kHz']:5.1f}kHz  L={c_all['L_uH']:6.1f}uH  "
                f"f_res={c_all['f_res_kHz']:5.1f}kHz  Q={c_all['Q_factor']:5.2f}  "
                f"R={c_all['R_ohm']:6.2f}ohm  worst: {worst[0]}={worst[1]:.2f}")
        detail_lines.append(line)

        if cycle_failed:
            fail_count += 1
            detail_lines.append("    *** FAILED PARAMS ***")
            for param, err in cycle_errors.items():
                if err > TOL[param]:
                    detail_lines.append(
                        f"    {param:>12s}: M={m_all.get(param):.4f}  C={c_all.get(param):.4f}  "
                        f"err={err:.2f} > tol={TOL[param]}")

    return {
        'errors': all_errors, 'passed': all_passed,
        'details': detail_lines, 'fail_count': fail_count,
        'n_cycles': n
    }, None


def param_summary_lines(all_errors):
    if not all_errors:
        return ["  (no data)"]
    lines = []
    lines.append(f"  {'Param':>14s} | {'MeanErr':>8s} | {'MaxErr':>8s} | {'Tol':>8s} | {'Status':>6s}")
    lines.append(f"  {'-'*14}-+-{'-'*8}-+-{'-'*8}-+-{'-'*8}-+-{'-'*6}")
    for param in TOL:
        vals = [e[param] for e in all_errors if param in e]
        if not vals:
            continue
        mean_e = sum(vals) / len(vals)
        max_e = max(vals)
        tol = TOL[param]
        status = "PASS" if max_e <= tol else "FAIL"
        lines.append(f"  {param:>14s} | {mean_e:7.2f} | {max_e:7.2f} | {tol:7.2f} | {status:>6s}")
    return lines


def main():
    lines = []
    def p(s=""):
        lines.append(s)

    p("=" * 80)
    p("Cross-Validation: C DLL (CalculateElecParams_20ms) vs MATLAB Golden")
    p(f"Data: {CSV_DIR}")
    p("=" * 80)

    gj = load_golden()
    file_results = {}
    total_cycles = 0
    total_fails = 0

    for fd in gj.get('files', []):
        csv_name = fd['name']
        golden_cycles = fd.get('cycles', [])

        p()
        p("=" * 80)
        p(f"[{csv_name}]  {len(golden_cycles)} cycles")
        p("=" * 80)

        result, error_msg = test_file(csv_name, golden_cycles)
        if result is None:
            p(f"  ERROR: {error_msg}")
            continue

        for line in result['details']:
            p(line)

        p()
        p("  --- Per-Parameter Summary ---")
        for line in param_summary_lines(result['errors']):
            p(line)

        file_results[csv_name] = result
        total_cycles += result['n_cycles']
        total_fails += result['fail_count']

    # Global Report
    p()
    p("=" * 80)
    p("GLOBAL REPORT")
    p("=" * 80)
    p()
    p(f"  {'File':<40s} {'Cycles':>6s} {'Failed':>6s} {'Status':>8s}")
    p(f"  {'-'*40} {'-'*6} {'-'*6} {'-'*8}")

    total_passed_files = 0
    for csv_name, result in file_results.items():
        status = "PASS" if result['passed'] else "FAIL"
        if result['passed']:
            total_passed_files += 1
        p(f"  {csv_name:<40s} {result['n_cycles']:>6d} {result['fail_count']:>6d} {status:>8s}")

    p()
    p(f"  Files: {total_passed_files}/{len(file_results)} all cycles PASS")
    p(f"  Total cycles: {total_cycles}, failed: {total_fails}")

    all_errors = []
    for result in file_results.values():
        all_errors.extend(result['errors'])

    if all_errors:
        p()
        p(f"  Global Parameter Statistics ({total_cycles} cycles):")
        for line in param_summary_lines(all_errors):
            p(line)

    failed_items = [(n, r['fail_count']) for n, r in file_results.items() if not r['passed']]
    p()
    if failed_items:
        p("  FAILED FILES:")
        for name, fc in failed_items:
            p(f"    {name}: {fc} cycles failed")
    else:
        p("  ALL FILES PASSED")

    p()
    p("=" * 80)
    p("END OF REPORT")
    p("=" * 80)

    report = '\n'.join(lines)
    with open(REPORT_PATH, 'w', encoding='utf-8') as f:
        f.write(report)
    print(report)
    print(f"\nReport saved to: {REPORT_PATH}")


if __name__ == "__main__":
    main()
