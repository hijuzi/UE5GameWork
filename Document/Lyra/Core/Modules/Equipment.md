---
module: equipment
complexity: Medium
loc: 1195
file_count: 12
---

# Equipment

## 架构

三层数据驱动装备系统：

```
ULyraEquipmentDefinition (DataAsset) —— 定义要生成什么
    → ULyraEquipmentInstance (UObject, 复制) —— 运行时实例
        → ULyraEquipmentManagerComponent (PawnComponent) —— 管理器
            → ULyraQuickBarComponent (ControllerComponent) —— 热键栏
```

- **EquipItem**: 读取 Definition CDO → 创建 Instance → 授予 AbilitySets → 生成 Actor
- **FastArray 复制**: `FLyraEquipmentList` + `PreReplicatedRemove/Add` → 客户端自动装备/卸载
- **Subobject 复制**: 每个 Instance 注册为 Actor 的复制子对象

## 关键类

| 类 | 职责 |
|----|------|
| `ULyraEquipmentManagerComponent` | 核心：EquipItem/UnequipItem，FastArray 复制，子对象管理 |
| `ULyraEquipmentInstance` | 运行时实例：生成/销毁 Actor，OnEquipped/OnUnequipped 事件 |
| `ULyraEquipmentDefinition` | DataAsset：定义 InstanceType, AbilitySets, ActorsToSpawn |
| `ULyraQuickBarComponent` | 热键栏：Slot 管理，ServerRPC 切换，GameplayMessage 广播 |
| `ULyraGameplayAbility_FromEquipment` | Ability 基类：追溯 SourceObject → EquipmentInstance → InventoryItem |

## 数据流

```
QuickBar → SetActiveSlotIndex(ServerRPC) → UnequipOld → EquipNew
  → EquipmentManager::EquipItem(DefinitionClass)
    → FastArray.AddEntry → PostReplicatedAdd
      → Instance.OnEquipped() → SpawnActors → GrantAbilitySets
```
