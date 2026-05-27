/**
 * 控件绑定管理器
 * 
 * 根据new.json配置，将UI控件与WASM逻辑层绑定
 * 实现按键点击、LED状态更新、数码管显示等功能
 */

class ControlBinder {
    constructor(adapter, configPath = 'new.json') {
        this.adapter = adapter;
        this.configPath = configPath;
        this.controls = [];
        this.keyMap = new Map();  // 按键名称 -> keyCode映射
        this.ledMap = new Map();  // LED名称 -> LED索引映射
        this.segmentDisplays = []; // 数码管显示组件
        
        // 按键名称到keyCode的映射（根据C代码定义）
        this.KEY_NAME_MAP = {
            '开始 / 暂停': 13,  // KEY_START_PAUSE
            '加水': 11,          // KEY_ADD_WATER
            '加时间': 12,        // KEY_ADD_TIME
            'M1': 1,
            'M2': 2,
            'M3': 3,
            'M4': 4,
            'M5': 5,
            'M6': 6,
            'M7': 7,
            'M8': 8,
            'M9': 9,
            'M10': 10,
            '电源': 0            // KEY_POWER
        };
        
        // LED名称到LED状态的映射
        this.LED_NAME_MAP = {
            '开始 / 暂停灯': 'start_pause',
            '加水灯': 'add_water',
            '加时间灯': 'add_time',
            'M1 灯': 'm1',
            'M2 灯': 'm2',
            'M3 灯': 'm3',
            'M4 灯': 'm4',
            'M5 灯': 'm5',
            'M6 灯': 'm6',
            'M7 灯': 'm7',
            'M8 灯': 'm8',
            'M9 灯': 'm9',
            'M10 灯': 'm10',
            'mL 指示灯': 'water_disp',
            '定时指示灯': 'time_disp'
        };
    }
    
    /**
     * 加载配置并绑定控件
     */
    async loadAndBind() {
        try {
            console.log('[ControlBinder] 加载配置文件:', this.configPath);
            const response = await fetch(this.configPath);
            const config = await response.json();
            
            this.controls = config.controls || [];
            console.log(`[ControlBinder] 加载了 ${this.controls.length} 个控件`);
            
            // 分类控件
            this._categorizeControls();
            
            // ✅ 关键：绑定元素位置信息（必须在handleCanvasClick之前调用）
            this.bindElements();
            
            // 绑定事件
            this._bindEvents();
            
            console.log('[ControlBinder] ✅ 控件绑定完成');
            console.log(`[ControlBinder] 已注册 ${this.keyMap.size} 个按键`);
            this.keyMap.forEach((info, name) => {
                if (info.element) {
                    console.log(`  - ${name}: (${info.element.x}, ${info.element.y}) [${info.element.width}x${info.element.height}]`);
                }
            });
            
            return true;
            
        } catch (error) {
            console.error('[ControlBinder] ❌ 加载配置失败:', error);
            return false;
        }
    }
    
    /**
     * 分类控件
     * @private
     */
    _categorizeControls() {
        this.keyMap.clear();
        this.ledMap.clear();
        this.segmentDisplays = [];
        
        this.controls.forEach(control => {
            if (control.type === '按键') {
                const keyCode = this.KEY_NAME_MAP[control.name];
                if (keyCode !== undefined) {
                    this.keyMap.set(control.name, {
                        keyCode: keyCode,
                        element: null  // 稍后绑定DOM元素
                    });
                    console.log(`[ControlBinder] 注册按键: ${control.name} -> keyCode ${keyCode}`);
                } else {
                    console.warn(`[ControlBinder] 未知按键: ${control.name}`);
                }
            } else if (control.type === 'LED灯') {
                const ledKey = this.LED_NAME_MAP[control.name];
                if (ledKey) {
                    this.ledMap.set(control.name, {
                        ledKey: ledKey,
                        element: null
                    });
                    console.log(`[ControlBinder] 注册LED: ${control.name} -> ${ledKey}`);
                }
            } else if (control.type === '数码管') {
                this.segmentDisplays.push({
                    name: control.name,
                    element: null,
                    config: control
                });
                console.log(`[ControlBinder] 注册数码管: ${control.name}`);
            }
        });
    }
    
