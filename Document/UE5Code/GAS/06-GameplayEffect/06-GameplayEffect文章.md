# 06 GameplayEffect — 效果与计算 (下)：执行流程与计算

---

**上篇** 讲了 GE 的数据结构：Duration Policy、Modifier 体系、Tags、Stacking、GameplayEffectComponent、Spec、Context。

**本篇** 进入运行时：从 `ApplyGameplayEffectSpecToSelf` 开始，追踪 Instant GE 直写属性 vs Duration GE 注册 Aggregator 两条路径的完整调用链，以及 ExecutionCalculation 自定义计算、Period 周期执行、网络同步策略、GameplayCue 触发时机。

---

## 6.1 问题驱动：施加一个 GE 到底发生了什么？

你可能以为 `ApplyGameplayEffectSpecToSelf` 就是"算一下数值然后写进去"，但实际上这背后有十几步检查、两条完全不同的执行路径、多个回调触发点、以及网络预测的分支处理。

```cpp
// 用户视角：一行调用
ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);

// 实际发生的：
// 1. 网络权限检查
// 2. Application Query 检查
// 3. CanApply() 检查 (TargetTagRequirements 等)
// 4. Attribute 有效性检查
// 5. [分叉] Instant → ExecuteGameplayEffect; Duration → ApplyGameplayEffectSpec
// 6. OnApplied 事件
// 7. OnGameplayEffectAppliedToSelf / OnGameplayEffectAppliedToTarget 回调
// 8. GameplayCue 触发
```

本章我们逐行比对这些步骤的源码实现。

---

## 6.2 完整调用链全景图

```
ApplyGameplayEffectSpecToSelf(Spec, PredictionKey)
    │
    ├─ ① Guard Checks
    │   ├─ Spec.Def == nullptr? → return
    │   ├─ HasNetworkAuthorityToApplyGameplayEffect? → return
    │   ├─ PredictionKey 安全校验 (Period 不允许预测)
    │   ├─ GameplayEffectApplicationQueries 检查
    │   ├─ Spec.Def->CanApply() → TagRequirements + GE Components 检查
    │   └─ Modifiers[i].Attribute.IsValid()
    │
    ├─ ② 路径分叉
    │   │
    │   ├─ [Duration / Infinite]
    │   │   └─ ActiveGameplayEffects.ApplyGameplayEffectSpec()
    │   │       ├─ FindStackableActiveGameplayEffect() → 叠加 or 新建
    │   │       ├─ 注册到 ActiveGameplayEffects 列表
    │   │       └─ Aggregator::SetBaseValue(NewBase)
    │   │           └─ Dirty → InternalUpdateNumericalAttribute → SetNumericValueChecked
    │   │               └─ Broadcast OnAttributeChange
    │   │
    │   └─ [Instant]
    │       └─ ExecuteGameplayEffect(*OurCopyOfSpec, PredictionKey)
    │           └─ ActiveGameplayEffects.ExecuteActiveEffectsFrom()
    │               │
    │               ├─ ③ Modifiers 执行 (for each ModIdx)
    │               │   ├─ SourceTags / TargetTags 检查
    │               │   ├─ GetModifierMagnitude(ModIdx)
    │               │   └─ InternalExecuteMod()
    │               │       ├─ PreGameplayEffectExecute()   ← Virtual, 可阻止
    │               │       ├─ ApplyModToAttribute()
    │               │       │   └─ StaticExecModOnBaseValue → SetAttributeBaseValue
    │               │       │       └─ Aggregator::SetBaseValue → Dirty → ...
    │               │       └─ PostGameplayEffectExecute()   ← Virtual
    │               │
    │               ├─ ④ Executions 执行 (for each ExecDef)
    │               │   └─ ExecCDO->Execute(ExecutionParams, ExecutionOutput)
    │               │       └─ for each OutputModifier:
    │               │           └─ InternalExecuteMod()  (同 ③)
    │               │
    │               └─ ⑤ GameplayCue Execute
    │
    ├─ ⑥ OnApplied (GE Component system)
    │
    ├─ ⑦ Callbacks
    │   ├─ OnGameplayEffectAppliedToSelf(InstigatorASC, *Spec, Handle)
    │   └─ OnGameplayEffectAppliedToTarget(this, *Spec, Handle)
    │
    └─ ⑧ Return Handle (InstantExecutedHandle / ActiveGE Handle)
```

