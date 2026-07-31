---
module: modular-gameplay-actors
purpose: Minimal subclasses of UE game framework classes that register as GameFrameworkComponentManager receivers
complexity: Low
loc: 572
file_count: 18
---

# ModularGameplayActors

## 用途

为 UE 标准游戏框架类提供**最小子类**，使其注册为 `UGameFrameworkComponentManager` 的"接收者"，从而 GameFeature 插件可以在运行时动态注入组件。

## 核心问题

默认情况下，UE 的 `AGameMode`、`APlayerController`、`APawn` 等**不参与模块化组件扩展生命周期**。没有这些 Modular 子类，GameFeature 插件无法在功能激活时向游戏框架 Actor 注入组件（血量、背包、技能系统等）。

## 统一生命周期

所有 Modular* 类遵循相同三步模式：

```
1. PreInitializeComponents → AddGameFrameworkComponentReceiver(this)
2. BeginPlay → SendExtensionEvent(NAME_GameActorReady)
3. EndPlay → RemoveGameFrameworkComponentReceiver(this)
```

## 类列表

| 类 | 基类 | 用途 |
|----|------|------|
| `AModularCharacter` | `ACharacter` | 可扩展的角色 |
| `AModularPawn` | `APawn` | 可扩展的 Pawn |
| `AModularPlayerController` | `APlayerController` | 可扩展的玩家控制器 |
| `AModularPlayerState` | `APlayerState` | 可扩展的玩家状态 |
| `AModularGameModeBase` | `AGameModeBase` | 可扩展的基础游戏模式 |
| `AModularGameMode` | `AModularGameModeBase` | 可扩展的标准游戏模式 |
| `AModularGameStateBase` | `AGameStateBase` | 可扩展的基础游戏状态 |
| `AModularGameState` | `AModularGameStateBase` | 可扩展的标准游戏状态 |
| `AModularAIController` | `AAIController` | 可扩展的 AI 控制器 |

## 设计模式

- **Adapter/Shim**: 薄适配层，不添加新公共 API
- **Base/Standard 双层继承**: `GameModeBase → GameMode` 和 `GameStateBase → GameState`

## 依赖

- UE: ModularGameplay (核心), AIModule, Engine, Core
