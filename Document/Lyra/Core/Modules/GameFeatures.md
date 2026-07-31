---
module: gamefeatures
complexity: High
loc: 1838
file_count: 16
---

# GameFeatures — GameFeature 集成桥接

## 架构

四层 GameFeature Action 体系：

```
策略层: ULyraGameFeaturePolicy
        └── 插件加载控制 (Client/Server分流, 预加载列表)
        └── 注册全局观察者 (HotfixManager, GameplayCuePaths)

观察者: ULyraGameFeature_HotfixManager
        ULyraGameFeature_AddGameplayCuePaths

抽象基类: UGameFeatureAction_WorldActionBase
          └── World生命周期: OnStartGameInstance→AddToWorld()

具体Action: (6个)
```

## 所有 Action

| Action | 职责 |
|--------|------|
| `AddAbilities` | 向 PawnData 扩展注入 AbilitySet (多 Pawn 类, 多 Set) |
| `AddInputContextMapping` | 向 EnhancedInput 子系统添加 InputMappingContext (支持多槽位和优先级) |
| `AddInputBinding` | 向 LyraHeroComponent 动态添加 InputConfig |
| `AddWidgets` | 通过 UIExtensionSubsystem 动态添加 UMG Widget |
| `SplitscreenConfig` | 配置分屏参数 |
| `AddGameplayCuePath` | 动态添加 GameplayCue 搜索路径，自动重建 CueSet |

## 激活/停用流程

```
GameFeature 激活/停用
  → UGameFeatureAction::OnGameFeatureActivating/Deactivating()
    → HandleGameFeatureStart/HandleGameFeatureDeactivate
      → AddToWorld(World) / 遍历 PerContextData
        → 授予/移除 AbilitySet
        → 添加/移除 InputMappingContext
        → 注册/注销 Widget Extension
```

## 设计模式

- **观察者**: `IGameFeatureStateChangeObserver` 监听所有插件的全局生命周期
- **扩展处理器**: GameFrameworkComponentManager + GameFeatureAction 实现运行时内容注入
- **策略模式**: Policy 层控制插件行为

## 依赖

GameFeaturesSubsystem, AbilitySystem, EnhancedInput, UIExtension, CommonUI
