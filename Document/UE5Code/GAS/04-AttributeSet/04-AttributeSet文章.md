# 04 — AttributeSet：属性定义、回调链与网络复制

---

## 一、问题引入：为什么不能直接 `float Health = 100`？

写 RPG 游戏的第一反应，一定是：**属性不就是个 float 吗？** 生命值、魔法值、攻击力、移动速度——定义几个 `float` 变量不就行了。

但当你真正开始做战斗系统时，问题一个接一个冒出来：

- **Buff 叠加**：一个「+10 攻击力」的 Buff 和「攻击力 ×1.2」的 Buff 同时生效，最后该是多少？
- **上限 Clamp**：生命值不能超过 MaxHealth，也不能低于 0。这个约束写在哪？
- **UI 刷新**：生命值变了，血条要更新。你打算每个地方都手动调 UI 吗？
- **网络复制**：服务器算出来的值，客户端怎么知道？如果客户端预测性地显示伤害数字，收到服务器确认后怎么纠正？
- **多来源释放**：两个 Buff 都加了同一个 Tag，其中一个过期了——属性是回退还是保留另一个？

这些问题不是"加几行 if 能解决"的。**你需要一整套属性管理系统，而不是几个 float。**

这正是 `UAttributeSet`（AttributeSet）存在的理由。它同时承担三件事：**属性定义**、**变更回调**、**网络复制**。

读完本文你会掌握：
- 如何用 `FGameplayAttributeData` 和宏体系定义自己的属性
- GE 修改属性值的完整调用链（从入口到回调）
- 六个虚函数回调分别在什么时机触发，各自适合做什么
- 属性网络复制的原理和客户端预测的正确性保证

---

## 二、核心概念：属性系统的三件套

在直接看源码之前，先理解三个核心类型。它们是属性系统的最小拼图。

### 2.1 FGameplayAttributeData：BaseValue 与 CurrentValue 的分离

`FGameplayAttributeData` 不是普通 float。它把一个属性拆成了**两个独立的值**：

| 字段 | 含义 | 修改方 |
|------|------|--------|
| `BaseValue` | 永久底值（permanent changes） | GE execution、SetNumericAttributeBase |
| `CurrentValue` | 最终生效值（含临时 buff） | Aggregator 聚合计算后写入 |

对应的源码定义（`AttributeSet.h` 中 `FGameplayAttributeData` 结构体）：

```
UPROPERTY(BlueprintReadOnly, Category = "Attribute")
float BaseValue;

UPROPERTY(BlueprintReadOnly, Category = "Attribute")
float CurrentValue;
```

**为什么需要两个值？** 举个例子就清楚了：

你吃了「永久 +20 攻击力」的药剂和一个「临时 +10%、持续 5 秒」的 Buff。结束时 Buff 过期，攻击力应该从 `(100+20)×1.1 = 132` 退回到 `100+20 = 120`。引擎需要知道哪个是永久变化（BaseValue=120），哪个是临时修正（Modifier 提供的 ×1.1）。

**BaseValue 是你真正"拥有"的属性值，CurrentValue 是所有修正聚合后的最终生效值。** 引擎保证每次 BaseValue 变更后，自动重新聚合所有 Modifier，算出最新的 CurrentValue。

### 2.2 FGameplayAttribute：属性的「身份证」

如果你只有 `FGameplayAttributeData` 的数据，怎么从外部访问它？C++ 的成员指针没法安全地跨类传递。GAS 的方案是 `FGameplayAttribute`——一个**用 UE 反射系统定位属性的描述符**。

核心是它保存了一个 `FProperty*` 指针（`AttributeSet.h` 中 `FGameplayAttribute` 结构体）：

```
TFieldPath<FProperty> Attribute;   // 指向属性在 UAttributeSet 子类中的 FProperty
UStruct* AttributeOwner;           // 属性的拥有者 UStruct
FString AttributeName;             // 属性名（缓存）
```

有了 `FProperty*`，`FGameplayAttribute` 可以做两件关键的事：

