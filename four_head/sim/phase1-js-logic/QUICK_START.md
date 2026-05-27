# 🚀 快速启动指南

## ✅ 后端服务器已启动

后端服务器正在运行：
- **地址**: http://localhost:5000
- **状态**: ✅ Running

---

## 📖 打开测试页面

### 方法1：直接打开文件（推荐）

在文件资源管理器中双击：
```
D:\Projects\emc-web\phase1-js-logic\comparison_test.html
```

或者在浏览器地址栏输入：
```
file:///D:/Projects/emc-web/phase1-js-logic/comparison_test.html
```

### 方法2：使用本地HTTP服务器

如果方法1有CORS问题，可以：

```powershell
cd D:\Projects\emc-web\phase1-js-logic
python -m http.server 8080
```

然后在浏览器访问：
```
http://localhost:8080/comparison_test.html
```

---

## 🧪 开始测试

1. **点击"🔧 初始化"按钮**
   - 这会同时初始化JS逻辑和C DLL
   
2. **选择测试用例**
   - ⚡ 上电自检
   - 👆 按键响应
   - 📋 功能选择
   - 🔢 参数调整
   - 🍳 启动烹饪

3. **查看结果**
   - ✅ PASS - JS和DLL输出一致
   - ❌ FAIL - 存在差异，需要修复

---

## 📊 预期输出

### 成功初始化的日志
```
[时间] 初始化对比测试工具...
[时间] ✅ JS逻辑层初始化成功
[时间] ⚠️ DLL逻辑层使用模拟模式
[时间] 系统就绪，请选择测试用例
```

### 测试运行的日志
```
========== 开始测试: 上电自检流程 ==========
[Step] wait {duration: 3000}
[JS Output] status: S_POWER_ON
[JS Output] display: seg=[127, 127, 127, 127]
[DLL Output] status: S_POWER_ON
[DLL Output] display: seg=[127, 127, 127, 127]
========== 测试完成: 上电自检流程 ==========

[对比结果]
  状态变化: JS=3, DLL=3
  显示更新: JS=300, DLL=300
  蜂鸣器:   JS=0, DLL=0
  差异数:   0
  测试结果: ✅ PASS
```

---

## ❓ 常见问题

### Q1: 点击"初始化"后没有反应
**检查**:
1. 后端服务器是否在运行？访问 http://localhost:5000/api/status
2. 浏览器控制台是否有错误？按F12查看

### Q2: CORS错误
**解决**: 使用方法2（HTTP服务器）而不是直接打开文件

### Q3: DLL加载失败
**检查**: DLL文件是否存在于：
```
D:\Projects\emc_framework_v1\03_编码实现\logic\emc_logic.dll
```

---

## 🔧 停止服务器

在运行dll_server.py的终端按 **Ctrl+C**

---

## 📝 下一步

1. 运行所有测试用例
2. 查看哪些测试FAIL
3. 根据差异报告修复JS代码
4. 重新测试直到全部PASS

祝测试顺利！🎉
