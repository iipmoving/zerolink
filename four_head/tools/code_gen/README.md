# four_head v2.3 代码自动生成器

## 概述

根据模块清单 + 数据流向表，自动生成 v2.3 PULL 架构的框架代码。

## 目录结构

```
code_gen/
├── README.md           # 本说明文件
├── code_generator.py   # 生成器主脚本
├── flow_config.json    # 模块清单 + 数据流向配置
└── gen_output/         # 生成输出目录（自动创建）
```

## 输入配置格式

### flow_config.json

```json
{
    "modules": [
        {"name": "AppHmi", "layer": "app"},
        {"name": "DrvKey", "layer": "drv"}
    ],
    "producers": {
        "AppHmi": ["DrvKey", "AppPower"]
    },
    "consumers": {
        "DrvKey": ["AppHmi", "AppCooking"]
    }
}
```

### 字段说明

| 字段 | 说明 |
|------|------|
| `modules` | 模块清单，name=模块名，layer=层级(app/drv) |
| `producers` | 生产者→消费者映射（KEY是消费者，VALUE是其生产者列表） |
| `consumers` | 消费者→生产者映射（KEY是生产者，VALUE是其消费者列表） |

## 使用方法

### 基本用法

```bash
cd four_head/tools/code_gen
python code_generator.py --config flow_config.json
```

### 指定输出目录

```bash
python code_generator.py --config flow_config.json --output ../tmp_gen
```

## 生成的文件

### 1. include/*.io.h

每个模块生成一个 io.h 文件，包含 INPUT/OUTPUT 结构定义。

### 2. core/data_switcher.c

- 槽位枚举、模块注册
- Consumer InputCallback（指针直穿）
- Switcher_Run() 主循环

### 3. app/*.c / drv/*.c

模块骨架文件，包含三层结构声明和填空区。

## v2.3 命名规则

| 命名 | 格式 | 示例 |
|------|------|------|
| OUTPUT_PARAMS | `{Producer}_to_{Consumer}_Params` | `DrvKey_to_AppHmi_Params` |
| OUTPUT 成员 | `{Consumer}_params` | `AppHmi_params` |
| INPUT 成员 | `{Producer}_params` | `DrvKey_params` |

## 状态位管理

| 操作 | 时机 | 代码 |
|------|------|------|
| Producer 置位 | 写数据后 | `link->status \|= ST_NEW \| ST_OUT;` |
| InputCallback | 不修改 | 仅做指针赋值 |
| ProcessInput 消费 | 检查后清除 | `link->status &= ~ST_NEW;` |