**写入**（`AttributeSet.cpp` 中 `FGameplayAttribute::SetNumericValueChecked`）：
```
void FGameplayAttribute::SetNumericValueChecked(float& NewValue, UAttributeSet* Dest) const
{
    // 如果是 FGameplayAttributeData 属性：
    FGameplayAttributeData* DataPtr = StructProperty->ContainerPtrToValuePtr<FGameplayAttributeData>(Dest);
    DataPtr->SetCurrentValue(NewValue);
    // 如果是原生 float 属性（旧版兼容）：
    NumericProperty->SetFloatingPointPropertyValue(ValuePtr, NewValue);
}
```

**读取**（`AttributeSet.cpp` 中 `FGameplayAttribute::GetNumericValue`）：
```
float FGameplayAttribute::GetNumericValue(const UAttributeSet* Src) const
{
    // 通过反射找到属性在内存中的位置，返回 CurrentValue
}
```

关键认知：**`FGameplayAttribute` 不存储数据，它是一把「万能钥匙」——给定任意 `UAttributeSet` 实例，它能定位到某个属性的内存位置并读写它。**

这样一来，GE、ASC、UI 绑定都不需要知道具体的 AttributeSet 子类，只要持有 `FGameplayAttribute` 就能操作属性。它是 GAS 各模块间解耦的关键桥梁。

### 2.3 UAttributeSet：六个虚函数回调点

`UAttributeSet` 是所有属性集的基类。你定义 `UMyAttributeSet` 时，继承的就是它。除了作为数据的容器外，它通过六个虚函数暴露了完整的回调链（`AttributeSet.h` 中 `UAttributeSet` 类定义）：

| 回调 | 触发时机 | 可修改值 | 典型用途 |
|------|----------|----------|----------|
| `PreGameplayEffectExecute` | GE 执行修改 base value **之前** | 可拒绝（return false） | 免疫检查 |
| `PostGameplayEffectExecute` | GE 执行修改 base value **之后** | 不可 | 伤害结算、死亡判定、受击 UI |
| `PreAttributeBaseChange` | BaseValue 即将被写入 | `NewValue` 可 Clamp | 约束 BaseValue 范围 |
| `PostAttributeBaseChange` | BaseValue 已被写入 | 不可 | 派生属性重算（如护甲→减伤率） |
| `PreAttributeChange` | CurrentValue 即将被写入 | `NewValue` 可 Clamp | 「Health = Clamp(NewHealth, 0, MaxHealth)」 |
| `PostAttributeChange` | CurrentValue 已被写入 | 不可 | 血条 UI 刷新 |

> 注释原文特别强调了 `PreAttributeChange` 的职责边界："This function is meant to enforce things like `Health = Clamp(Health, 0, MaxHealth)` and NOT things like 'trigger this extra thing if damage is applied'." Clamp 放 Pre，副作用放 Post，这是 GAS 的约定。

### 2.4 ATTRIBUTE_ACCESSORS 宏体系

每次为属性手动写 getter/setter/replication 很繁琐。引擎提供了宏一键生成（`AttributeSet.h` 中宏定义区域）。

**ATTRIBUTE_ACCESSORS_BASIC**——生成四个基础函数：

```
#define ATTRIBUTE_ACCESSORS_BASIC(ClassName, PropertyName)
    // 生成：
    //   static FGameplayAttribute GetHealthAttribute();   // 返回反射属性描述符
    //   float GetHealth() const;                          // 读 CurrentValue
    //   void SetHealth(float);                            // 写 BaseValue（走 ASC）
    //   void InitHealth(float);                           // 同时写 BaseValue 和 CurrentValue（初始化用）
```

四项展开的关键代码（已从源码验证）：

```
// Setter 的核心——它调用的是 ASC::SetNumericAttributeBase，不是直接写值
#define GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) 
    inline void Set##PropertyName(float NewVal)
    {
        AbilityComp->SetNumericAttributeBase(Get##PropertyName##Attribute(), NewVal);
    }

// Initter——直接写值，不走 ASC，不触发回调，仅用于初始化
#define GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
    inline void Init##PropertyName(float NewVal)
    {
        PropertyName.SetBaseValue(NewVal);
        PropertyName.SetCurrentValue(NewVal);
    }
```

**GAMEPLAYATTRIBUTE_REPNOTIFY**——用于 OnRep 网络回调（见 §三.3）：

```
#define GAMEPLAYATTRIBUTE_REPNOTIFY(ClassName, PropertyName, OldValue)
{
    GetOwningAbilitySystemComponentChecked()
        ->SetBaseAttributeValueFromReplication(FGameplayAttribute(ThisProperty), PropertyName, OldValue);
}
```