    /**
     * 绑定DOM元素和事件
     */
    bindElements(canvasOrContainer) {
        console.log('[ControlBinder] 绑定DOM元素...');
        
        // 这里需要根据实际的UI框架来绑定
        // 如果是Canvas绘制，需要在绘制时记录元素位置
        // 如果是HTML元素，需要查询DOM
        
        // 示例：假设使用Canvas，需要记录每个控件的位置
        this.controls.forEach((control, index) => {
            if (control.type === '按键') {
                const keyInfo = this.keyMap.get(control.name);
                if (keyInfo) {
                    keyInfo.element = {
                        x: control.x,
                        y: control.y,
                        width: control.width,
                        height: control.height,
                        name: control.name
                    };
                }
            } else if (control.type === 'LED灯') {
                const ledInfo = this.ledMap.get(control.name);
                if (ledInfo) {
                    ledInfo.element = {
                        x: control.x,
                        y: control.y,
                        width: control.width,
                        height: control.height,
                        name: control.name
                    };
                }
            } else if (control.type === '数码管') {
                if (this.segmentDisplays[index]) {
                    this.segmentDisplays[index].element = {
                        x: control.x,
                        y: control.y,
                        width: control.width,
                        height: control.height,
                        name: control.name
                    };
                }
            }
        });
        
        console.log('[ControlBinder] ✅ DOM元素绑定完成');
    }
    
    /**
     * 绑定事件监听器
     * @private
     */
    _bindEvents() {
        // 如果使用的是Canvas，需要监听canvas的点击事件
        // 如果使用的是HTML按钮，需要监听button的click事件
        
        // 这里提供通用的事件绑定方法
        console.log('[ControlBinder] 事件绑定就绪');
    }
    
    /**
     * 处理Canvas点击事件
     * @param {number} x - 点击X坐标
     * @param {number} y - 点击Y坐标
     */
    handleCanvasClick(x, y) {
        console.log(`[ControlBinder] 检测点击: (${x}, ${y})`);
        
        // 检查是否点击了按键
        for (const [name, keyInfo] of this.keyMap) {
            if (!keyInfo.element) {
                console.warn(`[ControlBinder] 按键 "${name}" 未绑定element`);
                continue;
            }
            
            const elem = keyInfo.element;
            const inRange = (x >= elem.x && x <= elem.x + elem.width &&
                           y >= elem.y && y <= elem.y + elem.height);
            
            console.log(`  检查 "${name}": 范围=[${elem.x},${elem.y}]-[${elem.x+elem.width},${elem.y+elem.height}], 命中=${inRange}`);
            
            if (inRange) {
                console.log(`[ControlBinder] ✅ 点击按键: ${name} (keyCode=${keyInfo.keyCode})`);
                
                // 发送按键事件到WASM
                this.adapter.clickKey(keyInfo.keyCode, 50).then(() => {
                    console.log(`[ControlBinder] ✅ 按键已发送: ${name}`);
                });
                
                return true;
            }
        }
        
        console.log(`[ControlBinder] ❌ 未找到匹配的按键`);
        return false;
    }
    
