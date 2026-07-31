# blueprint-cpp-integration 使用指南

## 功能

将 C++ 类/函数/属性暴露给蓝图 —— UFUNCTION/UPROPERTY 说明符、BlueprintCallable/Pure/ImplementableEvent、TSubclassOf。

## 使用方式

```
"这个函数怎么暴露给蓝图调用？"
"C++ 和蓝图之间怎么通信？"
"为什么蓝图里看不到我的函数？"
```

## 使用示例

### 示例 1：暴露函数给蓝图

> **你**: "让蓝图能调用获取血量的函数"

> **AI**:
> ```cpp
> UFUNCTION(BlueprintCallable, Category = "Health")
> float GetHealthPercent() const { return Health / MaxHealth; }
> ```

### 示例 2：蓝图可重写事件

> **你**: "C++ 定义受伤事件，让蓝图处理表现"

> **AI**:
> ```cpp
> UFUNCTION(BlueprintImplementableEvent)
> void OnDamageReceived(float Damage);
> // 或 BlueprintNativeEvent 提供 C++ 默认实现
> ```

### 示例 3：TSubclassOf 限制蓝图类型

> **你**: "让设计师在蓝图里选项目类型，但只能选 AItemBase 的子类"

> **AI**:
> ```cpp
> UPROPERTY(EditAnywhere, BlueprintReadWrite)
> TSubclassOf<AItemBase> ItemClass;
> ```

## 核心说明符

| 说明符 | 含义 |
|--------|------|
| BlueprintCallable | 蓝图可调用 |
| BlueprintPure | 纯函数（无副作用，R/O 节点） |
| BlueprintImplementableEvent | 蓝图重写，C++ 不实现 |
| BlueprintNativeEvent | 蓝图可重写，C++ 有默认实现 |
| BlueprintReadWrite | 蓝图可读写 |
| EditAnywhere | 编辑器可编辑 |

## 适用场景

- 设计 C++ ↔ Blueprint 边界 API
- 选择正确的 UFUNCTION/UPROPERTY 说明符
- 调试蓝图节点/属性/事件不显示的问题
