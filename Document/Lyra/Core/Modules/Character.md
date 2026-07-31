---
module: character
purpose: Component-driven character with PawnData configuration, init state machine, and GAS integration
complexity: High
loc: 2945
file_count: 16
---

# Character

## 用途

Lyra 的模块化角色系统，基于组件驱动和 PawnData 外部化配置，通过多阶段初始化状态机确保所有组件按正确顺序初始化，并与 GAS 深度集成。

## 架构

### 类层级

```
AModularPawn → ALyraPawn → AModularCharacter → ALyraCharacter
                                                    └── ALyraCharacterWithAbilities
```

### 核心组件 (ALyraCharacter)

| 组件 | 职责 |
|------|------|
| `ULyraPawnExtensionComponent` | 初始化协调器，持有 PawnData |
| `ULyraHealthComponent` | 生命值管理，死亡状态机 |
| `ULyraCameraComponent` | 第三人称相机 |
| `ULyraHeroComponent` | 玩家输入绑定，ASC 关联 |
| `ULyraCharacterMovementComponent` | 自定义移动 |

### 初始化状态机

```
Spawned → DataAvailable → DataInitialized → GameplayReady
```

- **PawnExtension**: 需要 PawnData 设置 → DataAvailable
- **HeroComponent**: 需要 PlayerState + Controller + InputComponent → DataAvailable
- 所有组件到达 DataAvailable → 集体进入 DataInitialized
- HeroComponent 在转换时绑定 ASC + Input + CameraMode

### PawnData

`ULyraPawnData` (PrimaryDataAsset):
- `PawnClass` — 生成的角色类
- `AbilitySets` — 授予的 AbilitySet 数组
- `InputConfig` — 输入绑定配置
- `DefaultCameraMode` — 默认相机模式

## 关键类

| 类 | 职责 |
|----|------|
| `ALyraCharacter` | 主角色类 |
| `ALyraCharacterWithAbilities` | 自含 ASC 的非玩家角色变体 |
| `ULyraPawnExtensionComponent` | 初始化协调器 |
| `ULyraHeroComponent` | 玩家输入/ASC/相机绑定 |
| `ULyraHealthComponent` | 生命值与死亡 |
| `ULyraPawnData` | 数据配置资产 |

## 设计模式

- **组件组合**: 行为通过组件添加，不通过继承
- **状态机**: GameFrameworkComponentManager InitState 统一管理
- **DataAsset 配置**: PawnData 使不同角色可无代码配置

## 依赖

ModularGA, AbilitySystem, Camera, Input, System, GameplayTags, Teams, Messages, GameFeatures
