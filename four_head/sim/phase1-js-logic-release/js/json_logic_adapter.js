/**
 * JSON规则驱动逻辑层适配器
 * 
 * 基于JSON配置实现LogicLayerAdapter接口
 * 支持声明式配置、元素状态标识化、独立进程等设计理念
 */

class JsonLogicAdapter extends LogicLayerAdapter {
    constructor(jsonConfig, panelConfig = null) {
        super();
        
        this.jsonConfig = jsonConfig;
        this.panelConfig = panelConfig;  // 界面配置（fourSave.json）
        this.headManager = null;
        this.hotHeadManager = null;
        this.powerOnExecutor = null;
        this.globalSM = null;
        
        // 建立keyCode到功能名称的映射
        this.keyCodeMap = this.buildKeyCodeMap(panelConfig);
        
        console.log('[JsonLogicAdapter] 创建JSON逻辑层适配器');
        console.log('[JsonLogicAdapter] keyCode映射表:', this.keyCodeMap);
        
        // 显示输出缓存
        this.displayCache = {
            seg: [0, 0, 0, 0],  // 4个数码管
            dp: 0,               // 小数点
            colon: 0,            // 冒号
            display: ""          // 文本显示
        };
        
        // LED状态缓存
        this.ledCache = {
            power: false,
            child_lock: false,
            zone: false,
            pause: false,
            timer: false,
            head_select: [false, false, false, false],  // 4个炉头选择灯
            power_level: [false, false, false, false, false, false, false, false, false, false]  // 0-9档
        };
        
        console.log('[JsonLogicAdapter] 创建JSON逻辑层适配器');
    }
    
    /**
     * 初始化JSON逻辑层
     * @returns {Promise<void>}
     */
    async init() {
        if (this.initialized) {
            console.log('[JsonLogicAdapter] 已初始化，跳过');
            return;
        }
        
        console.log('[JsonLogicAdapter] 初始化JSON逻辑层...');
        console.log('[JsonLogicAdapter] JSON配置:', this.jsonConfig.meta);
        
        try {
            // 1. 创建炉头实例管理器
            this.headManager = new HeadInstanceManager(
                this.jsonConfig.head_template,
                this.jsonConfig.meta.head_count
            );
            console.log('[JsonLogicAdapter] ✅ 炉头实例管理器创建成功');
            
            // 2. 创建热点炉头管理器
            this.hotHeadManager = new HotHeadManager(this.headManager);
            console.log('[JsonLogicAdapter] ✅ 热点炉头管理器创建成功');
            
            // 3. 执行上电序列
            if (this.jsonConfig.power_on_sequence) {
                this.powerOnExecutor = new PowerOnSequenceExecutor(
                    this.jsonConfig.power_on_sequence,
                    this
                );
                await this.powerOnExecutor.execute();
                console.log('[JsonLogicAdapter] ✅ 上电序列执行完成');
            }
            
            // 4. 创建全局状态机
            if (this.jsonConfig.global_state_machine) {
                this.globalSM = new GlobalStateMachine(
                    this.jsonConfig.global_state_machine,
                    this.hotHeadManager,
                    this  // 传入adapter引用
                );
                console.log('[JsonLogicAdapter] ✅ 全局状态机创建成功');
            }
            
            this.initialized = true;
            console.log('[JsonLogicAdapter] ✅ JSON逻辑层初始化完成');
            
        } catch (error) {
            console.error('[JsonLogicAdapter] ❌ 初始化失败:', error);
            throw error;
        }
    }
    
    /**
     * 运行单个周期
     */
    runCycle() {
        if (!this.initialized) {
            return;
        }
        
        // 1. 更新所有炉头的定时器
        this.headManager.updateAllTimers();
        
        // 2. 更新全局状态机
        if (this.globalSM) {
            this.globalSM.runCycle();
        }
        
        // 3. 更新显示缓存
        this.updateDisplayCache();
        
        // 4. 触发显示回调（与JSEmcLogicAdapter格式一致）
        if (this.callbacks.onDisplayOutput) {
            const segObj = {
                seg: this.displayCache.seg,
                dp_mask: this.displayCache.dp,
                colon_mask: this.displayCache.colon
            };
            const ledArray = this._ledStateToArray(this.getLedState());
            this.callbacks.onDisplayOutput(segObj, ledArray);
        }
    }
    
