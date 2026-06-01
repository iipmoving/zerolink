/**
 * 演示模式逻辑
 */

/**
 * ✅ 更新调试面板显示
 */
function updateDebugPanel() {
    // 更新状态
    const stateEl = document.getElementById('debug-state');
    if (stateEl && AppState.dllState && AppState.dllState.state !== null) {
        const stateNames = {
            0: 'S_POWER_ON', 1: 'S_VERSION', 2: 'S_SHUTDOWN',
            3: 'S_DEMO', 4: 'S_STANDBY', 5: 'S_FUNC_SELECT',
            6: 'S_COOKING', 7: 'S_PAUSE', 8: 'S_FAULT'
        };
        const stateCode = AppState.dllState.state;
        stateEl.textContent = `${stateNames[stateCode] || `UNKNOWN(${stateCode})`} (${stateCode})`;
    }
    
    // 更新数码管显示
    const displayEl = document.getElementById('debug-display');
    if (displayEl && AppState.dllState && AppState.dllState.seg) {
        const seg = AppState.dllState.seg;
        let displayText = '';
        for (let i = 0; i < 4; i++) {
            // ✅ 使用segCodeToChar将段码转换为字符
            if (typeof segCodeToChar === 'function') {
                displayText += segCodeToChar(seg[i]);
            } else {
                displayText += '?';
            }
        }
        displayEl.textContent = displayText || '----';
    }
}

/**
 * ✅ 更新按键信息显示
 * @param {string} keyName - 按键名称
 * @param {number} keyCode - 键码
 * @param {number|null} event - 事件类型（2=SHORT, 3=LONG）
 * @param {number|null} duration - 按住时长（ms）
 * @param {boolean|null} isLongPressKey - 是否为长按功能按键
 */
function updateKeyInfo(keyName, keyCode, event, duration, isLongPressKey) {
    const nameEl = document.getElementById('debug-key-name');
    const codeEl = document.getElementById('debug-key-code');
    const eventEl = document.getElementById('debug-key-event');
    const durationEl = document.getElementById('debug-key-duration');
    const longPressEl = document.getElementById('debug-key-longpress');
    
    if (nameEl) nameEl.textContent = keyName || '-';
    if (codeEl) codeEl.textContent = keyCode !== null ? keyCode : '-';
    if (eventEl) {
        const eventNames = { 2: 'SHORT', 3: 'LONG', 1: 'PRESS', 4: 'REPEAT', 5: 'RELEASE' };
        eventEl.textContent = event !== null ? `${eventNames[event] || 'UNKNOWN'} (${event})` : '-';
    }
    if (durationEl) durationEl.textContent = duration !== null ? `${duration} ms` : '-';
    if (longPressEl) longPressEl.textContent = isLongPressKey !== null ? (isLongPressKey ? 'true' : 'false') : '-';
}

// ✅ 暴露全局函数供其他模块调用
window.updateDebugPanel = updateDebugPanel;
window.updateKeyInfo = updateKeyInfo;

/**
 * 启动演示模式
 * @param {string} logicType - 'js' 或 'wasm'
 */
async function startDemo(logicType = 'js') {
    try {
        if (logicType === 'wasm') {
            await startDemoWithWasm();
        } else {
            await startDemoWithJS();  // ✅ 添加await
        }
    } catch (error) {
        log(`启动演示模式失败: ${error.message}`, 'error');
        console.error('startDemo错误详情:', error);
        throw error;
    }
}

/**
 * 使用JS逻辑层启动演示
 */