你的 AttributeSet 头部一般长这样：

```
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes")
FGameplayAttributeData Health;
ATTRIBUTE_ACCESSORS_BASIC(UMyAttributeSet, Health);
```

一个属性定义 + 一行宏，四件套齐了。

---

## 三、源码分析：一条完整的数据通路

现在把概念连起来，跟踪一次属性修改的完整生命周期。

![属性值完整管线](diagrams/Attr_Pipeline.png)

### 3.1 入口：SetNumericAttributeBase

当 GE 计算完毕决定修改某个属性时，不会直接操作 `FGameplayAttributeData`。它调用 ASC 的 `SetNumericAttributeBase`（`AbilitySystemComponent.h` 中 `UAbilitySystemComponent` 类）：

```
// 入口：GE/代码设置属性 BaseValue
void SetNumericAttributeBase(const FGameplayAttribute& Attribute, float NewBaseValue);
```

这一步做什么：
1. 找到该属性对应的 **`FAggregator`**（每个被 GE 修改的属性都会创建自己的评估器）
2. 调用 `Aggregator.SetBaseValue(NewBaseValue)`——**注意，这里修改的是 BaseValue，不是 CurrentValue**
3. 标记 Aggregator 为 dirty，触发重新评估

**FAggregator** 负责维护一个属性的所有 Modifier（来自各个 GE），并在需要时计算最终值。它的核心评估公式是：

```
CurrentValue = ((BaseValue + Additive) × Multiplicitive) ÷ Division × CompoundMultiply + AddFinal
```

公式解读：
- `Additive`（加法类）：来自 `EGameplayModOp::Additive` 的 Modifier 求和
- `Multiplicitive`（乘法类）：`EGameplayModOp::Multiplicitive` 的乘积
- `Division`（除法类）：`EGameplayModOp::Division` 的乘积
- `CompoundMultiply`：`MultiplyCompound` 修饰符
- `AddFinal`（最终加法）：`EGameplayModOp::AddFinal`，在所有乘除之后加上

> 无论 Modifier 用的是浮点数、曲线、Custom Calculation Class 还是 ExecCalc，最终都转化为这五项的贡献值，走同一套公式。

### 3.2 核心管线：SetNumericAttribute_Internal → SetNumericValueChecked

Aggregator 重新评估后，算出新的 `CurrentValue`，调用 `InternalUpdateNumericalAttribute` → `SetNumericAttribute_Internal`，最终到达本文最关键的函数——`FGameplayAttribute::SetNumericValueChecked`。

它的完整实现（`AttributeSet.cpp`）：

```
void FGameplayAttribute::SetNumericValueChecked(float& NewValue, UAttributeSet* Dest) const
{
    // 分支1：FGameplayAttributeData 属性（推荐用法）
    if (IsGameplayAttributeDataProperty(Attribute.Get()))
    {
        FGameplayAttributeData* DataPtr = StructProperty->ContainerPtrToValuePtr<FGameplayAttributeData>(Dest);
        float OldValue = DataPtr->GetCurrentValue();
        Dest->PreAttributeChange(*this, NewValue);       // ← ① 修改前回调
        DataPtr->SetCurrentValue(NewValue);               // ← ② 真正的写入
        Dest->PostAttributeChange(*this, OldValue, NewValue); // ← ③ 修改后回调
        MARK_PROPERTY_DIRTY(Dest, StructProperty);        // ← ④ 标记网络脏
    }
    // 分支2：原生 float 属性（旧版兼容，不推荐）
    else if (NumericProperty)
    {
        void* ValuePtr = NumericProperty->ContainerPtrToValuePtr<void>(Dest);
        float OldValue = *static_cast<float*>(ValuePtr);
        Dest->PreAttributeChange(*this, NewValue);
        NumericProperty->SetFloatingPointPropertyValue(ValuePtr, NewValue);
        Dest->PostAttributeChange(*this, OldValue, NewValue);
        MARK_PROPERTY_DIRTY(Dest, NumericProperty);
    }
}
```

这个函数是整个属性系统的「心脏」。每行都值得拆开看：

