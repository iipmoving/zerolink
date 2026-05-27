/**
 * renderer.js — Canvas渲染器 (V3 — 3态+正交进程)
 *
 * 加载 fourSave.json → 绘制所有控件 → 监听 MSG_DISPLAY_REFRESH → 更新显示
 *
 * 显示决策（渲染器内部）:
 *   1. seg_mode=off → 全灭
 *   2. seg_mode=ascii → 显示 seg_ascii (PA / 版本号)
 *   3. seg_mode=alternating → timer_active 的炉头: 5s档位 / 5s时间交替
 *   4. seg_mode=power/dash/timer_setting → 正常绘制 seg_chars
 *   5. seg_blink[head] → 该炉头 500ms 闪烁
 *
 * Segment映射:
 *   POT1显示上(4位): 左2位=炉头1(左上), 右2位=炉头4(右上)
 *   POT1显示下(4位): 左2位=炉头2(左下), 右2位=炉头3(右下)
 */

const Renderer = (function() {
    var canvas = null;
    var ctx = null;
    var config = null;
    var displayData = null;
    var bgImage = null;
    var hotHead = 0;

    /* ========== 初始化 ========== */
    function init(canvasEl, uiConfig, onReady) {
        canvas = canvasEl;
        ctx = canvas.getContext('2d');
        config = uiConfig;

        if (config.canvas_size) {
            canvas.width = config.canvas_size.width;
            canvas.height = config.canvas_size.height;
        }

        if (config.background && config.background.data) {
            bgImage = new Image();
            bgImage.onload = function() { draw(); if (onReady) onReady(); };
            bgImage.src = config.background.data;
        } else {
            if (onReady) onReady();
        }

        MessageBus.register(MsgId.MSG_DISPLAY_REFRESH, onDisplayRefresh);
        draw();
        console.log('[Renderer] 初始化完成');
    }

    /* ========== 显示刷新回调 ========== */
    function onDisplayRefresh(msgId, param, data_ptr) {
        displayData = data_ptr;
        if (data_ptr && data_ptr.hot_head !== undefined) {
            hotHead = data_ptr.hot_head;
        }
        draw();
    }

    /* ========== 主绘制 ========== */
    function draw() {
        if (!ctx || !config) return;

        ctx.clearRect(0, 0, canvas.width, canvas.height);

        if (bgImage) {
            var imgRatio = bgImage.naturalWidth / bgImage.naturalHeight;
            var canvasRatio = canvas.width / canvas.height;
            var dw, dh, dx, dy;
            if (imgRatio > canvasRatio) {
                dw = canvas.width;
                dh = canvas.width / imgRatio;
                dx = 0;
                dy = (canvas.height - dh) / 2;
            } else {
                dh = canvas.height;
                dw = canvas.height * imgRatio;
                dx = (canvas.width - dw) / 2;
                dy = 0;
            }
            ctx.drawImage(bgImage, dx, dy, dw, dh);
        } else {
            ctx.fillStyle = '#1a1a1a';
            ctx.fillRect(0, 0, canvas.width, canvas.height);
        }

        var controls = config.controls || [];
        /* LED和段码先画 */
        for (var i = 0; i < controls.length; i++) {
            var c = controls[i];
            if (c.type === 'led') drawLED(c);
            else if (c.type === 'segment') drawSegment(c);
        }
        /* 按钮后画(上层可点击) */
        for (var j = 0; j < controls.length; j++) {
            var b = controls[j];
            if (b.type === 'button') drawButton(b);
        }
    }

    /* ========== 按键绘制 ========== */
    function drawButton(ctrl) {
        var x = ctrl.x, y = ctrl.y, w = ctrl.width, h = ctrl.height;
        var style = ctrl.style || {};

        ctx.fillStyle = style.background_color || '#2a2a2a';
        ctx.fillRect(x, y, w, h);

        ctx.strokeStyle = style.border_color || '#555';
        ctx.lineWidth = 1;
        ctx.strokeRect(x, y, w, h);

        ctx.fillStyle = style.text_color || '#ccc';
        ctx.font = (style.font_size || 12) + 'px sans-serif';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText(ctrl.name || '', x + w/2, y + h/2);
    }

    /* ========== LED绘制 ========== */
    function drawLED(ctrl) {
        var x = ctrl.x, y = ctrl.y, w = ctrl.width || 10, h = ctrl.height || 10;
        var style = ctrl.style || {};
        var ledOn = isLEDOn(ctrl.name);

        /* 电源灯闪烁 (POWERED_OFF) */
        if (ctrl.name === '开关灯' && displayData &&
            displayData.global_mode === 'powered_off') {
            ledOn = Math.floor(Date.now() / 500) % 2 === 0;
        }

        if (ledOn) {
            var glow = ctx.createRadialGradient(x+w/2, y+h/2, 0, x+w/2, y+h/2, Math.min(w,h)*0.8);
            glow.addColorStop(0, style.on_color || '#00ff00');
            glow.addColorStop(1, 'rgba(0,0,0,0)');
            ctx.fillStyle = glow;
            ctx.beginPath();
            ctx.arc(x + w/2, y + h/2, Math.min(w,h)*0.8, 0, Math.PI * 2);
            ctx.fill();
        }

        ctx.fillStyle = ledOn ? (style.on_color || '#00ff00') : (style.off_color || '#333');
        ctx.beginPath();
        ctx.arc(x + w/2, y + h/2, Math.min(w, h)/2.5, 0, Math.PI * 2);
        ctx.fill();
    }

    function isLEDOn(name) {
        if (!displayData || !displayData.leds) return false;
        var l = displayData.leds;
        if (name === '开关灯')      return l.power;
        if (name === '童锁灯')      return l.child_lock;
        if (name === '定时灯')      return l.timer;
        if (name === '短暂降低灯')  return l.pause_btn;
        if (name === '档位0灯')     return l.power_level[0];
        if (name === '档位1灯')     return l.power_level[1];
        if (name === '档位2灯')     return l.power_level[2];
        if (name === '档位3灯')     return l.power_level[3];
        if (name === '档位4灯')     return l.power_level[4];
        if (name === '档位5灯')     return l.power_level[5];
        if (name === '档位6灯')     return l.power_level[6];
        if (name === '档位7灯')     return l.power_level[7];
        if (name === '档位8灯')     return l.power_level[8];
        if (name === '档位9-P灯')   return l.power_level[9];
        if (name === '新LED')       return l.no_zone;
        return false;
    }

    /* ========== 数码管绘制 ========== */
    function drawSegment(ctrl) {
        var x = ctrl.x, y = ctrl.y, w = ctrl.width, h = ctrl.height;
        var style = ctrl.style || {};

        ctx.fillStyle = style.background_color || 'rgba(0,0,0,0.6)';
        ctx.fillRect(x, y, w, h);

        if (!displayData) return;

        var segMode = displayData.seg_mode || 'power';
        if (segMode === 'off') return;

        var blinkMs = (displayData.display_rules && displayData.display_rules.blink_phase_ms) || 500;
        var altMs   = (displayData.display_rules && displayData.display_rules.alternate_phase_ms) || 5000;
        var blinkPhase500 = Math.floor(Date.now() / blinkMs) % 2;
        var altPhase5000 = Math.floor(Date.now() / altMs) % 2;

        /* 颜色 */
        var color = style.text_color || '#ff4444';
        if (segMode === 'ascii') color = '#ff8800';
        else if (segMode === 'timer_setting') color = '#44aaff';
        else if (segMode === 'dash') color = '#ff6666';

        var fontSize = style.font_size || 32;
        ctx.font = fontSize + 'px monospace';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';

        var headW = w / 2.2;
        var headX = x + w * 0.08;
        var leftCX  = headX + headW * 0.3;
        var rightCX = headX + headW * 1.6;

        if (ctrl.name === 'POT1显示上') {
            drawHeadSlot(0, leftCX,  y + h/2, headW * 0.6, blinkPhase500, altPhase5000, color, fontSize);
            drawHeadSlot(3, rightCX, y + h/2, headW * 0.6, blinkPhase500, altPhase5000, color, fontSize);
        } else if (ctrl.name === 'POT1显示下') {
            drawHeadSlot(1, leftCX,  y + h/2, headW * 0.6, blinkPhase500, altPhase5000, color, fontSize);
            drawHeadSlot(2, rightCX, y + h/2, headW * 0.6, blinkPhase500, altPhase5000, color, fontSize);
        }

        /* 热点炉头下划线 */
        for (var headIdx = 0; headIdx < 4; headIdx++) {
            if (headIdx !== hotHead) continue;
            var underlineY = y + h * 0.85;
            var underlineX, underlineW;
            if (ctrl.name === 'POT1显示上') {
                if (headIdx === 0) { underlineX = headX;        underlineW = headW * 0.6; }
                if (headIdx === 3) { underlineX = headX + headW * 1.3; underlineW = headW * 0.6; }
            } else if (ctrl.name === 'POT1显示下') {
                if (headIdx === 1) { underlineX = headX;        underlineW = headW * 0.6; }
                if (headIdx === 2) { underlineX = headX + headW * 1.3; underlineW = headW * 0.6; }
            }
            if (underlineX) {
                ctx.strokeStyle = '#00ff00';
                ctx.lineWidth = 2;
                ctx.beginPath();
                ctx.moveTo(underlineX, underlineY);
                ctx.lineTo(underlineX + underlineW, underlineY);
                ctx.stroke();
            }
        }
    }

    /* 绘制一个炉头的2位数码管区 (含交替/闪烁) */
    function drawHeadSlot(headIdx, cx, cy, slotW, blinkPhase, altPhase, color, fontSize) {
        var head = displayData.heads ? displayData.heads[headIdx] : null;
        var chars = getSlotChars(headIdx, head, altPhase);

        /* 闪烁: selecting 态 + 500ms暗相 = 不绘 */
        var blinkArr = displayData.seg_blink;
        if (blinkArr && blinkArr[headIdx] && blinkPhase === 0) return;

        ctx.fillStyle = color;
        ctx.font = fontSize + 'px monospace';
        ctx.fillText(chars, cx, cy);
    }

    /* 决策该炉头当前显示的2字符 */
    function getSlotChars(headIdx, head, altPhase) {
        var base = headIdx * 2;

        /* timer_active → 交替显示 */
        if (head && head.timer_active && displayData.seg_mode !== 'timer_setting') {
            if (altPhase === 1) {
                /* 时间相位: 显示 timer_value */
                var tv = head.timer_value;
                if (tv >= 10) return String(Math.floor(tv/10)) + String(tv % 10);
                else return String(tv) + ' ';
            }
            /* altPhase=0: 档位相位, 继续用 seg_chars */
        }

        var c1 = displayData.seg_chars[base] || ' ';
        var c2 = displayData.seg_chars[base+1] || ' ';
        return c1 + c2;
    }

    /* ========== 辅助 ========== */
    function getDisplayData() { return displayData; }
    function redraw() { draw(); }

    return { init, redraw, getDisplayData };
})();
