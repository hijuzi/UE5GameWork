# CatPopupManager

Cat弹窗管理系统插件，统一管理游戏内各类弹窗的显示、优先级、队列调度与生命周期。

弹窗分为下面几类：
| 类型 | 说明 |
|---|---|
| 飘字（数字） | 常见于游戏内，如伤害数值、经验值、金币值等 |
| `FCatFloatingTextStyle` | 浮字文本样式配置（字号、颜色、描边、图标、额外文本等） |
## 核心类

| 类 | 父类 | 说明 |
|---|---|---|
| `UCatPopupManager` | `UGameInstanceSubsystem` | 弹窗统一调度器，随 GameInstance 生命周期自动创建和销毁 |
| `UCatPopupSettings` | `UDeveloperSettingsBackedByCVars` | 全局配置，可通过 `项目设置 → Cat Popup Manager` 面板调整 |

## 数据类型

| 类型 | 说明 |
|---|---|
| `ECatFloatingTextSlotType` | 浮字插槽类型（Character头部 / UIBuff插槽） |
| `FCatFloatingTextStyle` | 浮字文本样式配置（字号、颜色、描边、图标、额外文本等） |

## 快速使用

```cpp
// 获取弹窗管理器
UCatPopupManager* Manager = UCatPopupManager::GetInstance(this);
```

## 依赖

- `Core`, `CoreUObject`, `Engine`
- `DeveloperSettings`
- `UMG`, `Slate`, `SlateCore`
