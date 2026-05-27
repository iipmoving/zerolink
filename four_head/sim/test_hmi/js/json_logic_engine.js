/**
 * json_logic_engine.js — JSON规则驱动逻辑引擎 V8.0
 *
 * 引擎只做解释：加载JSON规则 → 收消息 → 查路由表 → 执行动作函数。
 * 所有业务规则在 four_head_v4.json 中声明，改JSON即改行为。
 *
 * 全局状态: POWERING_UP / VERSION_SHOW / POWERED_OFF / WORKING /
 *           PAUSED / DEEP_SLEEP
 * Zone 状态: idle / selecting / cooking
 * 正交进程: timer_setting / timer_active / boost_active
 */

/* ========== 按键编码常量 ========== */
const KEY_NAME = {
    /* 与 MCU drv_key.h KeyCode_t 严格一致 */
    2:  'KEY_MINUS',      3:  'KEY_PLUS',       4:  'KEY_POWER',
    5:  'KEY_TIMER',      6:  'KEY_PAUSE',      8:  'KEY_CHILD_LOCK',
    9:  'KEY_ZONE',
    12: 'KEY_HEAD_4',     13: 'KEY_HEAD_3',
    14: 'KEY_HEAD_2',     15: 'KEY_HEAD_1',
    22: 'KEY_0',          23: 'KEY_1',          24: 'KEY_2',
    25: 'KEY_3',          26: 'KEY_4',          27: 'KEY_5',
    28: 'KEY_6',          29: 'KEY_7',          30: 'KEY_8',
    31: 'KEY_9'
};

/* KEY_NAME → JSON短名称映射 */
const KEY_SHORT = {
    'KEY_TIMER': 'TIMER', 'KEY_PAUSE': 'PAUSE', 'KEY_CHILD_LOCK': 'CHILD_LOCK',
    'KEY_POWER': 'POWER',
    'KEY_HEAD_1': 'HEAD_1', 'KEY_HEAD_2': 'HEAD_2',
    'KEY_HEAD_3': 'HEAD_3', 'KEY_HEAD_4': 'HEAD_4',
    'KEY_ZONE': 'ZONE', 'KEY_MINUS': 'MINUS', 'KEY_PLUS': 'PLUS',
    'KEY_0': '0', 'KEY_1': '1', 'KEY_2': '2', 'KEY_3': '3', 'KEY_4': '4',
    'KEY_5': '5', 'KEY_6': '6', 'KEY_7': '7', 'KEY_8': '8', 'KEY_9': '9'
};

/* 事件类型 → JSON短名 */
var EVT_SHORT = {};
EVT_SHORT[KeyEvent.TAP]  = 'tap';
EVT_SHORT[KeyEvent.LONG] = 'long';

function keyNameToCode(name) {
    for (var k in KEY_NAME) { if (KEY_NAME[k] === name) return parseInt(k); }
    return -1;
}

function keyNameToLevel(name) {
    var map = { 'KEY_0':0,'KEY_1':1,'KEY_2':2,'KEY_3':3,'KEY_4':4,
                'KEY_5':5,'KEY_6':6,'KEY_7':7,'KEY_8':8,'KEY_9':9 };
    return map[name] !== undefined ? map[name] : -1;
}

function headKeyToIndex(keyName) {
    /* KEY_HEAD_N → N-1 */
    var m = keyName.match(/KEY_HEAD_(\d)/);
    return m ? parseInt(m[1]) - 1 : -1;
}

