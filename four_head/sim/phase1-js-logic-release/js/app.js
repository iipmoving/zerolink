/**
 * 应用入口 - 初始化与事件绑定
 */

/**
 * 应用初始化
 */
function initApp() {
    log('EMC Framework Web版启动...', 'info');
    
    // 初始化Canvas
    if (!initCanvas()) {
        return;
    }
    
    // 加载默认配置
    loadConfig('default').then(() => {
        log('应用初始化完成', 'success');
    }).catch(err => {
        log(`初始化失败: ${err.message}`, 'error');
    });
    
    // 绑定事件
    bindEvents();
    
    // 初始渲染
    renderCanvas();
}

/**
 * 绑定事件
 */
function bindEvents() {
    const canvas = AppState.canvas;
    
    // Canvas鼠标事件
    canvas.addEventListener('mousedown', handleCanvasMouseDown);
    canvas.addEventListener('mousemove', handleCanvasMouseMove);
    canvas.addEventListener('mouseup', handleCanvasMouseUp);
    canvas.addEventListener('dblclick', handleCanvasDoubleClick);
    
    // ✅ Canvas右键事件 - 控件填充/透明切换
    canvas.addEventListener('contextmenu', (e) => {
        e.preventDefault();
        handleCanvasRightClick(e);
    });
    
    // 菜单事件
    bindMenuEvents();
    
    // 添加控件按钮
    document.getElementById('add-control-btn').addEventListener('click', () => {
        const select = document.getElementById('add-control-select');
        const type = select.value;
        if (type) {
            addNewControl(type);
            select.value = '';
        }
    });
    
    // 显示选项
    document.getElementById('show-grid-checkbox').addEventListener('change', (e) => {
        AppState.showGrid = e.target.checked;
        renderCanvas();
    });
    
    document.getElementById('show-guides-checkbox').addEventListener('change', (e) => {
        AppState.showGuides = e.target.checked;
    });
    
    // ✅ 控件填充模式切换 - 总开关（强制所有控件同步）
    const controlFillCheckbox = document.getElementById('control-fill-checkbox');
    if (controlFillCheckbox) {
        controlFillCheckbox.addEventListener('change', (e) => {
            AppState.controlFillMode = e.target.checked;
            
            // ✅ 总开关：强制所有控件同步切换到总开关状态
            const isTransparent = !AppState.controlFillMode;
            AppState.controls.forEach(control => {
                control.transparent = isTransparent;
            });
            
            log(`控件填充模式: ${AppState.controlFillMode ? '开启(填充)' : '关闭(透明)'} - 所有控件已同步`, 'info');
            renderCanvas();
        });
    }
    
    // 对齐工具
    document.querySelectorAll('.align-buttons button').forEach(btn => {
        btn.addEventListener('click', () => {
            const alignType = btn.dataset.align;
            handleAlign(alignType);
        });
    });
    
    // 复制日志按钮
    const copyLogBtn = document.getElementById('copy-log-btn');
    if (copyLogBtn) {
        copyLogBtn.addEventListener('click', () => {
            const logPanel = document.getElementById('log-panel');
            const logText = Array.from(logPanel.querySelectorAll('.log-entry'))
                .map(entry => entry.textContent)
                .join('\n');
            
            navigator.clipboard.writeText(logText).then(() => {
                const originalText = copyLogBtn.textContent;
                copyLogBtn.textContent = '已复制!';
                setTimeout(() => {
                    copyLogBtn.textContent = originalText;
                }, 2000);
            }).catch(err => {
                console.error('复制失败:', err);
                alert('复制失败，请手动选择日志内容');
            });
        });
    }
    
    // 键盘事件
    document.addEventListener('keydown', handleKeyDown);
}

/**
 * Canvas鼠标按下事件
 */
function handleCanvasMouseDown(event) {
    if (AppState.mode === 'edit') {
        handleEditMouseDown(event);
    }
    // ✅ demo模式和hardware_test模式的按键由setupCanvasEvents()统一处理
}

/**
 * Canvas鼠标移动事件
 */
function handleCanvasMouseMove(event) {
    if (AppState.mode === 'edit') {
        handleEditMouseMove(event);
    }
}

/**
 * Canvas鼠标松开事件
 */
function handleCanvasMouseUp(event) {
    if (AppState.mode === 'edit') {
        handleEditMouseUp(event);
    }
    // ✅ demo模式和hardware_test模式的按键由setupCanvasEvents()统一处理
}

/**
 * Canvas双击事件
 */