---

## 6.3 源码追踪：逐阶段分析

### 6.3.1 Guard Checks：能不能施加？

```cpp
// AbilitySystemComponent.cpp ~1009-1062
FActiveGameplayEffectHandle UAbilitySystemComponent::ApplyGameplayEffectSpecToSelf(
    const FGameplayEffectSpec &Spec, FPredictionKey PredictionKey)
{
    // 1. Null Def 检查
    if (Spec.Def == nullptr) return FActiveGameplayEffectHandle();

    // 2. 网络权限
    if (!HasNetworkAuthorityToApplyGameplayEffect(PredictionKey))
        return FActiveGameplayEffectHandle();

    // 3. Period 不允许预测（客户端）
    if (PredictionKey.IsValidKey() && Spec.GetPeriod() > 0.f)
    {
        if (IsOwnerActorAuthoritative())
            PredictionKey = FPredictionKey();  // Server 继续但不预测
        else
            return FActiveGameplayEffectHandle();  // Client 直接拒绝
    }

    // 4. Application Queries（自定义查询拦截）
    for (const FGameplayEffectApplicationQuery& ApplicationQuery :
         GameplayEffectApplicationQueries)
    {
        if (!ApplicationQuery.Execute(ActiveGameplayEffects, Spec))
            return FActiveGameplayEffectHandle();
    }

    // 5. CanApply — TargetTagRequirements 等
    if (!Spec.Def->CanApply(ActiveGameplayEffects, Spec))
        return FActiveGameplayEffectHandle();

    // 6. Attribute 有效性
    for (const FGameplayModifierInfo& Mod : Spec.Def->Modifiers)
    {
        if (!Mod.Attribute.IsValid())
            return FActiveGameplayEffectHandle();
    }
    // ...
}
```

**关键细节**：

- **Period 预测禁止**：预测的周期性效果会导致客户端累积过多重复执行，UE 直接禁止。
- **CanApply 是 GE Components 的入口**：`TargetTagRequirementsGameplayEffectComponent` 等组件在此被调用。
- **Application Queries** 是 GAS 框架暴露的扩展点，可以让外部逻辑控制 GE 是否可施加。

### 6.3.2 路径分叉：Instant vs Duration

```cpp
// AbilitySystemComponent.cpp ~1078-1161
if (Spec.Def->DurationPolicy != EGameplayEffectDurationType::Instant
    || bTreatAsInfiniteDuration)
{
    // Duration / Infinite 路径
    AppliedEffect = ActiveGameplayEffects.ApplyGameplayEffectSpec(
        Spec, PredictionKey, bFoundExistingStackableGE);
    
    MyHandle = AppliedEffect->Handle;
    OurCopyOfSpec = &(AppliedEffect->Spec);
    // Spec 已经被 ApplyGameplayEffectSpec 内部复制到 AppliedEffect 中
}
else
{
    // Instant 路径 — 我们自己持有 Spec 副本
    StackSpec = MakeUnique<FGameplayEffectSpec>(Spec);
    OurCopyOfSpec = StackSpec.Get();
    
    GlobalPreGameplayEffectSpecApply(*OurCopyOfSpec, this);
    OurCopyOfSpec->CaptureAttributeDataFromTarget(this);
}

// Instant: 执行一次就完事
if (Spec.Def->DurationPolicy == EGameplayEffectDurationType::Instant)
{
    ExecuteGameplayEffect(*OurCopyOfSpec, PredictionKey);
}
```

**两条路径的根本差异**：