    /**
     * 模拟按键按下
     * @param {number} keyCode - 按键代码
     * @param {number} eventType - 事件类型
     */
    pressKey(keyCode, eventType) {
        if (!this.initialized) {
            console.warn('[JsonLogicAdapter] 未初始化，忽略按键');
            return;
        }
        
        // TODO: 将keyCode映射到功能名称
        // 目前暂时硬编码映射
        const keyName = this.mapKeyCodeToKeyName(keyCode);
        
        console.log(`[JsonLogicAdapter] 按键: ${keyName} (${keyCode}), 事件: ${eventType}`);
        
        // 转发给全局状态机处理
        if (this.globalSM) {
            this.globalSM.pressKey(keyName, eventType);
        }
    }
    
    /**
     * 获取当前状态码
     * @returns {number}
     */
    getState() {
        if (!this.globalSM) {
            return 0;
        }
        
        // TODO: 返回当前状态码
        return this.globalSM.getState();
    }
    
    /**
     * 获取显示输出
     * @returns {Object} { seg: [4], dp: number, colon: number, display: string }
     */
    getDisplay() {
        return {
            seg: this.displayCache.seg,
            dp: this.displayCache.dp,
            colon: this.displayCache.colon,
            display: this._segToString(this.displayCache.seg, this.displayCache.dp, this.displayCache.colon)
        };
    }
    
    /**
     * 获取LED状态
     * @returns {Object} LED状态对象（与JSEmcLogicAdapter格式一致）
     */
    getLedState() {
        // 将ledCache转换为位域格式
        const ledValue = (
            (this.ledCache.power ? 1 : 0) |
            ((this.ledCache.child_lock ? 1 : 0) << 5) |  // bit5: child_lock
            ((this.ledCache.timer ? 1 : 0) << 6) |       // bit6: timer
            ((this.ledCache.pause ? 1 : 0) << 7) |       // bit7: pause
            ((this.ledCache.head_select[0] ? 1 : 0) << 1) |
            ((this.ledCache.head_select[1] ? 1 : 0) << 2) |
            ((this.ledCache.head_select[2] ? 1 : 0) << 3) |
            ((this.ledCache.head_select[3] ? 1 : 0) << 4)
        );
        
        return {
            power: this.ledCache.power,
            func_leds: ledValue >> 1,
            add_water: false,
            add_time: false,
            water_disp: false,
            time_disp: false,
            // 扩展字段，用于UI层直接读取
            child_lock: this.ledCache.child_lock,
            timer: this.ledCache.timer,
            pause: this.ledCache.pause,
            head_select: [...this.ledCache.head_select]  // 复制数组
        };
    }
    
    /**
     * 获取状态名称映射表
     * @returns {Object}
     */
    getStateNames() {
        return {
            0: "STANDBY",
            1: "POWER_ON",
            2: "HEAD_SELECTED",
            3: "WORKING",
            4: "BOOST",
            5: "PAUSED"
        };
    }
    
    // ========== 私有方法 ==========
    
    /**
     * 从界面配置建立keyCode到功能名称的映射
     */
    buildKeyCodeMap(panelConfig) {
        const map = {};
        
        if (!panelConfig || !panelConfig.controls) {
            console.warn('[JsonLogicAdapter] 没有界面配置，使用默认映射');
            return this.getDefaultKeyCodeMap();
        }
        
        panelConfig.controls.forEach(control => {
            if (control.config && control.config.key_code !== undefined) {
                const keyCode = control.config.key_code;
                const name = control.name;
                
                // 将中文名称转换为英文标识
                const keyName = this.convertControlNameToKey(name);
                
                map[keyCode] = keyName;
                console.log(`[JsonLogicAdapter] 映射: keyCode=${keyCode} -> ${name} (${keyName})`);
            }
        });
        
        return map;
    }
    