/* ========== JSON逻辑引擎 ========== */
const JsonLogicEngine = (function() {
    var rules = null;       /* 完整的JSON规则对象 */
    var heads = [];
    var slots = [];         /* SlotElement 实例, 每个Zone一个 */
    var levelLED = null;    /* LevelLED 档位灯组实例 */
    var statusLEDs = [];    /* StatusLED 状态灯实例列表 */
    var blinkRule = null;   /* BlinkRule 闪烁规则实例 */
    var modeRule = null;    /* ModeRule 显示模式规则实例 */
    var globalState = {};
    var hotHead = 0;
    var displayCache = null;
    var fastMode = false;

    /* 选择序列栈 */
    var selectStack = [];

    /* 计时器起点 */
    var idleSince = 0;
    var offSince = 0;

    /* ========== 蜂鸣器 ========== */
    var BUZZER_CMD = { VALID: 0, INVALID: 1 };
    function postBuzzer(valid) {
        MessageBus.post(MsgId.MSG_BUZZER_CTRL, valid ? BUZZER_CMD.VALID : BUZZER_CMD.INVALID, null);
    }

    /* ========== 超时读取(JSON优先, 回退hmi_config) ========== */
    function cfg(key) {
        if (rules && rules.timeouts && rules.timeouts[key] !== undefined)
            return rules.timeouts[key];
        /* 回退到 hmi_config.js 全局变量 */
        var map = {
            'select_confirm_ms': (typeof CFG_SELECT_TIMEOUT_MS !== 'undefined' ? CFG_SELECT_TIMEOUT_MS : 15000),
            'timer_confirm_ms': (typeof CFG_TIMER_CONFIRM_MS !== 'undefined' ? CFG_TIMER_CONFIRM_MS : 15000),
            'boost_max_ms': (typeof CFG_BOOST_TIMEOUT_MS !== 'undefined' ? CFG_BOOST_TIMEOUT_MS : 300000),
            'idle_to_standby_ms': (typeof CFG_STANDBY_MS !== 'undefined' ? CFG_STANDBY_MS : 15000),
            'idle_to_off_ms': (typeof CFG_IDLE_TO_OFF_MS !== 'undefined' ? CFG_IDLE_TO_OFF_MS : 30000),
            'off_to_sleep_ms': (typeof CFG_OFF_TO_SLEEP_MS !== 'undefined' ? CFG_OFF_TO_SLEEP_MS : 30000),
            'default_timer_min': (typeof CFG_DEFAULT_TIMER_MIN !== 'undefined' ? CFG_DEFAULT_TIMER_MIN : 15),
            'power_key_long_ms': (typeof CFG_POWER_KEY_LONG_MS !== 'undefined' ? CFG_POWER_KEY_LONG_MS : 1500)
        };
        return map[key] !== undefined ? map[key] : 15000;
    }

    /* ========== 初始化 ========== */
    function init(rulesJson) {
        rules = rulesJson;
        console.log('[JsonLogicEngine] 加载规则: %s v%s', rules.meta.name, rules.meta.version);
        console.log('[JsonLogicEngine] JSON路由规则数: global=%d, zone=%d, process=%d',
            countRoutes(rules.global_routes),
            countRoutes(rules.zone_routes),
            countRoutes(rules.process_routes));

        heads = [];
        for (var i = 0; i < 4; i++) { heads.push(createHeadInstance(i)); }

        /* 创建 SlotElement 实例: JSON elements.segment_slot → 属性; 不存在则默认 */
        slots = [];
        var slotProps = (rules && rules.elements && rules.elements.segment_slot)
            ? rules.elements.segment_slot : null;
        for (var j = 0; j < 4; j++) {
            slots.push(SlotElement.createSlotElement(slotProps));
        }

        /* 创建 LED 元素: JSON elements.led → 属性; 不存在则默认 */
        var ledProps = (rules && rules.elements && rules.elements.led)
            ? rules.elements.led : {};
        levelLED = LEDElement.createLevelLED(ledProps.level || null);

        /* 创建 StatusLED 实例: JSON elements.led.{power,timer,pause,child_lock,head_select} */
        statusLEDs = [];
        var statusNames = ['power', 'timer', 'pause', 'child_lock', 'head_select'];
        for (var sn = 0; sn < statusNames.length; sn++) {
            var name = statusNames[sn];
            var sp = ledProps[name] || null;
            if (sp) statusLEDs.push(LEDElement.createStatusLED(sp));
        }

        /* 创建 BlinkRule 实例: JSON elements.blink → 属性 */
        var blinkProps = (rules && rules.elements && rules.elements.blink)
            ? rules.elements.blink : null;
        blinkRule = BlinkRule.createBlinkRule(blinkProps);

        /* 创建 ModeRule 实例: JSON elements.mode → 属性 */
        var modeProps = (rules && rules.elements && rules.elements.mode)
            ? rules.elements.mode : null;
        modeRule = ModeRule.createModeRule(modeProps);

        globalState = {
            mode: 'power_on_seq',
            child_lock: false,
            paused: false,
            power_on_seq_step: 0
        };
        selectStack = [];
        idleSince = 0;
        offSince = 0;

        displayCache = createDisplayCache();

        MessageBus.register(MsgId.MSG_KEY_EVENT, onKeyEvent);
        MessageBus.register(MsgId.MSG_TIMER_100MS, onTimer100ms);
        MessageBus.register(MsgId.MSG_TIMER_1S, onTimer1s);

        runPowerOnSeqStep(0);
        console.log('[JsonLogicEngine] 初始化完成');
    }

    function countRoutes(section) {
        if (!section) return 0;
        var n = 0;
        for (var k in section) {
            if (section[k] && section[k].routes) {
                n += Object.keys(section[k].routes).length;
            }
        }
        return n;
    }

    /* ========== 实例工厂 ========== */
    function createHeadInstance(index) {
        return {
            index: index,
            node: 'idle',
            power_level: 0,
            original_power: 0,
            boost_active: false,
            boost_remaining: 0,
            timer_setting: false,
            timer_active: false,
            timer_value: 0,
            select_time: 0,
            timer_set_time: 0
        };
    }

    function createDisplayCache() {
        return {
            seg_chars: ['0','0','0','0','0','0','0','0'],
            seg_blink: [false, false, false, false],
            seg_mode: 'power',
            seg_ascii: '',
            leds: {
                power: false, child_lock: false, pause_btn: false, timer: false,
                no_zone: false,
                head_select: [false,false,false,false],
                power_level: [false,false,false,false,false,false,false,false,false,false]
            }
        };
    }

    /* ========== 上电序列 ========== */
    function runPowerOnSeqStep(step) {
        globalState.power_on_seq_step = step;

        /* 从JSON读取序列定义, 回退硬编码 */
        var seq = null;
        if (rules && rules.power_on_sequence) {
            for (var s = 0; s < rules.power_on_sequence.length; s++) {
                if (rules.power_on_sequence[s].step === step) {
                    seq = rules.power_on_sequence[s]; break;
                }
            }
        }

        if (seq && seq.goto) {
            /* goto值映射为动作: powered_off→go_powered_off, working→go_working */
            var gotoMap = {
                'powered_off': 'go_powered_off',
                'deep_sleep': 'go_deep_sleep',
                'working': 'go_working'
            };
            var action = gotoMap[seq.goto] || seq.goto;
            executeAction(action, 0, null, true);
            console.log('[PowerOnSeq] 完成→' + seq.goto);
            return;
        }

        if (seq) {
            applySeqDisplay(seq);
            var delay = cfg(seq.delay_ms_key);
            if (fastMode) { runPowerOnSeqStep(step + 1); }
            else { setTimeout(function() { runPowerOnSeqStep(step + 1); }, delay); }
        } else {
            /* 回退: 硬编码(确保引擎不崩溃) */
            if (step === 0) {
                displayCache.seg_chars = ['8','8','8','8','8','8','8','8'];
                displayCache.seg_mode = 'ascii'; displayCache.seg_ascii = '';
                allLEDsOn();
                postDisplay();
                var d0 = cfg('power_on_all_on_ms');
                if (fastMode) { runPowerOnSeqStep(1); }
                else { setTimeout(function() { runPowerOnSeqStep(1); }, d0); }
            } else if (step === 1) {
                displayCache.seg_chars = ['V','1','P','1','.','0','.','0'];
                displayCache.seg_mode = 'ascii'; displayCache.seg_ascii = 'V1.0/P1.0';
                postDisplay();
                var d1 = cfg('version_show_ms');
                if (fastMode) { runPowerOnSeqStep(2); }
                else { setTimeout(function() { runPowerOnSeqStep(2); }, d1); }
            } else {
                goPoweredOff();
                console.log('[PowerOnSeq] 完成→关机');
            }
        }
    }

    /* 将JSON seq步骤的显示配置应用到displayCache */
    function applySeqDisplay(seq) {
        /* seg_mode */
        if (seq.seg_mode) {
            displayCache.seg_mode = seq.seg_mode;
        }

        /* seg_chars: 8字符字符串 → 8元素数组 */
        if (seq.seg_chars && seq.seg_chars.length === 8) {
            displayCache.seg_chars = seq.seg_chars.split('');
        }

        /* seg_ascii */
        if (seq.seg_ascii) {
            displayCache.seg_ascii = seq.seg_ascii;
        } else if (seq.seg_chars) {
            displayCache.seg_ascii = seq.seg_chars;
        }

        /* leds */
        if (seq.leds === 'all_on') {
            allLEDsOn();
        } else if (seq.leds === 'all_off') {
            allLEDsOff();
        }

        postDisplay();
    }

    function allLEDsOn() {
        displayCache.leds.power = true;
        displayCache.leds.child_lock = true;
        displayCache.leds.pause_btn = true;
        displayCache.leds.timer = true;
        for (var j = 0; j < 4; j++) displayCache.leds.head_select[j] = true;
        for (var k = 0; k < 10; k++) displayCache.leds.power_level[k] = true;
    }

    /* ========== 核心: 消息→路由→动作 ========== */
    function onKeyEvent(msgId, param, data_ptr) {
        var keyState = (param >> 8) & 0xFF;
        var keyCode = param & 0xFF;
        var keyName = KEY_NAME[keyCode] || ('KEY_' + keyCode);
        var evt = EVT_SHORT[keyState];
        if (!evt) return;  /* 只处理TAP和LONG */

        /* --- 引擎级守卫(不上JSON) --- */

        /* 上电序列中忽略 */
        if (globalState.mode === 'power_on_seq' || globalState.mode === 'version_show') {
            postBuzzer(false); return;
        }

        /* 休眠态: 全局路由已尝试匹配, 不匹配则拒绝 */
        if (globalState.mode === 'deep_sleep') {
            postBuzzer(false); return;
        }

        /* --- JSON路由查找 --- */

        /* 1. 全局路由 (POWER/PAUSE/CHILD_LOCK/HEAD_1~4 等全局操作) */
        var globalCfg = getGlobalConfig(globalState.mode);
        if (globalCfg) {
            var routeKey = KEY_SHORT[keyName] + ' ' + evt;
            if (matchAndExecute(globalCfg.routes, routeKey, keyName, evt)) {
                if (globalState.mode === 'working') resetIdleTimer();
                return;
            }
        }

        /* 2. 关机态拒绝(开关已在全局匹配) */
        if (globalState.mode === 'powered_off') {
            postBuzzer(false); return;
        }

        /* 3. 童锁拦截: 读取JSON白名单, 白名单内按键放行 */
        if (globalState.child_lock) {
            var whitelist = null;
            if (rules && rules.global_routes && rules.global_routes.child_lock &&
                rules.global_routes.child_lock.block_all_except) {
                whitelist = rules.global_routes.child_lock.block_all_except;
            }
            if (whitelist && whitelist.indexOf(KEY_SHORT[keyName]) >= 0) {
                /* 白名单内按键放行, 继续路由 */
            } else {
                postBuzzer(false); return;
            }
        }

        /* 4. 暂停态拒绝(开关/暂停已在全局匹配) */
        if (globalState.paused) {
            postBuzzer(false); return;
        }

        /* 有效按键 → 重置空闲计时(所有后续路由都需要) */
        resetIdleTimer();

        /* 5. 确定操作目标炉头 */
        var target = resolveTarget();

        /* 6. 进程路由(优先级高于Zone路由) */
        if (target >= 0) {
            var h = heads[target];
            var procCfg = getActiveProcessConfig(h);
            if (procCfg) {
                var procRouteKey = KEY_SHORT[keyName] + ' ' + evt;
                if (matchAndExecute(procCfg.routes, procRouteKey, keyName, evt, target)) return;

                /* 进程已拥有此按键(防止fallthrough到Zone路由) */
                if (isKeyOwnedByProcess(procCfg, KEY_SHORT[keyName])) {
                    postBuzzer(true); return;
                }
            }
        }

        /* 8. Zone路由 */
        if (target >= 0) {
            var h2 = heads[target];
            var zoneCfg = (rules.zone_routes && rules.zone_routes[h2.node])
                ? rules.zone_routes[h2.node] : null;
            if (zoneCfg) {
                var zoneRouteKey = resolveHeadRouteKey(keyName, evt, target);
                if (zoneRouteKey && matchAndExecute(zoneCfg.routes, zoneRouteKey, keyName, evt, target)) return;
            }
        }

        /* 9. 未匹配 */
        postBuzzer(false);
    }

    /* 获取全局模式的JSON配置节点 */
    function getGlobalConfig(mode) {
        if (!rules || !rules.global_routes) return null;
        /* 映射引擎内部mode名 → JSON键名 */
        var map = {
            'powered_off': 'powered_off',
            'deep_sleep': 'deep_sleep',
            'working': 'working',
            'paused': 'paused'
        };
        var key = map[mode] || mode;
        return rules.global_routes[key] || null;
    }

    /* 获取当前活跃进程的JSON配置 */
    function getActiveProcessConfig(h) {
        if (!rules || !rules.process_routes) return null;
        if (h.timer_setting) return rules.process_routes.timer_setting || null;
        if (h.timer_active)  return rules.process_routes.timer_active || null;
        if (h.boost_active)  return rules.process_routes.boost_active || null;
        return null;
    }

    /* 解析炉头键路由: 自己的炉头→HEAD_SELF, 其他→HEAD_OTHER */
    function resolveHeadRouteKey(keyName, evt, target) {
        var headIdx = headKeyToIndex(keyName);
        if (headIdx < 0) return KEY_SHORT[keyName] + ' ' + evt;
        if (headIdx === target) return 'HEAD_SELF ' + evt;
        return 'HEAD_OTHER ' + evt;
    }

    /* 检查按键是否被进程"拥有"(该进程的任何event匹配即视为拥有) */
    function isKeyOwnedByProcess(procCfg, shortKey) {
        if (!procCfg || !procCfg.routes) return false;
        var keys = Object.keys(procCfg.routes);
        for (var i = 0; i < keys.length; i++) {
            if (keys[i].indexOf(shortKey + ' ') === 0) return true;
        }
        return false;
    }

    /* 在路由表中查找并执行 */
    function matchAndExecute(routes, routeKey, keyName, evt, optTarget) {
        if (!routes) return false;

        /* 精确匹配 */
        if (routes[routeKey]) {
            executeAction(routes[routeKey], optTarget, keyName);
            return true;
        }

        /* 通配符匹配: HEAD_OTHER tap 等动态键 */
        if (routeKey.indexOf('HEAD_') === 0 && evt) {
            var headIdx = headKeyToIndex(keyName);
            var isSelf = (headIdx === hotHead);
            /* 如果精确匹配未命中(比如JSON里只有HEAD_SELF/HEAD_OTHER没有HEAD_1~4) */
            if (isSelf && routes['HEAD_SELF ' + evt]) {
                executeAction(routes['HEAD_SELF ' + evt], optTarget, keyName);
                return true;
            }
            if (!isSelf && routes['HEAD_OTHER ' + evt]) {
                executeAction(routes['HEAD_OTHER ' + evt], optTarget, keyName);
                return true;
            }
        }

        return false;
    }

    /* 批量执行 enter/exit 动作列表(不发蜂鸣) */
    function applyActions(actionList) {
        if (!actionList || !actionList.length) return;
        for (var i = 0; i < actionList.length; i++) {
            executeAction(actionList[i], 0, null, true);
        }
    }

    /* ========== 动作分发器 (JSON action字符串 → 函数调用) ========== */
    function executeAction(actionStr, target, keyName, silent) {
        var parts = actionStr.split(' ');
        var action = parts[0];
        var param = parts.length > 1 ? parseInt(parts[1]) : 0;

        /* 对于HEAD_SELF/HEAD_OTHER等需要替换%head为实际索引 */
        if (parts.length > 1 && parts[1] === '%head') {
            param = headKeyToIndex(keyName);
            if (param < 0) param = target;
        }

        switch (action) {
            /* 全局 */
            case 'go_working':           goWorking(); break;
            case 'go_powered_off':       goPoweredOff(); break;
            case 'go_deep_sleep':        enterDeepSleep(); break;
            case 'toggle_pause':         togglePause(); break;
            case 'toggle_child_lock':    toggleChildLock(); break;
            case 'clear_hothead':        hotHead = -1; syncLED(); postDisplay(); break;
            case 'reset_idle_timer':     resetIdleTimer(); break;
            case 'reset_off_timer':      offSince = Date.now(); break;

            /* Zone */
            case 'select_head':          selectHead(param >= 0 ? param : headKeyToIndex(keyName)); break;
            case 'confirm_select':       confirmSelect(hotHead); break;
            case 'confirm_select_immediate': confirmSelect(hotHead); break;
            case 'set_power':            handlePowerKey(param); break;

            /* Boost */
            case 'enter_boost':          enterBoost(); break;
            case 'exit_boost':           exitBoost(hotHead); break;
            case 'exit_boost_set_power': exitBoostAndSetPower(param); break;

            /* Timer */
            case 'enter_timer_setting':  enterTimerSetting(); break;
            case 'confirm_timer':        confirmTimer(findTimerHead()); break;
            case 'cancel_timer_setting': cancelTimerSetting(); break;
            case 'cancel_timer_active':  cancelTimerActive(); break;
            case 'timer_adjust':         handleTimerAdjust(param); break;

            /* 显示 */
            case 'show_dash':            showDash(); break;
            case 'show_pa':              showPA(); break;
            case 'show_power_mode':      updateAllDisplays(); postDisplay(); break;
            case 'display_all_off':      showAllOff(); break;
            case 'clear_all_heads':      clearAllHeads(); break;
            case 'hothead_clear':        hotHead = -1; selectStack = []; break;

            /* LED */
            case 'led_power_on':         displayCache.leds.power = true; break;
            case 'led_power_blink':      displayCache.leds.power = true; break;
            case 'led_power_off':        displayCache.leds.power = false; break;
            case 'led_all_off':          allLEDsOff(); break;
            case 'led_pause_on':         displayCache.leds.pause_btn = true; break;
            case 'led_pause_off':        displayCache.leds.pause_btn = false; break;
            case 'led_timer_on':         displayCache.leds.timer = true; break;
            case 'led_timer_off':        displayCache.leds.timer = false; break;
            case 'led_child_lock_on':    displayCache.leds.child_lock = true; break;
            case 'led_child_lock_off':   displayCache.leds.child_lock = false; break;

            /* 蜂鸣 */
            case 'beep_valid':           postBuzzer(true); return;  /* 不postDisplay */
            case 'beep_invalid':         postBuzzer(false); return;

            default:
                console.log('[JsonLogicEngine] 未知动作: %s', action);
                return;
        }

        if (!silent) postBuzzer(true);
    }

    function allHeadsIdle() {
        for (var n = 0; n < 4; n++) {
            if (heads[n].node !== 'idle') return false;
        }
        return true;
    }

    /* ========== 计时器回调 ========== */
    function onTimer100ms(msgId, param) {
        if (globalState.mode === 'power_on_seq' || globalState.mode === 'version_show') return;

        /* Zone 超时: 从JSON zone_routes.*.timeout 读取 → 遍历匹配 */
        if (rules && rules.zone_routes) {
            var zoneStates = Object.keys(rules.zone_routes);
            for (var zi = 0; zi < zoneStates.length; zi++) {
                var zs = zoneStates[zi];
                var zCfg = rules.zone_routes[zs];
                if (!zCfg.timeout) continue;
                for (var i = 0; i < 4; i++) {
                    var h = heads[i];
                    if (h.node === zs && h.select_time > 0) {
                        if (Date.now() - h.select_time >= cfg(zCfg.timeout.ms_key)) {
                            executeAction(zCfg.timeout.action, i, null, true);
                        }
                    }
                }
            }
        }

        /* 进程超时: timer_setting → confirmTimer, boost → countdown(值由cfg()驱动) */
        for (var j = 0; j < 4; j++) {
            var ht = heads[j];
            if (ht.timer_setting && ht.timer_set_time > 0) {
                if (Date.now() - ht.timer_set_time >= cfg('timer_confirm_ms')) {
                    confirmTimer(j);
                }
            }
        }
        for (var k = 0; k < 4; k++) {
            if (heads[k].boost_active && heads[k].boost_remaining > 0) {
                heads[k].boost_remaining -= 100;
                if (heads[k].boost_remaining <= 0) exitBoost(k);
            }
        }

        /* JSON guards — 遍历全局模式的守卫列表 */
        var guardCfg = getGlobalConfig(globalState.mode);
        if (guardCfg && guardCfg.guards) {
            for (var g = 0; g < guardCfg.guards.length; g++) {
                var guard = guardCfg.guards[g];
                var triggered = false;

                if (guard.check === 'all_idle') {
                    if (idleSince > 0 && allHeadsIdle()) {
                        if (Date.now() - idleSince >= cfg(guard.ms_key)) triggered = true;
                    }
                } else if (guard.check === 'always') {
                    if (offSince > 0) {
                        if (Date.now() - offSince >= cfg(guard.ms_key)) triggered = true;
                    }
                }

                if (triggered) {
                    executeAction(guard.action, 0, null, true);
                    console.log('[JsonLogicEngine] guard触发: %s', guard.name);
                }
            }
        }
    }

    function onTimer1s(msgId, param) {
        if (globalState.mode === 'power_on_seq' || globalState.mode === 'version_show') return;

        for (var i = 0; i < 4; i++) {
            var h = heads[i];
            if (h.timer_active && h.timer_value > 0) {
                h.timer_value--;
                if (h.timer_value <= 0) {
                    h.timer_active = false;
                    h.node = 'idle';
                    h.power_level = 0;
                    h.boost_active = false;
                    h.boost_remaining = 0;
                    removeFromStack(i);
                    reassignHotHead(i);
                    console.log('[JsonLogicEngine] 炉头%d 定时归零→idle', i+1);
                }
                updateDisplayForHead(i);
            }
        }
        syncStatusLEDs();
        syncLED();
        postDisplay();
    }

    function checkAnyTimer() {
        for (var i = 0; i < 4; i++) {
            if (heads[i].timer_active) return true;
        }
        return false;
    }

    /* ========== 动作函数实现 ========== */

    /* --- 选中确认 --- */
    function confirmSelect(headIdx) {
        var h = heads[headIdx];
        console.log('[JsonLogicEngine] 炉头%d 选中确认 (power=%d)', headIdx+1, h.power_level);
        h.node = h.power_level > 0 ? 'cooking' : 'idle';
        h.select_time = 0;

        if (h.power_level > 0) {
            pushToStack(headIdx);
        } else {
            removeFromStack(headIdx);
            reassignHotHead(headIdx);
        }

        updateDisplayForHead(headIdx);
        syncStatusLEDs();
        syncLED();
        postDisplay();
    }

    /* --- 定时确认 --- */
    function confirmTimer(headIdx) {
        var h = heads[headIdx];
        h.timer_setting = false;
        h.timer_active = true;
        h.timer_set_time = 0;

        if (h.node === 'selecting') {
            h.node = h.power_level > 0 ? 'cooking' : 'idle';
            h.select_time = 0;
            if (h.power_level > 0) pushToStack(headIdx);
            else { removeFromStack(headIdx); reassignHotHead(headIdx); }
        }

        console.log('[JsonLogicEngine] 炉头%d 定时确认 (node=%s, value=%d分)', headIdx+1, h.node, h.timer_value);
        updateDisplayForHead(headIdx);
        syncStatusLEDs();
        syncLED();
        postDisplay();
    }

    /* --- 定时操作 --- */
    function findTimerHead() {
        return hotHead;
    }

    function enterTimerSetting() {
        var target = resolveTarget();
        if (target < 0) return;
        var h = heads[target];
        if (h.node !== 'selecting' && h.node !== 'cooking') return;
        h.timer_setting = true;
        h.timer_active = false;
        if (h.timer_value > 0) {
            /* 从 timer_active 重进: 保留当前剩余时间 */
        } else {
            h.timer_value = cfg('default_timer_min');
        }
        h.timer_set_time = Date.now();
        console.log('[JsonLogicEngine] 炉头%d→timer_setting (%d分)', target+1, cfg('default_timer_min'));
        updateAllDisplays();
        postDisplay();
    }

    function cancelTimerSetting() {
        var target = findTimerHead();
        if (target < 0) return;
        var h = heads[target];
        if (!h.timer_setting) return;
        h.timer_setting = false;
        h.timer_value = 0;
        h.timer_set_time = 0;
        console.log('[JsonLogicEngine] 炉头%d 定时设置取消', target+1);
        syncStatusLEDs();
        updateAllDisplays();
        postDisplay();
    }

    function cancelTimerActive() {
        var target = findTimerHead();
        if (target < 0) return;
        var h = heads[target];
        if (!h.timer_active) return;
        h.timer_active = false;
        h.timer_value = 0;
        console.log('[JsonLogicEngine] 炉头%d 定时取消(idle)', target+1);
        syncStatusLEDs();
        updateAllDisplays();
        postDisplay();
    }

    function handleTimerAdjust(delta) {
        var target = resolveTarget();
        if (target < 0) return;
        var h = heads[target];
        if (!h.timer_setting) return;
        var tCfg = (rules && rules.elements && rules.elements.timer) ? rules.elements.timer : {};
        var tStep = tCfg.adjust_step !== undefined ? tCfg.adjust_step : 1;
        var tMin = tCfg.adjust_min !== undefined ? tCfg.adjust_min : 1;
        var tMax = tCfg.adjust_max !== undefined ? tCfg.adjust_max : 99;
        h.timer_value += delta * tStep;
        if (h.timer_value < tMin) h.timer_value = tMin;
        if (h.timer_value > tMax) h.timer_value = tMax;
        h.timer_set_time = Date.now();
        console.log('[JsonLogicEngine] 炉头%d 定时→%d分', target+1, h.timer_value);
        updateAllDisplays();
        postDisplay();
    }

    /* --- Boost --- */
    function enterBoost() {
        var target = resolveTarget();
        if (target < 0) return;
        var h = heads[target];
        h.original_power = h.power_level;
        h.boost_active = true;
        h.boost_remaining = cfg('boost_max_ms');
        var bCfg = (rules && rules.elements && rules.elements.boost) ? rules.elements.boost : {};
        h.power_level = bCfg.power_level !== undefined ? bCfg.power_level : 9;
        syncLED();
        pushToStack(target);
        console.log('[JsonLogicEngine] 炉头%d→Boost (原=%d)', target+1, h.original_power);
        updateAllDisplays();
        postDisplay();
    }

    function exitBoost(index) {
        var h = heads[index];
        h.power_level = h.original_power;
        h.boost_active = false;
        h.boost_remaining = 0;
        if (h.node === 'selecting') confirmSelect(index);
        console.log('[JsonLogicEngine] 炉头%d Boost退出→%d档', index+1, h.power_level);
        syncLED();
        updateAllDisplays();
        postDisplay();
    }

    function exitBoostAndSetPower(level) {
        var target = hotHead;
        if (target < 0) return;
        var h = heads[target];
        if (!h.boost_active) { handlePowerKey(level); return; }
        h.boost_active = false;
        h.boost_remaining = 0;
        handlePowerKey(level);
        if (h.node === 'selecting') confirmSelect(target);
    }

    /* --- 选择序列 --- */
    function pushToStack(idx) {
        removeFromStack(idx);
        selectStack.push(idx);
        var maxDepth = 4;
        if (rules && rules.elements && rules.elements.stack) {
            maxDepth = rules.elements.stack.max_depth || 4;
        }
        if (selectStack.length > maxDepth) selectStack.shift();
        console.log('[JsonLogicEngine] 栈push: 头%d, 栈=%s', idx+1, JSON.stringify(selectStack));
    }

    function removeFromStack(idx) {
        var pos = selectStack.indexOf(idx);
        if (pos >= 0) selectStack.splice(pos, 1);
    }

    function getStackTop() {
        return selectStack.length > 0 ? selectStack[selectStack.length - 1] : -1;
    }

    function reassignHotHead(headIdx) {
        if (headIdx !== hotHead) return;
        if (heads[headIdx].node !== 'idle') return;
        var top = getStackTop();
        if (top >= 0) {
            console.log('[JsonLogicEngine] hotHead 移交: %d→%d', headIdx+1, top+1);
            hotHead = top;
        }
    }

    function syncLED() {
        if (!levelLED) return;
        if (hotHead < 0 || !heads[hotHead]) {
            levelLED.clear(displayCache.leds);
            return;
        }
        var h = heads[hotHead];
        levelLED.setLevel(h.power_level);
        levelLED.setBoost(h.boost_active);
        levelLED.syncToCache(displayCache.leds);
    }

    /* --- 全局操作 --- */
    function togglePower() {
        if (globalState.mode === 'powered_off' || globalState.mode === 'deep_sleep') {
            goWorking();
        } else {
            goPoweredOff();
        }
    }

    function goWorking() {
        /* 退出旧模式 */
        var oldCfg = getGlobalConfig(globalState.mode);
        if (oldCfg && oldCfg.exit_actions) applyActions(oldCfg.exit_actions);

        /* 核心状态切换 */
        globalState.mode = 'working';
        globalState.paused = false;
        globalState.child_lock = false;
        idleSince = Date.now();
        offSince = 0;

        /* JSON 声明的进入动作 */
        var cfg = getGlobalConfig('working');
        if (cfg && cfg.enter_actions) {
            applyActions(cfg.enter_actions);
        } else {
            /* 回退: 硬编码 */
            hotHead = -1; selectStack = [];
            for (var i = 0; i < 4; i++) {
                heads[i].node = 'idle'; heads[i].power_level = 0;
                heads[i].boost_active = false; heads[i].boost_remaining = 0;
                heads[i].timer_setting = false; heads[i].timer_active = false;
                heads[i].timer_value = 0; heads[i].select_time = 0; heads[i].timer_set_time = 0;
            }
            if (levelLED) levelLED.clear(displayCache.leds);
            syncStatusLEDs();
            updateAllDisplays();
        }
        syncStatusLEDs();
        postDisplay();
        console.log('[JsonLogicEngine] WORKING');
    }

    function goPoweredOff() {
        var oldCfg = getGlobalConfig(globalState.mode);
        if (oldCfg && oldCfg.exit_actions) applyActions(oldCfg.exit_actions);

        globalState.mode = 'powered_off';
        globalState.paused = false;
        idleSince = 0;
        offSince = Date.now();

        var cfg = getGlobalConfig('powered_off');
        if (cfg && cfg.enter_actions) {
            applyActions(cfg.enter_actions);
        } else {
            hotHead = -1; selectStack = [];
            for (var i = 0; i < 4; i++) {
                heads[i].node = 'idle'; heads[i].power_level = 0;
                heads[i].boost_active = false; heads[i].boost_remaining = 0;
                heads[i].timer_setting = false; heads[i].timer_active = false;
                heads[i].timer_value = 0; heads[i].select_time = 0; heads[i].timer_set_time = 0;
            }
            displayCache.seg_chars = ['-','-','-','-','-','-','-','-'];
            if (levelLED) levelLED.clear(displayCache.leds);
            syncStatusLEDs();
        }
        syncStatusLEDs();
        postDisplay();
        console.log('[JsonLogicEngine] POWERED_OFF');
    }

    function enterDeepSleep() {
        var oldCfg = getGlobalConfig(globalState.mode);
        if (oldCfg && oldCfg.exit_actions) applyActions(oldCfg.exit_actions);

        globalState.mode = 'deep_sleep';
        idleSince = 0;
        offSince = 0;

        var cfg = getGlobalConfig('deep_sleep');
        if (cfg && cfg.enter_actions) {
            applyActions(cfg.enter_actions);
        } else {
            displayCache.seg_chars = [' ',' ',' ',' ',' ',' ',' ',' '];
            if (levelLED) levelLED.clear(displayCache.leds);
            syncStatusLEDs();
        }
        syncStatusLEDs();
        postDisplay();
        console.log('[JsonLogicEngine] DEEP_SLEEP');
    }

    function wakeFromSleep() {
        goWorking();
        console.log('[JsonLogicEngine] 唤醒→WORKING');
    }

    function toggleChildLock() {
        globalState.child_lock = !globalState.child_lock;
        if (!globalState.child_lock) resetIdleTimer();
        syncStatusLEDs();
        console.log('[JsonLogicEngine] 童锁: %s', globalState.child_lock ? 'ON' : 'OFF');
        postDisplay();
    }

    function togglePause() {
        globalState.paused = !globalState.paused;

        if (globalState.paused) {
            var cfg = getGlobalConfig('paused');
            if (cfg && cfg.enter_actions) { applyActions(cfg.enter_actions); }
            else {
                displayCache.seg_ascii = 'PA';
                displayCache.seg_chars = ['P','A','P','A','P','A','P','A'];
                updateAllDisplays();
            }
        } else {
            var cfg2 = getGlobalConfig('paused');
            if (cfg2 && cfg2.exit_actions) { applyActions(cfg2.exit_actions); }
            else {
                resetIdleTimer();
                updateAllDisplays();
            }
        }
        syncStatusLEDs();
        postDisplay();
        console.log('[JsonLogicEngine] 暂停: %s', globalState.paused ? 'ON' : 'OFF');
    }

    /* --- 炉头选择 --- */
    function selectHead(index) {
        var h = heads[index];

        /* 按同一炉头 = 手动确认(仅selecting态). 非selecting则重新选中 */
        if (index === hotHead) {
            if (h && h.node === 'selecting') {
                console.log('[JsonLogicEngine] 炉头%d 自确认(手动)', index+1);
                confirmSelect(index);
                return;
            }
            /* idle/cooking 态: 重新选中(重置闪烁+计时) */
            h.node = 'selecting';
            h.select_time = Date.now();
            syncLED();
            syncStatusLEDs();
            updateAllDisplays();
            postDisplay();
            return;
        }

        /* 旧炉头自动确认 */
        var old = heads[hotHead];
        if (old && old.node === 'selecting') {
            old.node = old.power_level > 0 ? 'cooking' : 'idle';
            old.select_time = 0;
            if (old.power_level > 0) {
                pushToStack(hotHead);
            } else {
                removeFromStack(hotHead);
            }
        }

        hotHead = index;
        h.node = 'selecting';
        h.select_time = Date.now();
        syncLED();

        syncStatusLEDs();
        console.log('[JsonLogicEngine] 选中炉头%d (power=%d)', index+1, h.power_level);
        updateAllDisplays();
        postDisplay();
    }

    /* --- 功率键 --- */
    function handlePowerKey(level) {
        var target = resolveTarget();
        if (target < 0) { postBuzzer(false); return; }

        var h = heads[target];

        if (h.boost_active) {
            h.boost_active = false;
            h.boost_remaining = 0;
            if (level === 0) {
                h.power_level = h.original_power;
            }
        }

        if (level === 0) {
            h.power_level = 0;
            if (h.node === 'selecting') {
                h.select_time = Date.now();
            } else {
                h.node = 'idle';
                h.timer_active = false;
                h.timer_setting = false;
                h.timer_set_time = 0;
            }
            removeFromStack(target);
            reassignHotHead(target);
            console.log('[JsonLogicEngine] 炉头%d→0档', target+1);
        } else {
            h.power_level = level;
            if (h.node === 'selecting') {
                h.select_time = Date.now();
            } else {
                h.node = 'cooking';
                pushToStack(target);
            }
            console.log('[JsonLogicEngine] 炉头%d→%d档', target+1, level);
        }

        syncStatusLEDs();
        syncLED();
        updateAllDisplays();
        postDisplay();
    }

    function resolveTarget() {
        var hh = heads[hotHead];
        if (hh && hh.node !== 'idle') return hotHead;

        var cookingCount = 0, lastCooking = -1;
        for (var i = 0; i < 4; i++) {
            if (heads[i].node !== 'idle') { cookingCount++; lastCooking = i; }
        }
        if (cookingCount === 1) {
            hotHead = lastCooking;
            return lastCooking;
        }
        if (cookingCount === 0 && globalState.mode === 'working') {
            console.log('[JsonLogicEngine] 请先选炉头');
            return -1;
        }
        if (cookingCount > 1) {
            console.log('[JsonLogicEngine] 多头工作, 请先选炉头');
            return -1;
        }
        return hotHead;
    }

    /* --- 显示辅助 --- */
    function getPattern(name, fallback) {
        if (rules && rules.elements && rules.elements.display_patterns) {
            var p = rules.elements.display_patterns[name];
            if (p) return p.split('');
        }
        return fallback;
    }

    function showDash() {
        displayCache.seg_chars = getPattern('dash', ['-','-','-','-','-','-','-','-']);
    }

    function showPA() {
        displayCache.seg_ascii = 'PA';
        displayCache.seg_chars = getPattern('pa', ['P','A','P','A','P','A','P','A']);
    }

    function showAllOff() {
        displayCache.seg_chars = getPattern('off', [' ',' ',' ',' ',' ',' ',' ',' ']);
    }

    function clearAllHeads() {
        for (var i = 0; i < 4; i++) {
            heads[i].node = 'idle';
            heads[i].power_level = 0;
            heads[i].boost_active = false;
            heads[i].boost_remaining = 0;
            heads[i].timer_setting = false;
            heads[i].timer_active = false;
            heads[i].timer_value = 0;
            heads[i].select_time = 0;
            heads[i].timer_set_time = 0;
        }
        if (levelLED) levelLED.clear(displayCache.leds);
        syncStatusLEDs();
        hotHead = -1;
        selectStack = [];
    }

    function allLEDsOff() {
        displayCache.leds.power = false;
        displayCache.leds.timer = false;
        displayCache.leds.pause_btn = false;
        displayCache.leds.child_lock = false;
        displayCache.leds.no_zone = false;
        for (var j = 0; j < 4; j++) displayCache.leds.head_select[j] = false;
        if (levelLED) levelLED.clear(displayCache.leds);
    }

    function resetIdleTimer() {
        idleSince = Date.now();
        offSince = 0;
    }

    /* 同步全部状态LED: 从engine state求值→写入displayCache.leds */
    function syncStatusLEDs() {
        if (!statusLEDs.length) return;
        for (var i = 0; i < statusLEDs.length; i++) {
            var sl = statusLEDs[i];

            if (sl.states) {
                /* states-map型: global_mode → behavior (power LED) */
                var gm = globalState.mode;
                var behavior = sl.states[gm] || sl.states['default'] || 'on';
                sl.syncToCache(displayCache.leds, (behavior !== 'off'));
            } else if (sl.flag) {
                /* flag型: 直接读 globalState[flag] */
                sl.syncToCache(displayCache.leds, !!globalState[sl.flag]);
            } else if (sl.condition === 'any_timer_active') {
                sl.syncToCache(displayCache.leds, checkAnyTimer());
            } else if (sl.condition === 'head_selecting') {
                for (var j = 0; j < 4; j++) {
                    sl.syncToCache(displayCache.leds,
                        heads[j] && heads[j].node === 'selecting', j);
                }
            }
        }
    }

    /* 向后兼容: 旧 display.led_rules 路径 */
    function readLEDRule(ledName) {
        if (!rules || !rules.display || !rules.display.led_rules) return null;
        var rule = rules.display.led_rules[ledName];
        if (!rule) return null;
        if (rule.global_mode) {
            var gm = rule.global_mode;
            return gm[globalState.mode] || gm['default'] || 'on';
        }
        return null;
    }

    /* --- 显示更新 --- */
    function updateAllDisplays() {
        for (var i = 0; i < 4; i++) updateDisplayForHead(i);
    }

    function displayCharForHead(h) {
        if (slots && slots[h.index]) {
            return slots[h.index].render(h);
        }
        /* 回退: 旧 display 段 + 硬编码 (slots未初始化时) */
        if (rules && rules.display && rules.display.priority_chain) {
            var chain = rules.display.priority_chain;
            for (var i = 0; i < chain.length; i++) {
                var item = chain[i];
                if (item === 'timer_setting' && h.timer_setting) {
                    var t = h.timer_value;
                    if (t >= 10) return [String(Math.floor(t/10)), String(t % 10)];
                    else return [String(t), ' '];
                }
                if (item === 'boost_active' && h.boost_active) return ['P', ' '];
                if (item === 'power_level') {
                    if (h.power_level === 0) return ['0', '0'];
                    return [String(h.power_level), ' '];
                }
            }
            return ['0', '0'];
        }
        if (h.timer_setting) {
            var t2 = h.timer_value;
            if (t2 >= 10) return [String(Math.floor(t2/10)), String(t2 % 10)];
            else return [String(t2), ' '];
        }
        if (h.boost_active) return ['P', ' '];
        if (h.power_level === 0) return ['0', '0'];
        return [String(h.power_level), ' '];
    }

    function updateDisplayForHead(index) {
        var h = heads[index];
        var base = index * 2;
        var chars = displayCharForHead(h);
        displayCache.seg_chars[base]   = chars[0];
        displayCache.seg_chars[base+1] = chars[1];
    }

    function getDisplayRules() {
        var blinkMs = (blinkRule) ? blinkRule.phaseMs : 500;
        var altMs = 5000;
        if (rules && rules.display && rules.display.alternate_rule && rules.display.alternate_rule.phases)
            altMs = rules.display.alternate_rule.phases[0].duration_ms || 5000;
        return { blink_phase_ms: blinkMs, alternate_phase_ms: altMs };
    }

    function postDisplay() {
        /* 从全局状态+炉头状态派生 seg_mode (优先级链) */
        if (modeRule && globalState.mode !== 'power_on_seq' && globalState.mode !== 'version_show') {
            modeRule.syncToCache(globalState, heads, displayCache);
        }
        /* 从 head 状态派生闪烁 (含暂停覆盖) */
        if (blinkRule) blinkRule.syncToCache(heads, displayCache, globalState);
        MessageBus.post(MsgId.MSG_DISPLAY_REFRESH, 0, {
            seg_chars: displayCache.seg_chars.slice(),
            seg_blink: displayCache.seg_blink.slice(),
            seg_mode: displayCache.seg_mode,
            seg_ascii: displayCache.seg_ascii,
            leds: JSON.parse(JSON.stringify(displayCache.leds)),
            global_mode: globalState.mode,
            hot_head: hotHead,
            select_stack: selectStack.slice(),
            display_rules: getDisplayRules(),
            heads: heads.map(function(h) {
                return {
                    node: h.node, power_level: h.power_level,
                    boost_active: h.boost_active,
                    timer_setting: h.timer_setting,
                    timer_active: h.timer_active, timer_value: h.timer_value
                };
            })
        });
    }

    /* ========== 查询接口 ========== */
    function getState() {
        return {
            global_mode: globalState.mode,
            child_lock: globalState.child_lock,
            paused: globalState.paused,
            hot_head: hotHead,
            select_stack: selectStack.slice(),
            heads: heads.map(function(h) {
                return {
                    node: h.node, power_level: h.power_level,
                    boost_active: h.boost_active,
                    timer_setting: h.timer_setting,
                    timer_active: h.timer_active, timer_value: h.timer_value
                };
            })
        };
    }

    function getDisplay() { return displayCache; }
    function getRules() { return rules; }

    function reset() {
        MessageBus.flush();
        init(rules);
    }

    /* ========== 测试辅助 ========== */
    function setFastMode(enabled) {
        fastMode = enabled;
        console.log('[JsonLogicEngine] fastMode=%s', enabled);
    }

    function forceSelectTimeout(idx) {
        if (idx === undefined) idx = hotHead;
        var h = heads[idx];
        if (h && h.node === 'selecting') confirmSelect(idx);
        for (var i = 0; i < 4; i++) {
            if (heads[i].timer_setting) confirmTimer(i);
        }
    }

    function forceBoostTimeout(idx) {
        if (idx === undefined) idx = hotHead;
        var h = heads[idx];
        if (h && h.boost_active) { h.boost_remaining = 0; exitBoost(idx); }
    }

    function forceTimerExpire(idx) {
        if (idx === undefined) idx = hotHead;
        var h = heads[idx];
        if (h && h.timer_active && h.timer_value > 0) {
            h.timer_active = false;
            h.timer_value  = 0;
            h.node         = 'idle';
            h.power_level  = 0;
            h.boost_active = false;
            h.boost_remaining = 0;
            removeFromStack(idx);
            reassignHotHead(idx);
            updateDisplayForHead(idx);
        }
    }

    return {
        init: init, getState: getState, getDisplay: getDisplay, getRules: getRules,
        reset: reset,
        setFastMode: setFastMode, isFastMode: function() { return fastMode; },
        forceSelectTimeout: forceSelectTimeout,
        forceBoostTimeout: forceBoostTimeout,
        forceTimerExpire: forceTimerExpire,
        KEY_NAME: KEY_NAME, keyNameToCode: keyNameToCode
    };
})();
