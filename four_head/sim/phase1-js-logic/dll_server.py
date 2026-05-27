"""
EMC Logic 对比测试后端服务器
提供DLL调用API，供前端JavaScript调用
"""

from flask import Flask, request, jsonify
from flask_cors import CORS
import threading
import time
from dll_wrapper import EmcLogicDLL

app = Flask(__name__)
CORS(app)  # 允许跨域请求

# 全局DLL实例
dll_instance = None
cycle_timer = None
is_running = False

@app.route('/api/init', methods=['POST'])
def init_dll():
    """初始化DLL"""
    global dll_instance
    
    try:
        dll_instance = EmcLogicDLL()
        dll_instance.init()
        
        return jsonify({
            'success': True,
            'message': 'DLL初始化成功'
        })
        
    except Exception as e:
        return jsonify({
            'success': False,
            'error': str(e)
        }), 500

@app.route('/api/start_cycle', methods=['POST'])
def start_cycle():
    """启动周期运行"""
    global cycle_timer, is_running
    
    if is_running:
        return jsonify({
            'success': False,
            'error': '已在运行'
        })
    
    is_running = True
    
    def run_cycles():
        while is_running:
            if dll_instance:
                dll_instance.run_cycle()
            time.sleep(0.01)  # 10ms周期
    
    cycle_timer = threading.Thread(target=run_cycles, daemon=True)
    cycle_timer.start()
    
    return jsonify({
        'success': True,
        'message': '周期运行已启动'
    })

@app.route('/api/stop_cycle', methods=['POST'])
def stop_cycle():
    """停止周期运行"""
    global is_running
    
    is_running = False
    
    return jsonify({
        'success': True,
        'message': '周期运行已停止'
    })

@app.route('/api/key_input', methods=['POST'])
def key_input():
    """按键输入"""
    if not dll_instance:
        return jsonify({
            'success': False,
            'error': 'DLL未初始化'
        })
    
    data = request.json
    key_code = data.get('key_code')
    event = data.get('event')
    
    if key_code is None or event is None:
        return jsonify({
            'success': False,
            'error': '缺少参数: key_code, event'
        })
    
    try:
        dll_instance.key_input(key_code, event)
        
        return jsonify({
            'success': True
        })
        
    except Exception as e:
        return jsonify({
            'success': False,
            'error': str(e)
        })

@app.route('/api/get_outputs', methods=['GET'])
def get_outputs():
    """获取输出记录"""
    if not dll_instance:
        return jsonify({
            'success': False,
            'error': 'DLL未初始化'
        })
    
    outputs = dll_instance.get_outputs()
    
    return jsonify({
        'success': True,
        'outputs': outputs
    })

@app.route('/api/clear_outputs', methods=['POST'])
def clear_outputs():
    """清空输出记录"""
    if not dll_instance:
        return jsonify({
            'success': False,
            'error': 'DLL未初始化'
        })
    
    dll_instance.clear_outputs()
    
    return jsonify({
        'success': True
    })

@app.route('/api/status', methods=['GET'])
def get_status():
    """获取服务器状态"""
    return jsonify({
        'success': True,
        'dll_initialized': dll_instance is not None,
        'cycle_running': is_running
    })

if __name__ == '__main__':
    print("=" * 60)
    print("EMC Logic 对比测试后端服务器")
    print("=" * 60)
    print("\n启动服务器...")
    print("访问地址: http://localhost:5000")
    print("按 Ctrl+C 停止\n")
    
    app.run(host='0.0.0.0', port=5000, debug=True)
