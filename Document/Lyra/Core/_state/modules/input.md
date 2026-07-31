---
module: input
purpose: Thin but meaningful wrapper over Enhanced Input using GameplayTags as the bridge between player input and GAS ability activation.
roots:
  - "LyraStarterGame/Source/LyraGame/Input/"
complexity: Medium
loc: 893
file_count: 14
deps: [EnhancedInput, GameplayAbilities, GameplayTags, GameFeatures, LyraGame/Settings]
escalate: false
---

## Architecture <!-- c2d:s1 -->

Lyra Input 模块是 Enhanced Input 上的一层封装，核心设计：**以 GameplayTag 为桥梁连接玩家输入与 GAS 能力激活**。

### 数据流

```
硬件输入 → Enhanced Input (InputAction + IMC)
  → ULyraInputModifier* (SettingBasedScalar, DeadZone, 等)
  → ULyraInputComponent
    ├─ BindNativeAction() → 直接调用处理 (移动/视角)
    └─ BindAbilityActions() → ASC.AbilityInputTagPressed/Released
      → DynamicSpecSourceTags 匹配 InputTag
        → 激活 GameplayAbility
```

## API Reference <!-- c2d:s2 -->

### ULyraInputConfig (DataAsset)

```cpp
struct FLyraInputAction {
    TObjectPtr<const UInputAction> InputAction;
    FGameplayTag InputTag;  // Categories = "InputTag"
};
```

- `NativeInputActions` — 手动绑定的输入 (移动、视角、蹲伏)
- `AbilityInputActions` — 自动路由到 GAS 的输入

### ULyraInputComponent

```cpp
template<class UserClass, FuncType>
void BindNativeAction(ULyraInputConfig*, FGameplayTag, ETriggerEvent, UserClass*, FuncType);

void BindAbilityActions(ULyraInputConfig*, UserClass*, PressedFunc, ReleasedFunc, ...);
```

### 自定义 InputModifier

全量实现实时读取 `ULyraSettingsShared` 用户设置：
- `ULyraSettingBasedScalar` — 数值缩放
- `ULyraInputModifier_DeadZone` — 死区
- `ULyraInputModifier_GamepadSensitivity` — 手柄灵敏度
- `ULyraInputModifier_AimInversion` — 瞄准反转

## Key Files

| 文件 | 重要性 |
|------|--------|
| `LyraInputConfig.h/.cpp` | InputConfig DataAsset 定义 |
| `LyraInputComponent.h/.cpp` | 输入绑定组件 |
| `LyraSettingBasedScalar.h/.cpp` | 用户设置驱动修饰器 |
