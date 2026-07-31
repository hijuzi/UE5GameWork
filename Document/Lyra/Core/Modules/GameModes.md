---
module: gamemodes
complexity: High
loc: 2484
file_count: 20
---

# GameModes — 游戏模式与状态

## 架构

数据驱动的 Experience 系统：

```
ULyraExperienceDefinition (PrimaryDataAsset)
    ├─ GameFeaturesToEnable (GameFeature 插件列表)
    ├─ DefaultPawnData (默认 PawnData)
    └─ Actions (GameFeatureAction 数组)

ULyraExperienceManagerComponent (GameStateComponent)
    ├─ LoadStep: AssetLoad → GameFeatureActivate → Actions
    └─ Multi-delegate: HighPriority/MediumPriority/LowPriority

ALyraGameMode (ModularGameMode)
    ├─ BotCreation (LyraBotCreationComponent)
    └─ 胶水层: Player ↔ Character ↔ Spawning
```

## 关键类

| 类 | 职责 |
|----|------|
| `ALyraGameMode` | 模块化 GameMode，连接 Player/Character/Spawning |
| `ALyraGameState` | GameState: 持有 ExperienceManager, BotCreation |
| `ULyraExperienceManagerComponent` | 状态机，编排 Experience 加载流程 |
| `ULyraExperienceDefinition` | DataAsset: 定义 GameFeatures/PawnData/Actions |
| `ULyraUserFacingExperienceDefinition` | 面向用户的精简 Experience (名称/描述/地图) |
| `ULyraBotCreationComponent` | 服务器端 Bot 管理: AddBot/RemoveBot/SetNumBots |

## 加载流程

```
Experience Load
  ├─ AssetLoad: 加载所有 GameFeature 插件的资产
  ├─ GameFeatureActivate: 按顺序激活插件
  │    └─ Plugin.OnGameFeatureActivating → GameFeatureAction 执行
  └─ Actions: 执行 ExperienceDefinition 上的 Action 数组
  └─ 完成 → 广播 OnExperienceLoaded 委托 (三级优先级)
```

## 设计模式

- **三级委托优先级**: HighPriority → MediumPriority → LowPriority，有序解耦通知
- **DataAsset 驱动**: ExperienceDefinition 外部化游戏配置
- **状态机**: GameFrameworkComponentManager InitState 管理 Experience 加载阶段