function handleCanvasDoubleClick(event) {
    if (AppState.mode === 'edit') {
        handleEditDoubleClick(event);
    }
}

/**
 * 绑定菜单事件
 */
function bindMenuEvents() {
    // 文件菜单
    document.querySelector('[data-action="open-config"]').addEventListener('click', handleOpenConfig);
    document.querySelector('[data-action="load-imported"]').addEventListener('click', () => loadConfigSmart('panel_imported'));
    document.querySelector('[data-action="save-config"]').addEventListener('click', () => saveConfig());
    document.querySelector('[data-action="export-config"]').addEventListener('click', () => exportConfig());
    
    // 编辑菜单
    document.querySelector('[data-action="undo"]').addEventListener('click', () => undo());
    document.querySelector('[data-action="redo"]').addEventListener('click', () => redo());
    document.querySelector('[data-action="delete-control"]').addEventListener('click', () => deleteSelectedControl());
    
    // 视图菜单
    document.querySelector('[data-action="toggle-grid"]').addEventListener('click', () => {
        AppState.showGrid = !AppState.showGrid;
        document.getElementById('show-grid-checkbox').checked = AppState.showGrid;
        renderCanvas();
    });
    
    document.querySelector('[data-action="toggle-guides"]').addEventListener('click', () => {
        AppState.showGuides = !AppState.showGuides;
        document.getElementById('show-guides-checkbox').checked = AppState.showGuides;
    });
    
    // ✅ 底图菜单
    document.querySelector('[data-action="load-background"]').addEventListener('click', handleLoadBackground);
    document.querySelector('[data-action="clear-background"]').addEventListener('click', handleClearBackground);
    document.querySelector('[data-action="background-fit"]').addEventListener('click', handleBackgroundFit);
    
    // 演示菜单
    document.querySelector('[data-action="start-demo"]').addEventListener('click', () => setMode('demo'));
    document.querySelector('[data-action="stop-demo"]').addEventListener('click', () => setMode('edit'));
    document.querySelector('[data-action="switch-to-js"]').addEventListener('click', switchToJSLogic);
    document.querySelector('[data-action="switch-to-wasm"]').addEventListener('click', switchToWASMLogic);
    document.querySelector('[data-action="switch-to-json"]').addEventListener('click', switchToJSONLogic);
    document.querySelector('[data-action="hardware-test"]').addEventListener('click', () => setMode('hardware_test'));
    
    // ✅ 配置菜单
    document.querySelector('[data-action="control-mapping"]').addEventListener('click', openControlMapping);
    document.querySelector('[data-action="export-mapping"]').addEventListener('click', exportControlMapping);
    
    // 帮助菜单
    document.querySelector('[data-action="shortcuts"]').addEventListener('click', showShortcuts);
    document.querySelector('[data-action="about"]').addEventListener('click', showAbout);
}

/**
 * 打开配置文件
 */
function handleOpenConfig() {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    
    input.onchange = (e) => {
        const file = e.target.files[0];
        if (file) {
            loadConfigFromFile(file);
        }
    };
    
    input.click();
}

/**
 * 加载参考图（已废弃，使用handleLoadBackground代替）
 */
function handleLoadReference() {
    handleLoadBackground();
}

/**
 * ✅ 加载底图
 */
function handleLoadBackground() {
    log('开始加载底图...', 'info');
    
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = 'image/*';
    
    input.onchange = (e) => {
        log('文件选择完成', 'info');
        const file = e.target.files[0];
        if (file) {
            log(`选中文件: ${file.name}, 大小: ${(file.size / 1024).toFixed(2)} KB`, 'info');
            
            const reader = new FileReader();
            
            reader.onloadstart = () => {
                log('开始读取文件...', 'info');
            };
            
            reader.onprogress = (event) => {
                if (event.lengthComputable) {
                    const percent = (event.loaded / event.total * 100).toFixed(0);
                    log(`读取进度: ${percent}%`, 'info');
                }
            };
            
            reader.onload = (event) => {
                log('文件读取完成，创建Image对象...', 'info');
                
                AppState.backgroundImage = new Image();
                AppState.backgroundImage.src = event.target.result;
                AppState.backgroundImagePath = file.name; // 保存文件名
                
                AppState.backgroundImage.onload = () => {
                    log(`图片加载成功: ${AppState.backgroundImage.width}x${AppState.backgroundImage.height}`, 'success');
                    
                    // 自动适配窗口
                    fitBackgroundToWindow();
                    renderCanvas();
                    log(`底图已加载: ${file.name}`, 'success');
                };
                
                AppState.backgroundImage.onerror = () => {
                    log('图片加载失败', 'error');
                };
            };
            
            reader.onerror = () => {
                log('文件读取失败', 'error');
            };
            
            reader.readAsDataURL(file);
        } else {
            log('未选择文件', 'warning');
        }
    };
    
    input.click();
}

