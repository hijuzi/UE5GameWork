# save-and-load 使用指南

## 功能

使用 SaveGame 系统持久化和恢复游戏数据 —— 定义 USaveGame 存档对象、UGameplayStatics 存取、异步保存、序列化自定义数据。

## 使用方式

```
"帮我实现游戏存档功能"
"怎么存档玩家位置、装备、进度？"
"多个存档槽怎么管理？"
```

## 使用示例

### 示例 1：基础存档

> **你**: "保存和读取玩家位置"

> **AI**: 创建 `USaveGame` 子类：
> ```cpp
> UCLASS()
> class UMySaveGame : public USaveGame {
>     UPROPERTY() FVector PlayerLocation;
>     UPROPERTY() TArray<FName> InventoryItems;
> };
> // 保存:
> UGameplayStatics::SaveGameToSlot(SaveObj, "Slot1", 0);
> // 读取:
> USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot("Slot1", 0);
> ```

### 示例 2：异步保存

> **你**: "保存不要卡主线程"

> **AI**: 使用 `FAsyncSaveGameToSlotDelegate` 异步写入。

### 示例 3：多槽位

> **你**: "自动保存 + 手动保存 + 快速保存"

> **AI**: 每个槽位用不同名称 Index：
> - Slot: 0 = AutoSave
> - Slot: 1 = ManualSave
> - Slot: 2 = QuickSave

## 核心 API

| 函数 | 功能 |
|------|------|
| SaveGameToSlot | 同步保存 |
| AsyncSaveGameToSlot | 异步保存 |
| LoadGameFromSlot | 同步读取 |
| DoesSaveGameExist | 检查存档是否存在 |
| DeleteGameInSlot | 删除存档 |

## 适用场景

- 玩家进度存档
- 装备/物品持久化
- 多槽位管理
- 自动存档