    /**
     * 将控件名称转换为JSON中的按键标识
     */
    convertControlNameToKey(name) {
        const nameMap = {
            '定时': 'key_timer',
            '童锁': 'key_child_lock',
            '开关': 'key_power',
            '暂停': 'key_pause',
            '无区': 'key_zone',
            '短暂降低': 'key_minus',
            '短暂升高': 'key_plus',
            'POT1': 'key_head_1',
            'POT2': 'key_head_2',
            'POT3': 'key_head_3',
            'POT4': 'key_head_4'
        };
        
        // 数字键0-9
        if (/^\d+$/.test(name)) {
            return `key_${name}`;
        }
        
        return nameMap[name] || `key_${name.toLowerCase().replace(/\s+/g, '_')}`;
    }
    
    /**
     * 默认keyCode映射（备用）
     */
    getDefaultKeyCodeMap() {
        return {
            0: 'key_unknown_0',
            1: 'key_timer',
            2: 'key_minus',
            3: 'key_child_lock',
            4: 'key_power',
            5: 'key_head_1',
            6: 'key_head_2',
            7: 'key_head_3',
            8: 'key_head_4',
            // 档位键9-18对应0-9
            9: 'key_0',
            10: 'key_1',
            11: 'key_2',
            12: 'key_3',
            13: 'key_4',
            14: 'key_5',
            15: 'key_6',
            16: 'key_7',
            17: 'key_8',
            18: 'key_9'
        };
    }
    
    /**
     * 将段码数组转换为字符串
     */
    _segToString(segArray, dpMask, colonMask) {
        const segMap = {
            0x3F: '0', 0x06: '1', 0x5B: '2', 0x4F: '3',
            0x66: '4', 0x6D: '5', 0x7D: '6', 0x07: '7',
            0x7F: '8', 0x6F: '9',
            0x77: 'A', 0x7C: 'B', 0x39: 'C', 0x5E: 'd',
            0x79: 'E', 0x71: 'F', 0x3D: 'G', 0x76: 'H',
            0x1E: 'J', 0x38: 'L', 0x37: 'N', 0x73: 'P',
            0x3E: 'U',
            0x58: 'c', 0x54: 'n', 0x5C: 'o', 0x50: 'r',
            0x78: 't', 0x6E: 'y',
            0x40: '-', 0x00: ' ', 0x08: '_'
        };
        
        let result = '';
        for (let i = 0; i < segArray.length; i++) {
            const code = segArray[i];
            result += segMap[code] || '?';
            
            // 添加小数点
            if (dpMask & (1 << i)) {
                result += '.';
            }
        }
        
        return result;
    }
    
    /**
     * 更新显示缓存
     */
    updateDisplayCache() {
        // 获取热点炉头
        const hotHead = this.hotHeadManager.getCurrentHotHead();
        
        if (hotHead) {
            // 根据热点炉头的显示模式更新缓存
            const displayMode = hotHead.getDisplayMode();
            
            switch (displayMode) {
                case 'power':
                    this.displayCache.seg = this.renderPowerLevel(hotHead.getPowerLevel());
                    break;
                case 'dash':
                    this.displayCache.seg = this.renderDash();
                    break;
                case 'ascii':
                    this.displayCache.seg = this.renderAscii(hotHead.getAsciiText());
                    break;
                case 'timer':
                    this.displayCache.seg = this.renderTimer(hotHead.getTimerValue());
                    break;
                case 'alternating':
                    // TODO: 实现交替显示
                    this.displayCache.seg = this.renderPowerLevel(hotHead.getPowerLevel());
                    break;
                default:
                    this.displayCache.seg = [0, 0, 0, 0];
            }
        } else {
            // 没有热点炉头，显示待机
            this.displayCache.seg = this.renderDash();
        }
    }
    
    /**
     * 渲染功率档位
     */
    renderPowerLevel(level) {
        // 简单实现：将数字转换为段码
        const segCodes = [0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F];
        const code = segCodes[level] || 0x00;
        return [code, code, code, code];  // 4个数码管显示相同
    }
    
    /**
     * 渲染横杠（待机）
     */
    renderDash() {
        return [0x40, 0x40, 0x40, 0x40];  // '-'
    }
    
    /**
     * 渲染ASCII字符
     */
    renderAscii(text) {
        // TODO: 实现ASCII字符渲染
        return [0x00, 0x00, 0x00, 0x00];
    }
    