    /**
     * 更新LED状态
     * @param {Object} ledState - LED状态对象
     */
    updateLeds(ledState) {
        // ✅ 空值检查
        if (!ledState) {
            console.warn('[ControlBinder] ledState is undefined, skip LED update');
            return;
        }
        
        // 遍历所有LED，根据状态更新显示
        for (const [name, ledInfo] of this.ledMap) {
            if (!ledInfo.element) continue;
            
            const ledKey = ledInfo.ledKey;
            let isOn = false;
            
            try {
                // 根据LED类型判断状态
                if (ledKey === 'start_pause' || ledKey === 'add_water' || 
                    ledKey === 'add_time' || ledKey === 'water_disp' || 
                    ledKey === 'time_disp') {
                    isOn = ledState[ledKey] || false;
                } else if (ledKey.startsWith('m')) {
                    // 功能灯M1-M10，从func_leds位掩码中读取
                    const mIndex = parseInt(ledKey.substring(1)) - 1;
                    const funcLeds = ledState.func_leds || 0;
                    isOn = !!(funcLeds & (1 << mIndex));
                }
                
                // 更新LED显示（需要在UI层实现）
                this._updateLedDisplay(name, isOn);
            } catch (error) {
                console.error(`[ControlBinder] 更新LED "${name}" 失败:`, error);
            }
        }
    }
    
    /**
     * 更新单个LED显示
     * @private
     */
    _updateLedDisplay(ledName, isOn) {
        // 这里需要根据实际UI框架实现
        // Canvas: 重绘LED
        // HTML: 修改CSS类或样式
        
        console.log(`[ControlBinder] LED ${ledName}: ${isOn ? 'ON' : 'OFF'}`);
    }
    
    /**
     * 更新数码管显示
     * @param {Array} segCodes - 段码数组 [seg0, seg1, seg2, seg3]
     * @param {number} dpMask - 小数点掩码
     * @param {number} colonMask - 冒号掩码
     */
    updateSegmentDisplay(segCodes, dpMask, colonMask) {
        if (this.segmentDisplays.length === 0) return;
        
        const display = this.segmentDisplays[0]; // 假设有主显示
        
        // 将段码转换为可读字符串
        const displayStr = this.adapter.segToString(segCodes, dpMask, colonMask);
        
        console.log(`[ControlBinder] 数码管显示: ${displayStr}`);
        
        // 更新显示（需要在UI层实现）
        this._updateSegmentDisplayElement(display, displayStr, segCodes);
    }
    
    /**
     * 更新数码管显示元素
     * @private
     */
    _updateSegmentDisplayElement(display, text, segCodes) {
        // 根据实际UI框架实现
        console.log(`[ControlBinder] 更新${display.name}: "${text}"`, segCodes);
    }
    
    /**
     * 同步更新所有显示
     * 在每个周期后调用此方法
     */
    syncDisplay() {
        if (!this.adapter) return;
        
        // 获取当前状态
        const state = this.adapter.getState();
        const display = this.adapter.getDisplay();
        const led = this.adapter.getLedState();
        
        // 更新LED
        this.updateLeds(led);
        
        // 更新数码管
        this.updateSegmentDisplay(display.seg, display.dp, display.colon);
        
        return {
            state: state,
            stateName: this.adapter.STATE_NAMES[state],
            display: display.display,
            led: led
        };
    }
    
    /**
     * 启动自动同步
     * @param {number} interval - 同步间隔（ms），默认100ms
     */
    startAutoSync(interval = 100) {
        if (this.syncTimer) {
            this.stopAutoSync();
        }
        
        this.syncTimer = setInterval(() => {
            this.syncDisplay();
        }, interval);
        
        console.log(`[ControlBinder] 自动同步已启动 (间隔${interval}ms)`);
    }
    
    /**
     * 停止自动同步
     */
    stopAutoSync() {
        if (this.syncTimer) {
            clearInterval(this.syncTimer);
            this.syncTimer = null;
            console.log('[ControlBinder] 自动同步已停止');
        }
    }
    
    /**
     * 获取所有按键列表
     */
    getKeys() {
        return Array.from(this.keyMap.keys());
    }
    
    /**
     * 获取所有LED列表
     */
    getLeds() {
        return Array.from(this.ledMap.keys());
    }
    
    /**
     * 获取所有数码管列表
     */
    getSegments() {
        return this.segmentDisplays.map(s => s.name);
    }
}

// 导出
if (typeof module !== 'undefined' && module.exports) {
    module.exports = ControlBinder;
}
