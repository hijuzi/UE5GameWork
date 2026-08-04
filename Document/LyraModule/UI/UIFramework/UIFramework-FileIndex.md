# Lyra UI 框架 - 文件索引

> 本文档列出 Lyra UI 框架涉及的全部核心文件，按层级关系组织。

---

## 一、架构概览

```
CommonGameInstance
  └─ GameUIManagerSubsystem（抽象）
       └─ GameUIPolicy
            └─ PrimaryGameLayout（每个玩家一个）
                 ├─ Layer: UI.Layer.Game（游戏层）
                 ├─ Layer: UI.Layer.GameMenu（菜单层）
                 ├─ Layer: UI.Layer.Menu（主菜单层）
                 ├─ Layer: UI.Layer.Modal（模态层）
                 └─ ...（可扩展）
```

**调用链路**：`CommonGameInstance::AddLocalPlayer` → `GameUIManagerSubsystem::NotifyPlayerAdded` → `GameUIPolicy::NotifyPlayerAdded` → 创建/添加 `PrimaryGameLayout`

---

## 二、CommonGame Plugin（通用框架层）

> 路径前缀：`Plugins/CommonGame/Source/`

| 序号 | 文件 | 角色说明 |
|:---:|------|---------|
| 1 | `Public/CommonGameInstance.h`<br/>`Private/CommonGameInstance.cpp` | **通用 GameInstance**<br/>在 `AddLocalPlayer` 时调用 `GameUIManagerSubsystem::NotifyPlayerAdded`，触发 UI 创建；<br/>在 `RemoveLocalPlayer` 时调用 `NotifyPlayerDestroyed`，触发 UI 销毁 |
| 2 | `Public/GameUIManagerSubsystem.h`<br/>`Private/GameUIManagerSubsystem.cpp` | **UI 管理器子系统**（`UGameInstanceSubsystem`，Abstract）<br/>管理 `UGameUIPolicy` 的生命周期；<br/>提供 `NotifyPlayerAdded/Removed/Destroyed` 回调；<br/>通过 `config` 指定 `DefaultUIPolicyClass`；<br/>`ShouldCreateSubsystem` 防止子类存在时重复创建 |
| 3 | `Public/GameUIPolicy.h`<br/>`Private/GameUIPolicy.cpp` | **UI 策略**（`UObject`，`Within = GameUIManagerSubsystem`）<br/>管理每个玩家的 `UPrimaryGameLayout`（存储在 `RootViewportLayouts` 数组中）；<br/>支持三种多人模式：`PrimaryOnly` / `SingleToggle` / `Simultaneous`；<br/>负责 Layout 的创建、添加到视口、移除、释放 |
| 4 | `Public/PrimaryGameLayout.h`<br/>`Private/PrimaryGameLayout.cpp` | **主 UI 布局**（`UCommonUserWidget`）<br/>每个玩家一个实例，管理多层 Widget 堆栈（`TMap<FGameplayTag, UCommonActivatableWidgetContainerBase*>`）；<br/>提供 `PushWidgetToLayerStack` / `PushWidgetToLayerStackAsync`；<br/>支持 Dormant 状态（多人切换控制权）；<br/>Widget 切换时自动 Suspend/Resume Input |
| 5 | `Public/CommonUIExtensions.h`<br/>`Private/CommonUIExtensions.cpp` | **UI 扩展函数库**（`UBlueprintFunctionLibrary`）<br/>蓝图可直接调用的便捷 API：<br/>- `PushContentToLayer_ForPlayer` — 同步推入 Widget<br/>- `PushStreamedContentToLayer_ForPlayer` — 异步加载推入 Widget<br/>- `PopContentFromLayer` — 弹出 Widget<br/>- `SuspendInputForPlayer` / `ResumeInputForPlayer` — 输入挂起/恢复<br/>- `GetOwningPlayerInputType` — 获取输入类型 |
| 6 | `Public/CommonLocalPlayer.h`<br/>`Private/CommonLocalPlayer.cpp` | **通用 LocalPlayer**（`ULocalPlayer`）<br/>提供 `OnPlayerControllerSet` / `OnPlayerStateSet` / `OnPlayerPawnSet` 委托；<br/>`GetRootUILayout()` 获取关联的 PrimaryGameLayout |
| 7 | `Public/CommonPlayerController.h`<br/>`Private/CommonPlayerController.cpp` | **通用 PlayerController**（`AModularPlayerController`）<br/>`ReceivedPlayer` / `SetPawn` / `OnPossess` / `OnUnPossess` 触发 LocalPlayer 委托广播 |

---

## 三、LyraGame（项目定制层）

> 路径前缀：`Source/LyraGame/`

