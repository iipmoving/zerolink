"""ctypes bridge: 加载 power_calculator.dll, 映射 C 结构体/函数"""
import ctypes as ct
import os

_DLL_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
_DLL_PATH = os.path.join(_DLL_DIR, "power_calculator.dll")

_lib = ct.CDLL(_DLL_PATH)


# ---- C 结构体映射 ----

class PowerCalculatorInputDef(ct.Structure):
    _fields_ = [
        ("start",           ct.c_uint16),
        ("end",             ct.c_uint16),
        ("highOn",          ct.c_uint16),
        ("highOff",         ct.c_uint16),
        ("lowOn",           ct.c_uint16),
        ("lowOff",          ct.c_uint16),
        ("zero_cross_high", ct.c_uint16),
        ("zero_cross_low",  ct.c_uint16),
        ("perAdc",          ct.c_uint16),
        ("lagDuty",         ct.c_uint16),
    ]

class PowerResult(ct.Structure):
    _fields_ = [
        ("active_power",    ct.c_int32),
        ("active_current",  ct.c_int32),
        ("peak_current",    ct.c_uint16),
        ("zero_current",    ct.c_uint16),
        ("phase_angleUp",   ct.c_int16),
        ("phase_angleDown", ct.c_int16),
        ("zero_cross_high", ct.c_uint16),
        ("zero_cross_low",  ct.c_uint16),
        ("esr",             ct.c_uint16),
        ("voltage",         ct.c_uint16),
        ("P_W",             ct.c_float),
        ("I_rms",           ct.c_float),
        ("I_peak",          ct.c_float),
        ("Vdc_mean",        ct.c_float),
        ("phi_deg",         ct.c_float),
    ]

class ElecParamsDef(ct.Structure):
    _fields_ = [
        ("f_sw_kHz",    ct.c_float),
        ("f_res_kHz",   ct.c_float),
        ("D_U_pct",     ct.c_float),
        ("DT1_us",      ct.c_float),
        ("DT2_us",      ct.c_float),
        ("I_rms",       ct.c_float),
        ("I_peak",      ct.c_float),
        ("Vdc_mean",    ct.c_float),
        ("phi_deg",     ct.c_float),
        ("cos_phi",     ct.c_float),
        ("P_W",         ct.c_float),
        ("Z_mag_ohm",   ct.c_float),
        ("R_ohm",       ct.c_float),
        ("X_ohm",       ct.c_float),
        ("L_uH",        ct.c_float),
        ("Q_factor",    ct.c_float),
        ("L_corr_uH",   ct.c_float),
        ("L_kalman_uH", ct.c_float),
        ("anomaly",     ct.c_uint8),
        ("valid",       ct.c_uint8),
    ]


# ---- 函数签名 ----

_lib.DLL_CalculatePower.argtypes = [
    ct.POINTER(ct.c_uint16),   # resonant_current
    ct.POINTER(ct.c_uint16),   # hrtim_values
    ct.POINTER(ct.c_uint16),   # voltage_values
    ct.POINTER(PowerCalculatorInputDef),  # input
]
_lib.DLL_CalculatePower.restype = PowerResult

_lib.DLL_CalculatePower_FPU.argtypes = [
    ct.POINTER(ct.c_uint16),
    ct.POINTER(ct.c_uint16),
    ct.POINTER(ct.c_uint16),
    ct.POINTER(PowerCalculatorInputDef),
]
_lib.DLL_CalculatePower_FPU.restype = PowerResult

# CalculateElecParams_20ms: void func(uint16_t*[4], uint16_t*[4], uint16_t*[4], input*[4], elec[4])
_lib.DLL_CalculateElecParams_20ms.argtypes = [
    ct.POINTER(ct.POINTER(ct.c_uint16)),  # current_buf[4]
    ct.POINTER(ct.POINTER(ct.c_uint16)),  # hrtim_buf[4]
    ct.POINTER(ct.POINTER(ct.c_uint16)),  # voltage_buf[4]
    ct.POINTER(ct.POINTER(PowerCalculatorInputDef)),  # input[4]
    ct.POINTER(ElecParamsDef),            # elec[4]
]
_lib.DLL_CalculateElecParams_20ms.restype = None


