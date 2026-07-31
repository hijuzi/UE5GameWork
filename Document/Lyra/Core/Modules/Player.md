---
module: player
complexity: High
loc: 2813
file_count: 16
---

# Player — 玩家控制器与状态

## 架构

多层 TeamAgent 代理链 + ASC 持有者模式：

```
ALyraPlayerState (数据核心, 拥有 ASC + 属性集 + PawnData + TeamID)
    ├─ ASC 在 ActorInfo 中：Owner=PlayerState, Avatar=Pawn
    └─ Pawn 死亡/重生时 ASC 持久化

ALyraPlayerController (输入/UI 代理)
    └─ 观察 PlayerState.TeamID，传播团队状态

ALyraPlayerBotController (AI 代理)
    └─ 行为树 + Perception 组件

ALyraPlayerSpawningManagerComponent (GameStateComponent)
    └─ Controller 绑定的出生点管理
```

## 关键类

| 类 | 职责 |
|----|------|
| `ALyraPlayerState` | 数据核心: ASC, PawnData, TeamID, StatTags |
| `ALyraPlayerController` | 玩家输入代理, CheatManager, UI/相机关联 |
| `ALyraPlayerBotController` | Bot AI 控制器, Perception 组件 |
| `ALyraPlayerSpawningManagerComponent` | 出生点 Occupancy 管理 |
| `ULyraLocalPlayer` | 本地玩家, 观察团队变化传播到 UI |
| `ULyraCheatManager` | 全量 Cheat 命令注册 |

## ASC 持久化

关键设计: **ASC 在 PlayerState 上，不在 Character 上**。Pawn 死亡/重生时 ASC 数据和 GE 持续存在。`AbilityActorInfo->SetAvatarActor(NewPawn)` 更新引用。

## 出生点系统

```
ALyraPlayerStart 三元 Occupancy: Empty/Partial/Full
  └─ TryClaim(Controller) → 定时器 CheckUnclaimed
  └─ GetLocationOccupancy: EncroachingBlockingGeometry 检测
```

## 设计模式

- **多层代理**: PlayerState → Controller → LocalPlayer 团队传播
- **Occupancy 状态机**: 防止多玩家同点生成
- **条件编译 Cheat**: `WITH_SERVER_CODE && UE_WITH_CHEAT_MANAGER`
