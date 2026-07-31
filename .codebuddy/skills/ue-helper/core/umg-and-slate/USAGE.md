# umg-and-slate 使用指南

## 功能

构建游戏 UI —— UMG User Widget (UUserWidget) 生命周期、Slate 底层、数据绑定、widget 嵌套。

## 使用方式

```
"帮我做一个血条 Widget"
"C++ 怎么创建和管理 UMG Widget？"
"怎么实现数据驱动的 UI 更新？"
```

## 使用示例

### 示例 1：创建血条

> **你**: "C++ 创建一个血条 Widget，绑定角色血量"

> **AI**: 继承 `UUserWidget` → 添加 ProgressBar 和 Text → `NativeTick` 中更新：
> ```cpp
> if (HealthBar && OwnerCharacter.IsValid()) {
>     HealthBar->SetPercent(OwnerCharacter->GetHealthPercent());
> }
> ```

### 示例 2：显示/隐藏 Widget

> **你**: "打开背包时显示 UI，关闭时隐藏"

> **AI**:
> ```cpp
> if (!InventoryWidget) {
>     InventoryWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryClass);
> }
> InventoryWidget->AddToViewport();  // 显示
> InventoryWidget->RemoveFromParent();  // 隐藏
> ```

### 示例 3：蓝图数据绑定

> **你**: "UI 中金币数字自动跟随变量更新"

> **AI**: 用 `UPROPERTY(meta=(BindWidget))` 或蓝图中 Bind 函数到属性。

## UMG vs Slate

| | UMG | Slate |
|---|-----|-------|
| 层次 | 高级（可视化编辑） | 底层（纯 C++） |
| 适用 | 游戏 HUD、菜单 | 编辑器工具、高级控件 |
| 蓝图 | ✅ | ❌ |

## 适用场景

- HUD、血条、背包、菜单
- 数据驱动 UI
- 编辑器工具面板
