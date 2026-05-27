# 逻辑JSON格式 V4.0 规范

## 设计原则(继承V3.0)

| # | 原则 | 说明 |
|---|------|------|
| 1 | 声明式配置 | JSON只标识状态(`segment.set_mode("power")`),不关心具体值 |
| 2 | 元素状态标识化 | LED: off/on/blink, 数码管: dash/power/timer/ascii/alternating |
| 3 | 独立进程 | 定时/Boost/暂停/童锁以函数形式供主线调用 |
| 4 | 四炉头独立逻辑区 | 共享JSON模板×4独立实例 |
| 5 | 上电序列独立进程 | 与炉头工作逻辑完全分离 |

## V4.0 新增：消息驱动

V3.0使用回调接口(LogicLayerAdapter),V4.0改为消息机制:

```
事件输入: MSG_KEY_EVENT(id=0, param=(key_state<<8)|key_code)
定时驱动: MSG_TIMER_100MS(id=5), MSG_TIMER_1S(id=6)
显示输出: MSG_DISPLAY_REFRESH(id=4, data_ptr=DisplayData)
蜂鸣输出: MSG_BUZZER_CTRL(id=14, param=cmd)
```

## 顶层结构

```json
{
  "meta": {
    "name": "四头电磁炉 HMI逻辑",
    "version": "4.0",
    "head_count": 4
  },
  "key_map": { ... },
  "power_on_sequence": { ... },
  "global_state_machine": { ... },
  "head_template": { ... },
  "processes": { ... }
}
```

## 状态节点定义

每个节点包含:
- `enter`: 进入时执行的动作列表
- `exit`: 离开时执行的动作列表
- `on`: 事件映射表 { "MSG_KEY_EVENT": [事件规则] }

事件规则:
```json
{
  "match": { "key": "KEY_POWER", "event": "LONG" },
  "call": "processes.child_lock.toggle",
  "goto": "next_node"
}
```

## 当前实现状态

V4.0引擎当前为硬编码逻辑(json_logic_engine.js),外部JSON加载接口待后续版本实现。
引擎已覆盖全部功能:
- 上电序列(全显3s→版本3s→待机)
- 全局状态机(待机↔开机)
- 炉头状态机(idle→selected→working→boost→timer_setting→paused)
- 独立进程(定时倒计时、Boost 5min超时、暂停、童锁)
- Segment映射(2个4位→4个2位, 左上1左下2右下3右上4)
