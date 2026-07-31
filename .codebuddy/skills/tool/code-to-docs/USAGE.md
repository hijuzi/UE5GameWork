# code-to-docs 使用指南

## 功能

从代码库生成 Obsidian 文档知识库，包含架构图、API 参考、三层受众说明（入门/中级/高级）。

## 使用方式

直接在对话中说：

```
"给这个项目生成技术文档"
"帮我把代码库文档化，输出到指定目录"
```

## 两种模式

| 模式 | 命令示例 | 内容 |
|------|---------|------|
| **Quick（默认）** | `"生成项目文档"` | 架构概览、模块清单、API 参考、代码健康评估 |
| **Full** | `"完整模式生成项目文档"` | Quick + 设计模式、新手引导、交叉关注点、教程 |

## 使用示例

### 示例 1：快速生成文档

> **你**: "帮我给 `e:/project/src` 生成文档"

> **AI**: 自动执行 Phase 1（代码分析）→ Phase 2（文档生成）→ Phase 3（链接验证），输出到 `./docs-vault/`

### 示例 2：指定输出路径

> **你**: "生成完整模式文档，输出到 `D:/docs/my-obsidian-vault/`"

### 示例 3：部分模块文档

> **你**: "只给 Network 和 BattleSystem 模块生成文档"

## 输出结构

```
docs-vault/
├── Index.md                    # 入口索引
├── Architecture/
│   ├── System Overview.md      # 系统总览（Mermaid 架构图）
│   ├── System Map.canvas       # 可视化系统地图
│   └── Dependency Map.md       # 依赖关系图
├── Modules/
│   ├── NetworkModule.md        # 每个模块的详细文档
│   └── BattleModule.md         # 分 3 级难度讲解
├── Health/
│   ├── Health Summary.md       # 健康度总览
│   ├── Limitations.md          # 已知限制
│   └── Code Review.md          # 代码审查建议
└── _state/                     # 分析状态（用于增量更新）
```

## 适用场景

- 接手新项目，快速了解代码结构
- 生成 Obsidian 知识库供团队共享
- 代码审计后生成健康报告
- 归档项目文档