| | Instant | Duration / Infinite |
|---|---|---|
| 是否有 ActiveGE | 无 | 有，注册到 ActiveGameplayEffects |
| 属性修改方式 | 直接 SetAttributeBaseValue | 通过 Aggregator 管理 |
| 是否能被移除 | 不能 | 能 (RemoveActiveGameplayEffect) |
| 是否有 Stack | 不能 | 能 |
| 是否有 Period | 不能 | 能 |
| 预测处理 | 客户端伪造成 Infinite 持续 | 正常同步 |

#### Instant 的预测特殊处理

```cpp
// 客户端：Instant GE 被当作 Infinite Duration 处理
bool bTreatAsInfiniteDuration = 
    GetOwnerRole() != ROLE_Authority 
    && PredictionKey.IsLocalClientKey() 
    && Spec.Def->DurationPolicy == EGameplayEffectDurationType::Instant;
```

客户端预测的 Instant GE 会被转换为"Infinite Duration"形式临时存储，等 Server 确认后清理。这意味着客户端上这个"假 Infinite"**不会**被 Period 定期执行，因为它没有 Period。

---

## 6.4 ExecuteActiveEffectsFrom：执行核心

```cpp
// GameplayEffect.cpp ~3210
void FActiveGameplayEffectsContainer::ExecuteActiveEffectsFrom(
    FGameplayEffectSpec &Spec, FPredictionKey PredictionKey)
{
    // Step 0: 捕获目标 Tags
    Spec.CapturedTargetTags.GetActorTags().Reset();
    Owner->GetOwnedGameplayTags(Spec.CapturedTargetTags.GetActorTags());
    
    Spec.CalculateModifierMagnitudes();

    // ======= Step 1: Modifiers 执行 =======
    for (int32 ModIdx = 0; ModIdx < Spec.Modifiers.Num(); ++ModIdx)
    {
        const FGameplayModifierInfo& ModDef = Spec.Def->Modifiers[ModIdx];
        
        // Tag 条件检查
        if (!ModDef.SourceTags.IsEmpty() 
            && !ModDef.SourceTags.RequirementsMet(Spec.CapturedSourceTags.GetActorTags()))
            continue;
        if (!ModDef.TargetTags.IsEmpty() 
            && !ModDef.TargetTags.RequirementsMet(Spec.CapturedTargetTags.GetActorTags()))
            continue;

        FGameplayModifierEvaluatedData EvalData(
            ModDef.Attribute, ModDef.ModifierOp, 
            Spec.GetModifierMagnitude(ModIdx));
        
        InternalExecuteMod(Spec, EvalData);
    }

    // ======= Step 2: Executions 执行 =======
    for (const FGameplayEffectExecutionDefinition& CurExecDef : Spec.Def->Executions)
    {
        if (CurExecDef.CalculationClass)
        {
            const UGameplayEffectExecutionCalculation* ExecCDO = 
                CurExecDef.CalculationClass->GetDefaultObject<UGameplayEffectExecutionCalculation>();
            
            FGameplayEffectCustomExecutionParameters ExecutionParams(
                Spec, CurExecDef.CalculationModifiers, Owner, 
                CurExecDef.PassedInTags, PredictionKey);
            FGameplayEffectCustomExecutionOutput ExecutionOutput;
            
            ExecCDO->Execute(ExecutionParams, ExecutionOutput);

            // 将 Execution 的输出 Modifiers 逐个执行
            for (FGameplayModifierEvaluatedData& CurExecMod : 
                 ExecutionOutput.GetOutputModifiersRef())
            {
                InternalExecuteMod(Spec, CurExecMod);
            }
        }
    }

    // ======= Step 3: GameplayCue =======
    // ...
}
```

**执行顺序非常明确**：先执行所有 Modifiers，再执行 Executions。如果你在 Execution 中捕获了被 Modifiers 修改后的属性值，那么捕获到的是**已经修改过的值** — 因为 Modifiers 在 Executions 之前执行。

### 6.4.1 InternalExecuteMod：单个 Modifier 执行