async function startDemoWithJS() {
    try {
        log('正在初始化JS逻辑层适配器...', 'info');
        
        // ✅ 创建JS逻辑层适配器（统一接口）
        AppState.logicLayer = new JSEmcLogicAdapter();
        
        // 注册回调函数
        AppState.logicLayer.registerCallbacks(
            (seg, led) => {
                // 显示输出回调
                AppState.dllState.seg = seg;
                if (led && led.func_leds !== undefined) {
                    AppState.dllState.ledBits[0] = led.power ? 1 : 0;
                    for (let i = 0; i < 10; i++) {
                        AppState.dllState.ledBits[i + 1] = (led.func_leds >> i) & 1;
                    }
                    AppState.dllState.ledBits[11] = led.add_water ? 1 : 0;
                    AppState.dllState.ledBits[12] = led.add_time ? 1 : 0;
                    AppState.dllState.ledBits[13] = led.water_disp ? 1 : 0;
                    AppState.dllState.ledBits[14] = led.time_disp ? 1 : 0;
                }
                renderCanvas();
                updateDebugPanel();
            },
            (cmd) => {
                // 蜂鸣器回调
                console.log(`[JSEmcLogicAdapter] 🔊 BUZZER: cmd=${cmd}`);
            },
            (type, valPtr) => {
                // 状态变化回调
                if (type === 1 && valPtr) {
                    const newState = typeof valPtr === 'number' ? valPtr : valPtr;
                    AppState.dllState.state = newState;
                    console.log(`[JSEmcLogicAdapter] 🔄 状态变化: ${newState}`);
                    updateDebugPanel();
                }
            },
            null // 硬件输入回调（demo模式不使用）
        );
        
        // 初始化适配器（包含EmcLogic.init()启动上电流程）
        await AppState.logicLayer.init();
        log('✅ JS逻辑层适配器初始化成功', 'success');
        
        // ✅ 绑定Canvas事件（统一接口，适配JS和WASM）
        setupCanvasEvents();
        
        // 启动周期运行
        startPeriodicRun();
        
        log('演示模式已启动（完整状态机）', 'success');
        AppState.mode = 'demo';
        
        // ✅ 显示调试面板
        const debugPanel = document.getElementById('debug-panel');
        if (debugPanel) {
            debugPanel.style.display = 'block';
            log('✅ 调试面板已显示', 'info');
        }
        
    } catch (error) {
        log(`启动演示模式失败: ${error.message}`, 'error');
        console.error('startDemoWithJS错误详情:', error);
        throw error;
    }
}

/**
 * 停止演示模式
 */
function stopDemo() {
    try {
        // ✅ 移除所有Canvas事件监听器（统一接口）
        const canvas = AppState.canvas;
        if (canvas) {
            // 移除统一的事件监听器
            canvas.removeEventListener('mousedown', AppState.canvasMouseDownHandler);
            canvas.removeEventListener('mouseup', AppState.canvasMouseUpHandler);
            
            // 移除旧的WASM事件监听器（兼容旧代码）
            canvas.removeEventListener('mousedown', AppState.wasmMouseDownHandler);
            canvas.removeEventListener('mouseup', AppState.wasmMouseUpHandler);
            canvas.removeEventListener('click', AppState.wasmClickHandler);
            console.log('[stopDemo] 已移除所有Canvas事件监听器');
        }
        
        // 停止WASM演示
        if (AppState.wasmAdapter) {
            console.log('[stopDemo] 检测到WASM适配器，调用stopWasmDemo...');
            stopWasmDemo();
            console.log('[stopDemo] stopWasmDemo完成');
            // ✅ 不要return，继续执行后续清理
        }
        
        // 停止JS演示
        if (AppState.periodicTimer) {
            clearInterval(AppState.periodicTimer);
            AppState.periodicTimer = null;
        }
        
        // ✅ 停止硬件调试模式
        if (AppState.hardwareTestTimer) {
            clearInterval(AppState.hardwareTestTimer);
            AppState.hardwareTestTimer = null;
        }
        
        // ✅ 统一使用适配器的cleanup接口
        if (AppState.logicLayer && typeof AppState.logicLayer.cleanup === 'function') {
            console.log('[stopDemo] 调用logicLayer.cleanup()...');
            AppState.logicLayer.cleanup();
            console.log('[stopDemo] logicLayer.cleanup()完成');
        }
        
        AppState.logicLayer = null;
        AppState.keyHandler = null;
        AppState.pressedKeys = {};
        
        // 重置显示
        AppState.dllState.seg = [0x7F, 0x7F, 0x7F, 0x7F];
        AppState.dllState.ledBits.fill(0);
        
        renderCanvas();
        log('演示模式已停止', 'info');
    } catch (error) {
        console.error('[stopDemo] 错误:', error);
        log(`停止演示失败: ${error.message}`, 'error');
        throw error;  // 重新抛出，让上层知道失败了
    }
}