    /**
     * 渲染定时时间
     */
    renderTimer(minutes) {
        // TODO: 实现时间渲染
        return [0x00, 0x00, 0x00, 0x00];
    }
    
    /**
     * 映射keyCode到功能名称
     */
    mapKeyCodeToKeyName(keyCode) {
        return this.keyCodeMap[keyCode] || `key_unknown_${keyCode}`;
    }
    
    /**
     * 将LED状态对象转换为数组格式（与JSEmcLogicAdapter一致）
     */
    _ledStateToArray(ledState) {
        // LED位域：bit0=power, bit1-4=head_select[0-3], bit5=child_lock, bit6=timer, bit7=pause
        const ledValue = (
            (ledState.power ? 1 : 0) |
            ((ledState.head_select && ledState.head_select[0] ? 1 : 0) << 1) |
            ((ledState.head_select && ledState.head_select[1] ? 1 : 0) << 2) |
            ((ledState.head_select && ledState.head_select[2] ? 1 : 0) << 3) |
            ((ledState.head_select && ledState.head_select[3] ? 1 : 0) << 4) |
            ((ledState.child_lock ? 1 : 0) << 5) |
            ((ledState.timer ? 1 : 0) << 6) |
            ((ledState.pause ? 1 : 0) << 7)
        );
        
        return [ledValue];
    }
    
    /**
     * 触发显示回调（供GlobalStateMachine等模块调用）
     */
    triggerDisplayCallback() {
        if (this.callbacks.onDisplayOutput) {
            const segObj = {
                seg: this.displayCache.seg,
                dp_mask: this.displayCache.dp,
                colon_mask: this.displayCache.colon
            };
            const ledArray = this._ledStateToArray(this.getLedState());
            this.callbacks.onDisplayOutput(segObj, ledArray);
        }
    }
}

// ========== 辅助类（简化版，后续完善） ==========

class HeadInstanceManager {
    constructor(headTemplate, headCount) {
        this.heads = new Map();
        this.template = headTemplate;
        
        for (let i = 1; i <= headCount; i++) {
            const headId = `head_${i}`;
            this.heads.set(headId, new HeadInstance(headId, headTemplate));
        }
    }
    
    getHead(headId) {
        return this.heads.get(headId);
    }
    
    updateAllTimers() {
        this.heads.forEach(head => head.updateTimers());
    }
}

class HeadInstance {
    constructor(headId, template) {
        this.headId = headId;
        this.state = { ...template.state };
        this.displayMode = 'dash';
        this.asciiText = '';
        this.timerValue = 0;
    }
    
    getDisplayMode() {
        return this.displayMode;
    }
    
    getPowerLevel() {
        return this.state.power_level;
    }
    
    getAsciiText() {
        return this.asciiText;
    }
    
    getTimerValue() {
        return this.timerValue;
    }
    
    updateTimers() {
        // TODO: 更新定时器
    }
}

class HotHeadManager {
    constructor(headManager) {
        this.headManager = headManager;
        this.currentHotHead = null;
    }
    
    switchHotHead(headId) {
        this.currentHotHead = headId;
        console.log(`[HotHeadManager] 切换热点炉头: ${headId}`);
    }
    
    getCurrentHotHead() {
        return this.currentHotHead ? this.headManager.getHead(this.currentHotHead) : null;
    }
}

class PowerOnSequenceExecutor {
    constructor(config, adapter) {
        this.config = config;
        this.adapter = adapter;
    }
    
    async execute() {
        console.log('[PowerOnSequence] 开始执行上电序列...');
        
        for (const step of this.config.steps) {
            console.log(`[PowerOnSequence] 步骤 ${step.step}: ${step.description}`);
            
            for (const action of step.actions) {
                await this.executeAction(action);
            }
        }
        
        console.log('[PowerOnSequence] ✅ 上电序列完成');
        
        // 执行on_complete
        if (this.config.on_complete) {
            console.log(`[PowerOnSequence] ${this.config.on_complete.message}`);
        }
    }
    
