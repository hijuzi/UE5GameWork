---
module: ui
complexity: High
loc: 3280
file_count: 79
---

# UI — 用户界面与 HUD

Lyra 最大的单模块（79 文件），8 个子目录，完整实现 HUD / Frontend / Indicator / 武器 UI / 性能统计 / 消息系统，深植 CommonUI 插件。

## 目录结构

```
UI/
├── 根类 (9): LyraActivatableWidget, LyraHUD, LyraHUDLayout,
│            LyraTaggedWidget, LyraSettingScreen,
│            LyraJoystickWidget, LyraSimulatedInputWidget 等
├── Basic/ (1): MaterialProgressBar
├── Common/ (6): BoundActionButton, ListView, TabButtonBase,
│               WidgetFactory_Class/Blueprint
├── Foundation/ (5): ActionWidget, ButtonBase, ConfirmationScreen,
│                    ControllerDisconnectedScreen, LoadingScreenSubsystem
├── Frontend/ (3): FrontendStateComponent, LobbyBackground
├── IndicatorSystem/ (6): IActorIndicatorWidget, IndicatorDescriptor,
│                         IndicatorLayer, SActorCanvas
├── PerformanceStats/ (2): PerfStatContainerBase, PerfStatWidgetBase
├── Subsystem/ (2): LyraUIManagerSubsystem, LyraUIMessaging
└── Weapons/ (6): ReticleWidgetBase, WeaponUserInterface,
                  CircumferenceMarkerWidget, HitMarkerConfirmationWidget
```

## 关键框架

### LyraActivatableWidget + 输入模式

| 类 | 职责 |
|----|------|
| `ULyraActivatableWidget` | 扩展 `UCommonActivatableWidget`，添加 `ELyraWidgetInputMode` 枚举 (Default/GameAndMenu/Game/Menu)，自动设置 `DesiredInputConfig` |
| `ULyraHUDLayout` | 中央 HUD 编排器：Escape 菜单推送、断开控制器检测、`FTSTicker` 延迟处理 |

### IndicatorSystem

完整的 Actor→屏幕空间指示器系统：
```
IActorIndicatorWidget (接口) → UIndicatorDescriptor → UIndicatorLayer (管理) → SActorCanvas (Slate 画布)
```

### 武器 UI

`SReticleWidgetBase` + `SWeaponUserInterface` + `SCircumferenceMarkerWidget`（散布/热量可视化）+ `SHitMarkerConfirmationWidget`

### 性能统计

`LyraPerfStatWidgetBase` → `LyraPerfStatContainerBase` → 折线图渲染，支持 UMG + Slate 嵌入可滚动列表的 UI。

### Frontend

`ULyraFrontendStateComponent`: 基于 `FControlFlow` 的状态机（LoadingScreen → Splash → MainMenu → PressStart）。

## 依赖

CommonUI, CommonGame, EnhancedInput, SlateCore, UMG, GameplayAbilities, GameFeatures
