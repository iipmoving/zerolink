/**
 * slot_element.js — SlotElement 控件 (V1.0)
 *
 * 每个 Zone 一个实例，封装2位数码管slot的渲染逻辑。
 * 属性从JSON读取（elements.segment_slot），方法供引擎调用。
 *
 * 三层架构位置: Presentation Controls 层
 *   Zone Logic → 输出语义值
 *   SlotElement.render(head) → 查 value_map → 返回 ["1","0"]
 *   引擎 → seg_chars[base] = chars[0], seg_chars[base+1] = chars[1]
 */

var SlotElement = (function() {

    var DEFAULT_VALUE_MAP = {
        '0': '00', '1': '1 ', '2': '2 ', '3': '3 ', '4': '4 ',
        '5': '5 ', '6': '6 ', '7': '7 ', '8': '8 ', '9': '9 '
    };

    var DEFAULT_BOOST_CHAR = 'P ';
    var DEFAULT_ZERO_CHAR = '00';
    var DEFAULT_DIGIT_COUNT = 2;

    /**
     * 从JSON或默认值创建SlotElement
     * @param {Object} props — { digit_count, value_map, boost_char, zero_char }
     */
    function createSlotElement(props) {
        props = props || {};

        var digitCount = props.digit_count || DEFAULT_DIGIT_COUNT;
        var valueMap = props.value_map || DEFAULT_VALUE_MAP;
        var boostChar = props.boost_char || DEFAULT_BOOST_CHAR;
        var zeroChar = props.zero_char || DEFAULT_ZERO_CHAR;

        /* 确保 boost_char / zero_char 补齐到 digit_count 宽度 */
        boostChar = padToWidth(boostChar, digitCount);
        zeroChar = padToWidth(zeroChar, digitCount);

        /* 确保 value_map 中每个条目补齐宽度 */
        for (var k in DEFAULT_VALUE_MAP) {
            if (valueMap[k]) {
                valueMap[k] = padToWidth(valueMap[k], digitCount);
            }
        }

        /* ========== 渲染方法 ========== */

        function renderPowerLevel(level) {
            var key = String(level);
            if (valueMap[key]) {
                return splitToArray(valueMap[key], digitCount);
            }
            /* 回退: 数字转字符串 */
            var fallback = String(level);
            while (fallback.length < digitCount) fallback = ' ' + fallback;
            return splitToArray(fallback, digitCount);
        }

        function renderTimerValue(minutes) {
            /* 2位: >=10 → 两位, <10 → 一位+空格 */
            if (digitCount === 2) {
                if (minutes >= 10) {
                    return [String(Math.floor(minutes / 10)), String(minutes % 10)];
                } else {
                    return [String(minutes), ' '];
                }
            }
            /* 通用: 左补空格 */
            var s = String(minutes);
            while (s.length < digitCount) s = ' ' + s;
            return splitToArray(s, digitCount);
        }

        function renderBoost() {
            return splitToArray(boostChar, digitCount);
        }

        function renderZero() {
            return splitToArray(zeroChar, digitCount);
        }

        /**
         * 主渲染入口: 根据head状态决定显示内容
         * @param {Object} h — head 对象 { node, power_level, boost_active, timer_setting, timer_value, ... }
         * @returns {Array} — [char1, char2, ...] 长度为 digit_count
         */
        function render(h) {
            if (!h) return splitToArray(zeroChar, digitCount);

            /* 优先级: timer_setting > boost_active > power_level */
            if (h.timer_setting) {
                return renderTimerValue(h.timer_value || 0);
            }
            if (h.boost_active) {
                return renderBoost();
            }
            if (h.power_level === 0) {
                return renderZero();
            }
            return renderPowerLevel(h.power_level);
        }

        return {
            render: render,
            renderPowerLevel: renderPowerLevel,
            renderTimerValue: renderTimerValue,
            renderBoost: renderBoost,
            renderZero: renderZero
        };
    }

    /* 补齐到指定位数(左补空格) */
    function padToWidth(s, width) {
        while (s.length < width) s = ' ' + s;
        if (s.length > width) s = s.substring(0, width);
        return s;
    }

    /* 字符串按字符拆分为数组 */
    function splitToArray(s, width) {
        var arr = [];
        for (var i = 0; i < width; i++) {
            arr.push(s.charAt(i) || ' ');
        }
        return arr;
    }

    return { createSlotElement: createSlotElement };

})();
