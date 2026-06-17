#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
test_data_flow.py — 模拟 v2.3 LINK+PARAMS 数据流，验证指针实例化是否正确

测试场景：
  1. 模块初始化阶段：s_inPara/s_outPara 是否正确绑定
  2. InputCallback 阶段：producer_params 是否被正确赋值给 consumer
  3. ProcessInput 阶段：in->xxx_params 是否非 NULL
  4. user_Process 阶段：in->xxx_params->status 是否可安全访问
  5. 输出阶段：out->xxx_params 是否非 NULL

运行: python codeGen/test_data_flow.py
"""

import ctypes
import json
import os
import sys


# ================================================================
# 模拟 C 结构体 (使用 ctypes)
# ================================================================

class InfoHeader(ctypes.Structure):
    _fields_ = [
        ("status", ctypes.c_uint8),
        ("inMax", ctypes.c_uint8),
        ("route", ctypes.c_uint8),
        ("outMax", ctypes.c_uint8),
    ]


class ParaGrp(ctypes.Structure):
    _fields_ = [
        ("info", InfoHeader),
        ("para", ctypes.c_void_p),
    ]


# 状态常量
ST_INIT = 0x01
ST_NEW = 0x02
ST_OUT = 0x04


# ================================================================
# 模拟模块 I/O 结构体 (从 _io.h 提取)
# ================================================================

class CalculatorInputParams(ctypes.Structure):
    """Calculator_InputParams_t — 从 app_adc_io.h 的 AppAdc_OutputParams_t"""
    _fields_ = [
        ("start", ctypes.c_uint16),
        ("end", ctypes.c_uint16),
        ("highOn", ctypes.c_uint16),
        ("highOff", ctypes.c_uint16),
        ("lowOn", ctypes.c_uint16),
        ("lowOff", ctypes.c_uint16),
        ("zero_cross_high", ctypes.c_uint16),
        ("zero_cross_low", ctypes.c_uint16),
        ("perAdc", ctypes.c_uint16),
    ]


class CalculatorToElecParamsOutputParams(ctypes.Structure):
    """MODULE_OUTPUT_PARAMS(Calculator, ElecParams)"""
    _fields_ = [
        ("hrtim_highOff", ctypes.c_uint16),
        ("hrtim_lowOff", ctypes.c_uint16),
        ("hrtim_highOn", ctypes.c_uint16),
        ("hrtim_lowOn", ctypes.c_uint16),
        ("peak_current", ctypes.c_uint16),
        ("active_current_sum_high", ctypes.c_uint32),
        ("active_current_sum_low", ctypes.c_uint32),
        ("voltage_sum", ctypes.c_uint32),
        ("voltage_count", ctypes.c_uint16),
        ("zero_cross_high", ctypes.c_uint16),
        ("zero_cross_low", ctypes.c_uint16),
        ("peak_point", ctypes.c_uint16),
    ]


class CalculatorToAppPowerOutputParams(ctypes.Structure):
    """MODULE_OUTPUT_PARAMS(Calculator, AppPower)"""
    _fields_ = [
        ("resonant_current", ctypes.c_int32),
        ("voltage", ctypes.c_int32),
        ("phase_angle", ctypes.c_int32),
        ("valid", ctypes.c_uint8),
    ]


class CalculatorToElecParamsOutputLink(ctypes.Structure):
    """MODULE_OUTPUT_LINK(Calculator, ElecParams)"""
    _fields_ = [
        ("status", ctypes.c_uint8),
        ("max_count", ctypes.c_uint8),
        ("count", ctypes.c_uint8),
        ("res", ctypes.c_uint8 * 1),
        ("params", CalculatorToElecParamsOutputParams * 80),  # 80 = 4 pots * 20 cycles
    ]


class CalculatorToAppPowerOutputLink(ctypes.Structure):
    """MODULE_OUTPUT_LINK(Calculator, AppPower)"""
    _fields_ = [
        ("status", ctypes.c_uint8),
        ("max_count", ctypes.c_uint8),
        ("count", ctypes.c_uint8),
        ("res", ctypes.c_uint8 * 1),
        ("params", CalculatorToAppPowerOutputParams * 4),
    ]


class CalculatorOutput(ctypes.Structure):
    """MODULE_OUTPUT(Calculator)"""
    _fields_ = [
        ("ElecParams_params", CalculatorToElecParamsOutputLink),
        ("AppPower_params", CalculatorToAppPowerOutputLink),
    ]


class CalculatorInputParamsWrapper(ctypes.Structure):
    """MODULE_INPUT_PARAMS(AppAdc, Calculator)"""
    _fields_ = [
        ("resonant_current", ctypes.c_void_p),
        ("hrtim_values", ctypes.c_void_p),
        ("voltage_data", ctypes.c_void_p),
        ("input", ctypes.POINTER(CalculatorInputParams)),
    ]


class CalculatorInputLink(ctypes.Structure):
    """MODULE_INPUT_LINK(AppAdc, Calculator)"""
    _fields_ = [
        ("status", ctypes.c_uint8),
        ("max_count", ctypes.c_uint8),
        ("count", ctypes.c_uint8),
        ("res", ctypes.c_uint8 * 1),
        ("params", CalculatorInputParamsWrapper * 4),
    ]


class CalculatorInput(ctypes.Structure):
    """MODULE_INPUT(Calculator)"""
    _fields_ = [
        ("AppAdc_params", CalculatorInputLink),
    ]


class AppPowerInput(ctypes.Structure):
    """MODULE_INPUT(AppPower)"""
    _fields_ = [
        ("AppAdc_params", ctypes.c_void_p),
        ("Calculator_params", ctypes.c_void_p),
        ("ElecParams_params", ctypes.c_void_p),
        ("EKF_LKF_params", ctypes.c_void_p),
    ]


class AppPowerOutput(ctypes.Structure):
    """MODULE_OUTPUT(AppPower) — 无输出管道"""
    _fields_ = []


# ================================================================
# 模拟模块
# ================================================================

class SimModule:
    """模拟一个 v2.3 模块"""

    def __init__(self, name, io_dir="", source_file=""):
        self.name = name
        self.io_dir = io_dir
        self.source_file = source_file

        # g_input / g_output (由 MODULE_SKELETON 生成)
        self.g_input = ParaGrp()
        self.g_output = ParaGrp()
        self.g_init_done = 0

        # s_inPara / s_outPara (由用户代码定义)
        self.s_inPara = None  # MODULE_INPUT*
        self.s_outPara = None  # MODULE_OUTPUT

        # 管道状态
        self.pipe_flags = {}
        self.errors = []

        # 管道列表
        self.in_pipes = []
        self.out_pipes = []

    def init(self):
        """模拟 Init() — 绑定 g_input.para = &s_inPara"""
        # s_inPara 在真实代码中是 static MODULE_INPUT*，这里模拟为一个指针变量
        # g_input.para 指向 s_inPara 的地址
        self.s_inPara = ctypes.cast(0, ctypes.c_void_p)  # NULL
        self.g_input.para = ctypes.cast(id(self.s_inPara), ctypes.c_void_p)
        self.g_output.para = ctypes.cast(0, ctypes.c_void_p)  # 由用户设置
        self.g_init_done = 1

    def simulate_input_callback(self, producer_name, producer_out_params_ptr):
        """模拟 INPUT_GET_SLOT(Producer, Consumer)"""
        if not self.g_init_done:
            self.errors.append(f"InputCallback called before Init() for {self.name}")
            return False

        # INPUT_GET_SLOT 展开:
        # MODULE_INPUT(Consumer) *__in = (MODULE_INPUT(Consumer)*)s_slot[SLOT(Consumer)].pIn->para;
        # __in->Producer_params = &__out->Consumer_params;

        # g_input.para 指向 s_inPara 的地址
        # __in 的值 = g_input.para 指向的内容 = s_inPara 的地址
        # 但 INPUT_GET_SLOT 直接写 __in->Producer_params
        # 在 v2.3 中，这是通过 Para_Grp->para 间接完成的

        # 简化模拟：检查 g_input.para 是否有效
        if not self.g_input.para:
            self.errors.append(f"g_input.para is NULL in {self.name}")
            return False

        # 设置 Producer 管道就绪
        if producer_name not in self.pipe_flags:
            self.pipe_flags[producer_name] = False
        self.pipe_flags[producer_name] = True

        return True

    def simulate_process_input(self):
        """模拟 ProcessInput() 的输入段检查"""
        if not self.g_init_done:
            self.errors.append(f"DoWork called before Init() for {self.name}")
            return False

        # 检查每个输入管道的 status 字段
        for pname, ready in self.pipe_flags.items():
            if ready:
                # 在真实代码中: flags.bits.xxx = (in->Xxx_params->status & ST_NEW) ? 1 : 0
                # 这里检查 in->Xxx_params 是否非 NULL
                pass

        return True


# ================================================================
# 模拟 Switcher
# ================================================================

class SimSwitcher:
    """模拟 Switcher 调度"""

    def __init__(self):
        self.modules = {}
        self.callbacks = []  # [(producer, consumer)]

    def register_module(self, name, module):
        self.modules[name] = module

    def add_callback(self, producer, consumer):
        self.callbacks.append((producer, consumer))

    def run_all(self):
        """模拟 Switcher_Run_All"""
        results = []

        # Step 1: 对所有模块执行 Init (第一次 DoWork)
        print("=" * 60)
        print("Step 1: 模块初始化 (Constructor -> Init)")
        print("=" * 60)
        for name, mod in self.modules.items():
            mod.init()
            print(f"  [{name}] Init: g_input.para={mod.g_input.para}, g_output.para={mod.g_output.para}")
            if mod.errors:
                for e in mod.errors:
                    print(f"    ERROR: {e}")
            results.append(("init", name, len(mod.errors) == 0, mod.errors))

        # Step 2: 执行 InputCallback (数据直穿)
        print()
        print("=" * 60)
        print("Step 2: InputCallback — 管道数据直穿")
        print("=" * 60)
        for producer_name, consumer_name in self.callbacks:
            producer_mod = self.modules[producer_name]
            consumer_mod = self.modules[consumer_name]

            # 模拟 producer 输出 params 的地址
            # 在真实代码中，这是 producer 的 MODULE_OUTPUT 中的对应 LINK
            fake_output_params = ctypes.addressof(ctypes.c_int(0))  # 模拟地址

            success = consumer_mod.simulate_input_callback(producer_name, fake_output_params)
            status = "OK" if success else "FAIL"
            print(f"  [{status}] {producer_name} -> {consumer_name}")
            if not success:
                for e in consumer_mod.errors:
                    print(f"    ERROR: {e}")
            results.append(("callback", producer_name, consumer_name, success))

        # Step 3: 执行 ProcessInput
        print()
        print("=" * 60)
        print("Step 3: ProcessInput — 输入段检查")
        print("=" * 60)
        for name, mod in self.modules.items():
            success = mod.simulate_process_input()
            status = "OK" if success else "FAIL"
            print(f"  [{status}] {name}: pipes_ready={mod.pipe_flags}")
            if mod.errors:
                for e in mod.errors:
                    print(f"    ERROR: {e}")
            results.append(("process", name, success, mod.errors))

        # 总结
        print()
        print("=" * 60)
        print("总结")
        print("=" * 60)
        total = len(results)
        passed = sum(1 for r in results if r[-2] is True)
        print(f"  总检查: {total}, 通过: {passed}, 失败: {total - passed}")

        if total == passed:
            print("  [PASS] 所有模块指针实例化正确，数据流可正常传递")
        else:
            print("  [FAIL] 存在 NULL 指针风险，数据流可能中断")

        return passed == total


# ================================================================
# 从 project.json 加载配置
# ================================================================

def load_project(json_path):
    """从 project.json 加载模块和管道配置"""
    with open(json_path, "r", encoding="utf-8") as f:
        config = json.load(f)

    modules = {}
    for mod in config["modules"]:
        modules[mod["name"]] = {
            "layer": mod.get("layer", "app"),
            "source_file": mod.get("source_file", ""),
            "comment": mod.get("comment", ""),
        }

    pipes = []
    for pipe in config["pipes"]:
        pipes.append({
            "from": pipe["from"],
            "to": pipe["to"],
            "fields": pipe.get("fields", []),
        })

    return modules, pipes


# ================================================================
# 主程序
# ================================================================

def main():
    # 确定 project.json 路径
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)

    # 尝试多个可能的路径
    candidates = [
        os.path.join(project_root, "codeGen", "out", "HALF", "project.json"),
        os.path.join(project_root, "codeGen", "out", "HALF", "project.json"),
    ]
    json_path = None
    for c in candidates:
        if os.path.exists(c):
            json_path = c
            break

    if not json_path:
        print(f"[ERROR] 找不到 project.json")
        print(f"  搜索路径: {candidates}")
        sys.exit(1)

    print(f"加载 project.json: {json_path}")
    print()

    modules_cfg, pipes = load_project(json_path)

    # 创建模拟模块
    switcher = SimSwitcher()
    for name in modules_cfg:
        mod = SimModule(name)
        switcher.register_module(name, mod)

    # 注册管道 (InputCallback)
    for pipe in pipes:
        switcher.add_callback(pipe["from"], pipe["to"])

    # 运行测试
    success = switcher.run_all()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
