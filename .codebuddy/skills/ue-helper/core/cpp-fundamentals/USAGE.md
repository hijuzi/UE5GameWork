# cpp-fundamentals 使用指南

## 功能

编写正确的 UE5 C++ 反射系统代码，覆盖 UCLASS/USTRUCT/UENUM、UPROPERTY/UFUNCTION 说明符、生成的代码管线、类前缀、CDO、NewObject vs CreateDefaultSubobject。

## 使用方式

```
"帮我创建一个 UCLASS"
"这个 UPROPERTY 该怎么写才能暴露给蓝图？"
"为什么 UHT 编译报错？"
```

## 使用示例

### 示例 1：创建带反射的类

> **你**: "帮我创建一个武器类，需要暴露给蓝图和网络"

> **AI**:
> ```cpp
> UCLASS(Blueprintable)
> class MYGAME_API AWeapon : public AActor
> {
>     GENERATED_BODY()
> public:
>     UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
>     float Damage;
> };
> ```

### 示例 2：修复 UHT 编译错误

> **你**: "编译报错 'Unrecognized type', 头文件在下面"
> ```cpp
> UPROPERTY()
> TArray<int> Values;  // 缺少 #include "UObject/ObjectMacros.h"
> ```

> **AI**: TArray 需要包含 `Containers/Array.h`，且 `generated.h` 必须是最后一个 include。

### 示例 3：结构体暴露蓝图

> **你**: "创建一个蓝图可用的 FHitResult 自定义结构体"

> **AI**: 使用 `USTRUCT(BlueprintType)` 标记，成员用 `UPROPERTY(BlueprintReadWrite)`。

## 核心规则速查

| 需求 | 做法 |
|------|------|
| 类暴露蓝图 | `UCLASS(BlueprintType/Blueprintable)` |
| 属性暴露蓝图 | `UPROPERTY(BlueprintReadWrite)` |
| 函数暴露蓝图 | `UFUNCTION(BlueprintCallable)` |
| 网络复制 | `UPROPERTY(Replicated)` + `GetLifetimeReplicatedProps` |
| 保护 GC | 始终用 `UPROPERTY()` 持有 UObject 指针 |

## 适用场景

- 新建任何 UE C++ 类/结构体/枚举
- 暴露成员或函数给蓝图/编辑器/网络
- 修复 UHT 反射编译错误
- 选择指针和所有权类型