    /**
     * 执行单个动作
     */
    async executeAction(action) {
        const { action: actionName, args, set } = action;
        
        console.log(`[PowerOnSequence] 执行动作: ${actionName}`, args || set);
        
        switch (actionName) {
            // ========== 显示相关动作 ==========
            case 'display.all_segments.show':
                this.displayAllSegments(args[0]);
                break;
            
            case 'display.seg_1.show':
                this.displayMultiSegment(0, args[0]);
                break;
            
            case 'display.seg_2.show':
                this.displayMultiSegment(2, args[0]);
                break;
            
            case 'display.seg_all.show':
                this.displayAllSegments(args[0]);
                break;
            
            case 'display.all_leds.on':
                this.setAllLeds(true);
                break;
            
            case 'display.all_leds.off':
                this.setAllLeds(false);
                break;
            
            case 'display.decimal.on':
                this.setDecimalPoint(true);
                break;
            
            case 'display.decimal.off':
                this.setDecimalPoint(false);
                break;
            
            case 'display.seg_all.blink':
                // TODO: 实现闪烁效果
                console.warn('[PowerOnSequence] 闪烁功能暂未实现');
                break;
            
            case 'led.power.on':
                this.setLed('power', true);
                break;
            
            case 'led.power.off':
                this.setLed('power', false);
                break;
            
            case 'led.power.blink':
                // TODO: 实现LED闪烁
                console.warn('[PowerOnSequence] LED闪烁功能暂未实现');
                break;
            
            // ========== 状态相关动作 ==========
            case 'state.mode':
                if (set === 'standby') {
                    this.adapter.displayCache.seg = [0x40, 0x40, 0x40, 0x40]; // --
                    this.triggerDisplayCallback();
                }
                break;
            
            // ========== 延时动作 ==========
            case 'delay':
                await this.delay(args[0]);
                break;
            
            default:
                console.warn(`[PowerOnSequence] 未知动作: ${actionName}`);
        }
    }
    
    /**
     * 显示所有数码管
     */
    displayAllSegments(text) {
        const segCodes = this.textToSegCodes(text);
        this.adapter.displayCache.seg = segCodes;
        this.triggerDisplayCallback();
    }
    
    /**
     * 显示指定位置的数码管（单字符）
     */
    displaySegment(index, text) {
        const segCode = this.charToSegCode(text[0] || ' ');
        this.adapter.displayCache.seg[index] = segCode;
        this.triggerDisplayCallback();
    }
    
    /**
     * 显示多字符文本（从指定位置开始）
     */
    displayMultiSegment(startIndex, text) {
        for (let i = 0; i < text.length && (startIndex + i) < 4; i++) {
            const segCode = this.charToSegCode(text[i]);
            this.adapter.displayCache.seg[startIndex + i] = segCode;
        }
        this.triggerDisplayCallback();
    }
    
    /**
     * 设置所有LED
     */
    setAllLeds(on) {
        // TODO: 设置所有LED状态
        console.log(`[PowerOnSequence] 所有LED: ${on ? '开启' : '关闭'}`);
    }
    
    /**
     * 设置小数点
     */
    setDecimalPoint(on) {
        this.adapter.displayCache.dp = on ? 0xFF : 0x00;
        this.triggerDisplayCallback();
    }
    
    /**
     * 设置指定LED
     */
    setLed(ledName, on) {
        console.log(`[PowerOnSequence] LED ${ledName}: ${on ? '开启' : '关闭'}`);
        
        // 根据LED名称设置对应状态
        if (ledName === 'power') {
            this.adapter.ledCache.power = on;
        }
        
        // 触发显示回调以更新UI
        this.triggerDisplayCallback();
    }
    
    /**
     * 将文本转换为段码数组
     */
    textToSegCodes(text) {
        const codes = [];
        for (let i = 0; i < 4 && i < text.length; i++) {
            codes.push(this.charToSegCode(text[i]));
        }
        // 填充剩余位置为空格
        while (codes.length < 4) {
            codes.push(0x00);
        }
        return codes;
    }
    