/**
 * 启动周期运行（每10ms一次，匹配C版本）
 */
function startPeriodicRun() {
    if (AppState.periodicTimer) return;
    
    AppState.periodicTimer = setInterval(() => {
        if (AppState.logicLayer) {
            AppState.logicLayer.runCycle();
        }
    }, 10);  // C版本要求10ms周期
    
    log('周期运行已启动（10ms/周期）', 'info');
}

/**
 * 配置按键映射
 */
function setupKeyConfigs() {
    const configs = [];
    
    // ✅ 从配置读取long_press属性，不再硬编码
    AppState.controls.forEach(control => {
        if (control.type === 'button' && control.config?.key_code) {
            const triggerMode = control.config.long_press ? 'release' : 'press';
            configs.push(new KeyConfig(
                control.config.key_code,
                control.name,
                triggerMode,
                1500  // 长按阈值1.5秒
            ));
        }
    });
    
    AppState.keyHandler.setConfigs(configs);
    log(`已配置${configs.length}个按键`, 'info');
}

/**
 * 设置Canvas点击事件（统一接口，适配JS和WASM）
 */
function setupCanvasEvents() {
    const canvas = AppState.canvas;
    if (!canvas) return;
    
    // ✅ 移除所有旧的事件监听器
    canvas.removeEventListener('mousedown', AppState.canvasMouseDownHandler);
    canvas.removeEventListener('mouseup', AppState.canvasMouseUpHandler);
    
    // ✅ 统一的按键按下处理（适配JS和WASM）
    AppState.canvasMouseDownHandler = (event) => {
        const rect = canvas.getBoundingClientRect();
        const x = event.clientX - rect.left;
        const y = event.clientY - rect.top;
        
        // 查找被点击的控件
        const control = findControlByPosition(x, y);
        if (!control || control.type !== 'button') return;
        
        // ✅ 获取keyCode：优先使用config.key_code，否则从名称映射
        let keyCode = control.config?.key_code;
        if (keyCode === undefined || keyCode === null) {
            // 从控件名称映射keyCode（兼容旧配置）
            const KEY_NAME_MAP = {
                '开始 / 暂停': 13,
                '加水': 11,
                '加时间': 12,
                'M1': 1, 'M2': 2, 'M3': 3, 'M4': 4, 'M5': 5,
                'M6': 6, 'M7': 7, 'M8': 8, 'M9': 9, 'M10': 10,
                '电源': 0
            };
            keyCode = KEY_NAME_MAP[control.name];
            if (keyCode === undefined) {
                console.warn(`未知按键: ${control.name}`);
                return;
            }
        }
        
        const isLongPressKey = control.config?.long_press === true;
        
        // ✅ 阻止事件传播
        event.stopPropagation();
        event.preventDefault();
        
        // 记录按键按下时间
        const pressTime = Date.now();
        AppState.pressedKeys[control.name] = pressTime;
        
        // ✅ 更新标题栏按键信息显示
        updateTitleKeyInfo(control.name, 'PRESS');
        
        // ✅ 通过统一的logicLayer接口调用（不区分JS/WASM）
        if (AppState.logicLayer) {
            if (isLongPressKey) {
                // 长按功能按键：按下时不发送事件，等待松开时判断
                log(`🔘 [长按模式] 按键按下: ${control.name} (KeyCode=${keyCode})`, 'info');
                
                // ✅ 更新调试面板按键信息
                if (typeof updateKeyInfo === 'function') {
                    updateKeyInfo(control.name, keyCode, KeyEvent.KEY_EVENT_PRESS, null, isLongPressKey);
                }
            } else {
                // 普通按键：只记录按下，不发送PRESS（避免与mouseup的SHORT重复触发）
                log(`🔘 按键按下: ${control.name} (KeyCode=${keyCode})`, 'info');

                // ✅ 更新调试面板按键信息
                if (typeof updateKeyInfo === 'function') {
                    updateKeyInfo(control.name, keyCode, KeyEvent.KEY_EVENT_PRESS, null, isLongPressKey);
                }

                // ✅ 启动长按重复定时器（1.5s后开始，每500ms一次REPEAT）
                if (!AppState.longPressTimers) {
                    AppState.longPressTimers = {};
                }

                if (AppState.longPressTimers[keyCode]) {
                    clearTimeout(AppState.longPressTimers[keyCode]);
                    clearInterval(AppState.longPressTimers[keyCode]);
                }

                AppState.longPressTimers[keyCode] = setTimeout(() => {
                    if (AppState.logicLayer && AppState.pressedKeys[control.name]) {
                        AppState.logicLayer.pressKey(keyCode, KeyEvent.KEY_EVENT_REPEAT);
                        log(`按键长按重复: ${control.name} (KeyCode=${keyCode})`, 'info');

                        AppState.longPressTimers[keyCode] = setInterval(() => {
                            if (AppState.logicLayer && AppState.pressedKeys[control.name]) {
                                AppState.logicLayer.pressKey(keyCode, KeyEvent.KEY_EVENT_REPEAT);
                            } else {
                                clearInterval(AppState.longPressTimers[keyCode]);
                                delete AppState.longPressTimers[keyCode];
                            }
                        }, 500);
                    }
                }, 1500);
            }
        }
        
        // 重绘（显示按下效果）
        renderCanvas();
    };
    
    // ✅ 统一的按键松开处理（适配JS和WASM）
    AppState.canvasMouseUpHandler = (event) => {
        const rect = canvas.getBoundingClientRect();
        const x = event.clientX - rect.left;
        const y = event.clientY - rect.top;
        
        // 查找被点击的控件
        const control = findControlByPosition(x, y);
        if (!control || control.type !== 'button') return;
        
        // ✅ 获取keyCode：优先使用config.key_code，否则从名称映射
        let keyCode = control.config?.key_code;
        if (keyCode === undefined || keyCode === null) {
            const KEY_NAME_MAP = {
                '开始 / 暂停': 13,
                '加水': 11,
                '加时间': 12,
                'M1': 1, 'M2': 2, 'M3': 3, 'M4': 4, 'M5': 5,
                'M6': 6, 'M7': 7, 'M8': 8, 'M9': 9, 'M10': 10,
                '电源': 0
            };
            keyCode = KEY_NAME_MAP[control.name];
            if (keyCode === undefined) {
                return;
            }
        }
        
        const isLongPressKey = control.config?.long_press === true;
        
        // ✅ 阻止事件传播
        event.stopPropagation();
        event.preventDefault();
        
        // 计算按住时长
        const pressTime = AppState.pressedKeys[control.name];
        if (!pressTime) return;
        
        const elapsed = Date.now() - pressTime;
        delete AppState.pressedKeys[control.name];
        
        // ✅ 更新标题栏按键信息显示
        updateTitleKeyInfo(control.name, 'RELEASE');
        
        // ✅ 通过统一的logicLayer接口调用（不区分JS/WASM）
        if (AppState.logicLayer) {
            if (isLongPressKey) {
                // 长按功能按键：根据时长判断SHORT或LONG
                const eventType = elapsed >= 1500 ? KeyEvent.KEY_EVENT_LONG : KeyEvent.KEY_EVENT_SHORT;
                const eventTypeName = eventType === KeyEvent.KEY_EVENT_LONG ? 'LONG' : 'SHORT';
                
                log(`🔑 [长按模式] 按键松开: ${control.name} (${eventTypeName}, ${elapsed}ms)`, 'info');
                AppState.logicLayer.pressKey(keyCode, eventType);
                
                // ✅ 更新调试面板按键信息
                if (typeof updateKeyInfo === 'function') {
                    updateKeyInfo(control.name, keyCode, eventType, elapsed, isLongPressKey);
                }
            } else {
                // 普通按键：清除长按重复定时器
                if (AppState.longPressTimers && AppState.longPressTimers[keyCode]) {
                    clearTimeout(AppState.longPressTimers[keyCode]);
                    clearInterval(AppState.longPressTimers[keyCode]);
                    delete AppState.longPressTimers[keyCode];
                }
                
                // 根据时长发送SHORT或LONG事件
                if (elapsed < 1500) {
                    AppState.logicLayer.pressKey(keyCode, KeyEvent.KEY_EVENT_SHORT);
                    log(`按键松开: ${control.name} (短按, ${elapsed}ms)`, 'info');
                    
                    // ✅ 更新调试面板按键信息
                    if (typeof updateKeyInfo === 'function') {
                        updateKeyInfo(control.name, keyCode, KeyEvent.KEY_EVENT_SHORT, elapsed, isLongPressKey);
                    }
                } else {
                    AppState.logicLayer.pressKey(keyCode, KeyEvent.KEY_EVENT_LONG);
                    log(`按键松开: ${control.name} (长按结束, ${elapsed}ms)`, 'info');
                    
                    // ✅ 更新调试面板按键信息
                    if (typeof updateKeyInfo === 'function') {
                        updateKeyInfo(control.name, keyCode, KeyEvent.KEY_EVENT_LONG, elapsed, isLongPressKey);
                    }
                }
            }
        }
        
        // 重绘（取消按下效果）
        renderCanvas();
    };
    
    // ✅ 绑定统一的事件处理器
    canvas.addEventListener('mousedown', AppState.canvasMouseDownHandler);
    canvas.addEventListener('mouseup', AppState.canvasMouseUpHandler);
    
    log('✅ Canvas事件已绑定（统一接口，适配JS/WASM）', 'info');
}



