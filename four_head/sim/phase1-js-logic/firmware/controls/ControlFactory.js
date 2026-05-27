/**
 * 控件工厂 - 统一创建和管理控件
 */
class ControlFactory {
    constructor(eventBus) {
        this.eventBus = eventBus;
        this.controls = [];
        this.keyMap = new Map();      // 按键名称 -> keyCode映射
        this.ledMap = new Map();      // LED名称 -> LED状态映射
        this.segmentDisplays = [];     // 数码管显示组件

        // 按键名称到keyCode的标准映射
        this.STANDARD_KEY_MAP = {
            '开始 / 暂停': 13,   // KEY_START_PAUSE
            '开始/暂停': 13,
            '启动': 13,
            '暂停': 13,
            '加水': 11,           // KEY_ADD_WATER
            '加水量': 11,
            '水量+': 11,
            '加时间': 12,         // KEY_ADD_TIME
            '时间+': 12,
            '定时+': 12,
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
            '功能1': 1,
            '功能2': 2,
            '功能3': 3,
            '功能4': 4,
            '功能5': 5,
            '功能6': 6,
            '功能7': 7,
            '功能8': 8,
            '功能9': 9,
            '功能10': 10,
            '电源': 0,            // KEY_POWER
            '开/关': 0,
            '开关机': 0
        };

        // LED名称到LED状态的标准映射
        this.STANDARD_LED_MAP = {
            '电源灯': 'power',
            '电源指示灯': 'power',
            '开始灯': 'start_pause',
            '开始指示灯': 'start_pause',
            '暂停灯': 'start_pause',
            '加水灯': 'add_water',
            '水量灯': 'add_water',
            '加时间灯': 'add_time',
            '时间灯': 'add_time',
            '定时灯': 'add_time',
            'M1灯': 'm1',
            'M1 灯': 'm1',
            '功能1灯': 'm1',
            'M2灯': 'm2',
            'M2 灯': 'm2',
            '功能2灯': 'm2',
            'M3灯': 'm3',
            'M3 灯': 'm3',
            '功能3灯': 'm3',
            'M4灯': 'm4',
            'M4 灯': 'm4',
            '功能4灯': 'm4',
            'M5灯': 'm5',
            'M5 灯': 'm5',
            '功能5灯': 'm5',
            'M6灯': 'm6',
            'M6 灯': 'm6',
            '功能6灯': 'm6',
            'M7灯': 'm7',
            'M7 灯': 'm7',
            '功能7灯': 'm7',
            'M8灯': 'm8',
            'M8 灯': 'm8',
            '功能8灯': 'm8',
            'M9灯': 'm9',
            'M9 灯': 'm9',
            '功能9灯': 'm9',
            'M10灯': 'm10',
            'M10 灯': 'm10',
            '功能10灯': 'm10',
            '水量显示灯': 'water_disp',
            'mL指示灯': 'water_disp',
            'ml灯': 'water_disp',
            '定时指示灯': 'time_disp',
            '时间显示灯': 'time_disp'
        };
    }

    /**
     * 加载控件配置
     * @param {Array} controls - 控件配置数组
     */
    loadControls(controls) {
        this.controls = controls;
        this.keyMap.clear();
        this.ledMap.clear();
        this.segmentDisplays = [];

        controls.forEach(control => {
            switch (control.type) {
                case '按键':
                case 'button':
                    this._registerButton(control);
                    break;
                case 'LED灯':
                case 'led':
                    this._registerLed(control);
                    break;
                case '数码管':
                case 'segment':
                case 'display':
                    this._registerSegment(control);
                    break;
            }
        });

        this.eventBus?.emit('controlsLoaded', {
            keys: this.getKeys(),
            leds: this.getLeds(),
            segments: this.getSegments()
        });
    }

    /**
     * 注册按键
     * @private
     */
    _registerButton(control) {
        const keyCode = this.STANDARD_KEY_MAP[control.name];
        if (keyCode !== undefined) {
            this.keyMap.set(control.name, {
                keyCode: keyCode,
                config: control,
                element: null
            });
        } else {
            console.warn(`[ControlFactory] 未知按键: ${control.name}`);
        }
    }

    /**
     * 注册LED
     * @private
     */
    _registerLed(control) {
        const ledKey = this.STANDARD_LED_MAP[control.name];
        if (ledKey) {
            this.ledMap.set(control.name, {
                ledKey: ledKey,
                config: control,
                element: null,
                isOn: false
            });
        }
    }

