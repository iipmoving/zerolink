# 四炉头电磁炉JSON配置文件版本说明

## 版本概览

| 版本 | 文件名 | 特点 | 适用场景 |
|-----|--------|------|---------|
| V1.0 | `emc_controller_v1.0_basic.json` | 基础版：主线流程 + 上电序列 | 快速验证核心架构 |
| V2.0 | `emc_controller_v2.0_standard.json` | 标准版：完整功能（定时/Boost/暂停/童锁） | 生产环境使用 |
| V3.0 | `emc_controller_v3.0_minimal.json` | 简化版：极简配置，突出设计理念 | 学习理解架构 |

---

## V1.0 基础版

### 设计目标
- 验证JSON规则驱动架构的可行性
- 实现最核心的调功率流程
- 展示4炉头独立逻辑区的设计

### 包含功能
✅ 上电序列（全显3秒 → 版本显示3秒 → 待机）  
✅ 待机态 → 开机态 → 选炉头 → 调功率 → 工作  
✅ Boost模式（长按9键进入）  
✅ 5秒超时逻辑  
✅ 4炉头独立状态  

❌ 定时功能  
❌ 暂停功能  
❌ 童锁功能  
❌ 组合功能  

### JSON结构特点
```json
{
  "meta": {...},
  "power_on_sequence": {...},  // 上电序列
  "head_template": {...},      // 单炉头逻辑模板
  "global_state_machine": {...} // 全局状态机
}
```

### 核心节点
- `node_idle` - 空闲态
- `node_selected` - 选中闪烁态
- `node_working` - 工作态
- `node_boost` - Boost模式

---

## V2.0 标准版

### 设计目标
- 实现规格书中的全部功能（除组合外）
- 展示独立进程的设计理念
- 可作为生产环境的配置模板

### 包含功能
✅ V1.0的所有功能  
✅ 定时功能（设置、倒计时、超时）  
✅ 暂停功能（全局暂停/恢复）  
✅ 童锁功能（长按锁定/解锁）  
✅ Boost + 定时同时有效  
✅ 交替显示模式（时间+档位）  

❌ 组合功能（暂缓）  

### JSON结构特点
```json
{
  "meta": {...},
  "power_on_sequence": {...},
  "independent_processes": {     // 新增：独立进程库
    "timer_process": {...},
    "boost_process": {...},
    "pause_process": {...},
    "child_lock_process": {...}
  },
  "head_template": {...},
  "global_state_machine": {...}
}
```

### 核心节点（比V1.0增加）
- `node_timer_setting` - 定时设置态
- `node_working_with_timer` - 工作态+定时
- `node_boost_with_timer` - Boost+定时
- `node_paused` - 暂停态

### 独立进程调用示例
```json
{
  "trigger": "key_timer",
  "condition": "short_press",
  "call": "timer_process.start",
  "args": [15]
}
```

---

## V3.0 简化版

### 设计目标
- 突出"声明式配置"的核心理念
- 展示JSON配置的极简写法
- 便于学习和理解架构本质

### 包含功能
✅ 核心主线流程  
✅ 简化的独立进程定义  
✅ 极简的状态机配置  

### JSON结构特点
```json
{
  "meta": {...},
  "power_on_sequence": {         // 极简写法
    "steps": [...],
    "on_complete": "node_standby"
  },
  "independent_processes": {     // 简化定义
    "timer_process": {
      "functions": {...}
    }
  },
  "head_template": {...},        // 简化函数定义
  "global_state_machine": {...}
}
```

### 简化对比

**V2.0 标准写法**：
```json
{
  "functions": {
    "select": {
      "description": "选中炉头",
      "actions": [
        {"action": "state.selected", "set": true},
        {"action": "led.head_select.on"},
        {"action": "segment.set_blink", "args": [true]}
      ]
    }
  }
}
```

**V3.0 简化写法**：
```json
{
  "functions": {
    "select": [
      {"action": "state.selected", "set": true},
      {"action": "led.head_select.on"},
      {"action": "segment.set_blink", "args": [true]}
    ]
  }
}
```

### 设计理念体现
1. **声明式配置**：JSON只标识状态（如 `segment.set_mode("power")`），不关心具体值
2. **元素状态标识化**：显示模式用标识符表示（"power"、"ascii"、"dash"等）
3. **独立进程**：定时/Boost/暂停作为独立进程，以函数形式调用
4. **模板复用**：4个炉头共享同一套JSON模板

---

## 按键命名规范

所有版本统一使用功能名称作为按键标识：

| 功能名称 | JSON标识 | 说明 |
|---------|---------|------|
| 开关键 | `key_power` | 长按1.5秒开关机 |
| 童锁键 | `key_child_lock` | 长按1.5秒锁定/解锁 |
| 无区键 | `key_zone` | 进入组合设置 |
| 暂停键 | `key_pause` | 全局暂停/恢复 |
| 定时键 | `key_timer` | 设置定时 |
| 炉头选择键1-4 | `key_head_1` ~ `key_head_4` | 选中对应炉头 |
| 加键 | `key_plus` | 定时增加 |
| 减键 | `key_minus` | 定时减少 |
| 档位键0-9 | `key_0` ~ `key_9` | 设置功率档位 |

**注意**：后续会通过映射表将这些功能名称映射到实际的keyCode。

---

## 数码管显示模式标识

| 模式标识 | 说明 | 内部状态来源 |
|---------|------|-------------|
| `power` | 显示功率档位 | `internal_state.power_level` |
| `timer` | 显示定时时间 | `internal_state.timer_value` |
| `alternating` | 交替显示（时间+档位） | 自动切换 |
| `alternating_boost` | 交替显示（P + 时间） | 自动切换 |
| `ascii` | 显示ASCII字符（PA、P等） | `internal_state.ascii_text` |
| `dash` | 显示横杠（待机） | 常量 "--" |

---

## 使用建议

### 初学者
从 **V3.0 简化版** 开始，理解核心设计理念。

### 开发测试
使用 **V1.0 基础版**，快速验证JSON引擎的实现。

### 生产环境
使用 **V2.0 标准版**，包含完整功能，可直接部署。

### 扩展开发
基于 **V2.0 标准版** 添加新功能（如组合功能、跑马灯显示等）。

---

## 下一步工作

1. **实现JSON解析引擎**
   - 加载JSON配置文件
   - 解析状态机节点和事件
   - 执行动作和函数调用

2. **实现独立进程管理器**
   - 定时器引擎
   - Boost超时处理
   - 暂停/童锁状态管理

3. **实现4炉头实例化**
   - 从head_template克隆4份
   - 管理热点炉头切换
   - 更新公共显示区

4. **集成测试**
   - 对比JS/WASM行为一致性
   - 验证所有按键功能
   - 性能测试（60fps）

---

**文档版本**：V1.0  
**创建日期**：2026-02-03  
**作者**：EMC团队
