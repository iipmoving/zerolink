/**
 * 配置解析器 - 解析和验证JSON配置
 */
class ConfigParser {
    constructor() {
        // 控件类型映射
        this.CONTROL_TYPE_MAP = {
            'button': '按键',
            '按键': '按键',
            'key': '按键',
            'led': 'LED灯',
            'LED': 'LED灯',
            'LED灯': 'LED灯',
            'segment': '数码管',
            'display': '数码管',
            '数码管': '数码管',
            'seven_segment': '数码管'
        };

        // 标准控件属性
        this.STANDARD_PROPERTIES = {
            '按键': ['type', 'name', 'x', 'y', 'width', 'height', 
                    'bg_color', 'border_color', 'text_color'],
            'LED灯': ['type', 'name', 'x', 'y', 'width', 'height',
                     'bg_color', 'border_color', 'text_color'],
            '数码管': ['type', 'name', 'x', 'y', 'width', 'height',
                      'bg_color', 'border_color', 'text_color']
        };

        // 默认值
        this.DEFAULT_VALUES = {
            width: 40,
            height: 40,
            bg_color: '#2a2a2a',
            border_color: '#555555',
            text_color: '#cccccc'
        };
    }

    /**
     * 解析配置文件
     * @param {string} configJson - JSON字符串或对象
     * @returns {Object} 解析后的配置
     */
    parse(configJson) {
        let config;

        // 尝试解析JSON
        if (typeof configJson === 'string') {
            try {
                config = JSON.parse(configJson);
            } catch (error) {
                throw new Error(`JSON解析失败: ${error.message}`);
            }
        } else {
            config = configJson;
        }

        // 验证配置结构
        this._validateConfig(config);

        // 标准化控件
        if (config.controls) {
            config.controls = config.controls.map(control => 
                this._normalizeControl(control)
            );
        }

        return config;
    }

    /**
     * 从URL加载配置
     * @param {string} url - 配置文件URL
     * @returns {Promise<Object>} 配置对象
     */
    async loadFromUrl(url) {
        const response = await fetch(url);
        if (!response.ok) {
            throw new Error(`加载配置失败: ${response.status}`);
        }
        const configJson = await response.json();
        return this.parse(configJson);
    }

    /**
     * 验证配置结构
     * @private
     */
    _validateConfig(config) {
        if (!config || typeof config !== 'object') {
            throw new Error('配置必须是对象');
        }

        if (!config.controls) {
            console.warn('[ConfigParser] 配置中没有controls字段');
            config.controls = [];
        }

        if (!Array.isArray(config.controls)) {
            throw new Error('controls必须是数组');
        }
    }

    /**
     * 标准化控件配置
     * @private
     */
    _normalizeControl(control) {
        // 标准化类型名称
        const originalType = control.type;
        const normalizedType = this.CONTROL_TYPE_MAP[control.type] || control.type;
        
        // 创建标准化控件
        const normalized = {
            id: control.id || `control_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`,
            type: normalizedType,
            name: control.name || `控件${control.type}`,
            x: control.x || 0,
            y: control.y || 0,
            width: control.width || this.DEFAULT_VALUES.width,
            height: control.height || this.DEFAULT_VALUES.height,
            bg_color: control.bg_color || control.backgroundColor || this.DEFAULT_VALUES.bg_color,
            border_color: control.border_color || control.borderColor || this.DEFAULT_VALUES.border_color,
            text_color: control.text_color || control.textColor || this.DEFAULT_VALUES.text_color
        };

        // 保留其他自定义属性
        Object.keys(control).forEach(key => {
            if (!this.STANDARD_PROPERTIES[normalizedType]?.includes(key)) {
                normalized[key] = control[key];
            }
        });

        // 如果原始类型与标准化类型不同，记录映射
        if (originalType !== normalizedType) {
            normalized.originalType = originalType;
        }

        return normalized;
    }

    /**
     * 验证控件配置
     * @param {Object} control - 控件配置
     * @returns {Object} 验证结果
     */
    validateControl(control) {
        const errors = [];
        const warnings = [];

        // 检查必需字段
        if (!control.type) {
            errors.push('缺少type字段');
        }

        if (!control.name) {
            warnings.push('缺少name字段，将使用默认名称');
        }

        // 检查坐标
        if (typeof control.x !== 'number') {
            errors.push('x必须是数字');
        }

        if (typeof control.y !== 'number') {
            errors.push('y必须是数字');
        }

        // 检查尺寸
        if (typeof control.width !== 'number' || control.width <= 0) {
            warnings.push('width无效，将使用默认值');
        }

        if (typeof control.height !== 'number' || control.height <= 0) {
            warnings.push('height无效，将使用默认值');
        }

        return {
            valid: errors.length === 0,
            errors: errors,
            warnings: warnings
        };
    }

    /**
     * 导出配置
     * @param {Object} config - 配置对象
     * @param {string} format - 导出格式（'standard' | 'simple'）
     * @returns {string} JSON字符串
     */
    export(config, format = 'standard') {
        let exportConfig = JSON.parse(JSON.stringify(config));

        if (format === 'simple') {
            // 简化格式 - 适合AI理解
            exportConfig.controls = exportConfig.controls.map(control => {
                return {
                    类型: control.type,
                    名称: control.name,
                    位置: { x: control.x, y: control.y },
                    尺寸: { 宽: control.width, 高: control.height },
                    颜色: {
                        背景: control.bg_color,
                        边框: control.border_color,
                        文字: control.text_color
                    }
                };
            });
        }

        return JSON.stringify(exportConfig, null, 2);
    }

    /**
     * 获取配置摘要
     * @param {Object} config - 配置对象
     * @returns {Object} 摘要信息
     */
    getSummary(config) {
        const controls = config.controls || [];
        
        const summary = {
            total: controls.length,
            buttons: controls.filter(c => c.type === '按键').length,
            leds: controls.filter(c => c.type === 'LED灯').length,
            segments: controls.filter(c => c.type === '数码管').length,
            hasBackground: !!config.background_image
        };

        return summary;
    }
}

export { ConfigParser };