| 步骤 | 代码 | 含义 |
|------|------|------|
| ① | `Dest->PreAttributeChange(*this, NewValue)` | 给你最后一次机会修改 `NewValue`。在这里 Clamp：`NewValue = FMath::Clamp(NewValue, 0, MaxHealth.GetCurrentValue())` |
| ② | `DataPtr->SetCurrentValue(NewValue)` | 真正写入 CurrentValue |
| ③ | `Dest->PostAttributeChange(...)` | 值已生效。在这里通知 UI、触发派生属性重算 |
| ④ | `MARK_PROPERTY_DIRTY(Dest, StructProperty)` | 标记该属性为网络脏，下一帧将复制给客户端 |

四点值得单独拎出来说：

- `NewValue` 是 `float&` 引用，你在 `PreAttributeChange` 中改了它，最终写进去的就是改后的值
- `OldValue` 和 `NewValue` 分别记录修改前后的值，用于 `PostAttributeChange` 中判断变化幅度
- `MARK_PROPERTY_DIRTY` 是 PushModel 的一部分，配合网络复制系统，只同步真正变化的属性
- 写入的是 `SetCurrentValue`，不是 `SetBaseValue`。`SetNumericValueChecked` 管的是 CurrentValue（聚合后的最终值），BaseValue 由 Aggregator 管理

### 3.3 回调链实战：六个钩子的正确用法

以下是一次 GE 执行修改属性值的完整回调时序：

```
GE 执行（决定修改 Health）
 │
 ├─ ① PreGameplayEffectExecute(Data)
 │    Data.EvaluatedData.Magnitude = 修改后的 Magnitude
 │    返回 false 可以拒绝整个修改（免疫）
 │
 ├─ Aggregator.SetBaseValue(NewBaseValue)
 │    │
 │    ├─ ② PreAttributeBaseChange(Attribute, NewBaseValue)
 │    │    Clamp BaseValue（如不允许 BaseValue < 0）
 │    │
 │    └─ BaseValue = NewBaseValue（写入）
 │         │
 │         └─ ③ PostAttributeBaseChange(Attribute, OldValue, NewValue)
 │              BaseValue 已生效；重新计算派生属性
 │
 ├─ Aggregator.EvaluateWithBase（BaseValue + 所有 Modifier = CurrentValue）
 │
 └─ SetNumericAttribute_Internal → SetNumericValueChecked
      │
      ├─ ④ PreAttributeChange(Attribute, CurrentValue)
      │    经典：NewValue = FMath::Clamp(NewValue, 0, MaxHealth)
      │
      ├─ CurrentValue = NewValue（写入）
      │
      └─ ⑤ PostAttributeChange(Attribute, OldCurrentValue, NewCurrentValue)
           │  当前值已生效；刷新 UI、触发效果
           │
           └─ ⑥ PostGameplayEffectExecute(Data)
                GE 执行完毕；伤害结算、死亡判定
```

**实战代码——在自定义 AttributeSet 中覆写回调：**

```
void UMyAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    // Clamp 是 PreAttributeChange 的唯一职责
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
    }
    if (Attribute == GetManaAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
    }
}

void UMyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    // 副作用逻辑——在这个钩子里做伤害结算、死亡判定
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        // 扣血后检查死亡
        if (GetHealth() <= 0.f)
        {
            GetOwningAbilitySystemComponent()->HandleGameplayEvent(
                FGameplayTag::RequestGameplayTag("Event.Death"));
        }
    }
}
```

### 3.4 网络复制：DOREPLIFETIME + OnRep 的完整流程

属性网络复制的核心思路：**服务器只复制 BaseValue，客户端在自己的 Aggregator 之上重新聚合得到 CurrentValue。**

![属性网络复制时序](diagrams/Attr_Replication.png)

#### Step 1：声明复制

在 AttributeSet 子类中声明（`AttributeSet.h` 中宏展开后效果）：

```
// 头文件中声明
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes")
FGameplayAttributeData Health;

ATTRIBUTE_ACCESSORS_BASIC(UMyAttributeSet, Health);

// OnRep 回调声明
UFUNCTION()
void OnRep_Health(const FGameplayAttributeData& OldHealth);
```

#### Step 2：注册复制

在 `.cpp` 的 `GetLifetimeReplicatedProps` 中：

```
void UMyAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UMyAttributeSet, Health, COND_None, REPNOTIFY_Always);
}
```

