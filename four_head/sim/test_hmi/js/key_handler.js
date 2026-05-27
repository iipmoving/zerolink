/**
 * key_handler.js — 按键处理器
 *
 * Canvas点击 → 按键识别 → 长按/短按检测 → Msg_Post(MSG_KEY_EVENT)
 * 编码方式与 drv_key.c 一致: param = (key_state << 8) | key_code
 */

const KeyHandler = (function() {
    const LONG_THRESHOLD_MS = 1500;   /* 长按阈值 1.5s */
    const REPEAT_INTERVAL_MS = 300;   /* 连发间隔 300ms */

    let controls = [];          /* fourSave.json controls 数组 */
    let pressedKey = null;      /* 当前按下的按键信息 */
    let pressStartTime = 0;     /* 按下时刻 */
    let longFired = false;      /* 长按是否已触发 */
    let repeatTimer = null;     /* 连发定时器 */
    let longCheckTimer = null;  /* 长按检测定时器 */

    /**
     * 从 fourSave.json 加载控件配置
     * @param {Array} controlList - fourSave.json.controls
     */
    function loadControls(controlList) {
        controls = controlList.filter(function(c) { return c.type === 'button'; });
        console.log('[KeyHandler] 加载 %d 个按键控件', controls.length);
    }

    /**
     * 处理鼠标按下
     * @param {number} x - Canvas坐标X
     * @param {number} y - Canvas坐标Y
     */
    function onMouseDown(x, y) {
        var btn = findButton(x, y);
        if (!btn) return;

        pressedKey = btn;
        pressStartTime = Date.now();
        longFired = false;

        /* 发送 PRESS 事件 */
        MessageBus.post(MsgId.MSG_KEY_EVENT,
            (KeyEvent.PRESS << 8) | btn.config.key_code, null);

        /* 启动长按检测(1.5s) */
        longCheckTimer = setTimeout(function() {
            if (pressedKey === btn && !longFired) {
                longFired = true;
                MessageBus.post(MsgId.MSG_KEY_EVENT,
                    (KeyEvent.LONG << 8) | btn.config.key_code, null);

                /* 启动连发 */
                repeatTimer = setInterval(function() {
                    if (pressedKey === btn) {
                        MessageBus.post(MsgId.MSG_KEY_EVENT,
                            (KeyEvent.REPEAT << 8) | btn.config.key_code, null);
                    }
                }, REPEAT_INTERVAL_MS);
            }
        }, LONG_THRESHOLD_MS);

        console.log('[KeyHandler] PRESS: %s (keyCode=%d)',
            btn.name, btn.config.key_code);
    }

    /**
     * 处理鼠标释放
     */
    function onMouseUp() {
        if (!pressedKey) return;

        clearTimeout(longCheckTimer);
        clearInterval(repeatTimer);
        longCheckTimer = null;
        repeatTimer = null;

        var duration = Date.now() - pressStartTime;
        var btn = pressedKey;

        /* 发送 RELEASE 事件 */
        MessageBus.post(MsgId.MSG_KEY_EVENT,
            (KeyEvent.RELEASE << 8) | btn.config.key_code, null);

        /* 短按(<阈值)且未触发长按 → TAP */
        if (duration < LONG_THRESHOLD_MS && !longFired) {
            MessageBus.post(MsgId.MSG_KEY_EVENT,
                (KeyEvent.TAP << 8) | btn.config.key_code, null);
            console.log('[KeyHandler] TAP: %s (keyCode=%d, duration=%dms)',
                btn.name, btn.config.key_code, duration);
        }

        pressedKey = null;
    }

    /**
     * 根据坐标查找按键控件
     */
    function findButton(x, y) {
        for (var i = 0; i < controls.length; i++) {
            var c = controls[i];
            if (x >= c.x && x <= c.x + c.width &&
                y >= c.y && y <= c.y + c.height) {
                return c;
            }
        }
        return null;
    }

    /**
     * 测试命令注入——模拟按键序列
     * @param {number} keyCode
     * @param {string} eventName - 'press'|'tap'|'long'|'release'
     */
    function injectKey(keyCode, eventName) {
        var evtMap = {
            'press':   KeyEvent.PRESS,
            'tap':     KeyEvent.TAP,
            'long':    KeyEvent.LONG,
            'release': KeyEvent.RELEASE,
            'repeat':  KeyEvent.REPEAT
        };
        var evt = evtMap[eventName] || KeyEvent.TAP;
        MessageBus.post(MsgId.MSG_KEY_EVENT, (evt << 8) | keyCode, null);
        console.log('[KeyHandler] 注入按键: keyCode=%d event=%s', keyCode, eventName);
    }

    return { loadControls, onMouseDown, onMouseUp, injectKey };
})();
