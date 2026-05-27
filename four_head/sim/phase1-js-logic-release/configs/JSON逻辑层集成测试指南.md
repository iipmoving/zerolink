# JSON逻辑层集成测试指南

## 快速开始

### 1. 启动服务器

```bash
cd d:\Projects\emc-web\phase1-js-logic-release
python -m http.server 8080
```

或者使用现有的启动脚本：
```bash
启动服务器.bat
```

### 2. 打开浏览器

访问：http://localhost:8080/index.html

### 3. 加载界面配置

1. 点击菜单栏 **文件** → **加载示例配置**
2. 选择 `prj/lib1/fourSave.json`（四炉头面板布局）
   - 注意：这是**界面布局配置**，不是逻辑层配置

### 4. 启动演示模式

点击菜单栏 **演示** → **开始演示**

### 5. 切换到JSON逻辑层

点击菜单栏 **演示** → **📋 使用JSON逻辑层**

系统会自动：
- 停止当前演示
- 加载JSON逻辑层配置（configs/emc_controller_v2.0_standard.json）
- 重新启动演示

---

## 预期效果

### 上电序列
1. 全显3秒（所有数码管显示"8888"）
2. 版本显示3秒（seg_1显示"V1.0"，seg_2显示"P1.0"）
3. 进入待机态（显示"--"闪烁）

### 按键测试
目前JSON逻辑层是简化版本，支持以下基本功能：

- **开关键**（keyCode=0）：长按开机/关机
- **炉头选择键**（keyCode=49-52）：选中对应炉头
- **档位键**（keyCode=48-57）：设置功率档位

---

## 当前状态

### ✅ 已完成
- JSON逻辑层适配器基础框架
- 上电序列执行器
- 4炉头实例管理器
- 热点炉头管理器
- 全局状态机（简化版）
- index.html集成（菜单选项）
- demo.js集成（startDemoWithJSON函数）

### ⚠️ 待完善
- JSON解析引擎（完整实现）
- 独立进程管理器（定时/Boost/暂停/童锁）
- 条件表达式解析器
- 完整的状态机跳转逻辑
- keyCode到功能名称的映射表
- 显示渲染逻辑（段码转换）

---

## 调试方法

### 查看控制台日志

打开浏览器开发者工具（F12），查看Console标签：

```
[JsonLogicAdapter] 创建JSON逻辑层适配器
[JsonLogicAdapter] 初始化JSON逻辑层...
[JsonLogicAdapter] JSON配置: {name: "四头电磁炉控制器 - V2.0 标准版", ...}
[JsonLogicAdapter] ✅ 炉头实例管理器创建成功
[JsonLogicAdapter] ✅ 热点炉头管理器创建成功
[PowerOnSequence] 开始执行上电序列...
[PowerOnSequence] 步骤 1
[PowerOnSequence] 步骤 2
[PowerOnSequence] 步骤 3
[PowerOnSequence] ✅ 上电序列完成
[JsonLogicAdapter] ✅ JSON逻辑层初始化完成
```

### 测试按键

在Console中输入：
```javascript
AppState.logicLayer.pressKey(49, 2);  // 短按炉头1键
AppState.logicLayer.pressKey(50, 2);  // 短按数字2键
```

---

## 已知问题

1. **显示更新不完整**：目前的renderPowerLevel等函数是简化实现，只显示简单段码
2. **状态机未完全实现**：pressKey只是打印日志，没有实际的状态跳转
3. **定时器未实现**：updateTimers是空函数
4. **keyCode映射不完整**：mapKeyCodeToKeyName只有部分映射

---

## 下一步工作

### 阶段1：完善核心引擎
- [ ] 实现完整的JSON解析引擎
- [ ] 实现条件表达式解析器
- [ ] 实现状态机节点跳转逻辑
- [ ] 实现动作执行器

### 阶段2：实现独立进程
- [ ] 实现定时器引擎（CountdownTimer）
- [ ] 实现Boost超时处理
- [ ] 实现暂停/恢复逻辑
- [ ] 实现童锁功能

### 阶段3：完善显示逻辑
- [ ] 实现完整的段码渲染
- [ ] 实现交替显示模式
- [ ] 实现ASCII字符渲染
- [ ] 实现小数点控制

### 阶段4：集成测试
- [ ] 对比JS/WASM/JSON行为一致性
- [ ] 测试所有按键功能
- [ ] 性能测试（60fps）
- [ ] 内存泄漏检测

---

## 文件清单

### JSON配置文件
- `configs/emc_controller_v1.0_basic.json` - 基础版
- `configs/emc_controller_v2.0_standard.json` - 标准版
- `configs/emc_controller_v3.0_minimal.json` - 简化版
- `configs/README_JSON版本说明.md` - 版本说明文档

### JavaScript文件
- `js/json_logic_adapter.js` - JSON逻辑层适配器（新建）

### 修改的文件
- `index.html` - 添加JSON逻辑层菜单选项
- `js/app.js` - 添加switchToJSONLogic函数
- `js/demo.js` - 添加startDemoWithJSON函数

---

**文档版本**：V1.0  
**创建日期**：2026-02-03  
**作者**：EMC团队
