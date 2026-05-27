# HMI 仿真平台

四头电磁炉人机交互逻辑的 Web 仿真验证平台。通过面板编辑器 + 逻辑层适配器架构，在浏览器中完整模拟灯板按键、数码管显示、LED 和蜂鸣器行为。

## 快速启动

```bash
# 方式1：直接双击
start_server.bat

# 方式2：命令行
cd phase1-js-logic
python -m http.server 8080
```

浏览器打开 http://localhost:8080

## 目录结构

```
sim/
├── phase1-js-logic/          # 主开发版本（面板编辑器 + JS/WASM逻辑层）
│   ├── index.html            # 主页：编辑模式 + 演示模式
│   ├── js/                   # JS模块
│   │   ├── logic_layer_adapter.js   # 逻辑层适配器基类（统一接口）
│   │   ├── js_emc_logic_adapter.js  # JS状态机适配器
│   │   ├── ems_wasm_adapter.js      # WASM C状态机适配器
│   │   ├── control_binder.js        # 控件绑定器（界面↔逻辑层）
│   │   ├── emc_logic_js.js          # JS完整状态机（9状态14按键）
│   │   ├── demo.js                  # 演示模式
│   │   ├── editor.js                # 编辑模式
│   │   ├── key_handler.js           # 按键处理（短按/长按检测）
│   │   ├── seg_display.js           # 段码显示映射
│   │   └── ...
│   ├── configs/              # 面板布局JSON + 逻辑规则JSON
│   │   ├── emc_controller_v1.0_basic.json
│   │   ├── emc_controller_v2.0_standard.json
│   │   └── emc_controller_v3.0_minimal.json
│   ├── wasm/                 # WASM二进制 + Emscripten glue
│   └── assets/               # 底图、音效
│
├── phase1-js-logic-release/  # 发布版（新增JSON规则驱动逻辑层）
│   ├── js/json_logic_adapter.js     # JSON规则驱动逻辑层适配器
│   └── js/logic_layer.js            # 简化JS状态机
│
├── phase2-wasm-c-source/     # WASM C源码
│   ├── emc_logic.c/h         # C状态机实现（9状态，1399行）
│   ├── wasm_wrapper.c        # Emscripten桥接层
│   └── dll_wrapper.c/h       # Windows DLL导出接口
│
├── start_server.bat          # 一键启动HTTP服务器
└── README.md                 # 本文件
```

## 架构：逻辑层适配器模式

```
界面层（Canvas控件）
    │
    ▼
ControlBinder（控件绑定器）
    │
    ▼
LogicLayerAdapter（统一接口）
    ├── JSEmcLogicAdapter     → 纯JS状态机
    ├── EmsWasmAdapter        → WASM C状态机
    └── JsonLogicAdapter      → JSON规则驱动
```

三个逻辑层实现共享同一接口，可热切换对比验证。

## 两种模式

- **编辑模式**：拖拽控件调整位置/大小，修改属性，导入/导出JSON配置
- **演示模式**：点击按键 → 状态机运转 → 数码管/LED实时更新 → 蜂鸣器发声