/**
 * ✅ 清除底图
 */
function handleClearBackground() {
    AppState.backgroundImage = null;
    AppState.backgroundImagePath = null;
    AppState.backgroundScale = 1.0;
    renderCanvas();
    log('底图已清除', 'info');
}

/**
 * ✅ 底图适配窗口
 */
function handleBackgroundFit() {
    if (!AppState.backgroundImage) {
        log('请先加载底图', 'warning');
        return;
    }
    
    fitBackgroundToWindow();
    renderCanvas();
    log('底图已适配窗口', 'success');
}

/**
 * ✅ 自动适配底图到容器可视区域（不改变Canvas尺寸）
 */
function fitBackgroundToWindow() {
    if (!AppState.backgroundImage || !AppState.canvas) {
        log('无法适配：底图或Canvas不存在', 'error');
        return;
    }
    
    const container = AppState.canvas.parentElement;
    if (!container) {
        log('无法获取容器', 'error');
        return;
    }
    
    // ✅ 关键修复：Canvas尺寸不变，只缩放底图来适配容器可视区域
    const canvasWidth = AppState.canvas.width;
    const canvasHeight = AppState.canvas.height;
    
    // 获取图片的实际尺寸和比例
    const imgWidth = AppState.backgroundImage.width;
    const imgHeight = AppState.backgroundImage.height;
    
    log(`Canvas尺寸: ${canvasWidth}x${canvasHeight} (保持不变)`, 'info');
    log(`图片原始尺寸: ${imgWidth}x${imgHeight}`, 'info');
    
    // ✅ 计算缩放比例：确保图片完整显示在Canvas内
    const scaleX = canvasWidth / imgWidth;
    const scaleY = canvasHeight / imgHeight;
    const scale = Math.min(scaleX, scaleY);
    
    AppState.backgroundScale = scale;
    
    // 计算居中偏移量
    const scaledImgWidth = imgWidth * scale;
    const scaledImgHeight = imgHeight * scale;
    
    AppState.backgroundOffsetX = (canvasWidth - scaledImgWidth) / 2;
    AppState.backgroundOffsetY = (canvasHeight - scaledImgHeight) / 2;
    
    log(`缩放比例: ${(scale * 100).toFixed(2)}%`, 'info');
    log(`图片缩放后: ${scaledImgWidth.toFixed(0)}x${scaledImgHeight.toFixed(0)}`, 'info');
    log(`居中偏移: (${AppState.backgroundOffsetX.toFixed(0)}, ${AppState.backgroundOffsetY.toFixed(0)})`, 'info');
    log('✓ 底图已适配，控件位置不变', 'success');
}

/**
 * 处理对齐操作
 */
function handleAlign(alignType) {
    if (!AppState.selectedControlId) {
        log('请先选中控件', 'info');
        return;
    }
    
    const selectedControl = AppState.controls.find(c => c.id === AppState.selectedControlId);
    if (!selectedControl) return;
    
    switch (alignType) {
        case 'left':
            selectedControl.x = 0;
            break;
        case 'right':
            selectedControl.x = AppState.canvas.width - selectedControl.width;
            break;
        case 'top':
            selectedControl.y = 0;
            break;
        case 'bottom':
            selectedControl.y = AppState.canvas.height - selectedControl.height;
            break;
        case 'center':
            selectedControl.x = (AppState.canvas.width - selectedControl.width) / 2;
            selectedControl.y = (AppState.canvas.height - selectedControl.height) / 2;
            break;
    }
    
    pushHistory();
    renderCanvas();
    log(`已${alignType === 'left' ? '左' : alignType === 'right' ? '右' : alignType === 'top' ? '上' : alignType === 'bottom' ? '下' : '居中'}对齐`, 'success');
}

/**
 * 键盘事件处理
 */