/**
 * 显示输出回调（匹配C版本签名）
 * @param {Object} seg - 段码对象 {seg: [4], dp_mask, colon_mask}
 * @param {Array} led - LED数组 [16]
 */
function onDisplayOutput(seg, led) {
    // 更新数码管状态
    if (seg && seg.seg) {
        const oldSeg = AppState.dllState.seg ? [...AppState.dllState.seg] : null;
        AppState.dllState.seg = Array.from(seg.seg);
        AppState.dllState.dp_mask = seg.dp_mask || 0;
        AppState.dllState.colon_mask = seg.colon_mask || 0;
        
        // 调试：如果段码变化，输出日志
        if (oldSeg && !oldSeg.every((v, i) => v === AppState.dllState.seg[i])) {
            console.log(`[Demo] 数码管更新: [${AppState.dllState.seg.map(v => '0x'+v.toString(16).toUpperCase()).join(', ')}]`);
        }
    }
    
    // 更新LED状态
    if (led) {
        AppState.dllState.ledBits = Array.from(led);
    }
    
    scheduleRender();
}

/**
 * 蜂鸣器回调（匹配C版本签名）
 * @param {number} cmd - 蜂鸣器命令 (BUZZ_CLICK=1, BUZZ_DOUBLE=2, etc.)
 */
