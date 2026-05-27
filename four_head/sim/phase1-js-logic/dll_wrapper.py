"""
EMC Logic DLL 调用封装 - 用于对比测试
"""

import ctypes
import os
from ctypes import c_uint8, c_uint32, c_void_p, Structure, POINTER

# ============================================================================
# C结构体定义（匹配emc_logic.h）
# ============================================================================

class SegCode_t(Structure):
    _fields_ = [
        ("seg", c_uint8 * 4),
        ("dp_mask", c_uint8),
        ("colon_mask", c_uint8)
    ]

class LedState_t(Structure):
    _fields_ = [
        ("led_bits", c_uint8 * 16)
    ]

class HardwareInputs_t(Structure):
    _fields_ = [
        ("keys", c_uint8 * 16),
        ("key_count", c_uint8)
    ]

# ============================================================================
# 回调函数类型定义
# ============================================================================

DISP_OUTPUT_CB = ctypes.CFUNCTYPE(None, POINTER(SegCode_t), POINTER(LedState_t))
BUZZER_CB = ctypes.CFUNCTYPE(None, c_uint8)
STATUS_CHANGE_CB = ctypes.CFUNCTYPE(None, c_uint8, c_void_p)
GET_HW_INPUTS_CB = ctypes.CFUNCTYPE(None, POINTER(HardwareInputs_t))

# ============================================================================
# EMC Logic DLL 封装类
# ============================================================================

class EmcLogicDLL:
    def __init__(self, dll_path=None):
        """
        初始化DLL
        
        Args:
            dll_path: DLL文件路径，默认为当前目录下的emc_logic.dll
        """
        if dll_path is None:
            # 尝试多个可能的路径（按优先级）
            possible_paths = [
                # 1. 当前目录（最优先）
                os.path.join(os.path.dirname(__file__), 'emc_logic.dll'),
                # 2. 绝对路径
                r"D:\Projects\projects\emc_framework_v1\03_编码实现\logic\emc_logic.dll",
                # 3. 相对路径
                os.path.join(
                    os.path.dirname(__file__),
                    '..',
                    '..',
                    'projects',
                    'emc_framework_v1',
                    '03_编码实现',
                    'logic',
                    'emc_logic.dll'
                )
            ]
            
            # 找到第一个存在的路径
            for path in possible_paths:
                if os.path.exists(path):
                    dll_path = path
                    break
            
            if dll_path is None:
                raise FileNotFoundError(
                    f"DLL文件未找到！\n"
                    f"尝试的路径:\n" +
                    "\n".join(f"  - {p}" for p in possible_paths)
                )
        
        if not os.path.exists(dll_path):
            raise FileNotFoundError(f"DLL文件不存在: {dll_path}")
        
        print(f"[EmcLogicDLL] 加载DLL: {dll_path}")
        self.dll = ctypes.CDLL(dll_path)
        
        # 输出记录
        self.outputs = []
        
        # 注册回调
        self._setup_callbacks()
    
    def _setup_callbacks(self):
        """设置回调函数"""
        
        # 显示输出回调
        @DISP_OUTPUT_CB
        def disp_output_cb(p_seg, p_led):
            seg_data = {
                'seg': list(p_seg.contents.seg),
                'dp_mask': p_seg.contents.dp_mask,
                'colon_mask': p_seg.contents.colon_mask
            }
            led_data = list(p_led.contents.led_bits)
            
            record = {
                'type': 'display',
                'data': {'seg': seg_data, 'led': led_data}
            }
            self.outputs.append(record)
            print(f"[DLL Output] display: seg={seg_data['seg']}")
        
        # 蜂鸣器回调
        @BUZZER_CB
        def buzzer_cb(cmd):
            record = {
                'type': 'buzzer',
                'data': {'cmd': cmd}
            }
            self.outputs.append(record)
            print(f"[DLL Output] buzzer: cmd={cmd}")
        
        # 状态变化回调
        @STATUS_CHANGE_CB
        def status_change_cb(type_val, p_value):
            if type_val == 1:  # STATUS_STATE_CHANGE
                # 读取状态值
                state = ctypes.cast(p_value, POINTER(c_uint8)).contents.value
                
                record = {
                    'type': 'status',
                    'data': {'type': type_val, 'value': state}
                }
                self.outputs.append(record)
                
                state_names = ['S_POWER_ON', 'S_VERSION', 'S_STANDBY', 
                              'S_FUNC_SELECT', 'S_COOKING']
                state_name = state_names[state] if state < len(state_names) else f'S_{state}'
                print(f"[DLL Output] status: {state_name}")
        
        # 保存回调引用（防止被垃圾回收）
        self.disp_output_cb = disp_output_cb
        self.buzzer_cb = buzzer_cb
        self.status_change_cb = status_change_cb
        
        # 注册到DLL（使用正确的导出函数名）
        self.dll.emc_register_callbacks(
            self.disp_output_cb,
            self.buzzer_cb,
            self.status_change_cb,
            None  # get_hw_inputs 暂不使用
        )
    
    def init(self):
        """初始化逻辑层"""
        print("[DLL] 初始化逻辑层...")
        self.dll.emc_initialize()  # 使用正确的导出函数名
        self.outputs = []  # 清空输出记录
    
    def run_cycle(self):
        """运行一个周期（10ms）"""
        self.dll.emc_run_cycle()  # 使用正确的导出函数名
    
    def key_input(self, key_code, event):
        """
        按键输入
        
        Args:
            key_code: 按键代码 (1-14)
            event: 事件类型 (1=PRESS, 5=RELEASE)
        """
        self.dll.emc_simulate_key_press(c_uint8(key_code), c_uint8(event))  # 使用正确的导出函数名
    
    def get_ctrl(self):
        """获取控制结构指针（用于调试）"""
        return self.dll.emc_logic_get_ctrl()
    
    def clear_outputs(self):
        """清空输出记录"""
        self.outputs = []
    
    def get_outputs(self):
        """获取所有输出记录"""
        return self.outputs.copy()


# ============================================================================
# 测试示例
# ============================================================================

if __name__ == '__main__':
    print("=" * 60)
    print("EMC Logic DLL 测试")
    print("=" * 60)
    
    try:
        # 创建DLL实例
        dll = EmcLogicDLL()
        
        # 初始化
        dll.init()
        
        # 运行几个周期，观察上电自检
        print("\n运行上电自检（3秒）...")
        for i in range(300):  # 300个周期 = 3秒
            dll.run_cycle()
        
        # 测试按键
        print("\n测试M1按键...")
        dll.key_input(1, 1)  # M1按下
        for i in range(5):   # 保持50ms
            dll.run_cycle()
        dll.key_input(1, 5)  # M1释放
        
        # 再运行几个周期
        for i in range(50):
            dll.run_cycle()
        
        # 输出统计
        outputs = dll.get_outputs()
        print(f"\n总输出数: {len(outputs)}")
        
        display_count = sum(1 for o in outputs if o['type'] == 'display')
        buzzer_count = sum(1 for o in outputs if o['type'] == 'buzzer')
        status_count = sum(1 for o in outputs if o['type'] == 'status')
        
        print(f"  显示更新: {display_count}")
        print(f"  蜂鸣器:   {buzzer_count}")
        print(f"  状态变化: {status_count}")
        
        print("\n✅ DLL测试完成")
        
    except Exception as e:
        print(f"\n❌ 错误: {e}")
        import traceback
        traceback.print_exc()