关键的 `REPNOTIFY_Always`：**即使新旧值相同也触发 OnRep**。这对 GAS 至关重要——属性的 BaseValue 可能没变，但服务器上的一个 Modifier 过期了，CurrentValue 变了。不触发 OnRep 客户端就不会重新聚合，CurrentValue 就会是旧的。

#### Step 3：客户端接收——GAMEPLAYATTRIBUTE_REPNOTIFY

```
void UMyAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UMyAttributeSet, Health, OldHealth);
}
```

宏展开后调用 `SetBaseAttributeValueFromReplication`，做了两件事：
1. 将服务器同步过来的 **BaseValue** 写入客户端 Aggregator
2. 触发客户端 Aggregator 重新评估——**BaseValue（来自服务器）+ 客户端已有的预测性 Modifier = 正确的 CurrentValue**

这就是为什么客户端预测可以正确工作：客户端本地已经复制了所有预测性 GE 的 Modifier，收到服务器 BaseValue 后重新走一遍聚合公式即可得到正确结果。

#### Step 4：PreNetReceive / PostNetReceive——锁住聚合器

网络复制可能分多个包到达。在收到一半属性时触发中间状态的聚合会产生错误值。引擎通过 `PreNetReceive` 和 `PostNetReceive` 实现批量更新锁（`AttributeSet.cpp`）：

```
void UAttributeSet::PreNetReceive()
{
    // 锁定所有 Aggregator，暂不评估
    FScopedAggregatorOnDirtyBatch::BeginNetReceiveLock();
}

void UAttributeSet::PostNetReceive()
{
    // 解锁，一次性处理所有累积的 dirty 标记
    FScopedAggregatorOnDirtyBatch::EndNetReceiveLock();
}
```

### 3.5 DataTable 初始化

对于属性初始值的配置，引擎提供了 `InitFromMetaDataTable` 方案（`AttributeSet.cpp`）：

```
void UAttributeSet::InitFromMetaDataTable(const UDataTable* DataTable)
{
    for (TFieldIterator<FProperty> It(GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
    {
        FProperty* Property = *It;
        if (FGameplayAttribute::IsSupportedProperty(Property))
        {
            // 按 "属性集名称.属性名" 查找 DataTable 行
            FString RowNameStr = FString::Printf(TEXT("%s.%s"), 
                *Property->GetOwnerVariant().GetName(), *Property->GetName());
            
            FAttributeMetaData* MetaData = DataTable->FindRow<FAttributeMetaData>(...);
            if (MetaData)
            {
                DataPtr->SetBaseValue(MetaData->BaseValue);
                DataPtr->SetCurrentValue(MetaData->BaseValue);
            }
        }
    }
}
```

配合 `FAttributeSetInitter` 和 CurveTable 还可以实现**按等级**初始化属性值（文档注释中有详细示例，格式为 `[GroupName].[AttributeSetName].[Attribute]`，列代表等级）。对于不需要等级缩放的简单项目，也可以直接在 AttributeSet 构造函数中写死默认值。

---

## 四、设计思考：为什么 AttributeSet 是这样设计的

### 4.1 为什么分 BaseValue 和 CurrentValue，而不是一股脑算出最终值？

这是一个「状态 vs 视图」的经典取舍。如果只存一个值，GE 过期时你必须回溯"这个 GE 加了什么、当时的值是多少"——本质上等于把 Modifier 信息重新发明一遍。不如直接分开：**BaseValue 是事实，Modifier 是状态，CurrentValue 是计算出的视图。**

这样的好处：
- GE 过期时不需要回溯：直接 Remove 对应 Modifier，重新聚合
- 多来源不冲突：两个 +10 和 +15 的 GE 同时作用，Aggregator 自然得出 +25
- 预测友好：客户端收到正确的 BaseValue，配合本地已有 Modifier 重算即可

### 4.2 为什么用反射（FProperty），而不是直接 C++ 指针？

一个 GE 要修改任意 AttributeSet 的任意属性。如果用 C++ 指针，你需要：
- 知道目标 AttributeSet 的具体子类
- 通过类型转换拿到成员指针
- 无法序列化、无法网络传递

`FGameplayAttribute` 保存 `FProperty*`，通过 `ContainerPtrToValuePtr` 在运行时动态计算偏移。配合 UE 的反射系统，它可以：
- 被序列化（作为 Tag、GE、技能的资源引用）
- 在蓝图中暴露属性选择器
- 自动处理属性重定向（`PostSerialize` 中有完整的路径重定向逻辑）

