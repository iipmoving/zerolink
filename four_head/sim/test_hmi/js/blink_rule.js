/**
 * blink_rule.js — BlinkRule 元素 (V1.1)
 *
 * 声明式闪烁规则：从 head 状态推导 seg_blink，属性从 JSON 读取。
 * 三层架构位置: JSON Binding Layer
 *   Zone Logic 输出 head.node → BlinkRule.shouldBlink() → displayCache.seg_blink
 */

var BlinkRule = (function() {

    function createBlinkRule(props) {
        props = props || {};
        var phaseMs = props.phase_ms || 500;
        var condition = props.condition || 'node == selecting';
        var excludeWhen = props.exclude_when || ['timer_setting', 'boost_active'];
        var pauseOverride = props.pause_override !== undefined ? props.pause_override : false;

        function shouldBlink(head) {
            if (!head) return false;
            if (condition === 'node == selecting') {
                if (head.node !== 'selecting') return false;
            }
            for (var i = 0; i < excludeWhen.length; i++) {
                if (head[excludeWhen[i]]) return false;
            }
            return true;
        }

        function syncToCache(heads, displayCache, globalState) {
            if (globalState && globalState.paused) {
                for (var i = 0; i < heads.length; i++) {
                    displayCache.seg_blink[i] = pauseOverride;
                }
                return;
            }
            for (var i = 0; i < heads.length; i++) {
                displayCache.seg_blink[i] = shouldBlink(heads[i]);
            }
        }

        return { shouldBlink: shouldBlink, syncToCache: syncToCache, phaseMs: phaseMs };
    }

    return { createBlinkRule: createBlinkRule };

})();