function onBuzzerOutput(cmd) {
    // ✅ 根据C代码枚举：BUZZ_CLICK=1(有效), 其他为无效
    // 有效：4KHz 0.4S
    // 无效：1KHz 0.4S
    log(`🔊 BUZZER回调: cmd=${cmd}`, 'info');
    playBuzzerSound(cmd);
}

/**
 * 状态变化回调
 */
function onStateChange(state, userData) {
    log(`状态变化: ${STATE_NAMES[state] || state}`, 'success');
    
    // ✅ 更新AppState.dllState.state
    if (AppState.dllState) {
        AppState.dllState.state = state;
    }
    
    // ✅ 更新调试面板
    if (typeof updateDebugPanel === 'function') {
        updateDebugPanel();
    }
}

/**
 * 播放蜂鸣器音效
 * @param {number} cmd - 蜂鸣器命令
 *   BUZZ_CLICK=1: 有效 - 4KHz 0.4S
 *   其他: 无效 - 1KHz 0.4S
 */
function playBuzzerSound(cmd) {
    log(`🔊 开始播放蜂鸣器: cmd=${cmd}`, 'info');
    
    // 初始化音频上下文
    if (!AppState.audioContext) {
        log('创建音频上下文', 'info');
        AppState.audioContext = new (window.AudioContext || window.webkitAudioContext)();
    }
    
    // 恢复音频上下文（浏览器策略要求）
    if (AppState.audioContext.state === 'suspended') {
        log('恢复音频上下文...', 'info');
        AppState.audioContext.resume().then(() => {
            log('✅ 音频上下文已恢复', 'success');
        }).catch(err => {
            log(`❌ 音频上下文恢复失败: ${err.message}`, 'error');
        });
    }
    
    log(`音频上下文状态: ${AppState.audioContext.state}`, 'info');
    
    // ✅ 根据C代码枚举设置频率和时长
    let frequency;
    const duration = 400;  // 统一0.4秒
    
    switch (cmd) {
        case 1:  // BUZZ_CLICK - 有效
            frequency = 4000;  // 4KHz
            break;
        default:  // 其他命令 - 无效
            frequency = 1000;  // 1KHz
            break;
    }
    
    log(`🔊 频率: ${frequency}Hz, 时长: ${duration}ms`, 'info');
    
    // 创建振荡器
    const oscillator = AppState.audioContext.createOscillator();
    const gainNode = AppState.audioContext.createGain();
    
    oscillator.frequency.value = frequency;
    oscillator.type = 'square';  // 方波，更像蜂鸣器声音
    oscillator.connect(gainNode);
    gainNode.connect(AppState.audioContext.destination);
    
    // 设置音量
    gainNode.gain.setValueAtTime(0.3, AppState.audioContext.currentTime);
    gainNode.gain.exponentialRampToValueAtTime(0.01, AppState.audioContext.currentTime + duration / 1000);
    
    oscillator.start();
    oscillator.stop(AppState.audioContext.currentTime + duration / 1000);
    
    log('✅ 蜂鸣器音效已播放', 'success');
}