    /**
     * 将字符转换为段码
     */
    charToSegCode(char) {
        const segMap = {
            '0': 0x3F, '1': 0x06, '2': 0x5B, '3': 0x4F,
            '4': 0x66, '5': 0x6D, '6': 0x7D, '7': 0x07,
            '8': 0x7F, '9': 0x6F,
            'A': 0x77, 'B': 0x7C, 'C': 0x39, 'D': 0x5E,
            'E': 0x79, 'F': 0x71, 'G': 0x3D, 'H': 0x76,
            'J': 0x1E, 'L': 0x38, 'N': 0x37, 'P': 0x73,
            'U': 0x3E,
            'c': 0x58, 'n': 0x54, 'o': 0x5C, 'r': 0x50,
            't': 0x78, 'y': 0x6E,
            '-': 0x40, ' ': 0x00, '_': 0x08,
            'V': 0x3E  // V用U的段码近似
        };
        return segMap[char] || 0x00;
    }
    
    /**
     * 触发显示回调
     */
    triggerDisplayCallback() {
        if (this.adapter.callbacks.onDisplayOutput) {
            const segObj = {
                seg: this.adapter.displayCache.seg,
                dp_mask: this.adapter.displayCache.dp,
                colon_mask: this.adapter.displayCache.colon
            };
            const ledArray = this.adapter._ledStateToArray(this.adapter.getLedState());
            this.adapter.callbacks.onDisplayOutput(segObj, ledArray);
        }
    }
    
    /**
     * 将LED状态对象转换为数组格式（与JSEmcLogicAdapter一致）
     */
    _ledStateToArray(ledState) {
        // LED位域：bit0=power, bit1-4=head_select[0-3], bit5=child_lock, bit6=timer, bit7=pause
        const ledValue = (
            (ledState.power ? 1 : 0) |
            ((ledState.head_select && ledState.head_select[0] ? 1 : 0) << 1) |
            ((ledState.head_select && ledState.head_select[1] ? 1 : 0) << 2) |
            ((ledState.head_select && ledState.head_select[2] ? 1 : 0) << 3) |
            ((ledState.head_select && ledState.head_select[3] ? 1 : 0) << 4) |
            ((ledState.child_lock ? 1 : 0) << 5) |
            ((ledState.timer ? 1 : 0) << 6) |
            ((ledState.pause ? 1 : 0) << 7)
        );
        
        return [ledValue];
    }
    
    delay(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
}

class GlobalStateMachine {
    constructor(config, hotHeadManager, adapter) {
        this.config = config;
        this.hotHeadManager = hotHeadManager;
        this.adapter = adapter;  // 保存adapter引用
        this.currentState = config.state;
        this.currentNode = config.main_flow.entry;
        
        // 独立进程
        this.childLockProcess = new ChildLockProcess(adapter);
        this.timerProcess = new TimerProcess();
        this.boostProcess = new BoostProcess();
        this.pauseProcess = new PauseProcess();
    }
    
    pressKey(keyName, eventType) {
        console.log(`[GlobalSM] 按键: ${keyName}, 事件: ${eventType}, 节点: ${this.currentNode}`);
        
        // 获取当前节点配置
        const nodeConfig = this.config.main_flow.nodes[this.currentNode];
        if (!nodeConfig) {
            console.error(`[GlobalSM] 未找到节点配置: ${this.currentNode}`);
            return;
        }
        
        // 查找匹配的事件
        const matchedEvent = this.findMatchingEvent(nodeConfig.events, keyName, eventType);
        if (!matchedEvent) {
            console.log(`[GlobalSM] 无匹配事件`);
            return;
        }
        
        console.log(`[GlobalSM] 匹配事件:`, matchedEvent);
        
        // 执行call（传入keyName用于参数解析）
        if (matchedEvent.call) {
            this.executeCall(matchedEvent.call, matchedEvent.args, keyName);
        }
        
        // 状态转换
        if (matchedEvent.next) {
            this.transitionTo(matchedEvent.next);
        }
    }
    
    /**
     * 查找匹配的事件
     */
    findMatchingEvent(events, keyName, eventType) {
        if (!events) return null;
        
        for (const event of events) {
            // 检查trigger是否匹配
            const triggerMatch = this.checkTrigger(event.trigger, keyName);
            if (!triggerMatch) continue;
            
            // 检查condition是否匹配
            const conditionMatch = this.checkCondition(event.condition, eventType);
            if (!conditionMatch) continue;
            
            return event;
        }
        
        return null;
    }
    
    /**
     * 检查trigger是否匹配
     */
    checkTrigger(trigger, keyName) {
        if (Array.isArray(trigger)) {
            return trigger.includes(keyName);
        }
        return trigger === keyName;
    }
    
