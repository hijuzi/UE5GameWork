---
module: common-game
purpose: Bridge layer between UE base game framework and Lyra-specific subclasses with opinionated UI/player defaults
complexity: Medium
loc: 3138
file_count: 29
---

# CommonGame

## 用途

提供 UE 引擎游戏框架类与 Lyra 特定子类之间的**桥接层**，为通用游戏系统（UI 管理、玩家生命周期、异步操作、按键显示）提供"有主见的默认行为"。

## 架构

```
UCommonGameInstance
    ├── 创建 UCommonLocalPlayer
    │     ├── 拥有 ACommonPlayerController
    │     └── 触发 UGameUIManagerSubsystem
    │           └── 委托给 UGameUIPolicy
    │                 └── 管理 UPrimaryGameLayout (Layer/Tag 系统)
    └── 支持系统
          ├── UCommonMessagingSubsystem (消息/对话框)
          ├── UCommonExtensionSubsystem
          └── AsyncAction_* (3 个异步 UI Action)
```

## 核心类

### 游戏实例/玩家层

| 类 | 职责 |
|----|------|
| `UCommonGameInstance` | 重写 Init/ReturnToMainMenu，提供模板框架 |
| `UCommonLocalPlayer` | 管理本地玩家，提供 `GetUIManager()` |
| `ACommonPlayerController` | 通用玩家控制器 |

### UI 管理层

| 类 | 职责 |
|----|------|
| `UGameUIManagerSubsystem` | WorldSubsystem，UI 管理中枢 |
| `UGameUIPolicy` | UI 策略基类（可插拔） |
| `UPrimaryGameLayout` | 主布局，Layer/Tag 系统 |

### 异步 UI Action

| 类 | 用途 |
|----|------|
| `AsyncAction_CreateWidgetAsync` | 异步创建 Widget |
| `AsyncAction_PushContentToLayer` | 向 UI Layer 推送内容 |
| `AsyncAction_ShowConfirmation` | 显示确认对话框 |

## Layer/Tag 系统

`UPrimaryGameLayout` 使用 `FGameplayTag` 管理 UI 层：

```cpp
RegisterLayer(TAG_UI_LAYER_GAME, GameLayerWidget);
RegisterLayer(TAG_UI_LAYER_MENU, MenuLayerWidget);

// 使用时按标签查找
GetLayerWidget(TAG_UI_LAYER_GAME)->PushWidgetToLayer(...);
```

## 设计模式

- **Template Method**: 虚函数提供扩展钩子
- **Strategy**: `UGameUIPolicy` 可插拔策略
- **Mediator**: `UGameUIManagerSubsystem` 协调各子系统
- **Observer**: 多播委托事件通知

## 依赖

UE: CommonInput, CommonUI, CommonUser, GameplayTags, ModularGameplayActors, Slate/UMG

---

**相关**: [系统总览](../Architecture/System%20Overview.md)
