/**
 * run_tests_node.js — Node.js HMI自动化测试
 *
 * 加载浏览器JS模块到Node环境, 运行全部30条数据流测试。
 * 用法: node run_tests_node.js
 */
'use strict';

const fs = require('fs');
const path = require('path');
const vm = require('vm');

/* ========== 创建共享沙箱 ========== */
const sandbox = {
    console: console,
    setTimeout: setTimeout,
    clearTimeout: clearTimeout,
    setInterval: setInterval,
    clearInterval: clearInterval,
    Date: Date,
    Math: Math,
    JSON: JSON,
    Array: Array,
    Object: Object,
    String: String,
    Number: Number,
    Boolean: Boolean,
    parseInt: parseInt,
    Promise: Promise
};
vm.createContext(sandbox);

function loadScript(filePath) {
    /* 将 const 替换为 var, 使其在 VM 上下文中作为 sandbox 属性可访问 */
    let code = fs.readFileSync(filePath, 'utf-8');
    code = code.replace(/\bconst\s+(MsgId|KeyEvent|Msg\b|MessageBus|KeyHandler|Renderer|JsonLogicEngine|KEY_NAME|BlinkRule|ModeRule|TestRunner)\s*=/g,
        'var $1 =');
    const script = new vm.Script(code, { filename: path.basename(filePath) });
    script.runInContext(sandbox);
    console.log('[load] ' + path.basename(filePath));
}

/* ========== 加载模块 ========== */
const JS_DIR = __dirname + '/js/';
loadScript(JS_DIR + 'message_bus.js');
loadScript(JS_DIR + 'key_handler.js');
loadScript(JS_DIR + 'slot_element.js');
loadScript(JS_DIR + 'led_element.js');
loadScript(JS_DIR + 'blink_rule.js');
loadScript(JS_DIR + 'mode_rule.js');
loadScript(JS_DIR + 'json_logic_engine.js');
loadScript(JS_DIR + 'test_runner.js');

/* ========== 引用 ========== */
const MsgId = sandbox.MsgId;
const MessageBus = sandbox.MessageBus;
const JsonLogicEngine = sandbox.JsonLogicEngine;
const TestRunner = sandbox.TestRunner;

/* ========== 初始化 & 运行 ========== */
let totalPass = 0, totalFail = 0;

function log(msg, cls) {
    const prefix = cls === 'ok' ? '  [PASS] ' :
                   cls === 'err' ? '  [FAIL] ' :
                   cls === 'warn' ? '  [WARN] ' :
                   cls === 'info' ? '  [INFO] ' : '    ';
    console.log(prefix + msg);
}

async function run() {
    console.log('============================================');
    console.log('  四头电磁炉 HMI 单向数据流 自动测试');
    console.log('  基于 hmi-data-flow-table.md (30条流向)');
    console.log('============================================\n');

    /* 加载逻辑JSON */
    var logicRules;
    var jsonPath = path.join(__dirname, 'logic', 'four_head_v4.json');
    try {
        logicRules = JSON.parse(fs.readFileSync(jsonPath, 'utf-8'));
        console.log('[OK] JSON规则加载: ' + logicRules.meta.name + ' v' + logicRules.meta.version);
    } catch(e) {
        console.log('[WARN] JSON规则加载失败: ' + e.message + ' → 使用硬编码回退');
        logicRules = { meta: { name: 'fallback', version: '0', head_count: 4 } };
    }

    /* 初始化引擎 (fastMode必须在init之前设置) */
    MessageBus.init();
    JsonLogicEngine.setFastMode(true);
    JsonLogicEngine.init(logicRules);

    /* 启动100ms/1s节拍 */
    let tick100 = 0;
    setInterval(() => {
        MessageBus.post(MsgId.MSG_TIMER_100MS, 0, null);
        tick100++;
        if (tick100 % 10 === 0) {
            MessageBus.post(MsgId.MSG_TIMER_1S, 0, null);
        }
    }, 100);

    /* 消息消费 */
    setInterval(() => { MessageBus.run1ms(); }, 50);

    /* 等待上电序列完成(fastMode下瞬时完成) */
    await sleep(200);

    /* 注册日志 */
    TestRunner.setLogger(log);
    TestRunner.setOnDone((report) => {
        totalPass = report.pass;
        totalFail = report.fail;
    });

    /* 执行全部测试 */
    await TestRunner.runAll();

    /* 最终输出 */
    console.log('\n============================================');
    console.log('  总结果: ' + totalPass + '/' + (totalPass + totalFail) + ' 通过' +
                '  (' + (totalPass/(totalPass+totalFail)*100).toFixed(1) + '%)');
    console.log('============================================');

    /* 退出(给定时器一点时间) */
    setTimeout(() => process.exit(totalFail > 0 ? 1 : 0), 500);
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

run().catch(e => {
    console.error('测试异常:', e);
    process.exit(1);
});