/**
 * 使用WASM逻辑层启动演示
 */
async function startDemoWithWasm() {
    try {
        log('正在初始化WASM...', 'info');
        
        // 创建WASM适配器
        AppState.wasmAdapter = new EmsWasmAdapter();
        await AppState.wasmAdapter.init();
        
        log('✅ WASM初始化成功', 'success');
        log('⚠️ 注意：上电后会自动进入自检流程（约3秒）', 'info');

        /* 关键: 将 WASM 适配器挂到统一 logicLayer 接口, Canvas 事件通过它发按键 */
        AppState.logicLayer = AppState.wasmAdapter;
        
        // ✅ 单向数据流：注册显示更新回调
        AppState.wasmAdapter.setDisplayUpdateCallback((seg, led) => {
            // WASM主动推送显示数据，UI层被动接收
            AppState.dllState.seg = seg;
            
            // 更新LED状态
            if (led && led.func_leds !== undefined) {
                AppState.dllState.ledBits[0] = led.power ? 1 : 0;  // 电源灯
                for (let i = 0; i < 10; i++) {
                    AppState.dllState.ledBits[i + 1] = (led.func_leds >> i) & 1;
                }
                AppState.dllState.ledBits[11] = led.add_water ? 1 : 0;
                AppState.dllState.ledBits[12] = led.add_time ? 1 : 0;
                AppState.dllState.ledBits[13] = led.water_disp ? 1 : 0;
                AppState.dllState.ledBits[14] = led.time_disp ? 1 : 0;
            }
            
            // 重新渲染Canvas
            renderCanvas();
            
            // ✅ 更新调试面板
            if (typeof updateDebugPanel === 'function') {
                updateDebugPanel();
            }
        });
        log('✅ 显示更新回调已注册（单向数据流）', 'success');
        
        // ✅ 关键：自动启动周期运行（避免之前踩过的坑）
        log('🔄 自动启动周期运行...', 'info');
        AppState.cycleCount = 0;
        
        // ✅ 单向数据流：WASM内部通过回调主动推送显示数据
        // 不需要JS层主动查询，只需启动周期即可
        AppState.wasmAdapter.startCycle(() => {
            AppState.cycleCount++;
            // 不再需要手动调用updateDisplayFromWasm()
            // 显示更新由WASM内部的disp_output_callback自动触发
        });
        log('✅ 周期运行已启动，系统将自动执行上电自检', 'success');
        
        // ✅ 关键：加载配置并绑定控件（避免bindElements未调用的问题）
        log('正在加载配置并绑定控件...', 'info');
        AppState.controlBinder = new ControlBinder(AppState.wasmAdapter, 'configs/default.json');
        const bindSuccess = await AppState.controlBinder.loadAndBind();
        
        if (bindSuccess) {
            log('✅ 控件绑定成功', 'success');
            log(`   按键: ${AppState.controlBinder.getKeys().join(', ')}`, 'info');
            log(`   LED: ${AppState.controlBinder.getLeds().join(', ')}`, 'info');
            log(`   数码管: ${AppState.controlBinder.getSegments().join(', ')}`, 'info');
        } else {
            log('⚠️ 控件绑定失败，但WASM仍可工作', 'warning');
        }
        
        // 设置Canvas点击事件（统一接口）
        setupCanvasEvents();
        
        // 清空按键状态
        AppState.pressedKeys = {};
        
        log('演示模式已启动（WASM逻辑层）', 'success');
        
        // ✅ 显示调试面板
        const debugPanel = document.getElementById('debug-panel');
        if (debugPanel) {
            debugPanel.style.display = 'block';
            log('✅ 调试面板已显示', 'info');
        }
    } catch (error) {
        log(`WASM演示启动失败: ${error.message}`, 'error');
        console.error('startDemoWithWasm错误详情:', error);
        throw error;
    }
}