| 序号 | 文件 | 角色说明 |
|:---:|------|---------|
| 8 | `UI/Subsystem/LyraUIManagerSubsystem.h`<br/>`UI/Subsystem/LyraUIManagerSubsystem.cpp` | **Lyra UI 管理器**（继承 `UGameUIManagerSubsystem`）<br/>覆盖 `Initialize`/`Deinitialize`；<br/>每帧 Tick 中调用 `SyncRootLayoutVisibilityToShowHUD`，根据 HUD 的 `bShowHUD` 同步 RootLayout 可见性 |
| 9 | `UI/LyraActivatableWidget.h`<br/>`UI/LyraActivatableWidget.cpp` | **Lyra 可激活 Widget**（继承 `UCommonActivatableWidget`）<br/>增加 `ELyraWidgetInputMode` 枚举（`Default`/`GameAndMenu`/`Game`/`Menu`）；<br/>覆盖 `GetDesiredInputConfig` 自动驱动输入模式；<br/>编辑器下编译时验证 `BP_GetDesiredFocusTarget` 是否实现 |
| 10 | `Player/LyraLocalPlayer.h`<br/>`Player/LyraLocalPlayer.cpp` | **Lyra LocalPlayer**（继承 `UCommonLocalPlayer`，实现 `ILyraTeamAgentInterface`）<br/>管理玩家本地设置（`LyraSettingsLocal`）和共享设置（`LyraSettingsShared`）；<br/>UI 框架通过其基类 `UCommonLocalPlayer` 获取 `GetRootUILayout()` 以及 `OnPlayerControllerSet` 等委托 |
| 11 | `System/LyraGameInstance.h`<br/>`System/LyraGameInstance.cpp` | **Lyra GameInstance**（继承 `UCommonGameInstance`，`Config = Game`）<br/>作为 UI 框架的最终入口点，继承 `CommonGameInstance` 对 `AddLocalPlayer`/`RemoveLocalPlayer` 的处理逻辑 |

---

## 四、关键配置文件

> 以下配置将 LyraGame 定制类**缝合**到 CommonGame 通用框架中。

| 配置文件 | 节 / 键 | 值 | 作用 |
|---------|---------|---|------|
| `Config/DefaultEngine.ini` | `[/Script/EngineSettings.GameMapsSettings]`<br/>`GameInstanceClass` | `/Game/B_LyraGameInstance.B_LyraGameInstance_C` | 设置项目 GameInstance 为 `LyraGameInstance` 蓝图子类（继承 `CommonGameInstance`） |
| `Config/DefaultEngine.ini` | `[/Script/Engine.Engine]`<br/>`LocalPlayerClassName` | `/Script/LyraGame.LyraLocalPlayer` | 设置 LocalPlayer 类为 `ULyraLocalPlayer`（继承 `UCommonLocalPlayer`） |
| `Config/DefaultGame.ini` | `[/Script/LyraGame.LyraUIManagerSubsystem]`<br/>`DefaultUIPolicyClass` | `/Game/UI/B_LyraUIPolicy.B_LyraUIPolicy_C` | 设置 UIManager 使用的 UIPolicy 蓝图类（继承 `UGameUIPolicy`，蓝图中可配置 `LayoutClass`） |

```
配置装配链：
  DefaultEngine.ini
    ├─ GameInstanceClass → B_LyraGameInstance (LyraGameInstance)
    └─ LocalPlayerClassName → ULyraLocalPlayer
  DefaultGame.ini
    └─ DefaultUIPolicyClass → B_LyraUIPolicy (GameUIPolicy)
         └─ LayoutClass → B_LyraPrimaryGameLayout (PrimaryGameLayout)  [蓝图中配置]
```

---

## 五、关键依赖关系

| 源文件 | 依赖的关键类型 |
|--------|--------------|
| `CommonGameInstance` | → `UGameUIManagerSubsystem` |
| `GameUIManagerSubsystem` | → `UGameUIPolicy`（通过 `DefaultUIPolicyClass` 配置） |
| `GameUIPolicy` | → `UPrimaryGameLayout`（通过 `LayoutClass` 配置）<br/>→ `UCommonLocalPlayer` |
| `PrimaryGameLayout` | → `UCommonActivatableWidgetContainerBase`<br/>→ `UCommonUIExtensions` |
| `CommonUIExtensions` | → `GameUIManagerSubsystem` → `GameUIPolicy` → `PrimaryGameLayout` |
| `LyraGameInstance` | → 继承 `UCommonGameInstance` |
| `LyraLocalPlayer` | → 继承 `UCommonLocalPlayer` |
| `LyraUIManagerSubsystem` | → 继承 `UGameUIManagerSubsystem` |
| `LyraActivatableWidget` | → 继承 `UCommonActivatableWidget` |

---

## 六、核心流程

### 6.1 玩家加入 → UI 创建

```
GameInstance::AddLocalPlayer
  └─ GameUIManagerSubsystem::NotifyPlayerAdded(LocalPlayer)
       └─ GameUIPolicy::NotifyPlayerAdded(LocalPlayer)
            ├─ 监听 LocalPlayer::OnPlayerControllerSet
            └─ 若已有 LayoutInfo → AddLayoutToViewport
               若无 → CreateLayoutWidget → AddLayoutToViewport
                     └─ SetPlayerContext → AddToPlayerScreen(1000)
```

### 6.2 显示 Widget → 推入 Layer

```
CommonUIExtensions::PushContentToLayer_ForPlayer(LocalPlayer, LayerTag, WidgetClass)
  └─ GameUIManagerSubsystem → GetCurrentUIPolicy() → GetRootLayout(LocalPlayer)
       └─ PrimaryGameLayout::PushWidgetToLayerStack(LayerTag, WidgetClass)
            └─ GetLayerWidget(LayerTag) → AddWidget(WidgetClass)
```

### 6.3 Lyra Tick → HUD 可见性同步

```
LyraUIManagerSubsystem::Tick
  └─ SyncRootLayoutVisibilityToShowHUD
       └─ 遍历所有 LocalPlayer
            └─ 检查 AHUD::bShowHUD
                 └─ 同步 RootLayout 的 Visibility
```
