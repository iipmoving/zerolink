---
name: check-json
description: "Validate project.json correctness: pipe symmetry, nested structs, array sizes, file paths. Trigger: 检查JSON, 验证JSON, check-json, validate-json"
user-invocable: true
---

# /check-json — project.json 正确性验证

## 用法

```bash
# 基本检查
python codeGen/check_json.py codeGen/out/<项目>/project.json

# 带 KEIL 项目路径核对
python codeGen/check_json.py codeGen/out/<项目>/project.json --keil <路径>/project.uvprojx
```

## 检查项

| # | 检查项 | 说明 |
|---|--------|------|
| 1 | Schema 合规 | output_root 绝对路径、模块/管道必填字段 |
| 2 | 源文件存在性 | 所有 source_file 和 _io.h 实际存在 |
| 3 | 管道对称性 | 嵌套 struct types[] 完整性、指针类型引用检查 |
| 4 | slot_order 完整性 | 所有模块都在 slot_order 中 |
| 5 | KEIL 路径一致性 | source_file 路径与 KEIL 项目文件匹配 |

## 执行后

- PASS/FAIL/WARN 汇总
- FAIL 项必须修复后才能用 GUI 生成