/**
 * 根据位置查找控件
 */
function findControlByPosition(x, y) {
    if (!AppState.controls) return null;
    return AppState.controls.find(c => 
        x >= c.x && x <= c.x + c.width &&
        y >= c.y && y <= c.y + c.height
    );
}

/**
 * 停止WASM演示模式
 */
function stopWasmDemo() {
    try {
        if (AppState.wasmAdapter) {
            console.log('[stopWasmDemo] 停止WASM周期...');
            AppState.wasmAdapter.stopCycle();
            console.log('[stopWasmDemo] 清理WASM适配器...');
            AppState.wasmAdapter.cleanup();
            AppState.wasmAdapter = null;
            console.log('[stopWasmDemo] WASM适配器已清理');
        }
        
        if (AppState.controlBinder) {
            AppState.controlBinder = null;
        }
        
        // 移除Canvas点击事件（统一使用canvasMouseDownHandler和canvasMouseUpHandler）
        if (AppState.canvas) {
            AppState.canvas.removeEventListener('mousedown', AppState.canvasMouseDownHandler);
            AppState.canvas.removeEventListener('mouseup', AppState.canvasMouseUpHandler);
            AppState.canvasMouseDownHandler = null;
            AppState.canvasMouseUpHandler = null;
        }
        
        AppState.cycleCount = 0;
        AppState.pressedKeys = {};
        
        // 重置显示
        AppState.dllState.seg = [0x7F, 0x7F, 0x7F, 0x7F];
        AppState.dllState.ledBits.fill(0);
        
        renderCanvas();
        log('WASM演示模式已停止', 'info');
    } catch (error) {
        console.error('[stopWasmDemo] 错误:', error);
        log(`WASM停止失败: ${error.message}`, 'error');
    }
}

/**
 * ✅ 启动硬件调试模式
 */
function startHardwareTest() {
    log('正在启动硬件调试模式...', 'info');
    
    // 获取所有LED、数码管、按键控件
    const leds = AppState.controls.filter(c => c.type === 'led' || c.type === 'LED灯');
    const segments = AppState.controls.filter(c => c.type === 'segment' || c.type === '数码管');
    const buttons = AppState.controls.filter(c => c.type === 'button' || c.type === '按键');
    
    log(`硬件调试模式: LED=${leds.length}, 数码管=${segments.length}, 按键=${buttons.length}`, 'info');
    
    // 初始化状态
    AppState.hardwareTestPhase = 'led';  // led -> smg
    AppState.hardwareTestLedIdx = 0;
    AppState.hardwareTestSmgIdx = 0;
    AppState.hardwareTestSmgChars = '0123456789ABCDEF';
    AppState.hardwareTestRunning = true;
    
    // 清空显示
    AppState.dllState.ledBits.fill(0);
    AppState.dllState.seg = [0x7F, 0x7F, 0x7F, 0x7F];
    renderCanvas();
    
    // 启动LED顺序点亮定时器（300ms）
    AppState.hardwareTestTimer = setInterval(() => {
        hardwareTestTick();
    }, 300);
    
    log('✅ 硬件调试模式已启动，按ESC退出', 'success');
}

/**
 * ✅ 硬件调试模式定时器回调
 */
