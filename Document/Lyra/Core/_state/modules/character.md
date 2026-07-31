---
module: character
purpose: Modular, component-driven character system with PawnData-driven configuration, multi-phase initialization state machine, and deep GAS integration.
roots:
  - "LyraStarterGame/Source/LyraGame/Character/"
complexity: High
loc: 2945
file_count: 16
deps: [ModularGameplay, AbilitySystem, Teams, Camera, Input, System, GameplayTags, Messages, GameFeatures]
escalate: false
---

## Architecture <!-- c2d:s1 -->

Lyra 角色系统是基于**组件驱动**和 **PawnData 配置**的模块化架构。

### 类层级

```
AModularPawn
  └── ALyraPawn (团队归属接口)
       └── AModularCharacter
            └── ALyraCharacter (主角色: ASC + 生命值 + 相机)
                 └── ALyraCharacterWithAbilities (自含 ASC 变体)
```

### 核心组件 (ALyraCharacter)

| 组件 | 职责 |
|------|------|
| `ULyraPawnExtensionComponent` | 初始化协调器、PawnData 持有者 |
| `ULyraHealthComponent` | 生命值与死亡状态机 |
| `ULyraCameraComponent` | 第三人称相机 |
| `ULyraCharacterMovementComponent` | 自定义移动组件 |
| `ULyraHeroComponent` | 玩家输入与相机模式 (PlayerState ASC 绑定) |

### 初始化状态机

```
Spawned → DataAvailable → DataInitialized → GameplayReady
```

- `ULyraPawnExtensionComponent`: 需要 PawnData 设置才能进入 DataAvailable
- `ULyraHeroComponent`: 需要 PlayerState + 有效 Controller 才能进入 DataAvailable
- 所有组件到达 DataAvailable → 集体进入 DataInitialized
- HeroComponent 在 DataAvailable→DataInitialized 时绑定 ASC + Input + Camera

### PawnData

`ULyraPawnData` (PrimaryDataAsset, Const): PawnClass, AbilitySets, TagRelationshipMapping, InputConfig, DefaultCameraMode。外部化配置，无需修改代码即可定义不同职业/NPC。

## Key Files

| 文件 | 重要性 |
|------|--------|
| `LyraCharacter.h/.cpp` | 主角色类 |
| `LyraPawnExtensionComponent.h/.cpp` | 初始化协调器 |
| `LyraHeroComponent.h/.cpp` | 玩家输入与 ASC 绑定 |
| `LyraHealthComponent.h/.cpp` | 生命值与死亡 |
| `LyraPawnData.h` | 角色配置数据资产 |
