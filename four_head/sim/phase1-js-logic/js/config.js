/**
 * 配置管理模块
 */

/**
 * 将底图转换为Base64（用于保存到JSON）
 */
function exportBackgroundImage() {
    return new Promise((resolve) => {
        if (!AppState.backgroundImage) {
            resolve(null);
            return;
        }
        
        try {
            // 创建Canvas来转换图片为Base64
            const canvas = document.createElement('canvas');
            const ctx = canvas.getContext('2d');
            
            canvas.width = AppState.backgroundImage.width;
            canvas.height = AppState.backgroundImage.height;
            
            // 绘制图片
            ctx.drawImage(AppState.backgroundImage, 0, 0);
            
            // 转换为Base64
            const base64Data = canvas.toDataURL('image/png');
            
            resolve({
                data: base64Data,
                scale: AppState.backgroundScale || 1.0,
                offsetX: AppState.backgroundOffsetX || 0,
                offsetY: AppState.backgroundOffsetY || 0
            });
        } catch (error) {
            console.error('底图导出失败:', error);
            resolve(null);
        }
    });
}

/**
 * 从 Base64加载底图（用于从 JSON恢复）
 */
function importBackgroundImage(backgroundConfig) {
    return new Promise((resolve) => {
        if (!backgroundConfig || !backgroundConfig.data) {
            resolve(false);
            return;
        }
        
        try {
            const img = new Image();
            img.onload = () => {
                AppState.backgroundImage = img;
                AppState.backgroundScale = backgroundConfig.scale || 1.0;
                AppState.backgroundOffsetX = backgroundConfig.offsetX || 0;
                AppState.backgroundOffsetY = backgroundConfig.offsetY || 0;
                AppState.backgroundImagePath = 'embedded'; // 标记为嵌入的图片
                
                log('底图已从配置中恢复', 'success');
                resolve(true);
            };
            
            img.onerror = () => {
                log('底图加载失败', 'error');
                resolve(false);
            };
            
            img.src = backgroundConfig.data;
        } catch (error) {
            console.error('底图导入失败:', error);
            resolve(false);
        }
    });
}

/**
 * 加载配置文件（已废弃，使用loadConfigSmart代替）
 */
async function loadConfig(configId = 'default') {
    return await loadConfigSmart(configId);
}

/**
 * 保存配置文件
 */
async function saveConfig(configId = null) {
    const id = configId || AppState.currentConfigId;
    
    const config = {
        version: "1.0",
        panel_name: `${id}面板`,
        canvas_size: {
            width: AppState.canvas ? AppState.canvas.width : 800,
            height: AppState.canvas ? AppState.canvas.height : 600
        },
        controls: deepClone(AppState.controls),
        // ✅ 保存底图信息（包含Base64数据）
        background: await exportBackgroundImage()
    };
    
    // 创建下载链接
    const dataStr = JSON.stringify(config, null, 2);
    const dataBlob = new Blob([dataStr], { type: 'application/json' });
    const url = URL.createObjectURL(dataBlob);
    
    const link = document.createElement('a');
    link.href = url;
    link.download = `${id}.json`;
    link.click();
    
    URL.revokeObjectURL(url);
    
    log(`配置已导出: ${id}.json`, 'success');
}

/**
 * 导出配置（简化格式，用于AI生成）
 */
function exportConfig() {
    const simpleConfig = {
        controls: AppState.controls.map(control => ({
            type: control.type === 'button' ? '按键' : 
                  control.type === 'led' ? 'LED灯' : '数码管',
            x: control.x,
            y: control.y,
            name: control.name,
            width: control.width,
            height: control.height,
            bg_color: control.style.background_color || '#e0e0e0',
            border_color: control.style.border_color || '#999999',
            text_color: control.style.text_color || '#000000',
            long_press: control.config?.long_press || false
        }))
    };
    
    const dataStr = JSON.stringify(simpleConfig, null, 2);
    const dataBlob = new Blob([dataStr], { type: 'application/json' });
    const url = URL.createObjectURL(dataBlob);
    
    const link = document.createElement('a');
    link.href = url;
    link.download = `export_${AppState.currentConfigId}.json`;
    link.click();
    
    URL.revokeObjectURL(url);
    
    log('配置已导出（简化格式）', 'success');
}

/**
 * 验证配置格式
 */
function validateConfig(config) {
    if (!config || typeof config !== 'object') {
        return false;
    }
    
    // 至少要有controls数组
    if (!Array.isArray(config.controls)) {
        return false;
    }
    
    // 验证每个控件
    for (const control of config.controls) {
        if (!control.type || !control.x === undefined || !control.y === undefined) {
            return false;
        }
        
        // 验证类型
        if (!['button', 'led', 'segment'].includes(control.type)) {
            return false;
        }
    }
    
    return true;
}

/**
 * 从文件加载配置
 */
function loadConfigFromFile(file) {
    const reader = new FileReader();
    
    reader.onload = (e) => {
        try {
            const rawConfig = JSON.parse(e.target.result);
            const format = detectConfigFormat(rawConfig);
            
            let config;
            if (format === 'simple') {
                log('检测到简化格式，正在转换...', 'info');
                config = convertConfig(rawConfig);
                log(`配置转换完成：${config.controls.length}个控件`, 'success');
            } else if (format === 'internal') {
                config = rawConfig;
                log('检测到内部格式，直接加载', 'success');
            } else {
                throw new Error('未知的配置格式');
            }
            
            if (!validateConfig(config)) {
                throw new Error('配置格式无效');
            }
            
            AppState.controls = config.controls || [];
            
            // 调整Canvas大小
            if (config.canvas_size && AppState.canvas) {
                AppState.canvas.width = config.canvas_size.width;
                AppState.canvas.height = config.canvas_size.height;
            }
            
            // ✅ 加载底图配置（支持Base64嵌入）
            if (config.background) {
                if (config.background.data) {
                    // Base64嵌入的底图
                    log('检测到嵌入的底图，正在加载...', 'info');
                    importBackgroundImage(config.background).then(success => {
                        if (success) {
                            pushHistory();
                            renderCanvas();
                            updateControlList();
                            log(`已从文件加载: ${file.name}`, 'success');
                        }
                    });
                    return; // 等待异步加载完成
                } else if (config.background.path) {
                    // 旧版本的路径引用
                    AppState.backgroundImagePath = config.background.path;
                    AppState.backgroundScale = config.background.scale || 1.0;
                    AppState.backgroundOffsetX = config.background.offsetX || 0;
                    AppState.backgroundOffsetY = config.background.offsetY || 0;
                    log(`底图配置已加载: ${config.background.path}`, 'info');
                    log('注意：底图文件需要手动重新加载', 'warning');
                }
            }
            
            pushHistory();
            renderCanvas();
            updateControlList();
            
            log(`已从文件加载: ${file.name}`, 'success');
        } catch (error) {
            log(`文件加载失败: ${error.message}`, 'error');
            console.error('文件加载错误:', error);
        }
    };
    
    reader.onerror = () => {
        log('文件读取失败', 'error');
    };
    
    reader.readAsText(file);
}