function hardwareTestTick() {
    if (!AppState.hardwareTestRunning) return;
    
    const leds = AppState.controls.filter(c => c.type === 'led' || c.type === 'LED灯');
    const segments = AppState.controls.filter(c => c.type === 'segment' || c.type === '数码管');
    
    if (AppState.hardwareTestPhase === 'led') {
        // LED顺序点亮测试
        if (leds.length === 0) {
            // 没有LED，直接进入数码管测试
            hardwareTestLedOff();
            return;
        }
        
        // 熄灭所有LED
        AppState.dllState.ledBits.fill(0);
        
        // 点亮当前LED
        const idx = AppState.hardwareTestLedIdx % leds.length;
        // 找到对应的LED索引并点亮
        const ledControl = leds[idx];
        const ledIndex = AppState.controls.indexOf(ledControl);
        if (ledIndex >= 0 && ledIndex < 16) {
            AppState.dllState.ledBits[ledIndex] = 1;
        }
        
        log(`💡 LED[${idx}] '${ledControl.name}' 点亮`, 'info');
        
        AppState.hardwareTestLedIdx++;
        
        // 每轮循环完后稍作停顿
        if (idx === leds.length - 1) {
            // 一轮结束，暂停后进入下一轮
            clearInterval(AppState.hardwareTestTimer);
            AppState.hardwareTestTimer = setTimeout(() => {
                AppState.hardwareTestTimer = setInterval(() => {
                    hardwareTestTick();
                }, 300);
            }, 800);
        }
        
        // 2轮循环后自动进入数码管测试
        if (AppState.hardwareTestLedIdx >= leds.length * 2) {
            clearInterval(AppState.hardwareTestTimer);
            AppState.hardwareTestTimer = setTimeout(() => {
                hardwareTestLedOff();
            }, 500);
        }
        
        renderCanvas();
        
    } else if (AppState.hardwareTestPhase === 'smg') {
        // 数码管滚动显示0-F
        const chars = AppState.hardwareTestSmgChars;
        const idx = AppState.hardwareTestSmgIdx % chars.length;
        const ch = chars[idx];
        
        // 将字符转换为段码（简化版，只显示字符本身）
        // 这里使用简单的ASCII码作为段码值，实际应该查表
        const segCode = ch.charCodeAt(0) & 0x7F;  // 确保最高位为0（共阳极）
        AppState.dllState.seg = [segCode, segCode, segCode, segCode];
        
        log(`🔢 数码管显示: ${ch}`, 'info');
        
        AppState.hardwareTestSmgIdx++;
        
        renderCanvas();
    }
}

/**
 * ✅ LED全灭，切换到数码管测试
 */
function hardwareTestLedOff() {
    AppState.dllState.ledBits.fill(0);
    renderCanvas();
    
    log(`✅ LED测试完成`, 'info');
    
    AppState.hardwareTestPhase = 'smg';
    AppState.hardwareTestSmgIdx = 0;
    
    // 继续运行定时器（如果还在运行）
    if (!AppState.hardwareTestTimer) {
        AppState.hardwareTestTimer = setInterval(() => {
            hardwareTestTick();
        }, 400);
    }
}

/**
 * ✅ 停止硬件调试模式
 */
function stopHardwareTest() {
    AppState.hardwareTestRunning = false;
    
    if (AppState.hardwareTestTimer) {
        clearInterval(AppState.hardwareTestTimer);
        clearTimeout(AppState.hardwareTestTimer);
        AppState.hardwareTestTimer = null;
    }
    
    // 重置显示
    AppState.dllState.ledBits.fill(0);
    AppState.dllState.seg = [0x7F, 0x7F, 0x7F, 0x7F];
    AppState.pressedKeys = {};
    
    renderCanvas();
    log('硬件调试模式已停止', 'info');
}

/**
 * ✅ 更新标题栏按键信息显示
 */
function updateTitleKeyInfo(keyName, eventType) {
    const keyInfoEl = document.getElementById('key-info');
    if (!keyInfoEl) return;
    
    const eventText = eventType === 'PRESS' ? '按下' : '松开';
    keyInfoEl.textContent = `按键: ${keyName} | 状态: ${eventText}`;
    
    // 2秒后恢复默认显示
    setTimeout(() => {
        if (keyInfoEl.textContent.includes(keyName)) {
            keyInfoEl.textContent = '按键: -- | 状态: --';
        }
    }, 2000);
}