    /**
     * 检查condition是否匹配
     */
    checkCondition(condition, eventType) {
        if (!condition) return true; // 无条件则默认匹配
        
        // PRESS=1(短按), LONG_PRESS=3(长按)
        if (condition === 'short_press') {
            return eventType === 1;
        } else if (condition === 'long_press') {
            return eventType === 3;
        }
        
        return false;
    }
    
    /**
     * 执行call指定的函数
     */
    executeCall(callPath, args, triggerKey = null) {
        console.log(`[GlobalSM] 执行调用: ${callPath}`, args);
        
        // 解析调用路径，如 "child_lock_process.toggle" 或 "global.functions.power_on"
        const parts = callPath.split('.');
        
        if (parts.length === 2) {
            // 两层路径: process.method
            this.executeTwoLevelCall(parts[0], parts[1], args, triggerKey);
        } else if (parts.length === 3 && parts[0] === 'global' && parts[1] === 'functions') {
            // 三层路径: global.functions.method
            this.executeGlobalFunction(parts[2], args, triggerKey);
        } else {
            console.error(`[GlobalSM] 无效的调用路径: ${callPath}`);
        }
    }
    
    /**
     * 执行两层路径调用
     */
    executeTwoLevelCall(processName, methodName, args, triggerKey) {
        let processObj = null;
        switch (processName) {
            case 'child_lock_process':
                processObj = this.childLockProcess;
                break;
            case 'timer_process':
                processObj = this.timerProcess;
                break;
            case 'boost_process':
                processObj = this.boostProcess;
                break;
            case 'pause_process':
                processObj = this.pauseProcess;
                break;
            default:
                console.error(`[GlobalSM] 未知的进程: ${processName}`);
                return;
        }
        
        if (processObj && typeof processObj[methodName] === 'function') {
            processObj[methodName](args);
        } else {
            console.error(`[GlobalSM] 方法不存在: ${processName}.${methodName}`);
        }
    }
    
    /**
     * 执行全局函数
     */
    executeGlobalFunction(functionName, args, triggerKey) {
        switch (functionName) {
            case 'power_on':
                this.globalPowerOn();
                break;
            case 'power_off':
                this.globalPowerOff();
                break;
            case 'switch_hot_head':
                this.switchHotHead(args, triggerKey);
                break;
            default:
                console.error(`[GlobalSM] 未知的全局函数: ${functionName}`);
        }
    }
    
    /**
     * 全局开机函数
     */
    globalPowerOn() {
        console.log('[GlobalSM] 执行开机操作');
        // 设置电源LED
        this.adapter.ledCache.power = true;
        this.adapter.triggerDisplayCallback();
    }
    
    /**
     * 全局关机函数
     */
    globalPowerOff() {
        console.log('[GlobalSM] 执行关机操作');
        // 关闭电源LED
        this.adapter.ledCache.power = false;
        this.adapter.triggerDisplayCallback();
    }
    
    /**
     * 切换热点炉头
     */
    switchHotHead(args, triggerKey) {
        console.log('[GlobalSM] 切换热点炉头:', args, 'triggerKey:', triggerKey);
        
        // 解析args中的参数
        let headId = null;
        if (args && args.length > 0) {
            const argName = args[0];
            console.log('[GlobalSM] 参数名:', argName);
            
            // 如果参数是 "trigger_key_head_id"，从triggerKey中提取
            if (argName === 'trigger_key_head_id' && triggerKey) {
                // triggerKey格式: key_head_1, key_head_2, etc.
                const match = triggerKey.match(/key_head_(\d+)/);
                console.log('[GlobalSM] 正则匹配结果:', match);
                if (match) {
                    headId = parseInt(match[1]);
                    console.log('[GlobalSM] 提取的炉头ID:', headId);
                }
            } else {
                console.warn('[GlobalSM] 参数不匹配或triggerKey为空');
            }
        } else {
            console.warn('[GlobalSM] args为空或长度为0');
        }
        
        if (!headId || headId < 1 || headId > 4) {
            console.error('[GlobalSM] 无效的炉头ID:', headId);
            return;
        }
        
        console.log(`[GlobalSM] 切换到炉头 ${headId}`);
        
        // 清除所有炉头选择LED
        for (let i = 0; i < 4; i++) {
            this.adapter.ledCache.head_select[i] = false;
        }
        
        // 设置当前炉头选择LED
        this.adapter.ledCache.head_select[headId - 1] = true;
        console.log('[GlobalSM] LED状态更新:', this.adapter.ledCache.head_select);
        
        // 触发显示回调以更新UI
        if (this.adapter.callbacks.onDisplayOutput) {
            const segObj = {
                seg: this.adapter.displayCache.seg,
                dp_mask: this.adapter.displayCache.dp,
                colon_mask: this.adapter.displayCache.colon
            };
            const ledArray = this.adapter._ledStateToArray(this.adapter.getLedState());
            console.log('[GlobalSM] LED位域:', ledArray[0].toString(2).padStart(8, '0'));
            this.adapter.callbacks.onDisplayOutput(segObj, ledArray);
        }
    }
    
