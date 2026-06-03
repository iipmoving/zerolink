"""测试套件: C DLL vs MATLAB golden 交叉验证"""
import sys, os, json, argparse
import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ctypes_bridge import (calculate_power_fpu, calculate_power,
                           result_to_dict, elec_to_dict,
                           calculate_elec_params_20ms)
from csv_loader import load_cycles, load_frame_params

# 允许误差
TOL = {
    "P_W":      20.0,   # % — 直接积分 vs 基波法, 允许较大偏差
    "I_rms":    15.0,
    "I_peak":   20.0,
    "Vdc_mean":  5.0,
    "phi_deg":  10.0,   # 绝对误差(度)
    "f_sw_kHz":  5.0,
    "D_U_pct":   5.0,
    "DT1_us":   20.0,
    "DT2_us":   20.0,
}


def run_test(csv_path, golden_cycles=None, verbose=True):
    """对单个 CSV 运行所有周期测试, 返回 [(c_result, golden, errors), ...]"""
    cycles, data = load_cycles(csv_path, min_len=15)
    csv_frames = load_frame_params(csv_path)
    results = []

    for i, (cur, hrt, vlt, inp, s, e) in enumerate(cycles):
        # C 计算结果
        r_fpu = calculate_power_fpu(cur, hrt, vlt, inp)
        r_old = calculate_power(cur, hrt, vlt, inp)
        c_fpu = result_to_dict(r_fpu)
        c_old = result_to_dict(r_old)

        # Golden: 优先用 JSON, 其次 CSV 嵌入参数
        golden = {}
        if golden_cycles and i < len(golden_cycles):
            g = golden_cycles[i].get("golden", {})
            for k, v in g.items():
                if v is not None and not (isinstance(v, float) and np.isnan(v)):
                    golden[k] = v
        if not golden and csv_frames and i < len(csv_frames):
            fp = csv_frames[i]
            mapping = {
                "P_W": "P_W", "I_rms": "I_RMS_A", "I_peak": "I_peak_A",
                "phi_deg": "phi_deg", "f_sw_kHz": "f_sw_kHz",
                "D_U_pct": "D_U_pct", "DT1_us": "DT1_us", "DT2_us": "DT2_us",
                "L_uH": "L_uH", "Q_factor": "Q", "R_ohm": "R_ohm",
                "f_res_kHz": "f_res_kHz",
            }
            for ck, fk in mapping.items():
                if fk in fp:
                    golden[ck] = fp[fk]

        errors = {}
        for key in TOL:
            cv = c_fpu.get(key)
            gv = golden.get(key)
            if cv is not None and gv is not None and gv != 0:
                if key == "phi_deg":
                    err = abs(cv - gv)
                else:
                    err = abs(cv - gv) / max(abs(gv), 0.001) * 100
                errors[key] = err

        result = {
            "cycle": i, "n_pts": len(cur),
            "c_fpu": c_fpu, "c_old": c_old,
            "golden": golden, "errors": errors,
            "input": inp,
        }
        results.append(result)

        if verbose:
            p_err = errors.get("P_W", 999)
            i_err = errors.get("I_rms", 999)
            phi_err = errors.get("phi_deg", 999)
            status = "PASS" if p_err < TOL["P_W"] and i_err < TOL["I_rms"] else "FAIL"
            print(f"  C{i:2d} {status}: P_W={c_fpu['P_W']:7.1f}W (err={p_err:5.1f}%)  "
                  f"I_rms={c_fpu['I_rms']:5.3f}A (err={i_err:5.1f}%)  "
                  f"phi={c_fpu['phi_deg']:5.1f}deg (err={phi_err:5.1f}deg)")

    return results


def run_20ms_test(csv_path, verbose=True):
    """20ms 四头计算测试 (单头数据复用4次)"""
    cycles, data = load_cycles(csv_path, min_len=15)
    if len(cycles) < 4:
        print(f"  Need >=4 cycles for 20ms test, got {len(cycles)}")
        return None

    # 取前4个周期, 模拟4炉头
    cur_bufs = [None]*4
    hrt_bufs = [None]*4
    vlt_bufs = [None]*4
    inputs   = [None]*4
    for i in range(4):
        cur_bufs[i] = cycles[i][0]
        hrt_bufs[i] = cycles[i][1]
        vlt_bufs[i] = cycles[i][2]
        inputs[i]   = cycles[i][3]

    elec = calculate_elec_params_20ms(cur_bufs, hrt_bufs, vlt_bufs, inputs)
    for i, e in enumerate(elec):
        ed = elec_to_dict(e)
        if verbose:
            print(f"  Head {i}: valid={ed['valid']}  P_W={ed['P_W']:.1f}W  "
                  f"L={ed['L_uH']:.1f}uH  L_k={ed['L_kalman_uH']:.1f}uH  "
                  f"Q={ed['Q_factor']:.2f}  f_res={ed['f_res_kHz']:.2f}kHz  "
                  f"anomaly={ed['anomaly']}")
    return [elec_to_dict(e) for e in elec]


def summary(results):
    """打印汇总"""
    n = len(results)
    if n == 0:
        print("No results.")
        return
    passed = 0
    for r in results:
        ok = all(r["errors"].get(k, 999) < TOL[k] for k in TOL if k in r["errors"])
        if ok:
            passed += 1

    print(f"\n{'='*60}")
    print(f"Summary: {passed}/{n} cycles passed")
    for key, tol in TOL.items():
        vals = [r["errors"][key] for r in results if key in r["errors"]]
        if vals:
            print(f"  {key:12s}: mean_err={np.mean(vals):6.1f}  max_err={np.max(vals):6.1f}  tol={tol}")
    print(f"{'='*60}")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("csv", nargs="?", default="../../matlab_half_bridge/r104850.csv")
    ap.add_argument("--golden", help="golden JSON path")
    ap.add_argument("--test-20ms", action="store_true")
    ap.add_argument("-q", action="store_true", help="quiet")
    args = ap.parse_args()

    # 加载 golden
    golden_cycles = None
    if args.golden and os.path.exists(args.golden):
        with open(args.golden) as f:
            gj = json.load(f)
        for fd in gj.get("files", []):
            if os.path.basename(args.csv) == fd.get("name"):
                golden_cycles = fd.get("cycles", [])
                print(f"Loaded {len(golden_cycles)} golden cycles from {args.golden}")
                break

    if args.test_20ms:
        run_20ms_test(args.csv, verbose=not args.q)
    else:
        results = run_test(args.csv, golden_cycles, verbose=not args.q)
        if not args.q:
            summary(results)