# ---- Pythonic wrappers ----

def calculate_power(current_arr, hrtim_arr, voltage_arr, input_def):
    """调用旧版整数 CalculatePower"""
    cur = (ct.c_uint16 * len(current_arr))(*current_arr)
    hrt = (ct.c_uint16 * len(hrtim_arr))(*hrtim_arr)
    vlt = (ct.c_uint16 * len(voltage_arr))(*voltage_arr)
    inp = PowerCalculatorInputDef(**input_def) if isinstance(input_def, dict) else input_def
    return _lib.DLL_CalculatePower(cur, hrt, vlt, ct.byref(inp))


def calculate_power_fpu(current_arr, hrtim_arr, voltage_arr, input_def):
    """调用新版 FPU CalculatePower"""
    cur = (ct.c_uint16 * len(current_arr))(*current_arr)
    hrt = (ct.c_uint16 * len(hrtim_arr))(*hrtim_arr)
    vlt = (ct.c_uint16 * len(voltage_arr))(*voltage_arr)
    inp = PowerCalculatorInputDef(**input_def) if isinstance(input_def, dict) else input_def
    return _lib.DLL_CalculatePower_FPU(cur, hrt, vlt, ct.byref(inp))


def calculate_elec_params_20ms(current_bufs, hrtim_bufs, voltage_bufs, inputs):
    """
    调用 20ms 四头计算.
    每个参数是长度为 4 的 list, 空头填 None (使用 dummy 数据).
    返回长度为 4 的 ElecParamsDef list.
    """
    n_heads = 4
    c_cur = (ct.POINTER(ct.c_uint16) * n_heads)()
    c_hrt = (ct.POINTER(ct.c_uint16) * n_heads)()
    c_vlt = (ct.POINTER(ct.c_uint16) * n_heads)()
    c_inp = (ct.POINTER(PowerCalculatorInputDef) * n_heads)()
    elec  = (ElecParamsDef * n_heads)()

    for i in range(n_heads):
        if i < len(current_bufs) and current_bufs[i] is not None:
            arr = (ct.c_uint16 * len(current_bufs[i]))(*current_bufs[i])
            c_cur[i] = ct.cast(arr, ct.POINTER(ct.c_uint16))
            arr_h = (ct.c_uint16 * len(hrtim_bufs[i]))(*hrtim_bufs[i])
            c_hrt[i] = ct.cast(arr_h, ct.POINTER(ct.c_uint16))
            arr_v = (ct.c_uint16 * len(voltage_bufs[i]))(*voltage_bufs[i])
            c_vlt[i] = ct.cast(arr_v, ct.POINTER(ct.c_uint16))
            inp = PowerCalculatorInputDef(**inputs[i]) if isinstance(inputs[i], dict) else inputs[i]
            c_inp[i] = ct.pointer(inp)
        else:
            # dummy empty
            c_cur[i] = ct.POINTER(ct.c_uint16)()
            c_hrt[i] = ct.POINTER(ct.c_uint16)()
            c_vlt[i] = ct.POINTER(ct.c_uint16)()
            c_inp[i] = ct.POINTER(PowerCalculatorInputDef)()

    _lib.DLL_CalculateElecParams_20ms(c_cur, c_hrt, c_vlt, c_inp, elec)
    return [elec[i] for i in range(n_heads)]


def result_to_dict(r):
    """PowerResult → dict"""
    return {f[0]: getattr(r, f[0]) for f in PowerResult._fields_}


def elec_to_dict(e):
    """ElecParamsDef → dict"""
    return {f[0]: getattr(e, f[0]) for f in ElecParamsDef._fields_}
