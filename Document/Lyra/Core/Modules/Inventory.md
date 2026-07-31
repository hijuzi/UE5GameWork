---
module: inventory
complexity: Medium
loc: 1049
file_count: 16
---

# Inventory

## 架构

Fragment 模式库存系统：

```
ULyraInventoryItemDefinition (UObject, Const) —— 定义模板
    └── Fragments: [UInventoryFragment_*] (可独立编辑)
        ├─ EquippableItem —— 链接到 EquipmentDefinition
        ├─ PickupIcon —— 显示模型/名称/颜色
        ├─ QuickBarIcon —— Slate 笔刷
        └─ SetStats —— 初始 GameplayTag 统计

ULyraInventoryItemInstance (UObject, 复制) —— 运行时实例
    └── FGameplayTagStackContainer —— 可变统计标签

ULyraInventoryManagerComponent → FLyraInventoryList (FastArray)
    → FLyraInventoryEntry (Instance + StackCount)
```

## 关键类

| 类 | 职责 |
|----|------|
| `ULyraInventoryItemDefinition` | 模板定义，Fragments 数组 |
| `ULyraInventoryItemInstance` | 运行时实例，复制 StatTags |
| `ULyraInventoryItemFragment` | 抽象 Fragment 基类，OnInstanceCreated 钩子 |
| `ULyraInventoryManagerComponent` | FastArray 复制存储，StackChanged 消息广播 |
| `IPickupable` | 拾取接口，桥接世界物体 → 库存 |
| `UPickupableStatics` | 蓝图库：模板/实例物品转移 |

## 设计模式

- **Fragment/Composition**: 定义由独立 Fragment 组成，非单一继承
- **FastArray 复制**: 每 Entry 独立增删，PostReplicatedAdd/Remove 触发消息
- **GameplayMessageSubsystem**: StackChanged 事件广播解耦 UI 更新