function handleKeyDown(event) {
    // Ctrl+Z 撤销
    if (event.ctrlKey && event.key === 'z') {
        event.preventDefault();
        undo();
    }
    
    // Ctrl+Y 重做
    if (event.ctrlKey && event.key === 'y') {
        event.preventDefault();
        redo();
    }
    
    // Delete 删除
    if (event.key === 'Delete' && AppState.mode === 'edit') {
        deleteSelectedControl();
    }
    
    // ESC 退出演示模式
    if (event.key === 'Escape' && AppState.mode !== 'edit') {
        setMode('edit');
    }
    
    // Ctrl+S 保存
    if (event.ctrlKey && event.key === 's') {
        event.preventDefault();
        saveConfig();
    }
}

/**
 * 显示快捷键说明
 */
function showShortcuts() {
    alert(`快捷键说明：

编辑模式：
- Ctrl+Z: 撤销
- Ctrl+Y: 重做
- Delete: 删除选中控件
- Ctrl+S: 保存配置
- 双击控件: 编辑属性

演示模式：
- ESC: 退出演示模式`);
}

/**
 * 显示关于信息
 */
function showAbout() {
    alert(`EMC Framework Web版 v1.0

嵌入式家电控制面板仿真平台

技术栈：
- HTML5 Canvas
- Vanilla JavaScript
- 零依赖

第一阶段：JS逻辑层
第二阶段：C DLL集成`);
}

/**
 * ✅ 打开控件映射配置页面
 */
function openControlMapping() {
    window.open('control_mapping_editor.html', '_blank');
    log('已打开控件映射配置页面', 'info');
}

/**
 * ✅ 导出控件映射配置
 */
function exportControlMapping() {
    // 生成JavaScript格式的映射配置
    let jsCode = '/**\n';
    jsCode += ' * 控件映射配置 - 自动生成\n';
    jsCode += ` * 生成时间: ${new Date().toLocaleString('zh-CN')}\n`; 
    jsCode += ' * \n';
    jsCode += ' * 此文件定义了控件名称与数组索引的映射关系\n';
    jsCode += ' * - KEY_NAME_MAP: 按键名称 -> KeyCode\n';
    jsCode += ' * - LED_NAME_MAP: LED名称 -> dllState.ledBits索引\n';
    jsCode += ' */\n\n';
    
    // 从当前配置生成
    if (AppState.controlBinder) {
        jsCode += '// 按键映射\n';
        jsCode += 'const KEY_NAME_MAP = {\n';
        AppState.controlBinder.keyMap.forEach((info, name) => {
            jsCode += `    '${name}': ${info.keyCode},\n`;
        });
        jsCode += '};\n\n';
        
        jsCode += '// LED映射\n';
        jsCode += 'const LED_NAME_MAP = {\n';
        AppState.controlBinder.ledMap.forEach((info, name) => {
            jsCode += `    '${name}': '${info.ledKey}',\n`;
        });
        jsCode += '};\n';
    } else {
        jsCode += '// 注意：需要先启动演示模式才能导出当前配置\n';
        jsCode += '// 请使用 "配置" -> "控件映射配置" 菜单进行编辑\n';
    }
    
    // 下载文件
    const blob = new Blob([jsCode], {type: 'text/javascript'});
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.download = `control_mapping_${Date.now()}.js`;
    link.click();
    URL.revokeObjectURL(url);
    
    log('✅ 控件映射配置已导出', 'success');
}

/**
 * ✅ Canvas右键点击事件 - 控件填充/透明切换
 */
function handleCanvasRightClick(e) {
    // 只在编辑模式下生效
    if (AppState.mode !== 'edit') {
        return;
    }
    
    const rect = AppState.canvas.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;
    
    // 查找被点击的控件
    const control = hitTest(x, y);
    
    if (!control) {
        // 没有点击到控件，不处理
        return;
    }
    
    // 创建右键菜单
    const menu = document.createElement('div');
    menu.style.position = 'fixed';
    menu.style.left = e.clientX + 'px';
    menu.style.top = e.clientY + 'px';
    menu.style.backgroundColor = '#ffffff';
    menu.style.border = '1px solid #cccccc';
    menu.style.boxShadow = '0 2px 8px rgba(0,0,0,0.2)';
    menu.style.padding = '5px 0';
    menu.style.zIndex = '9999';
    menu.style.minWidth = '150px';
    menu.style.borderRadius = '4px';
    
    // 菜单项：切换填充/透明
    const menuItem = document.createElement('div');
    menuItem.textContent = control.transparent ? '切换到填充模式' : '切换到透明模式';
    menuItem.style.padding = '8px 15px';
    menuItem.style.cursor = 'pointer';
    menuItem.style.fontSize = '14px';
    menuItem.style.color = '#333333';
    
    menuItem.addEventListener('mouseenter', () => {
        menuItem.style.backgroundColor = '#f0f0f0';
    });
    
    menuItem.addEventListener('mouseleave', () => {
        menuItem.style.backgroundColor = 'transparent';
    });
    
    menuItem.addEventListener('click', () => {
        // 切换控件的透明状态
        control.transparent = !control.transparent;
        log(`控件 "${control.name}" 已${control.transparent ? '透明' : '填充'}`, 'info');
        
        // 移除菜单
        document.body.removeChild(menu);
        
        // 重绘Canvas
        renderCanvas();
    });
    
    menu.appendChild(menuItem);
    document.body.appendChild(menu);
    
    // 点击其他地方关闭菜单
    const closeMenu = (event) => {
        if (!menu.contains(event.target)) {
            document.body.removeChild(menu);
            document.removeEventListener('click', closeMenu);
        }
    };
    
    // 延迟添加事件，避免立即触发
    setTimeout(() => {
        document.addEventListener('click', closeMenu);
    }, 100);
}

