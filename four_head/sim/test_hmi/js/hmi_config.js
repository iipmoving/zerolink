/**
 * hmi_config.js — HMI 可设常量（等价于嵌入式 hmi_config.h）
 *
 * 所有超时/定时/默认值在此集中定义。
 * 移植到 C 时对应 #define / const。
 */

/* ========== 确认超时 ========== */
var CFG_SELECT_TIMEOUT_MS    = 15000;   /* 选中→确认 15s */
var CFG_TIMER_CONFIRM_MS     = 15000;   /* 定时设置→自动确认 15s */

/* ========== 全局模式切换 ========== */
var CFG_STANDBY_MS           = 15000;   /* 全idle/0功率→待机(hotHead清空) 15s */
var CFG_IDLE_TO_OFF_MS       = 30000;   /* 全idle→POWERED_OFF 30s */
var CFG_OFF_TO_SLEEP_MS      = 30000;   /* POWERED_OFF→DEEP_SLEEP 30s */

/* ========== Boost ========== */
var CFG_BOOST_TIMEOUT_MS     = 300000;  /* Boost→恢复 5min */

/* ========== 默认定时值 ========== */
var CFG_DEFAULT_TIMER_MIN    = 15;      /* 默认定时 15分钟 */
