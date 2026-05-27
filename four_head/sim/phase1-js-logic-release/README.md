# EMC 面板模拟器 - 成品版本

## 📦 包含内容

- ✅ index.html - 主页面
- ✅ js/ - JavaScript逻辑层代码
- ✅ css/ - 样式文件
- ✅ assets/ - 资源文件（底图、音效等）
- ✅ wasm/ - WASM模块（C逻辑层编译结果）
- ✅ configs/ - 配置文件

## 🚀 快速开始

### 方法1：自动启动（推荐）⭐

**双击 `自动启动.bat`**

脚本会自动完成以下步骤：
1. ✅ 检查Python是否已安装
2. ✅ 检查端口8080是否被占用
3. ✅ 如果端口空闲，自动在后台启动HTTP服务器
4. ✅ 自动打开默认浏览器访问页面

**特点**：
- 🚀 完全自动化，无需任何手动操作
- 🔍 智能检测，避免端口冲突
- 🖥️ 服务器在后台运行，关闭窗口不影响服务
- 🌐 自动打开浏览器，立即开始使用

**注意**：脚本界面显示英文是为了避免编码问题，但功能完全相同。

---

### 方法2：手动启动服务器

```bash
# 在此文件夹中打开命令行
python -m http.server 8080

# 然后在浏览器中访问
http://localhost:8080/index.html
```

### 方法3：使用Node.js http-server

```bash
# 安装（如果还没有）
npm install -g http-server

# 启动服务器
http-server -p 8080

# 然后在浏览器中访问
http://localhost:8080/index.html
```

### 方法4：直接使用浏览器打开（不推荐）

直接双击 `index.html` 文件，但某些功能可能受限（如WASM加载）。

## 🎮 使用说明

### 编辑模式
- 右键点击Canvas可以切换编辑/演示模式
- 在编辑模式下可以调整控件位置和大小
- 可以加载/保存配置文件

### 演示模式
- 点击"演示" → "开始演示"启动模拟器
- 默认使用JS逻辑层
- 可以通过菜单切换逻辑层：
  - 演示 → 🔧 使用JS逻辑层
  - 演示 → ⚙️ 使用WASM逻辑层

### 上电流程
1. S_POWER_ON (0-1.5秒) - 显示"8888"
2. S_VERSION (1.5-3秒) - 显示"F1.0"
3. S_SHUTDOWN (3秒后) - 显示"---"
4. 按电源键或START_PAUSE开机 → S_STANDBY (显示"85°")

### 按键操作
- M1-M10：选择功能
- ADD_WATER：调整水量
- ADD_TIME：调整时间
- START_PAUSE：开始/暂停烹饪（长按关机）
- POWER：电源键（长按关机）

## 🔧 技术架构

### 双逻辑层支持
- **JS逻辑层**：纯JavaScript实现的EmcLogic状态机
- **WASM逻辑层**：C代码编译的WebAssembly模块

### 统一适配器接口
所有逻辑层调用都通过统一的适配器接口：
```javascript
AppState.logicLayer.pressKey(keyCode, event);
AppState.logicLayer.getState();
AppState.logicLayer.runCycle();
```

### 10ms周期机制
每10ms运行一次runCycle()，确保：
- 不丢失按键事件
- 及时更新显示
- 准确的状态转移

## 📝 注意事项

1. **必须使用HTTP服务器**：直接打开HTML文件可能导致WASM加载失败
2. **浏览器兼容性**：推荐使用Chrome、Edge或Firefox
3. **缓存问题**：如果修改了代码，请强制刷新（Ctrl+F5）
4. **文件夹位置**：可以将整个文件夹复制到任何位置，脚本会自动处理路径
5. **不要修改结构**：保持index.html、js/、css/等文件的相对位置不变

## 🐛 故障排除

### WASM加载失败
- 确认使用了HTTP服务器而不是直接打开文件
- 检查浏览器控制台是否有错误信息
- 确认wasm文件夹中有emc_test.js和emc_test.wasm文件

### 按键无反应
- 确认已启动演示模式
- 检查浏览器控制台是否有JavaScript错误
- 尝试切换到另一种逻辑层（JS或WASM）

### 显示不更新
- 强制刷新浏览器（Ctrl+F5）
- 检查控制台日志，确认回调是否正常触发

## 📄 版本信息

- 最后更新：2026-05-19
- 基于Git提交：96585a6
- 包含修复：
  - ✅ 统一JS/WASM事件处理
  - ✅ 正确的上电流程（S_VERSION → S_SHUTDOWN）
  - ✅ 修复S_SHUTDOWN状态显示问题
  - ✅ 移除所有测试文件

## 📞 技术支持

如有问题，请查看浏览器控制台的错误信息，并提供：
1. 使用的浏览器和版本
2. 完整的错误日志
3. 复现步骤
