---
module: input
purpose: GameplayTag bridge between Enhanced Input and GAS ability activation
complexity: Medium
loc: 893
file_count: 14
---

# Input

## 用途

Enhanced Input 之上的一层薄封装，核心设计：**以 GameplayTag 为桥梁**，将玩家输入连接到 GAS 能力激活，同时提供用户设置驱动的自定义 InputModifier。

## 架构

### 数据流

```
硬件输入
  → Enhanced Input (InputAction + IMC)
    → ULyraInputModifier* (SettingBasedScalar/DeadZone/等)
      → ULyraInputComponent
        ├─ BindNativeAction() → 直接调用 (移动/视角)
        └─ BindAbilityActions() → ASC.AbilityInputTagPressed/Released
            → DynamicSpecSourceTags 匹配 InputTag
              → 激活 GameplayAbility
```

## 关键类

### ULyraInputConfig (DataAsset)

```cpp
struct FLyraInputAction {
    TObjectPtr<const UInputAction> InputAction;
    FGameplayTag InputTag;
};
```
- `NativeInputActions` — 手动绑定 (移动、视角)
- `AbilityInputActions` — GAS 自动路由

### ULyraInputComponent

提供模板化的绑定方法：
- `BindNativeAction<UserClass>(Config, Tag, Trigger, Object, Func)` 
- `BindAbilityActions<UserClass>(Config, Object, PressedFunc, ReleasedFunc)`

### 自定义 InputModifier

| Modifier | 用途 |
|------|------|
| `ULyraSettingBasedScalar` | 按用户设置缩放值 |
| `ULyraInputModifier_DeadZone` | 可配置死区 |
| `ULyraInputModifier_GamepadSensitivity` | 手柄灵敏度 |
| `ULyraInputModifier_AimInversion` | 瞄准反转 |

全部实时读取 `ULyraSettingsShared` 用户设置。

## 设计模式

- **DataAsset 配置**: InputConfig 外部化输入映射
- **双通道**: Native（直接绑定）和 Ability（GAS 路由）
- **GameFeature 支持**: 运行时动态添加/移除 InputConfig

## 依赖

EnhancedInput, GameplayAbilities, GameplayTags, Settings