```cpp
// GameplayEffect.cpp ~4090
bool FActiveGameplayEffectsContainer::InternalExecuteMod(
    FGameplayEffectSpec& Spec, FGameplayModifierEvaluatedData& ModEvalData)
{
    UAttributeSet* AttributeSet = Owner->GetAttributeSubobject(
        ModEvalData.Attribute.GetAttributeSetClass());
    
    if (AttributeSet)
    {
        FGameplayEffectModCallbackData ExecuteData(Spec, ModEvalData, *Owner);

        // ① PreGameplayEffectExecute — Virtual，可返回 false 阻止此 Modifier
        if (AttributeSet->PreGameplayEffectExecute(ExecuteData))
        {
            // ② 直接写 BaseValue
            ApplyModToAttribute(
                ModEvalData.Attribute, 
                ModEvalData.ModifierOp, 
                ModEvalData.Magnitude, 
                &ExecuteData);
            
            // ③ PostGameplayEffectExecute — 跨属性联动
            AttributeSet->PostGameplayEffectExecute(ExecuteData);
        }
    }
    return bExecuted;
}
```

**Pre / Post 回调用途**：

```cpp
// 典型用法 — 在 AttributeSet 子类中重写
void UMyAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData &Data)
{
    // Clamp 逻辑：伤害不能低于 0
    if (Data.EvaluatedData.Attribute == GetDamageAttribute())
    {
        if (Data.EvaluatedData.Magnitude < 0) Data.EvaluatedData.Magnitude = 0;
    }
}

void UMyAttributeSet::PostGameplayEffectExecute(FGameplayEffectModCallbackData &Data)
{
    // 跨属性联动：Health 变化后更新 UI
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
    }
    // 同步修改计算属性
    if (Data.EvaluatedData.Attribute == GetAttackBaseAttribute())
    {
        UpdateAttackPower();
    }
}
```

### 6.4.2 ApplyModToAttribute：直写 BaseValue

```cpp
// GameplayEffect.cpp ~4155
void FActiveGameplayEffectsContainer::ApplyModToAttribute(
    const FGameplayAttribute &Attribute, 
    EGameplayModOp::Type ModifierOp, 
    float ModifierMagnitude, 
    const FGameplayEffectModCallbackData* ModData)
{
    CurrentModcallbackData = ModData;
    
    float CurrentBase = GetAttributeBaseValue(Attribute);
    float NewBase = FAggregator::StaticExecModOnBaseValue(CurrentBase, ModifierOp, ModifierMagnitude);
    
    SetAttributeBaseValue(Attribute, NewBase);  // → 触发 Dirty 链
}
```

`StaticExecModOnBaseValue` 就是执行 ModOp 运算：

```cpp
float FAggregator::StaticExecModOnBaseValue(float BaseValue, EGameplayModOp::Type ModifierOp, float Magnitude)
{
    switch (ModifierOp)
    {
    case EGameplayModOp::Additive:        return BaseValue + Magnitude;
    case EGameplayModOp::Multiplicitive:  return BaseValue * Magnitude;
    case EGameplayModOp::Division:        return BaseValue / Magnitude;
    case EGameplayModOp::Override:        return Magnitude;
    }
    return BaseValue;
}
```

---

## 6.5 Duration GE 的 Aggregator 路径

Instant GE 通过 `ExecuteActiveEffectsFrom` 直写属性。Duration GE 则走了完全不同的路径：

### 6.5.1 ApplyGameplayEffectSpec：注册到容器

