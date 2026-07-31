# code-to-docs-digest 使用指南

## 功能

**只读**加载已有文档知识库的上下文到对话中。编程前快速了解项目架构、模块和已知问题。

## 使用方式

```
"加载文档知识库"
"回顾项目架构文档"
```

## 使用示例

### 示例 1：编程前加载上下文

> **你**: "我先了解一下项目结构，加载文档上下文"

> **AI**: 读取 `docs-vault/_state/analysis.json`，输出架构概览、模块清单、健康状态。

### 示例 2：聚焦特定问题

> **你**: "看看项目有哪些已知 bug，加载 issue 部分"

> **AI**: 聚焦加载 `Health/Limitations.md` 和 `Code Review.md`，列出所有已知问题。

### 示例 3：查看特定模块

> **你**: "加载 NetworkModule 和 BattleModule 的详细文档"

> **AI**: 完整加载这两个模块的三层文档，其余模块仅显示概览。

## 输出格式

```
## 架构概览
- 项目名称: MyGame
- 架构类型: Client-Server
- 模块数量: 12

## 模块清单
| 模块 | 复杂度 | 状态 |
| NetworkModule | High | active |
| BattleModule | Medium | active |
| ...

## 已知问题
- [open] NetworkModule: 重连超时无处理
- [open] BattleModule: 内存泄漏风险
```

## 适用场景

- 每天开始编码前快速了解项目
- 接手别人代码前的准备工作
- 修复 bug 前了解相关模块上下文
- Code Review 前了解代码库状态

## 注意事项

- **严格只读**，不会写任何文件
- 需要先运行 `code-to-docs` 生成过文档
- 如果没有文档库，会提示先运行 `code-to-docs`
