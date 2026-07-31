# core-types-and-containers 使用指南

## 功能

使用 UE5 内置容器和类型（TArray/TMap/TSet、FString/FName/FText、FVector/FRotator/FQuat）替代标准库。

## 使用方式

```
"帮忙选一下用 FString 还是 FName"
"把这段 std::vector 改成 TArray"
"FVector 怎么做旋转计算？"
```

## 使用示例

### 示例 1：选字符串类型

> **你**: "这三个要用什么类型？玩家名字、UI 显示文字、ID 标识符"

> **AI**:
> - 玩家名字 → `FString`（可变、运行时修改）
> - UI 显示 → `FText`（本地化、用户可见）
> - ID 标识符 → `FName`（轻量、哈希查找、不可变）

### 示例 2：STL 转 UE 容器

> **你**: "把这段 STL 改成 UE 风格"
> ```cpp
> std::vector<AActor*> actors;
> std::map<FString, int32> scores;
> ```

> **AI**:
> ```cpp
> TArray<TObjectPtr<AActor>> Actors;
> TMap<FString, int32> Scores;
> ```

### 示例 3：3D 数学

> **你**: "计算 A 点指向 B 点的方向向量"

> **AI**:
> ```cpp
> FVector Direction = (B - A).GetSafeNormal();
> ```

## 容器选择指南

| 需求 | 用 |
|------|---|
| 动态数组 | `TArray` |
| 键值对 | `TMap` |
| 无重复集合 | `TSet` |
| 线程安全队列 | `TQueue` |
| 只读视图 | `TArrayView` |
| 可选值 | `TOptional` |
| 变体类型 | `TVariant` |

## 适用场景

- 任何存储集合、操作字符串、3D 数学的场景
- 在 FString/FName/FText 间抉择
- 用 FRotator vs FQuat 做旋转时