/**
 * ✅ 切换到JS逻辑层
 */
function switchToJSLogic() {
    console.log('[DEBUG] switchToJSLogic called');
    console.log('[DEBUG] AppState.mode:', AppState.mode);
    
    if (AppState.mode !== 'demo') {
        log('⚠️ 请先启动演示模式', 'warning');
        return;
    }
    
    log('🔄 正在切换到JS逻辑层...', 'info');
    
    // 停止当前演示
    if (typeof stopDemo === 'function') {
        stopDemo();
        console.log('[DEBUG] stopDemo called');
    } else {
        console.error('[ERROR] stopDemo is not defined');
    }
    
    // 设置使用JS逻辑层
    AppState.useWasm = false;
    console.log('[DEBUG] AppState.useWasm set to false');
    
    // 重新启动演示
    setTimeout(() => {
        if (typeof startDemo === 'function') {
            startDemo('js');
            console.log('[DEBUG] startDemo("js") called');
            log('✅ 已切换到JS逻辑层', 'success');
        } else {
            console.error('[ERROR] startDemo is not defined');
        }
    }, 500);
}

/**
 * ✅ 切换到WASM逻辑层
 */
function switchToWASMLogic() {
    console.log('[DEBUG] switchToWASMLogic called');
    console.log('[DEBUG] AppState.mode:', AppState.mode);
    
    if (AppState.mode !== 'demo') {
        log('⚠️ 请先启动演示模式', 'warning');
        return;
    }
    
    log('🔄 正在切换到WASM逻辑层...', 'info');
    
    // 停止当前演示
    if (typeof stopDemo === 'function') {
        stopDemo();
        console.log('[DEBUG] stopDemo called');
    } else {
        console.error('[ERROR] stopDemo is not defined');
    }
    
    // 设置使用WASM逻辑层
    AppState.useWasm = true;
    console.log('[DEBUG] AppState.useWasm set to true');
    
    // 重新启动演示
    setTimeout(() => {
        if (typeof startDemo === 'function') {
            startDemo('wasm');
            console.log('[DEBUG] startDemo("wasm") called');
            log('✅ 已切换到WASM逻辑层', 'success');
        } else {
            console.error('[ERROR] startDemo is not defined');
        }
    }, 500);
}

/**
 * ✅ 切换到JSON逻辑层
 */
function switchToJSONLogic() {
    console.log('[DEBUG] switchToJSONLogic called');
    console.log('[DEBUG] AppState.mode:', AppState.mode);
    
    if (AppState.mode !== 'demo') {
        log('⚠️ 请先启动演示模式', 'warning');
        return;
    }
    
    log('🔄 正在切换到JSON逻辑层...', 'info');
    
    // 停止当前演示
    if (typeof stopDemo === 'function') {
        stopDemo();
        console.log('[DEBUG] stopDemo called');
    } else {
        console.error('[ERROR] stopDemo is not defined');
    }
    
    // 设置使用JSON逻辑层
    AppState.useWasm = false;
    AppState.useJson = true;
    console.log('[DEBUG] AppState.useJson set to true');
    
    // 重新启动演示
    setTimeout(() => {
        if (typeof startDemo === 'function') {
            startDemo('json');
            console.log('[DEBUG] startDemo("json") called');
            log('✅ 已切换到JSON逻辑层', 'success');
        } else {
            console.error('[ERROR] startDemo is not defined');
        }
    }, 500);
}

// DOM加载完成后初始化
document.addEventListener('DOMContentLoaded', initApp);
