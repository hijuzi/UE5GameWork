# memory-and-gc 使用指南

## 功能

管理 UObject 生命周期和普通 C++ 内存。覆盖 GC 可达性、UPROPERTY 保护、弱引用 TWeakObjectPtr、TSoftObjectPtr、非 UObject 智能指针 TSharedPtr/TUniquePtr。

## 使用方式

```
"这个 UObject 为什么被 GC 回收了？"
"非 UObject 类怎么安全持有 UObject？"
"该用 TSharedPtr 还是 TUniquePtr？"
```

## 使用示例

### 示例 1：防止 GC 回收

> **你**: "我的 Enemy 在几秒后指针变空了，怎么回事？"
> ```cpp
> class AMyActor : public AActor
> {
>     AEnemy* CurrentTarget;  // ❌ 会被 GC 回收！
> };
> ```

> **AI**: 需要用 `UPROPERTY()` 标记：
> ```cpp
> UPROPERTY()
> TObjectPtr<AEnemy> CurrentTarget;  // ✅ GC 知道你有引用
> ```

### 示例 2：弱引用安全访问

> **你**: "我要引用一个可能会销毁的 Widget"

> **AI**: 用 `TWeakObjectPtr`:
> ```cpp
> TWeakObjectPtr<UUserWidget> WeakWidget;
> if (WeakWidget.IsValid()) {
>     WeakWidget->DoSomething();
> }
> ```

### 示例 3：非 UObject 持有 UObject

> **你**: "我的纯 C++ 类需要持有 UObject 引用"

> **AI**: 用 `TStrongObjectPtr`（防止 GC）或 `FGCObject::AddReferencedObjects`。

## 指针类型选择

| 场景 | 类型 |
|------|------|
| UObject 成员（所有权） | `UPROPERTY() TObjectPtr<T>` |
| UObject 弱引用 | `TWeakObjectPtr<T>` |
| 软引用（蓝图指定） | `TSoftObjectPtr<T>` |
| 普通 C++ 共享 | `TSharedPtr<T>` |
| 普通 C++ 唯一 | `TUniquePtr<T>` |
| 非 UObject 持有 UObject | `TStrongObjectPtr<T>` |

## 适用场景

- 选择正确的指针/所有权类型
- 调试 GC 后崩溃或指针变空
- 非 UObject 类安全引用 UObject