    /**
     * 状态转换
     */
    transitionTo(nextNode) {
        console.log(`[GlobalSM] 状态转换: ${this.currentNode} -> ${nextNode}`);
        
        const nextNodeConfig = this.config.main_flow.nodes[nextNode];
        if (!nextNodeConfig) {
            console.error(`[GlobalSM] 目标节点不存在: ${nextNode}`);
            return;
        }
        
        // 执行enter_actions
        if (nextNodeConfig.enter_actions) {
            this.executeActions(nextNodeConfig.enter_actions);
        }
        
        // 更新当前节点
        this.currentNode = nextNode;
    }
    
    /**
     * 执行动作列表
     */
    executeActions(actions) {
        for (const action of actions) {
            console.log(`[GlobalSM] 执行动作:`, action);
            // TODO: 实现具体的动作执行逻辑
        }
    }
    
    runCycle() {
        // TODO: 执行状态机循环（处理定时器等）
    }
    
    getState() {
        // TODO: 返回状态码
        return 0;
    }
}

// ========== 独立进程类 ==========

class ChildLockProcess {
    constructor(adapter) {
        this.adapter = adapter;
        this.enabled = false;
    }
    
    toggle() {
        this.enabled = !this.enabled;
        console.log(`[ChildLockProcess] 童锁状态: ${this.enabled ? '开启' : '关闭'}`);
        
        // 更新童锁LED
        if (this.adapter && this.adapter.ledCache) {
            this.adapter.ledCache.child_lock = this.enabled;
            // 触发显示回调以更新UI
            if (this.adapter.callbacks.onDisplayOutput) {
                const segObj = {
                    seg: this.adapter.displayCache.seg,
                    dp_mask: this.adapter.displayCache.dp,
                    colon_mask: this.adapter.displayCache.colon
                };
                const ledArray = this.adapter._ledStateToArray(this.adapter.getLedState());
                this.adapter.callbacks.onDisplayOutput(segObj, ledArray);
            }
        }
    }
}

class TimerProcess {
    constructor() {
        this.active = false;
        this.value = 0;
    }
    
    start(minutes) {
        this.active = true;
        this.value = minutes || 15;
        console.log(`[TimerProcess] 定时启动: ${this.value}分钟`);
    }
    
    pause() {
        this.active = false;
        console.log('[TimerProcess] 定时暂停');
    }
    
    resume() {
        this.active = true;
        console.log('[TimerProcess] 定时恢复');
    }
    
    stop() {
        this.active = false;
        this.value = 0;
        console.log('[TimerProcess] 定时停止');
    }
    
    reset() {
        this.value = 0;
        console.log('[TimerProcess] 定时重置');
    }
}

class BoostProcess {
    constructor() {
        this.active = false;
    }
    
    toggle() {
        this.active = !this.active;
        console.log(`[BoostProcess] Boost状态: ${this.active ? '开启' : '关闭'}`);
    }
}

class PauseProcess {
    constructor() {
        this.paused = false;
    }
    
    toggle() {
        this.paused = !this.paused;
        console.log(`[PauseProcess] 暂停状态: ${this.paused ? '暂停' : '运行'}`);
    }
}

// 导出
if (typeof module !== 'undefined' && module.exports) {
    module.exports = JsonLogicAdapter;
}
