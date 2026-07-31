---
module: settings
complexity: High
loc: 3500
file_count: 41
---

# Settings — 游戏设置

## 架构

Lyra 设置系统基于 UE5 的 `UGameSettingRegistry` 框架，深植 CommonInput，分三层：

```
数据层: ULyraSettingsLocal (本地持久化)
        ULyraSettingsShared (共享, 可复制到其他平台)
        └── 派生: ULyraSettingScreen, 自定义 Setting 子类

注册层: ULyraGameSettingRegistry
        └── 动态构建树: Collection → Setting → Displays

UI层:   ULyraSettingScreen (ActivatableWidget)
        ULyraSettingValueDisplayer* (标量/离散/动态)
        └── DebugSettingPage, BrightnessControl, SafeZoneEditor
```

## 关键类

| 类 | 职责 |
|----|------|
| `ULyraSettingsLocal` | 本地设置持久化，含输入灵敏度/反转/死区/音频/安全区/着色器版本 |
| `ULyraSettingsShared` | 可跨平台共享的设置 (ControlScheme, ForcedInputMode) |
| `ULyraGameSettingRegistry` | 动态构建设置树: Collection → Setting → Display |
| `ULyraSettingScreen` | 全屏设置 UI (CommonActivatableWidget) |
| `ULyraSettingValueDisplayerScalar/Dynamic/Discrete` | 设置值显示控件 |

## 设置类型

- **GamepadSensitivity / AimSensitivity**: 手柄/瞄准灵敏度，ReadValue→Scalar→PostProcess
- **InvertLookX/Y**: 反转轴布尔设置
- **CustomSettings**: 射击游戏特有设置（武器准星等）

## 设计模式

- **DeveloperSettings 驱动**: ULyraSettingsLocal 通过 `GetCustomSettings()` 暴露给 Designer 配置
- **Dynamic Binding**: 设置值变化通过委托链同步到 InputModifier 和 UI
