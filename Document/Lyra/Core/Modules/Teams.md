---
module: teams
complexity: Medium
loc: 1836
file_count: 22
---

# Teams — 队伍系统

## 架构

基于 Unreal `IGenericTeamAgentInterface` 的完整队伍管理层：

```
ULyraTeamCreationComponent (GameStateComponent)
    └── Experience加载完成 → ServerCreateTeams
        └── 每队: ALyraTeamPublicInfo + ALyraTeamPrivateInfo (AInfo 副本)

ULyraTeamSubsystem (WorldSubsystem)
    └── TMap<TeamId, FLyraTeamTrackingInfo> (权威注册表)

ALyraPlayerState (IGenericTeamAgentInterface)
    └── SetGenericTeamId() → 复制 + 广播 TeamChanged 委托
```

## 关键类

| 类 | 职责 |
|----|------|
| `ULyraTeamCreationComponent` | 服务器创建队伍 + 分配玩家 (LeastPopulated算法) |
| `ULyraTeamSubsystem` | 世界级队伍注册表，CompareTeams, FindTeamFromObject |
| `ALyraTeamPublicInfo` | 公开队伍数据: DisplayAsset, 复制 Tags |
| `ALyraTeamPrivateInfo` | 私有队伍数据 (存根，待复制图限制) |
| `ULyraTeamDisplayAsset` | 外观数据: 颜色/纹理/材质参数/队名 |
| `ULyraTeamStatics` | BlueprintLibrary: 颜色/标量/纹理 Fallback |
| `UAsyncAction_ObserveTeam/Colors` | 异步观察者 (UI 绑定) |

## 队伍对比

```cpp
ELyraTeamComparison CompareTeams(A, B):
  OnSameTeam: 同队
  DifferentTeams: 异队 → 允许伤害
  InvalidArgument: 无队伍 → 有标签容器时检查特定标签
```

## 建造与销毁

- **建造**: `ServerCreateTeam()` → 各队 `SpawnActor` PublicInfo + PrivateInfo → `RegisterTeamInfo()`
- **销毁**: 无显式销毁 API。Info Actors 随 World/GameState 生存，World 销毁时自然清理

## 设计模式

- **Info Actor 复制**: PublicInfo (DisplayAsset, Tags) / PrivateInfo (存根)
- **FastArray**: TeamTags 使用 `FGameplayTagStackContainer`
- **三级观察链**: AsyncAction 同时监听 TeamChanged + DisplayAssetChanged
