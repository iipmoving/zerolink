/**
 * message_bus.js — JS版消息调度器
 *
 * 1:1 对应嵌入式 Claude/core/msg_def.h + msg_scheduler.c
 * 环形队列深度8 + 回调注册表上限16 + 消息结构 ≤ 16字节等效
 */

/* ========== 消息ID (与 msg_def.h 对齐) ========== */
const MsgId = {
    MSG_KEY_EVENT:        0,   /* 按键事件      param=(key_state<<8)|key_code  */
    MSG_COOKING_CTRL:     1,   /* 烹饪控制命令   param:高8=炉头号,低8=命令      */
    MSG_POWER_CTRL:       2,   /* 功率下发       data_ptr→PowerCtrl            */
    MSG_FAN_CTRL:         3,   /* 风机控制       param=档位0-3                  */
    MSG_DISPLAY_REFRESH:  4,   /* 显示刷新       data_ptr→DisplayCmd            */
    MSG_TIMER_100MS:      5,   /* 100ms节拍      param=无                      */
    MSG_TIMER_1S:         6,   /* 1秒节拍        param=无                      */
    MSG_COMM_TX_DONE:     7,   /* MODBUS收发完成  param=炉头号                  */
    MSG_COMM_DATA_UPDATE: 8,   /* 通讯数据更新    data_ptr→ModbusData            */
    MSG_SYSTEM_ERROR:     9,   /* 系统错误        data_ptr→FaultInfo             */
    MSG_TEST_CHAT_A:      10,  /* 测试对话A       param=轮次                     */
    MSG_TEST_CHAT_B:      11,  /* 测试对话B       param=轮次                     */
    MSG_COMM_POLL_TICK:   12,  /* 通信轮询节拍    param=炉头索引0-3              */
    MSG_REG_DATA_READY:   13,  /* 寄存器数据就绪  data_ptr→RegData              */
    /* HMI测试专用扩展 */
    MSG_BUZZER_CTRL:      14,  /* 蜂鸣器控制      param=cmd                     */
    MSG_HMI_STATE_CHANGE: 15,  /* HMI状态变化     data_ptr→{head,from,to}       */
    MSG_COUNT:            16
};

/* ========== 按键事件类型(与参考程序一致) ========== */
const KeyEvent = {
    NONE:    0,
    PRESS:   1,   /* 按下(消抖通过) */
    LONG:    2,   /* 长按(≥1s)     */
    REPEAT:  3,   /* 连发(长按后每300ms) */
    RELEASE: 4,   /* 释放          */
    TAP:     5    /* 轻触(<1s释放,从未LONG) */
};

/* ========== 消息结构(JS等效) ========== */
class Msg {
    constructor(id, param, data_ptr) {
        this.id = id;
        this.param = param || 0;
        this.data_ptr = data_ptr || null;
    }
}

/* ========== 消息调度器 ========== */
const MessageBus = (function() {
    const QUEUE_DEPTH = 8;
    const MAX_HANDLERS = 16;

    let queue = [];
    let handlers = [];   /* { id, callback } */

    function init() {
        queue = [];
        handlers = [];
        console.log('[MessageBus] 初始化完成 (队列深度=%d, 最大回调=%d)', QUEUE_DEPTH, MAX_HANDLERS);
    }

    /**
     * 注册消息处理器
     * @param {number} msgId - 消息ID
     * @param {Function} callback - 回调(msgId, param, data_ptr)
     */
    function register(msgId, callback) {
        if (handlers.length >= MAX_HANDLERS) {
            console.error('[MessageBus] 回调注册表已满(%d)', MAX_HANDLERS);
            return;
        }
        handlers.push({ id: msgId, cb: callback });
        console.log('[MessageBus] 注册回调: msgId=%d (%s)', msgId, getMsgName(msgId));
    }

    /**
     * 投递消息到环形队列
     * @param {number} msgId
     * @param {number} param
     * @param {*} data_ptr
     * @returns {boolean} false=队列满,消息丢弃
     */
    function post(msgId, param, data_ptr) {
        if (queue.length >= QUEUE_DEPTH) {
            console.warn('[MessageBus] 队列满,消息丢弃: %s', getMsgName(msgId));
            return false;
        }
        queue.push(new Msg(msgId, param, data_ptr));
        return true;
    }

    /**
     * 每1ms消费1条消息
     * 调用所有匹配的已注册回调
     */
    function run1ms() {
        if (queue.length === 0) return;
        const msg = queue.shift();
        let dispatched = 0;
        for (let i = 0; i < handlers.length; i++) {
            if (handlers[i].id === msg.id) {
                try {
                    handlers[i].cb(msg.id, msg.param, msg.data_ptr);
                    dispatched++;
                } catch (e) {
                    console.error('[MessageBus] 回调异常: %s - %s', getMsgName(msg.id), e.message);
                }
            }
        }
        if (dispatched === 0 && msg.id !== MsgId.MSG_TIMER_100MS) {
            console.log('[MessageBus] 消息无处理器: %s param=%d', getMsgName(msg.id), msg.param);
        }
    }

    /**
     * 清空队列(用于停止/重置)
     */
    function flush() {
        queue = [];
    }

    function getMsgName(id) {
        for (let key in MsgId) {
            if (MsgId[key] === id) return key;
        }
        return 'UNKNOWN(' + id + ')';
    }

    return { init, register, post, run1ms, flush, getMsgName };
})();

/* 全局导出 */
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { MsgId, KeyEvent, Msg, MessageBus };
}
