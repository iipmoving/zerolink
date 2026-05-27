/**
 * led_element.js — LED 控件元素 (V1.0)
 *
 * 每个LED组一个实例，属性从JSON读取（elements.led），方法供引擎调用。
 *
 * 三层架构位置: Presentation Controls 层
 *   Zone Logic → 输出语义值(power_level, boost_active, global_mode)
 *   LEDElement.syncToCache(leds) → 按属性(single/gradient)写displayCache.leds
 */

var LEDElement = (function() {

    /**
     * createLevelLED — 档位灯组(gradient LED bar)
     * @param {Object} props — { count, mode, boost_override }
     *   count: LED数量 (默认10)
     *   mode: "single" 单点亮 / "gradient" 从0到当前档位全亮
     *   boost_override: Boost时点亮哪个灯 (默认9)
     */
    function createLevelLED(props) {
        props = props || {};

        var count = props.count || 10;
        var mode = props.mode || 'single';
        var boostOverride = (props.boost_override !== undefined)
            ? parseInt(props.boost_override) : (count - 1);

        var currentLevel = 0;
        var isBoost = false;

        function setLevel(n) {
            n = Math.max(0, Math.min(n, count - 1));
            currentLevel = n;
        }

        function setBoost(b) {
            isBoost = !!b;
        }

        function syncToCache(leds) {
            /* 清空 */
            clear(leds);

            var targetLevel = isBoost ? boostOverride : currentLevel;

            if (mode === 'gradient') {
                /* 梯度: 0→targetLevel 全部亮 */
                for (var j = 0; j <= targetLevel; j++) {
                    if (j < count) leds.power_level[j] = true;
                }
            } else {
                /* 单点: 只亮targetLevel */
                if (targetLevel < count) leds.power_level[targetLevel] = true;
            }
        }

        function clear(leds) {
            for (var k = 0; k < count; k++) {
                leds.power_level[k] = false;
            }
        }

        return { setLevel: setLevel, setBoost: setBoost,
                 syncToCache: syncToCache, clear: clear };
    }

    /**
     * createStatusLED — 状态灯
     * @param {Object} props
     *   states:   { powered_off: "on"/"off"/"blink", ... } — global_mode→behavior
     *   flag:     "paused" — 直接读取 globalState[flag]
     *   condition: "any_timer_active" / "head_selecting" — 引擎侧求值
     *   ledField: cache字段名 (默认 "power")
     *   array:    true — 数组型LED (head_select[4])
     */
    function createStatusLED(props) {
        props = props || {};
        var states = props.states || null;
        var flag = props.flag || null;
        var condition = props.condition || null;
        var ledField = props.ledField || 'power';
        var isArray = props.array || false;

        function syncToCache(leds, value, index) {
            if (isArray) {
                if (index !== undefined && leds[ledField]) {
                    leds[ledField][index] = !!value;
                }
            } else {
                if (leds.hasOwnProperty(ledField)) {
                    leds[ledField] = !!value;
                }
            }
        }

        return {
            states: states, flag: flag, condition: condition,
            ledField: ledField, isArray: isArray,
            syncToCache: syncToCache
        };
    }

    return { createLevelLED: createLevelLED, createStatusLED: createStatusLED };

})();
