# levels-and-world-partition 使用指南

## 功能

构建和流送 UE5 关卡 —— UWorld、World Partition、Data Layer、Level Streaming、One File Per Actor。

## 使用方式

```
"帮我设置大世界的分块加载"
"怎么用 Data Layer 控制哪些区域显示？"
"Level Streaming 和 World Partition 选哪个？"
```

## 使用示例

### 示例 1：World Partition 配置

> **你**: "16km × 16km 开放世界怎么规划？"

> **AI**: World Settings → Enable World Partition → 设置 Grid Size → 用 `UWorldPartitionStreamingPolicy` 按距离流送。

### 示例 2：Data Layer

> **你**: "白天和夜晚使用不同的场景配置"

> **AI**: 创建 "Day" 和 "Night" Data Layer → 相关 Actor 分配到对应 Layer → 运行时 `UDataLayerSubsystem::SetDataLayerRuntimeState()` 切换。

### 示例 3：Streaming Volume

> **你**: "进入建筑内部才加载室内关卡"

> **AI**: 放置 `ULevelStreamingDynamic` Volume → Player 进入时动态加载 Streaming Level。

## 流送方式

| 方式 | 适用 |
|------|------|
| World Partition | UE5 大世界 |
| Level Streaming | 传统关卡流送 |
| Data Layer | 逻辑分组控制 |
| OFPA | 每个 Actor 独立文件 |

## 适用场景

- 开放世界分块
- 室内/室外按需加载
- 多版本场景切换