```cpp
// GameplayEffect.cpp ~4171
FActiveGameplayEffect* FActiveGameplayEffectsContainer::ApplyGameplayEffectSpec(
    const FGameplayEffectSpec& Spec, FPredictionKey& InPredictionKey, 
    bool& bFoundExistingStackableGE)
{
    // 1. 查找是否有可叠加的现有 GE
    FActiveGameplayEffect* ExistingStackableGE = FindStackableActiveGameplayEffect(Spec);

    // 2. 不存在则创建新的 FActiveGameplayEffect
    FActiveGameplayEffect& NewEffect = GameplayEffects_Internal[GameplayEffects_Internal.Emplace()];
    NewEffect.Spec = Spec;           // 复制 Spec
    NewEffect.StartServerWorldTime = GetWorld()->GetTimeSeconds();
    NewEffect.StartWorldTime = ...;
    
    // 3. 为每个 Modifier 创建 Aggregator Mod
    for (int32 ModIdx = 0; ModIdx < Spec.Def->Modifiers.Num(); ++ModIdx)
    {
        FAggregatorRef& AggregatorRef = FindOrCreateAttributeAggregator(ModDef.Attribute);
        
        FAggregatorMod* Mod = new FAggregatorMod();
        Mod->EvaluatedData = ...;           // ModOp, Magnitude
        Mod->SourceTagReqs = ModDef.SourceTags;
        Mod->TargetTagReqs = ModDef.TargetTags;
        Mod->ActiveHandle = NewEffect.Handle;
        
        AggregatorRef.Get()->AddMod(Mod);   // 注册到 Aggregator
    }
    
    // 4. 设置定时器（Duration Expiry + Period）
    if (NewEffect.GetDuration() > 0)
    {
        // SetTimer 在 Duration 秒后调用 CheckDurationExpired
    }
    if (NewEffect.GetPeriod() > 0)
    {
        // SetTimer 每 Period 秒调用 ExecutePeriodicGameplayEffect
    }
    
    return &NewEffect;
}
```

### 6.5.2 Aggregator 的工作方式

```
BaseValue  --------+
                    |
                    v
[GE1 Mod: +20] → Aggregator → EvaluateWithBase → CurrentValue
[GE2 Mod: *1.5] →            ↑
[GE3 Mod: +10]  →            |
                              When Dirty:
                              InternalUpdateNumericalAttribute
                              → SetNumericValueChecked
                              → Broadcast Delegates
```

Duration GE 不直接写 `BaseValue`，而是把 Mod 注册到 Aggregator：

```cpp
// GameplayEffectAggregator.cpp
float FAggregator::EvaluateWithBase(float InlineBaseValue, 
    const FAggregatorEvaluateParameters& Parameters)
{
    float Result = InlineBaseValue;
    
    for (FAggregatorMod* Mod : ModList)
    {
        // 检查 Tag Requirements 是否满足
        if (!Mod->Qualifies(Parameters)) continue;
        
        Result = FAggregator::StaticExecModOnBaseValue(Result, Mod->ModifierOp, Mod->Magnitude);
    }
    return Result;
}
```

BaseValue 不变，但 CurrentValue 通过 `EvaluateWithBase` 动态计算。

当 BaseValue 改变（或 Mod 增删）时，Aggregator 变 Dirt → 触发 `InternalUpdateNumericalAttribute` → 写 CurrentValue → 触发属性变化回调。这个链路在[第 04 篇]中已详细分析。

---

## 6.6 Execution Calculation：自定义计算

当普通的 Modifier 计算不够用时，使用 `UGameplayEffectExecutionCalculation`。

### 6.6.1 与 Modifier 的区别

| | Modifier | Execution Calculation |
|---|---|---|
| 输入 | 仅 Magnitude | 可捕获任意属性 |
| 输出 | 单个 ModOp + Magnitude | 多个独立 Modifier |
| 逻辑 | 纯数值 | 任意 C++ 逻辑 |
| 网络 | 自动同步 | 只在 Server 执行 |
| 典型场景 | 固定值伤害、Buff | 复杂伤害公式、属性联动 |

### 6.6.2 编写 ExecutionCalculation

