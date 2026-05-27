# ADR: 文档迁移到工作区

**日期**: 2026-05-21
**状态**: 已批准

## 决策

外部 `docs/` 目录整体迁移到 `four_head/docs/`，外部资料路径记录到 CLAUDE.md。

## 理由

1. `docs/` 放在 `four_head/` 外部时，与 `.claude/specs/` 形成两套文档体系，来源不清
2. `docs/` 中的架构方案、规格书、审查报告都是本项目产物，应随项目版本管理
3. 父目录下虽有多个变体（four_head_ih_cooker 等），但它们是早期原型，当前唯一活跃工作区是 four_head
4. 外部大文件（芯片资料、参考程序）不迁移，改为在 CLAUDE.md 中记录路径引用

## 迁移映射

| 源 (外部) | 目标 (four_head 内) |
|-----------|---------------------|
| `../docs/ai-guidelines/` | `docs/ai-guidelines/` |
| `../docs/architecture/` | `docs/architecture/` |
| `../docs/references/` | `docs/references/` |
| `../docs/reviews/` | `docs/reviews/` |
| `../docs/specs/` | `docs/specs/` |
| `../IH_MoBus版协议_25.12.02_V1.1.xls` | `docs/specs/IH_MoBus版协议_25.12.02_V1.1.xls` |

## 外部引用（不迁移，只记录路径）

| 用途 | 路径 |
|------|------|
| 参考程序 | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\参考程序` |
| 芯片资料 | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\芯片资料` |

## 后续清理

- 填充 `.claude/specs/tech-stack.md`、`code-style.md`、`api-conventions.md`
- 删除外部空 `docs/` 目录（迁移完成后）
