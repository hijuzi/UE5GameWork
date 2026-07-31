---
module: ui-extension
purpose: GameplayTag-driven UI extension point system for decoupled widget/data composition
complexity: Medium
loc: 940
file_count: 7
---

# UIExtension

## 用途

基于 **GameplayTag** 的 UI 扩展点系统，实现**松耦合的 UI 组件组合**。Widget 通过标签注册为"扩展"，UI 布局通过标签声明"扩展点"，系统自动匹配并实例化。

## 架构

```
UUIExtensionSubsystem (WorldSubsystem)
├── ExtensionPointMap: FGameplayTag → TArray<ExtensionPoint>
│     每个 ExtensionPoint 声明:
│       - 标签 (匹配键)
│       - 匹配策略 (ExactMatch / PartialMatch)
│       - 允许的数据类型
│       - 回调委托
└── ExtensionMap: FGameplayTag → TArray<Extension>
      每个 Extension 携带:
        - 标签 (匹配键)
        - 上下文对象 (可选的 UObject*)
        - 数据 (Widget 类或 UObject*)
        - 优先级 (int32)

UUIExtensionPointWidget (UMG Widget)
├── ExtensionPointTag
├── ExtensionPointTagMatch
├── DataClasses
└── 自动注册 3 个上下文: Generic / LocalPlayer / PlayerState
```

## 匹配流程

```
扩展点注册 "UI.Layer.Game"
     ↓ 双向通知
扩展注册 "UI.Layer.Game", HUDWidget (Priority=0)
     ↓ 匹配成功
扩展点回调: OnAddOrRemoveExtension(Added=true, Request)
     ↓
UUIExtensionPointWidget::CreateEntryInternal(HUDWidget)
     ↓
Widget 添加到 DynamicEntryBox
```

## Tag 匹配策略

| 策略 | 匹配规则 |
|------|---------|
| `ExactMatch` | "A.B" 只匹配 "A.B" |
| `PartialMatch` | "A.B" 匹配 "A.B.C"（扩展更具体） |

## API

### C++ 注册

```cpp
auto* Sub = World->GetSubsystem<UUIExtensionSubsystem>();

// 注册扩展点
FUIExtensionPointHandle PtHandle = Sub->RegisterExtensionPoint(
    TAG_UI_LAYER_GAME, EUIExtensionPointMatch::ExactMatch,
    {UMyData::StaticClass()}, Callback);

// 注册扩展
FUIExtensionHandle ExtHandle = Sub->RegisterExtensionAsWidget(
    TAG_UI_LAYER_GAME, UMyHUDWidget::StaticClass(), /*Priority*/ 0);
```

### UMG Designer

1. 将 `UUIExtensionPointWidget` 添加到 UMG Canvas
2. 设置 `ExtensionPointTag` 和 `ExtensionPointTagMatch`
3. 绑定 `GetWidgetClassForData` 和 `ConfigureWidgetForData` 委托
4. Widget 会在标签匹配扩展可用时自动填充

## 依赖

UE: CommonUI, CommonGame, GameplayTags, Slate/UMG

---

**相关**: [CommonGame](CommonGame.md) — UIExtension 依赖 CommonGame 的 Layer/Tag 系统