**性能代价**：多一次反射查询（O(1) 的指针解引用 + 偏移计算），相比直接 float 访问有微小开销。但 GAS 的属性修改频率远低于渲染/物理系统的每帧操作，这个代价完全可接受。

### 4.3 对比传统 MMO 属性系统

传统方案（自研引擎或早期 Unity）通常是一个 `Dictionary<string, float>` 或者把所有属性定义在一个大 struct 里：

| 方案 | 类型安全 | 性能 | 可配置性 | 网络复制 |
|------|---------|------|---------|---------|
| `Dictionary<string, float>` | 差（字符串 key 无编译期检查） | 差（字符串哈希） | 好（运行时任意添加） | 需自建 |
| 大 struct | 好 | 好 | 差（改属性要改代码） | 需自建 |
| **GAS AttributeSet** | **好**（宏生成类型化 getter） | **好**（反射后等同直接访问） | **好**（DataTable 配置） | **内置**（DOREPLIFETIME + 预测兼容） |

GAS 的方案在三个维度上都拿到了优势，代价是理解门槛——你需要同时理解 UE 的反射系统、UObject 复制和 FAggregator 的评估模型。

### 4.4 与其他引擎的对比

**Unity（无内置 GAS 类似系统）**：通常用 ScriptableObject + Odin Inspector 手动搭建属性系统。基础值和修正值分离、回调链、网络同步都需要从零实现。社区方案（如 Unity Atoms）提供了类似的基础设施，但远不如 GAS 完整。

**Godot**：Godot 的 `Resource` 系统可以实现类似 AttributeSet 的数据容器，`StringName` 提供了高效的字符串查找。但没有内置的 Aggregator、网络复制标记系统和回调链。要实现 GAS 级别的属性管理，需要额外搭建 Modifier 堆栈和网络同步机制。

---

## 五、总结回顾

AttributeSet 不是一个数据存储的"哑巴盒子"。它是 GAS 中**数据的定义层** + **变更的拦截层** + **网络的同步层**。

本文覆盖了三条脉络：

1. **定义**：`FGameplayAttributeData`（BaseValue + CurrentValue）+ `ATTRIBUTE_ACCESSORS_BASIC` 宏，一键生成 getter/setter/init/FGameplayAttribute 访问器
2. **回调链**：六个虚函数分三组——GE 级（Pre/PostGameplayEffectExecute）、BaseValue 级（Pre/PostAttributeBaseChange）、CurrentValue 级（Pre/PostAttributeChange），各自有明确的职责边界
3. **网络复制**：`DOREPLIFETIME_CONDITION_NOTIFY` + `GAMEPLAYATTRIBUTE_REPNOTIFY` → `SetBaseAttributeValueFromReplication` → 客户端重聚合。服务器只复制 BaseValue，客户端用自己的 Aggregator 算出 CurrentValue，天然兼容预测

AttributeSet 让属性变得可测量、可追踪、可拦截。下一篇文章《GameplayEffect》将深入 GE 如何利用这些能力来修改属性值。

---

> **系列导航**
> 
> | 阶段 | 篇章 | 内容 | 状态 |
> |------|------|------|------|
> | 🟢 基础 | 01 | GAS 总览与核心架构 | ✅ |
> | | 02 | ASC — 核心调度器 | ✅ |
> | | 03 | GameplayTags — 通用语言 | ✅ |
> | | **04** | **AttributeSet — 属性定义与复制** | ✅ |
> | 🔵 核心 | 05-06 | GameplayEffect — 效果与计算 (上/下) | 📝 |
> | | 07-08 | GameplayAbility — 技能激活与任务 (上/下) | 📝 |
> | | 09 | GameplayCue — 表现层触发机制 | 📝 |
> | 🔴 高级 | 10 | Prediction — 预测与回滚 | 📝 |
> | | 11 | GE Components — 组件化架构演进 | 📝 |
> | | 12 | Network & Serial — 网络序列化 | 📝 |
> | | 13 | Targeting — 瞄准系统 | 📝 |
> | | 14 | Debug & Optimization — 调试与优化 | 📝 |
> | | 15 | 终篇回顾 — 全景复习 | 📝 |