    /**
     * 注册数码管
     * @private
     */
    _registerSegment(control) {
        this.segmentDisplays.push({
            name: control.name,
            config: control,
            element: null
        });
    }

    /**
     * 处理Canvas点击
     * @param {number} x - 点击X坐标
     * @param {number} y - 点击Y坐标
     * @returns {boolean} 是否命中按键
     */
    handleClick(x, y) {
        for (const [name, keyInfo] of this.keyMap) {
            const config = keyInfo.config;
            if (!config) continue;

            const inRange = (x >= config.x && x <= config.x + (config.width || 40) &&
                           y >= config.y && y <= config.y + (config.height || 40));

            if (inRange) {
                this.eventBus?.emit('keyPressed', {
                    name: name,
                    keyCode: keyInfo.keyCode,
                    x: x,
                    y: y
                });
                return true;
            }
        }
        return false;
    }

    /**
     * 更新LED状态
     * @param {Object} ledState - LED状态对象
     */
    updateLeds(ledState) {
        if (!ledState) return;

        for (const [name, ledInfo] of this.ledMap) {
            let isOn = false;

            const ledKey = ledInfo.ledKey;
            if (ledKey === 'power') {
                isOn = ledState.power || false;
            } else if (ledKey === 'add_water') {
                isOn = ledState.add_water || false;
            } else if (ledKey === 'add_time') {
                isOn = ledState.add_time || false;
            } else if (ledKey === 'water_disp') {
                isOn = ledState.water_disp || false;
            } else if (ledKey === 'time_disp') {
                isOn = ledState.time_disp || false;
            } else if (ledKey.startsWith('m')) {
                const mIndex = parseInt(ledKey.substring(1)) - 1;
                const funcLeds = ledState.func_leds || 0;
                isOn = !!(funcLeds & (1 << mIndex));
            }

            ledInfo.isOn = isOn;

            this.eventBus?.emit('ledStateChange', {
                name: name,
                isOn: isOn,
                config: ledInfo.config
            });
        }
    }

    /**
     * 更新数码管显示
     * @param {Array} segCodes - 段码数组
     * @param {number} dpMask - 小数点掩码
     * @param {number} colonMask - 冒号掩码
     */
    updateSegmentDisplay(segCodes, dpMask, colonMask) {
        if (this.segmentDisplays.length === 0) return;

        const displayStr = this._segToString(segCodes, dpMask, colonMask);

        this.eventBus?.emit('segmentDisplayChange', {
            display: displayStr,
            segCodes: segCodes,
            dpMask: dpMask,
            colonMask: colonMask,
            config: this.segmentDisplays[0].config
        });
    }

    /**
     * 段码转字符串
     * @private
     */
    _segToString(segArray, dpMask = 0, colonMask = 0) {
        const SEG_MAP = {
            0x3F: '0', 0x06: '1', 0x5B: '2', 0x4F: '3',
            0x66: '4', 0x6D: '5', 0x7D: '6', 0x07: '7',
            0x7F: '8', 0x6F: '9',
            0x77: 'A', 0x7C: 'B', 0x39: 'C', 0x5E: 'd',
            0x79: 'E', 0x71: 'F', 0x37: 'N', 0x73: 'P',
            0x40: '-', 0x00: ' '
        };

        let result = [];
        for (let i = 0; i < 4; i++) {
            let char = SEG_MAP[segArray[i]] || '?';
            if (dpMask & (1 << i)) char += '.';
            result.push(char);
        }
        return result.join('');
    }

    /**
     * 获取按键列表
     */
    getKeys() {
        return Array.from(this.keyMap.keys());
    }

    /**
     * 获取LED列表
     */
    getLeds() {
        return Array.from(this.ledMap.keys());
    }

    /**
     * 获取数码管列表
     */
    getSegments() {
        return this.segmentDisplays.map(s => s.name);
    }

    /**
     * 获取按键信息
     * @param {string} name - 按键名称
     * @returns {Object} 按键信息
     */
    getKeyInfo(name) {
        return this.keyMap.get(name);
    }

    /**
     * 获取LED信息
     * @param {string} name - LED名称
     * @returns {Object} LED信息
     */
    getLedInfo(name) {
        return this.ledMap.get(name);
    }

    /**
     * 获取数码管配置
     * @param {number} index - 索引
     * @returns {Object} 数码管配置
     */
    getSegmentConfig(index = 0) {
        return this.segmentDisplays[index]?.config;
    }
}

export { ControlFactory };