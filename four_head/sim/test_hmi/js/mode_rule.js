/**
 * mode_rule.js — ModeRule 元素 (V1.0)
 *
 * 声明式显示模式规则：从 globalState + heads 派生 seg_mode。
 * 三层架构位置: JSON Binding Layer
 *   globalState.mode + heads.* → ModeRule.evaluate() → displayCache.seg_mode
 */

var ModeRule = (function() {

    var DEFAULT_RULES = [
        { when: 'powered_off',       value: 'dash' },
        { when: 'deep_sleep',        value: 'off' },
        { when: 'paused',            value: 'ascii' },
        { when: 'any_timer_setting', value: 'timer_setting' }
    ];
    var DEFAULT_FALLBACK = 'power';

    function createModeRule(props) {
        props = props || {};
        var rules = props.rules || DEFAULT_RULES;
        var fallback = props['default'] || DEFAULT_FALLBACK;

        function evaluate(globalState, heads) {
            for (var i = 0; i < rules.length; i++) {
                var r = rules[i];
                if (r.when === 'powered_off' && globalState.mode === 'powered_off')
                    return r.value;
                if (r.when === 'deep_sleep' && globalState.mode === 'deep_sleep')
                    return r.value;
                if (r.when === 'paused' && globalState.paused)
                    return r.value;
                if (r.when === 'any_timer_setting') {
                    for (var j = 0; j < heads.length; j++) {
                        if (heads[j].timer_setting) return r.value;
                    }
                }
            }
            return fallback;
        }

        function syncToCache(globalState, heads, displayCache) {
            displayCache.seg_mode = evaluate(globalState, heads);
        }

        return { evaluate: evaluate, syncToCache: syncToCache };
    }

    return { createModeRule: createModeRule };

})();