```cpp
UCLASS()
class UMyDamageExecution : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()
    
public:
    UMyDamageExecution()
    {
        // 声明需要捕获的属性
        FGameplayEffectAttributeCaptureDefinition AttackDef;
        AttackDef.AttributeToCapture = FGameplayAttribute::FindAttribute("AttackPower");
        AttackDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
        RelevantAttributesToCapture.Add(AttackDef);
        
        FGameplayEffectAttributeCaptureDefinition DefenseDef;
        DefenseDef.AttributeToCapture = FGameplayAttribute::FindAttribute("Defense");
        DefenseDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
        RelevantAttributesToCapture.Add(DefenseDef);
    }

    virtual void Execute_Implementation(
        const FGameplayEffectCustomExecutionParameters& ExecutionParams,
        FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override
    {
        // 捕获属性值
        float Attack = 0.f;
        ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
            RelevantAttributesToCapture[0], FAggregatorEvaluateParameters(), Attack);
        
        float Defense = 0.f;
        ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
            RelevantAttributesToCapture[1], FAggregatorEvaluateParameters(), Defense);
        
        // 自定义计算
        float Damage = FMath::Max(0.f, Attack - Defense * 0.5f);
        
        // 输出到目标属性
        OutExecutionOutput.AddOutputModifier(
            FGameplayModifierEvaluatedData(
                FGameplayAttribute::FindAttribute("Health"),
                EGameplayModOp::Additive, -Damage));
    }
};
```

### 6.6.3 Capture 的时机与 Source/Target

`RelevantAttributesToCapture` 在构函数中声明。框架在 GE 执行前自动 Snapshot 这些属性值（取决于 `SnapshotPolicy`）：

- **Snapshot**：在 GE Spec 创建时捕获 → 后续使用冻结值
- **Not Snapshot**：在每次 evaluation 时捕获 → 动态计算

`AttributeSource` 决定从谁身上取：

| Source | 含义 |
|--------|------|
| Source | 施法者 |
| Target | 目标 |

---

## 6.7 Period 周期执行

Periodic GE 的工作方式是将 `ExecuteActiveEffectsFrom` 定时重复执行。

### 6.7.1 注册 Period Timer

```cpp
// GameplayEffect.cpp, ApplyGameplayEffectSpec 内
if (ActiveEffect.Spec.GetPeriod() > UGameplayEffect::NO_PERIOD)
{
    FTimerManager& TimerManager = World->GetTimerManager();
    FTimerDelegate Delegate = FTimerDelegate::CreateUObject(Owner, 
        &UAbilitySystemComponent::ExecutePeriodicGameplayEffect, Handle);
    
    TimerManager.SetTimer(Delegate, Period, bLoop);
}
```

### 6.7.2 周期执行函数

```cpp
// GameplayEffect.cpp ~4762
void FActiveGameplayEffectsContainer::InternalExecutePeriodicGameplayEffect(
    FActiveGameplayEffect& ActiveEffect)
{
    if (!ActiveEffect.bIsInhibited)
    {
        // 清空上次周期的修改记录
        ActiveEffect.Spec.ModifiedAttributes.Empty();
        
        // 再次执行 ExecuteActiveEffectsFrom — 和 Instant 一样的逻辑！
        ExecuteActiveEffectsFrom(ActiveEffect.Spec);
        
        // 触发 Period 专用回调
        Owner->OnPeriodicGameplayEffectExecuteOnSelf(SourceASC, ActiveEffect.Spec, Handle);
        if (SourceASC)
            SourceASC->OnPeriodicGameplayEffectExecuteOnTarget(Owner, ActiveEffect.Spec, Handle);
    }
}
```

**关键点**：
- Period 执行和 Instant 执行走完全相同的 `ExecuteActiveEffectsFrom` 路径
- 每次执行前清空 `ModifiedAttributes`，只保留最近一次周期的修改记录
- `bIsInhibited = true` 时（OngoingTagRequirements 不满足），周期执行跳过

### 6.7.3 Period 与 Stacking 的组合

如果 GE 有 `Stacking` 且 `Period` 存在：

1. Period 执行时读取 `Spec.GetStackCount()` — 层数
2. ExecutionCalculation 中可以通过 `ExecutionParams.GetOwningSpec().GetStackCount()` 获取当前层数
3. 层数变化时不需要重新创建 Timer — 同一 FActiveGameplayEffect 的层数增加时，Damage 增大即可

---

## 6.8 网络同步策略

### 6.8.1 同步什么

| 同步内容 | 机制 |
|----------|------|
| BaseValue | `DOREPLIFETIME` UPROPERTY 自动同步 |
| FActiveGameplayEffect | Server 仅同步 **Handle + Spec Level + Source** |
| CurrentValue | **不同步** — 客户端用本地 Aggregator 计算 |
| GE Tags (GrantedTags) | 通过 GameplayTag 系统同步 |

