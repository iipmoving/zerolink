---
name: init-project
description: "Initialize a new embedded project with full zero-coupling methodology bindings — directory structure, templates, tools, deps config, pre-commit hooks. Triggers on: init project, create project, new project, init-project, 初始化项目, 创建项目."
---

# /init-project — 项目初始化向导

创建符合零耦合架构的完整嵌入式 C 项目骨架。

**Methodology Seed**: `../methodology-seed-v2.0/`

---

## Step 1: 回答问题

交互式 5 问：

```
1. 项目名称？(kebab-case, 如 "motor_controller")
2. 项目目录？(默认: ../{name}/)
3. MCU 型号？
   A. SC32L14T (Cortex-M0+) — 赛元 (四头灯板)
   B. RX32G410 (Cortex-M4F) — 瑞芯微 (半桥 EKF)
   C. STM32F103 (Cortex-M3)
   D. 其他
4. 编译器？
   A. ARMCC V5.06 (Keil MDK)
   B. GCC (ARM Embedded)
5. 层目录名？(默认: app/ drv/ hal/ proto/ core/ cfg/)
```

---

## Step 2: 创建目录结构

```
{project}/
  app/           # 应用层（业务逻辑）
  drv/           # 驱动层（设备封装）
  hal/           # 硬件抽象层（寄存器操作）
  proto/         # 协议层（编解码）
  core/          # 核心基础设施（msg_scheduler, data_switcher）
  cfg/           # 配置（structs.json）
  include/       # 公开 IO 接口（_io.h）
  src/           # 入口（main.c）
  tools/         # 方法论检查工具
  docs/          # 项目文档
```

---

## Step 3: 复制模板

从 `../methodology-seed-v2.0/templates/` 复制：

| 模板 | 目标 | 用途 |
|------|------|------|
| `CLAUDE.md` | `{project}/CLAUDE.md` | 项目指令 |
| `tech-stack.md` | `{project}/.claude/specs/tech-stack.md` | 技术栈声明 |
| `code-style.md` | `{project}/.claude/specs/code-style.md` | 代码规范 |
| `api-conventions.md` | `{project}/.claude/specs/api-conventions.md` | 接口约定 |
| `ubiquitous-language.md` | `{project}/.claude/specs/ubiquitous-language.md` | DDD 术语 |
| `structs.json` | `{project}/cfg/structs.json` | 跨模块结构体源 |
| `interface_map.h` | `{project}/cfg/interface_map.h` | __weak 配对表 |

---

## Step 4: 复制工具

从 `../methodology-seed-v2.0/tools/` 复制到 `{project}/tools/`：

- `check_deps.py` — 层依赖审计
- `check_weak_pairs.py` — __weak 配对一致性
- `generate_structs.py` — 结构体生成
- `check_structs.py` — 结构体一致性
- `check_include.py` — _io.h include 权限审计

---

## Step 5: 生成 deps_config.json

根据 Q5 的层目录名生成依赖白名单：

```json
{
  "project": "{name}",
  "layer_dirs": ["app", "drv", "hal", "proto", "core", "cfg"],
  "rules": {
    "app":   { "allowed": ["app/", "core/", "proto/", "cfg/", "<"], "forbidden": ["drv/", "hal/"] },
    "drv":   { "allowed": ["drv/", "core/", "hal/", "cfg/", "<"], "forbidden": ["app/", "proto/"] },
    "hal":   { "allowed": ["hal/", "<"], "forbidden": ["core/", "app/", "drv/", "proto/", "cfg/"] },
    "proto": { "allowed": ["proto/", "core/", "<"], "forbidden": ["app/", "drv/", "hal/", "cfg/"] },
    "core":  { "allowed": ["core/", "include/", "<"], "forbidden": ["app/", "drv/", "hal/", "proto/", "cfg/"] },
    "cfg":   { "allowed": ["cfg/", "app/", "<"], "forbidden": ["drv/", "hal/", "proto/", "core/"] }
  }
}
```

---

## Step 6: 验证

```bash
cd {project}
python tools/check_deps.py --self-test
python tools/generate_structs.py --check
```

全部 PASS → 项目就绪。
