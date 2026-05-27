/**
 * 配置格式转换器
 * 将AI生成的简化格式转换为系统内部格式
 */

/**
 * 按键名称到KeyCode的映射
 */
const KEY_NAME_MAP = {
    '开始 / 暂停': 13,
    '开始/暂停': 13,
    '电源': 13,
    '加水': 11,
    '加时间': 12,
    'M1': 1,
    'M2': 2,
    'M3': 3,
    'M4': 4,
    'M5': 5,
    'M6': 6,
    'M7': 7,
    'M8': 8,
    'M9': 9,
    'M10': 10
};

/**
 * LED名称到位号的映射
 */
const LED_NAME_MAP = {
    '电源灯': 0,
    '电源/暂停灯': 0,
    '开始 / 暂停灯': 0,
    'M1灯': 1,
    'M1 灯': 1,
    'M2灯': 2,
    'M2 灯': 2,
    'M3灯': 3,
    'M3 灯': 3,
    'M4灯': 4,
    'M5灯': 5,
    'M6灯': 6,
    'M7灯': 7,
    'M8灯': 8,
    'M9灯': 9,
    'M10灯': 10,
    '加水灯': 11,
    '加时间灯': 12,
    'mL指示灯': 13,
    'mL 指示灯': 13,
    '水量灯': 13,
    '定时指示灯': 14,
    '时间灯': 14
};

/**
 * 转换单个控件
 */
function convertControl(simpleControl, index) {
    const typeMap = {
        '按键': 'button',
        'LED灯': 'led',
        '数码管': 'segment'
    };
    
    const type = typeMap[simpleControl.type] || simpleControl.type;
    const id = generateId(type === 'button' ? 'btn' : type === 'led' ? 'led' : 'seg');
    
    // 基础属性
    const control = {
        id: id,
        type: type,
        name: simpleControl.name,
        x: simpleControl.x,
        y: simpleControl.y,
        width: simpleControl.width,
        height: simpleControl.height,
        style: {
            background_color: simpleControl.bg_color || '#e0e0e0',
            border_color: simpleControl.border_color || '#999999',
            text_color: simpleControl.text_color || '#000000'
        },
        config: {}
    };
    
    // 根据类型添加特定配置
    if (type === 'button') {
        // 按键配置
        const keyCode = KEY_NAME_MAP[simpleControl.name] || (index + 1);
        control.style.font_size = simpleControl.font_size || 12;
        control.config = {
            long_press: simpleControl.long_press || false,
            key_code: keyCode,
            trigger_mode: 'press'
        };
    } else if (type === 'led') {
        // LED配置
        const ledBit = LED_NAME_MAP[simpleControl.name] || 0;
        control.style.on_color = simpleControl.on_color || '#00ff00';
        control.style.off_color = simpleControl.bg_color || '#666666';
        control.config = {
            led_bit: ledBit
        };
    } else if (type === 'segment') {
        // 数码管配置
        control.style.font_size = simpleControl.font_size || 32;
        control.config = {
            digits: 4
        };
    }
    
    return control;
}

/**
 * 转换整个配置
 */
function convertConfig(simpleConfig) {
    if (!simpleConfig || !simpleConfig.controls) {
        throw new Error('无效的配置格式');
    }
    
    const convertedControls = simpleConfig.controls.map((ctrl, index) => 
        convertControl(ctrl, index)
    );
    
    // 计算画布大小（根据控件位置自动调整）
    let maxX = 800;
    let maxY = 600;
    
    convertedControls.forEach(ctrl => {
        maxX = Math.max(maxX, ctrl.x + ctrl.width + 50);
        maxY = Math.max(maxY, ctrl.y + ctrl.height + 50);
    });
    
    return {
        version: "1.0",
        panel_name: simpleConfig.panel_name || "导入的面板配置",
        canvas_size: {
            width: maxX,
            height: maxY
        },
        controls: convertedControls,
        background_image: simpleConfig.background_image || null
    };
}

/**
 * 检测配置格式
 */
function detectConfigFormat(config) {
    if (!config || !config.controls || config.controls.length === 0) {
        return 'unknown';
    }
    
    const firstControl = config.controls[0];
    
    // 检查是否是简化格式（中文类型名）
    if (['按键', 'LED灯', '数码管'].includes(firstControl.type)) {
        return 'simple';
    }
    
    // 检查是否是内部格式（英文类型名）
    if (['button', 'led', 'segment'].includes(firstControl.type)) {
        return 'internal';
    }
    
    return 'unknown';
}

/**
 * 智能加载配置（自动检测格式并转换）
 */
async function loadConfigSmart(configId = 'default') {
    try {
        const response = await fetch(`configs/${configId}.json`);
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        const rawConfig = await response.json();
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
        
        // 验证配置
        if (!validateConfig(config)) {
            throw new Error('配置格式无效');
        }
        
        // 更新状态
        AppState.currentConfigId = configId;
        AppState.controls = config.controls || [];
        
        // 调整Canvas大小
        if (config.canvas_size && AppState.canvas) {
            AppState.canvas.width = config.canvas_size.width;
            AppState.canvas.height = config.canvas_size.height;
            log(`Canvas大小调整为: ${config.canvas_size.width}x${config.canvas_size.height}`, 'info');
        }
        
        // 推入历史栈
        pushHistory();
        
        // 渲染
        renderCanvas();
        updateControlList();
        
        log(`已加载配置: ${config.panel_name || configId}`, 'success');
        
        return config;
    } catch (error) {
        log(`加载配置失败: ${error.message}`, 'error');
        console.error('加载配置错误:', error);
        throw error;
    }
}