### 6.8.2 客户端收到 BaseValue 后

```cpp
// 客户端 OnRep → SetBaseAttributeValueFromReplication
void UAbilitySystemComponent::SetBaseAttributeValueFromReplication(
    FGameplayAttribute Attribute, float NewValue)
{
    // 与 Server 的 SetAttributeBaseValue 一样
    // Aggregator Dirty → InternalUpdateNumericalAttribute → SetNumericValueChecked
}
```

客户端**不存**所有 GE 的完整 Mod 数据，只存 BaseValue。`EvaluateWithBase` 时客户端本地 Aggregator 算出 CurrentValue。这意味着**客户端无法区分 CurrentValue 是由哪些 GE 贡献的**，但这对游戏逻辑来说是够用的（客户端只需要最终值）。

### 6.8.3 预测的特殊处理

```cpp
// 客户端预测 Instant GE：
// 1. 客户端创建一个 Fake Infinite Duration GE
// 2. 执行效果（本地预测，立即看到结果）
// 3. 发送 RPC 给 Server，Server 真正执行
// 4. Server 回复 → 客户端清理 Fake GE ← 关键：如果 Server 拒绝了怎么办？

// 如果 Server 拒绝，通过 RejectedPredictionKey 回滚：
void UAbilitySystemComponent::RejectedGameplayEffect(FActiveGameplayEffectHandle Handle)
{
    // 从客户端列表移除
    ActiveGameplayEffects.RemoveActiveGameplayEffect(Handle);
    // Aggregator 自动重新计算
}
```

---

## 6.9 GameplayCue 触发时机

GameplayCue 是 GAS 的可视化反馈系统（将在后续文章详述），这里只讲它在 GE 中的触发时机。

### 6.9.1 触发方式

| GE 类型 | GC 类型 | 触发时机 |
|---------|---------|----------|
| Instant | `Execute` | `ExecuteActiveEffectsFrom` 执行完后立即触发 |
| Duration | `OnActive` + `WhileActive` | 施加时触发；持续期间可以持续触发 |
| Duration 移除 | `Removed` | GE 移除时触发 |

### 6.9.2 源码中的触发逻辑

```cpp
// Instant GE -> Execute GameplayCue
if (Spec.Def->DurationPolicy == EGameplayEffectDurationType::Instant)
{
    // ExecuteActiveEffectsFrom 内：
    if (InvokeGameplayCueExecute)
    {
        GameplayCueManager->InvokeGameplayCueExecuted_FromSpec(...);
    }
}

// Duration GE -> Added + WhileActive
if (bInvokeGameplayCueApplied)
{
    GameplayCueManager->InvokeGameplayCueAddedAndWhileActive_FromSpec(...);
}
```

### 6.9.3 抑制规则

- `bRequireModifierSuccessToTriggerCues = true`：必须有 Modifier 成功执行才触发
- `bSuppressStackingCues`：叠加时不重复触发
- `GameplayCuesWereManuallyHandled = true`：ExecutionCalculation 手动处理 GC

---

## 6.10 GE 的移除

### Duration 过期自动移除

```cpp
void UAbilitySystemComponent::CheckDurationExpired(FActiveGameplayEffectHandle Handle)
{
    ActiveGameplayEffects.CheckDuration(Handle);
    // → InternalRemoveActiveGameplayEffect
    //   → Aggregator 清理 Mod
    //   → InternalUpdateNumericalAttribute (CurrentValue 重算)
    //   → OnGameplayEffectRemoved 回调
    //   → GameplayCue Removed
}
```

### 手动移除

```cpp
ASC->RemoveActiveGameplayEffect(Handle, StacksToRemove);  // 按层移除
ASC->RemoveActiveGameplayEffectBySourceEffect(GE_Class);     // 按 GE 类型移除
```

---

## 6.11 设计思考

### 为什么 Instant 不用 Aggregator？

