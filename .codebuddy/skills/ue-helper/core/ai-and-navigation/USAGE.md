# ai-and-navigation 使用指南

## 功能

构建 UE5 AI —— AIController 驱动 Pawn、Behavior Tree + Blackboard、导航网格/EQS 环境查询。

## 使用方式

```
"帮我做一个巡逻 AI，发现玩家后追击"
"EQS 怎么找到最好的掩体位置？"
"AI 行为树节点怎么自定义？"
```

## 使用示例

### 示例 1：巡逻+追击 AI

> **你**: "做一个敌人 AI：巡逻 → 发现玩家 → 追击 → 近距离攻击"

> **AI**: 创建 AIController + Behavior Tree + Blackboard（HasLineOfSight、TargetActor Key）→ 行为树用 Selector 和 Sequence 编排。

### 示例 2：EQS 找掩体

> **你**: "AI 受伤时找最近的有视线遮挡的位置"

> **AI**: EQS 生成网格点 → 测试距离 + 视线遮挡 → 选择最高分。

### 示例 3：自定义 Task 节点

> **你**: "行为树缺少一个"播放嘲讽动画"的节点"

> **AI**: 继承 `UBTTaskNode`，重写 `ExecuteTask()`：
> ```cpp
> virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMem) override;
> ```

## 核心组件

| 组件 | 用途 |
|------|------|
| AIController | 控制 Pawn 的 AI 大脑 |
| Behavior Tree | 分支+序列决策树 |
| Blackboard | AI 共享数据（Key-Value） |
| NavMesh | 寻路网格 |
| EQS | 环境查询（找最佳位置） |
| Perception | AI 感知（视觉/听觉） |

## 适用场景

- 巡逻/追击/战斗 AI
- EQS 环境查询
- 感知系统配置
- 自定义行为树节点
