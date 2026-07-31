# Core Synthesis — 核心游戏代码

## Architecture Narrative

Lyra 的核心游戏代码层（Core）是游戏逻辑的中枢，包含 5 个紧密协作的子系统：

```
Camera ────┐
           ├── Character ── AbilitySystem
Input ─────┘                    └── Animation (标签映射)
```

**Character** 是物理中心，通过 PawnExtensionComponent 协调初始化流程，持有 Camera 和 Health 组件。**AbilitySystem** 是逻辑中心，所有战斗交互（技能、伤害、治疗、标签条件）通过 ASC 路由。**Input** 将玩家按键通过 GameplayTag 桥接到 GAS 能力激活。**Camera** 提供可堆叠的模式混合。**Animation** 桥接 GAS 标签到动画蓝图。

## Architecture Type

组件驱动 + 数据驱动架构。行为由 PawnData (DataAsset) 外部化配置，组件通过 GameFrameworkComponentManager 的初始化状态机协调加载顺序。

## System-Wide Patterns

- **InitState 状态机**: 所有组件遵循统一的多阶段初始化链
- **GameplayTag 路由**: 输入→能力、标签→动画变量、能力关联
- **DataAsset 配置**: PawnData, InputConfig, AbilitySet 外部化所有配置
- **Determiner 委托**: CameraMode 选择通过可替换的委托

## Module Summary

| 模块 | LOC | 文件 | 复杂度 | 关键类 |
|------|-----|------|--------|--------|
| AbilitySystem | 5,400 | 51 | High | LyraAbilitySystemComponent, LyraGameplayAbility, AbilitySet, HealthSet |
| Character | 2,945 | 16 | High | LyraCharacter, PawnExtensionComponent, HeroComponent, PawnData |
| Camera | 1,690 | 12 | Medium | LyraCameraComponent, CameraModeStack, CameraMode_ThirdPerson |
| Input | 893 | 14 | Medium | LyraInputConfig, LyraInputComponent, InputModifiers |
| Animation | 113 | 2 | Low | LyraAnimInstance |

**总计: 11,041 行，95 文件**
