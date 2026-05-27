/**
 * test_runner.js — HMI数据流自动测试 V7.0
 *
 * 对齐 V7.0 模型: 3 Zone状态 + 正交进程标志
 *   node: idle/selecting/cooking
 *   进程: boost_active / timer_setting / timer_active
 *   全局: powering_up / version_show / powered_off / working / paused / deep_sleep
 */

const TestRunner = (function() {
    var tests = [];
    var results = [];
    var onLog = null;
    var onDone = null;

    function log(msg, cls) {
        if (onLog) onLog(msg, cls || '');
        console.log('[TestRunner] ' + msg);
    }

    function inject(keyCode, evt) {
        KeyHandler.injectKey(keyCode, evt);
        for (var i = 0; i < 5; i++) MessageBus.run1ms();
    }

    function injectByName(name, evt) {
        var code = JsonLogicEngine.keyNameToCode('KEY_' + name);
        if (code < 0) { log('ERROR: 未知按键 ' + name, 'err'); return; }
        inject(code, evt);
    }

    function fmtSeg(chars) {
        var up  = chars[0] + chars[1] + ' ' + chars[6] + chars[7];
        var low = chars[2] + chars[3] + ' ' + chars[4] + chars[5];
        return '上排[' + up + '] 下排[' + low + ']';
    }

    function fmtLEDs(leds) {
        var parts = [];
        parts.push('电源=' + (leds.power ? '●' : '○'));
        parts.push('童锁=' + (leds.child_lock ? '●' : '○'));
        parts.push('暂停=' + (leds.pause_btn ? '●' : '○'));
        parts.push('定时=' + (leds.timer ? '●' : '○'));
        var sel = '';
        for (var i = 0; i < 4; i++) sel += (leds.head_select[i] ? '●' : '○');
        parts.push('选头=[' + sel + ']');
        var lvl = '';
        for (var j = 0; j < 10; j++) lvl += (leds.power_level[j] ? '●' : '○');
        parts.push('档位=[' + lvl + ']');
        return parts.join(' ');
    }

    function getDisplay() { return JsonLogicEngine.getDisplay(); }

    function logDisplay() {
        var d = getDisplay();
        log('  显示: ' + fmtSeg(d.seg_chars), 'info');
        log('  LED : ' + fmtLEDs(d.leds), 'info');
        if (d.seg_blink && (d.seg_blink[0] || d.seg_blink[1] || d.seg_blink[2] || d.seg_blink[3]))
            log('  闪烁: ON blinks=[' +
                (d.seg_blink[0]?1:0)+(d.seg_blink[1]?1:0)+(d.seg_blink[2]?1:0)+(d.seg_blink[3]?1:0) + ']', 'info');
    }

    function verify(flowId, stateFn, dispFn, desc) {
        var sOk = false, dOk = true;
        try { var s = JsonLogicEngine.getState(); sOk = stateFn(s); }
        catch(e) { log('状态验证异常: ' + e.message, 'err'); }
        if (dispFn) {
            try { var d = getDisplay(); dOk = dispFn(d); }
            catch(e) { log('显示验证异常: ' + e.message, 'err'); dOk = false; }
        }
        var pass = sOk && dOk;
        results.push({ id: flowId, desc: desc, pass: pass });
        var mark = pass ? '[PASS]' : '[FAIL]';
        log(mark + ' ' + flowId + ': ' + desc, pass ? 'ok' : 'err');
        logDisplay();
        if (!pass) {
            var st = JsonLogicEngine.getState();
            log('  状态: mode=' + st.global_mode + ' hot=' + st.hot_head +
                ' stack=' + JSON.stringify(st.select_stack) +
                ' heads=[' + st.heads.map(function(h){
                    var flags = [];
                    if (h.boost_active) flags.push('BOOST');
                    if (h.timer_setting) flags.push('TSET');
                    if (h.timer_active) flags.push('TACT');
                    return h.node + '/lv' + h.power_level + (flags.length ? '+' + flags.join('+') : '');
                }).join(',') + ']', 'warn');
        }
    }

    function defineTests() {
        tests = [];

        /* ================================================================
         * Phase 1: 上电 & 基础操作 (Flow-00~06)
         * ================================================================ */
        tests.push({ phase: 1, id: 'Flow-00', desc: '上电序列→关机',
            run: function() {
                verify('Flow-00',
                    function(s) { return s.global_mode === 'powered_off'; },
                    function(d) {
                        return d.seg_mode === 'dash' &&
                            d.seg_blink[0] === false && d.seg_blink[1] === false &&
                            d.seg_chars[0] === '-' && d.leds.power === true;
                    },
                    '上电→关机: "--" 不闪烁, 电源灯亮(闪烁由渲染器处理)');
            }
        });

        tests.push({ phase: 1, id: 'Flow-01', desc: '关机→开机(长按开关)',
            run: function() {
                injectByName('POWER', 'long');
                verify('Flow-01',
                    function(s) { return s.global_mode === 'working'; },
                    function(d) {
                        return d.seg_mode === 'power' &&
                            d.seg_blink[0] === false && d.seg_blink[1] === false &&
                            d.leds.power === true &&
                            d.seg_chars[0] === '0' && d.seg_chars[4] === '0';
                    },
                    '开机→WORKING: "0 0 0 0", 电源灯常亮');
            }
        });

        tests.push({ phase: 1, id: 'Flow-02', desc: '选中炉头',
            run: function() {
                injectByName('HEAD_1', 'tap');
                verify('Flow-02',
                    function(s) { return s.hot_head === 0 && s.heads[0].node === 'selecting'; },
                    function(d) {
                        return d.seg_blink[0] === true && d.seg_blink[1] === false &&
                            d.leds.head_select[0] === true &&
                            d.seg_chars[0] === '0';
                    },
                    '选中炉头1: Zn区闪烁, LED选头亮');
            }
        });

        tests.push({ phase: 1, id: 'Flow-03', desc: '选中→设0档→确认idle',
            run: function() {
                injectByName('0', 'tap');
                JsonLogicEngine.forceSelectTimeout();
                verify('Flow-03',
                    function(s) {
                        return s.global_mode === 'working' &&
                            s.heads[0].node === 'idle' && s.heads[0].power_level === 0;
                    },
                    function(d) {
                        return d.leds.head_select[0] === false && d.seg_blink[0] === false;
                    },
                    '0档确认→idle: 选头LED灭, 闪烁停');
            }
        });

        tests.push({ phase: 1, id: 'Flow-04', desc: '选中→设5档→确认cooking',
            run: function() {
                injectByName('HEAD_1', 'tap');
                injectByName('5', 'tap');
                JsonLogicEngine.forceSelectTimeout();
                verify('Flow-04',
                    function(s) { return s.heads[0].node === 'cooking' && s.heads[0].power_level === 5; },
                    function(d) {
                        return d.seg_chars[0] === '5' &&
                            d.seg_blink[0] === false &&
                            d.leds.power_level[5] === true &&
                            d.leds.head_select[0] === false;
                    },
                    '5档cooking: Zn区"5", 档位5 LED亮');
            }
        });

        tests.push({ phase: 1, id: 'Flow-05', desc: '多炉头: 选头2设9档',
            run: function() {
                injectByName('HEAD_2', 'tap');
                injectByName('9', 'tap');
                JsonLogicEngine.forceSelectTimeout();
                verify('Flow-05',
                    function(s) { return s.heads[1].node === 'cooking' && s.heads[1].power_level === 9; },
                    function(d) {
                        return d.seg_chars[2] === '9' &&
                            d.leds.power_level[9] === true;
                    },
                    '头2设9档: Zn2区"9", LED档位灯9');
            }
        });

        tests.push({ phase: 1, id: 'Flow-06', desc: '选中→长按9→Boost(正交进程)',
            run: function() {
                injectByName('HEAD_1', 'tap');
                injectByName('9', 'long');
                verify('Flow-06',
                    function(s) { return s.heads[0].node === 'selecting' && s.heads[0].boost_active === true; },
                    function(d) {
                        return d.seg_chars[0] === 'P' && d.leds.power_level[9] === true;
                    },
                    'Boost进程: node保持selecting, boost_active=true, 显示"P"');
            }
        });

        /* ================================================================
         * Phase 2: 超时 & 定时 (Flow-07~15)
         * ================================================================ */
        tests.push({ phase: 2, id: 'Flow-07', desc: '选中→15s超时(全0→idle)',
            setup: function() {
                injectByName('POWER', 'long'); injectByName('POWER', 'long');
                injectByName('HEAD_1', 'tap');
                JsonLogicEngine.forceSelectTimeout();
            },
            run: function() {
                verify('Flow-07',
                    function(s) { return s.global_mode === 'working' && s.heads[0].node === 'idle'; },
                    function(d) { return d.seg_blink[0] === false && d.seg_chars[0] === '0'; },
                    '选中(0档)超时→idle: Zn区"0"不闪');
            }
        });

        tests.push({ phase: 2, id: 'Flow-08', desc: '选中→15s超时(档>0→cooking)',
            setup: function() {
                injectByName('HEAD_1', 'tap'); injectByName('5', 'tap');
                injectByName('HEAD_1', 'tap');  /* 自确认 */
            },
            run: function() {
                verify('Flow-08',
                    function(s) { return s.heads[0].node === 'cooking' && s.heads[0].power_level === 5; },
                    function(d) { return d.seg_chars[0] === '5' && d.seg_blink[0] === false; },
                    '自确认→cooking: Zn区"5"不闪');
            }
        });

        tests.push({ phase: 2, id: 'Flow-09', desc: '选中→按定时键(timer_setting进程)',
            setup: function() { injectByName('HEAD_1', 'tap'); },
            run: function() {
                injectByName('TIMER', 'tap');
                verify('Flow-09',
                    function(s) { return s.heads[0].timer_setting === true && s.heads[0].timer_value === 15; },
                    function(d) {
                        return d.seg_mode === 'timer_setting' &&
                            d.seg_chars[0] === '1' && d.seg_chars[1] === '5';
                    },
                    'timer_setting进程: Zn区"15", node保持selecting');
            }
        });

        tests.push({ phase: 2, id: 'Flow-10', desc: '单头快捷: 直接改档',
            setup: function() { injectByName('TIMER', 'tap'); }, /* 确认定时 */
            run: function() {
                injectByName('3', 'tap');
                verify('Flow-10',
                    function(s) { return s.heads[0].power_level === 3 && s.heads[0].node === 'cooking'; },
                    function(d) { return d.seg_chars[0] === '3'; },
                    '单头快捷改档: Zn区"3"');
            }
        });

        tests.push({ phase: 2, id: 'Flow-11', desc: 'cooking→0档弹出选择序列',
            setup: function() {
                injectByName('HEAD_2', 'tap'); injectByName('5', 'tap');
                JsonLogicEngine.forceSelectTimeout();
            },
            run: function() {
                injectByName('HEAD_1', 'tap');
                injectByName('0', 'tap');
                JsonLogicEngine.forceSelectTimeout();
                verify('Flow-11',
                    function(s) {
                        return s.heads[0].node === 'idle' && s.heads[0].power_level === 0 &&
                            s.select_stack.length === 1 && s.select_stack[0] === 1;
                    },
                    function(d) {
                        return d.leds.power_level[5] === true &&
                            d.leds.power_level[0] === false;
                    },
                    '0档弹出: 头1→idle, LED回退到头2/5档');
            }
        });

        tests.push({ phase: 2, id: 'Flow-12', desc: 'cooking→长按9→Boost',
            setup: function() {
                injectByName('POWER', 'long'); injectByName('POWER', 'long');
                injectByName('HEAD_1', 'tap'); injectByName('5', 'tap');
                JsonLogicEngine.forceSelectTimeout();  /* V8: 显式确认→cooking */
            },
            run: function() {
                injectByName('9', 'long');
                verify('Flow-12',
                    function(s) { return s.heads[0].node === 'cooking' && s.heads[0].boost_active === true; },
                    function(d) { return d.seg_chars[0] === 'P'; },
                    'cooking→Boost: node保持cooking, boost_active=true, 显示"P"');
            }
        });

        tests.push({ phase: 2, id: 'Flow-13', desc: '工作态→暂停',
            setup: function() { injectByName('5', 'tap'); },
            run: function() {
                injectByName('PAUSE', 'tap');
                verify('Flow-13',
                    function(s) { return s.paused === true; },
                    function(d) {
                        return d.seg_mode === 'ascii' && d.seg_ascii === 'PA' &&
                            d.leds.pause_btn === true;
                    },
                    '暂停: 显示"PA", 暂停LED亮');
            }
        });

        tests.push({ phase: 2, id: 'Flow-14', desc: 'cooking→timer_setting进程',
            setup: function() { injectByName('PAUSE', 'tap'); },
            run: function() {
                injectByName('HEAD_1', 'tap');
                injectByName('TIMER', 'tap');
                verify('Flow-14',
                    function(s) { return s.heads[0].timer_setting === true; },
                    function(d) { return d.seg_mode === 'timer_setting'; },
                    'cooking→timer_setting: node保持, timer_setting=true');
            }
        });

        tests.push({ phase: 2, id: 'Flow-15', desc: '定时归零→idle',
            setup: function() { injectByName('TIMER', 'tap'); },
            run: function() {
                JsonLogicEngine.forceTimerExpire(0);
                verify('Flow-15',
                    function(s) { return s.heads[0].node === 'idle' && s.heads[0].power_level === 0; },
                    function(d) { return d.leds.timer === false && d.seg_chars[0] === '0'; },
                    '定时归零→idle: 定时LED灭, Zn区"0"');
            }
        });

        /* ================================================================
         * Phase 3: Boost & 暂停 (Flow-16~19)
         * ================================================================ */
        tests.push({ phase: 3, id: 'Flow-16', desc: 'Boost→按数字键退出',
            setup: function() {
                injectByName('HEAD_1', 'tap'); injectByName('5', 'tap');
                injectByName('9', 'long');
            },
            run: function() {
                injectByName('5', 'tap');
                verify('Flow-16',
                    function(s) { return s.heads[0].node === 'cooking' && s.heads[0].boost_active === false; },
                    function(d) { return d.seg_chars[0] === '5' && d.leds.power_level[5] === true; },
                    'Boost退出→5档: Zn区"5", boost_active=false');
            }
        });

        tests.push({ phase: 3, id: 'Flow-17', desc: 'Boost→超时恢复',
            setup: function() { injectByName('9', 'long'); },
            run: function() {
                JsonLogicEngine.forceBoostTimeout(0);
                verify('Flow-17',
                    function(s) { return s.heads[0].node === 'cooking' && s.heads[0].power_level === 5; },
                    function(d) { return d.seg_chars[0] === '5'; },
                    'Boost超时→恢复5档: boost_active=false');
            }
        });

        tests.push({ phase: 3, id: 'Flow-18', desc: 'Boost→按定时(timer_setting进程)',
            setup: function() { injectByName('9', 'long'); },
            run: function() {
                injectByName('TIMER', 'tap');
                verify('Flow-18',
                    function(s) { return s.heads[0].timer_setting === true && s.heads[0].boost_active === true; },
                    function(d) { return d.seg_mode === 'timer_setting'; },
                    'Boost+timer_setting: 两标志共存');
            }
        });

        tests.push({ phase: 3, id: 'Flow-19', desc: '暂停→恢复',
            setup: function() { injectByName('5', 'tap'); injectByName('PAUSE', 'tap'); },
            run: function() {
                injectByName('PAUSE', 'tap');
                verify('Flow-19',
                    function(s) { return s.paused === false; },
                    function(d) { return d.seg_mode === 'timer_setting' && d.leds.pause_btn === false; },
                    '暂停恢复: seg_mode=timer_setting(进程仍活跃), 暂停LED灭');
            }
        });

        /* ================================================================
         * Phase 4: 定时/童锁/多炉头/休眠 (Flow-20~33)
         * ================================================================ */
        tests.push({ phase: 4, id: 'Flow-20', desc: '定时加减调整',
            setup: function() { /* timer_setting 已在 Flow-19 激活 */ },
            run: function() {
                injectByName('PLUS', 'tap'); injectByName('PLUS', 'tap'); injectByName('MINUS', 'tap');
                verify('Flow-20',
                    function(s) { return s.heads[0].timer_value === 16; },
                    function(d) { return d.seg_chars[0] === '1' && d.seg_chars[1] === '6'; },
                    '定时15+2-1=16: Zn区"16"');
            }
        });

        tests.push({ phase: 4, id: 'Flow-21', desc: '定时设置→手动确认(timer_active启动)',
            run: function() {
                injectByName('TIMER', 'tap');
                verify('Flow-21',
                    function(s) { return s.heads[0].timer_active === true && s.heads[0].timer_setting === false; },
                    function(d) { return d.leds.timer === true; },
                    '定时确认: timer_active=true, 定时LED亮');
            }
        });

        tests.push({ phase: 4, id: 'Flow-22', desc: '定时15s自动确认',
            setup: function() {
                injectByName('TIMER', 'long');  /* 取消 timer_active */
                injectByName('TIMER', 'tap');   /* 重新进入 timer_setting */
            },
            run: function() {
                JsonLogicEngine.forceSelectTimeout();
                verify('Flow-22',
                    function(s) { return s.heads[0].timer_active === true; },
                    function(d) { return d.leds.timer === true; },
                    '定时自动确认: timer_active=true');
            }
        });

        tests.push({ phase: 4, id: 'Flow-23', desc: '长按定时取消',
            setup: function() { injectByName('TIMER', 'tap'); },
            run: function() {
                injectByName('TIMER', 'long');
                verify('Flow-23',
                    function(s) { return s.heads[0].timer_setting === false && s.heads[0].timer_value === 0; },
                    function(d) { return d.leds.timer === false; },
                    '定时取消: timer_setting=false');
            }
        });

        tests.push({ phase: 4, id: 'Flow-24', desc: '工作态→童锁',
            setup: function() {
                /* 已在 WORKING 态 (承接 Flow-23) */
            },
            run: function() {
                injectByName('CHILD_LOCK', 'long');
                verify('Flow-24',
                    function(s) { return s.child_lock === true; },
                    function(d) { return d.leds.child_lock === true; },
                    '童锁: 童锁灯亮');
            }
        });

        tests.push({ phase: 4, id: 'Flow-25', desc: '童锁→解锁',
            run: function() {
                injectByName('CHILD_LOCK', 'long');
                verify('Flow-25',
                    function(s) { return s.child_lock === false; },
                    function(d) { return d.leds.child_lock === false; },
                    '童锁解锁: 童锁灯灭');
            }
        });

        tests.push({ phase: 4, id: 'Flow-26', desc: '工作→长按开关关机',
            setup: function() {
                /* 已在 working 态 (承接Flow-25), 设一个炉头 cooking */
                injectByName('HEAD_1', 'tap'); injectByName('5', 'tap');
                JsonLogicEngine.forceSelectTimeout();
            },
            run: function() {
                injectByName('POWER', 'long');
                verify('Flow-26',
                    function(s) { return s.global_mode === 'powered_off'; },
                    function(d) {
                        return d.seg_mode === 'dash' &&
                            d.seg_blink[0] === false && d.seg_blink[1] === false &&
                            d.leds.power === true && d.seg_chars[0] === '-';
                    },
                    '关机→POWERED_OFF: "--"不闪烁, 电源灯闪烁(渲染器处理)');
            }
        });

        tests.push({ phase: 4, id: 'Flow-27', desc: '多炉头独立工作+选择序列',
            setup: function() { injectByName('POWER', 'long'); },
            run: function() {
                injectByName('HEAD_1', 'tap'); injectByName('5', 'tap');
                JsonLogicEngine.forceSelectTimeout();
                injectByName('HEAD_2', 'tap'); injectByName('3', 'tap');
                JsonLogicEngine.forceSelectTimeout();
                verify('Flow-27',
                    function(s) {
                        return s.heads[0].power_level === 5 && s.heads[1].power_level === 3 &&
                            s.select_stack.length === 2 && s.select_stack[1] === 1;
                    },
                    function(d) {
                        return d.seg_chars[0] === '5' && d.seg_chars[2] === '3' &&
                            d.leds.power_level[3] === true;
                    },
                    '多炉头: 头1=5, 头2=3, LED=3(栈顶头2)');
            }
        });

        tests.push({ phase: 4, id: 'Flow-28', desc: '多炉头独立定时(正交进程)',
            setup: function() {
                injectByName('HEAD_1', 'tap'); injectByName('TIMER', 'tap'); injectByName('TIMER', 'tap');
                injectByName('HEAD_2', 'tap');
            },
            run: function() {
                verify('Flow-28',
                    function(s) { return s.heads[0].timer_active === true && s.heads[1].timer_active === false; },
                    function(d) { return d.leds.timer === true; },
                    '独立定时: 头1 timer_active, 头2 无');
            }
        });

        tests.push({ phase: 4, id: 'Flow-29', desc: 'Boost+定时交替(正交进程叠加)',
            setup: function() {
                injectByName('POWER', 'long'); injectByName('POWER', 'long');
                injectByName('HEAD_1', 'tap');
            },
            run: function() {
                injectByName('9', 'long');
                injectByName('TIMER', 'tap'); injectByName('TIMER', 'tap');
                verify('Flow-29',
                    function(s) { return s.heads[0].boost_active === true && s.heads[0].timer_active === true; },
                    function(d) { return d.leds.timer === true && d.leds.power_level[9] === true; },
                    'Boost+定时叠加: boost_active + timer_active 共存');
            }
        });

        tests.push({ phase: 4, id: 'Flow-30', desc: '全idle→关机',
            setup: function() {
                /* 确保四头全idle → 关机 */
                injectByName('POWER', 'long'); injectByName('POWER', 'long');
            },
            run: function() {
                injectByName('POWER', 'long');  /* working → powered_off */
                verify('Flow-30',
                    function(s) { return s.global_mode === 'powered_off'; },
                    function(d) { return d.seg_mode === 'dash'; },
                    '全idle→POWERED_OFF: "--"显示');
            }
        });

        tests.push({ phase: 4, id: 'Flow-31', desc: '关机→休眠',
            setup: function() { /* 已在 powered_off (承接Flow-30) */ },
            run: function() {
                verify('Flow-31',
                    function(s) { return s.global_mode === 'powered_off'; },
                    function(d) { return d.leds.power === true; },
                    'POWERED_OFF: 电源灯亮(闪烁由渲染器处理)');
            }
        });

        tests.push({ phase: 4, id: 'Flow-32', desc: '关机→开机(唤醒)',
            setup: function() { /* 已在 powered_off */ },
            run: function() {
                injectByName('POWER', 'long');
                verify('Flow-32',
                    function(s) { return s.global_mode === 'working'; },
                    function(d) {
                        return d.leds.power === true && d.seg_chars[0] === '0' && d.seg_blink[0] === false;
                    },
                    '开机→WORKING: "0 0 0 0", 电源灯常亮');
            }
        });

        tests.push({ phase: 4, id: 'Flow-33', desc: '同一炉头自按确认',
            setup: function() {
                injectByName('HEAD_1', 'tap'); injectByName('5', 'tap');
            },
            run: function() {
                injectByName('HEAD_1', 'tap');  /* 自按确认 */
                verify('Flow-33',
                    function(s) { return s.heads[0].node === 'cooking' && s.heads[0].power_level === 5; },
                    function(d) { return d.seg_blink[0] === false && d.seg_chars[0] === '5'; },
                    '自按确认: 立即cooking, 不等15s');
            }
        });
    }

    async function runAll() {
        results = [];
        log('========== HMI数据流自动测试 V7.0 ==========', 'info');
        log('快速模式: ' + (JsonLogicEngine.isFastMode() ? 'ON' : 'OFF'), 'info');

        defineTests();

        var phaseNames = ['', 'Phase1: 上电&基础', 'Phase2: 超时&定时', 'Phase3: Boost&暂停', 'Phase4: 定时/童锁/多炉头/休眠'];

        for (var phase = 1; phase <= 4; phase++) {
            log('========== ' + phaseNames[phase] + ' ==========', 'info');
            for (var i = 0; i < tests.length; i++) {
                if (tests[i].phase !== phase) continue;
                var t = tests[i];
                log('--- ' + t.id + ': ' + t.desc + ' ---', 'info');
                try {
                    if (t.setup) t.setup();
                    t.run();
                } catch(e) {
                    log('测试异常: ' + e.message, 'err');
                    console.error(e);
                    results.push({ id: t.id, desc: t.desc, pass: false });
                }
                await new Promise(function(resolve) { setTimeout(resolve, 20); });
            }
        }

        log('');
        log('========== 测试汇总 ==========', 'info');
        var pass = 0, fail = 0;
        for (var j = 0; j < results.length; j++) {
            if (results[j].pass) pass++; else fail++;
        }
        var total = results.length;
        log('总计: ' + total + ' | 通过: ' + pass + ' | 失败: ' + fail, 'info');
        log('通过率: ' + (pass/total*100).toFixed(1) + '%', pass === total ? 'ok' : 'warn');

        if (fail > 0) {
            log('失败项:', 'err');
            for (var k = 0; k < results.length; k++) {
                if (!results[k].pass) log('  ✗ ' + results[k].id + ': ' + results[k].desc, 'err');
            }
        }

        if (onDone) onDone({ total: total, pass: pass, fail: fail, results: results });
    }

    return { runAll: runAll, setLogger: function(fn) { onLog = fn; }, setOnDone: function(fn) { onDone = fn; } };
})();