Instant GE = 一次性计算 = 不需要撤销。伤害劈下来那一刻，Health 直接减少，不需要记住是谁造成的。Duration Buff 的 AttackPower +20 则需要在 Buff 过期时撤销效果，这就是 Aggregator 的价值。

### ExecutionCalculation 在 Server 执行

这意味着：
1. 客户端无法运行自定义伤害公式，伤害公式对客户端不透明
2. 客户端的预测 GE 必须用简单 Modifier，不能用 ExecutionCalculation（或用简化版本）
3. 需要 ExecutionCalculation 的 GE，客户端只能等 Server 回复

### Modifier 先于 Execution

如果你同时使用了 Modifiers 和 Executions：
- Modifiers 先执行，修改 BaseValue
- Execution 随后执行，如果它捕获了被 Modifier 修改的属性，则捕获到的是修改后的值

这是一个容易踩的坑：别让 Modifier 和 Execution 同时操作同一个属性，除非你明确知道这个执行顺序的语义。

### Period 的"重复 Instant"本质

Period GE 不是"每 N 秒维持一个效果"，它是**每 N 秒执行一次 Instant**。每次执行都走 `ExecuteActiveEffectsFrom`，每次都会触发 `PreGameplayEffectExecute / PostGameplayEffectExecute`。
- 每秒 100 点伤害 = 每秒执行一次 -100 Health 的 Additive Mod
- 不是"维持 -100/s 的 Aggregator Mod"

---

## 6.12 总结

| 流程节点 | 职责 |
|----------|------|
| Guard Checks | 网络权限、Tag 条件、Attribute 有效性 |
| 路径分叉 | Instant → Execute; Duration → Register + Aggregator |
| ExecuteActiveEffectsFrom | 执行 Modifiers + Executions + GameplayCue |
| InternalExecuteMod | Pre → ApplyModToAttribute → Post |
| ApplyModToAttribute | StaticExecModOnBaseValue → SetAttributeBaseValue |
| Aggregator (Duration) | 管理多个 Mod，动态计算 CurrentValue |
| ExecutionCalculation | 自定义 C++ 计算，捕获任意属性，产生任意输出 |
| Period | Timer → 重复 ExecuteActiveEffectsFrom |
| 网络同步 | Server 同步 BaseValue，客户端本地 Aggregator 计算 CurrentValue |

### 两条路径对比速查表

| | Instant | Duration / Infinite |
|---|---|---|
| ActiveGE | 无 | 有 |
| 属性修改 | 直接 SetBaseValue | Aggregator AddMod |
| 可撤销 | 不可 | 可 |
| Stack | 无 | 有 |
| Period | 无 | 有 |
| GameplayCue | Execute | Added + WhileActive + Removed |
| 网络 | Server 执行后不保留 | 保留 FActiveGameplayEffect，同步 Handle |

> **系列导航**
> 
> | 阶段 | 篇章 | 内容 | 状态 |
> |------|------|------|------|
> | 🟢 基础 | 01 | GAS 总览与核心架构 | 📝 |
> | | 02 | ASC — 核心调度器 | 📝 |
> | | 03 | GameplayTags — 通用语言 | 📝 |
> | | 04 | AttributeSet — 属性定义与复制 | 📝 |
> | 🔵 核心 | 05 | GameplayEffect — 效果与计算 (上) | 📝 |
> | | **06** | **GameplayEffect — 效果与计算 (下)** | ✅ |
> | | 07 | GameplayAbility — 技能激活与核心框架 (上) | 📝 |
> | | 08 | GameplayAbility — Task/输入/预测 (下) | 📝 |
> | | 09 | GameplayCue — 表现层触发机制 | 📝 |
> | 🔴 高级 | 10 | Prediction — 预测与回滚 | 📝 |
> | | 11 | GE Components — 组件化架构演进 | 📝 |
> | | 12 | Network & Serial — 网络序列化 | 📝 |
> | | 13 | Targeting — 瞄准系统 | 📝 |
> | | 14 | Debug & Optimization — 调试与优化 | 📝 |
> | | 15 | 终篇回顾 — 全景复习 | 📝 |
