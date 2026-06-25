# Codex 审查日志索引

> 这是 Codex（审查者角色）的独立记忆，与 `memory/`（开发记忆，Claude 维护）分离。
> Codex 只读 `memory/` 作参考，不写入；审查产出全部记录在本目录 `audits/` 下。

## 角色边界

- **Codex = 审查者**：核对方法论合规、跟踪缺口、对比基线判断净进展、给优先级。
- **不做开发目的的事**：不自行重构、不替开发记忆背书。改动需用户确认后按 SOP 推进。
- **记忆独立**：审查基线与历史只进 `.codex-review/`，不污染 `memory/MEMORY.md` 索引。

## 审查记录

| 日期 | 文件 | 结论摘要 |
|------|------|----------|
| 2026-06-20 | [progress-audit](audits/2026-06-20-progress-audit.md) | v2.3 范式落地~70%，迁移过渡期；缺 deps_config/interface_map/structs/7个工具；check_deps 基线 29 违规 |

## 当前基线（最新）

- 来源：`audits/2026-06-20-progress-audit.md`
- check_deps：扫描 67 文件，29 违规（app 6 / drv 5 / proto 1 / core 15 / weak 2）
- 下次审查：对比此数字判断净进展。

| 2026-06-21 | [audit-2026-06-21](audits/2026-06-21-audit.md) | �׿�ģʽ��飺���÷���B�����ļ���Ƕ��ʽ�Σ���proto_modbus_module.c ��ɾ�� |
