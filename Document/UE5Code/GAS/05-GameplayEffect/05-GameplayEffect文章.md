# 05 GameplayEffect — 效果与计算 (上)：数据结构与配置

---

**上篇讲数据结构与配置**：GE 作为 Data Asset 的设计哲学、Duration Policy、Modifier 体系、Tags 系统、Stacking 机制、GameplayEffectComponent 架构、FGameplayEffectSpec 运行时实例、FGameplayEffectContext 上下文。

**下篇讲执行流程与计算**：`ApplyGameplayEffectSpecToSelf` 完整调用链、Instant GE 直写 vs Duration GE 注册、Modifier 计算链路（Magnitude → ModOp → Aggregator → SetBaseValue）、ExecutionCalculation 自定义计算、Period 周期执行、网络同步策略、GameplayCue 触发时机。

---

## 5.1 问题驱动：为什么需要 GameplayEffect？

如果你写过任何游戏逻辑，大概率做过这样的事：

```cpp
// 简单粗暴 —— 直接在代码里加减
void ApplyDamage(AActor* Target, float Amount) {
    Target->Health -= Amount;
    if (Target->Health <= 0) Target->Die();
}

void ApplyBuff(AActor* Target) {
    Target->Speed *= 1.5f;
    // ... 你还得记得什么时候取消这个 Buff
}
```

这段代码能跑，但问题很明显：

1. **硬编码**：策划想调伤害值？改代码。伤害计算要跟防御属性联动？再改代码。
2. **状态管理混乱**：Buff 怎么叠加？谁移除谁？过期了谁清理？
3. **无法复用**：每类 Buff/Debuff 都要写不同的逻辑，没有统一框架。
4. **网络同步困难**：属性改变没有统一的触发点，不好做网络同步。

**GameplayEffect (GE) 就是 GAS 对这个问题的回答。**

GE 不是一段代码，它是一个 Data Asset。策划/设计师可以在编辑器中配置属性修改的完整规则：数值、运算方式、持续时间、触发条件、标签约束等。运行时由 GAS 框架接管执行，开发者只需要：

- 在编辑器中创建一个 GE Asset（蓝图或 C++ 子类的 Data Asset）
- 调用 `ApplyGameplayEffectSpecToSelf` 施加它

框架会自动处理持续时间、叠加、过期清理、网络同步等所有"脏活"。

### 核心设计理念：模板→实例分离

| 层级 | 类 | 角色 |
|------|-----|------|
| Data Asset (模板) | `UGameplayEffect` | CDO，定义"修改规则"，不可修改 |
| Runtime Spec (实例) | `FGameplayEffectSpec` | 冻结的运行时拷贝，携带 Context/Level 等运行时上下文 |
| Active Effect (活跃) | `FActiveGameplayEffect` | 正在作用于某个目标的效果，跟踪开始时间、持续时间、堆叠计数等 |

```
UGameplayEffect (CDO)
    ↓ MakeOutgoingSpec()
FGameplayEffectSpec (冻结 + 运行时上下文)
    ↓ ApplyGameplayEffectSpecToSelf()
FActiveGameplayEffect (开始计时，Aggregator 接管属性计算)
```

**思考**：为什么需要 Spec 这一层？同一 GE 可能被不同来源、不同 Level 多次施加。Spec 是 CDO 的冻结快照，捕获了施加那一刻的 Level、Context、SetByCallerMagnitudes 等运行时参数。

---

## 5.2 核心概念速览

在深入源码之前，先建立全局认知：

```
UGameplayEffect
├── DurationPolicy        → 决定效果持续多久（Instant / Duration / Infinite）
├── Modifiers[]           → 修改哪些属性、怎么改
├── Tags                  → 标签系统（条件、授予、清理）
├── Stacking              → 叠加规则
├── Period                → 周期执行间隔（仅 Duration 和 Infinite）
├── GameplayEffectComponents[] → 模块化组件扩展
└── 其他配置             → 概率、显示信息、GameplayCue 等
```

| 属性族 | 作用 | 典型配置 |
|--------|------|----------|
| Duration Policy | 效果持续多久 | Instant — 一次性；Duration — 持续 N 秒；Infinite — 永久 |
| Modifiers | 改什么属性、怎么改 | `Health.Add(-20)`; `Speed.Multiply(1.5)` |
| Tags | 条件与副作用 | Application Tag Requirements: 目标必须有 `Player` Tag 才能生效 |
| Stacking | 多次施加行为 | AggregateBySource — 同一来源不叠加，不同来源可叠加 |

---

## 5.3 Duration Policy：三种时间模型

```cpp
// GameplayEffectTypes.h / GameplayEffect.h
UENUM(BlueprintType)
enum class EGameplayEffectDurationType : uint8
{
    Instant,      // 立即执行 → 修改 BaseValue → 然后销毁（无 ActiveGE）
    Duration,     // 持续一段时间 → 激活 Aggregator → 到期自动移除
    Infinite,     // 永久生效 → 激活 Aggregator → 需手动移除
};
```

三种模式的本质区别不是"时长"，而是 **BaseValue 修改方式** 和 **是否有 ActiveGameplayEffect**。

### 5.3.1 Instant

Instant GE 的本质是 **直接写 BaseValue**。它不创建 `FActiveGameplayEffect`，执行完立刻销毁。类比：吃药瞬间回血。

```
Instant GE 施加 → ExecuteActiveEffectsFrom → SetAttributeBaseValue → 结束
```

关键特征：
- **不注册** 到 ActiveGameplayEffects 容器
- **没有** 持续时间，没有过期
- **不能** 被 Period 周期执行
- **不能** 被 Stack
- 常用于伤害、治疗、一次性效果

### 5.3.2 Duration

Duration GE 创建 `FActiveGameplayEffect`，BaseValue 进入 Aggregator 管理。效果持续指定的秒数，到期自动移除。

```
Duration GE 施加 → 创建 FActiveGameplayEffect → Aggregator::SetBaseValue → 倒计时 → 到期 → Aggregator 清理 → FActiveGameplayEffect 移除
```

关键特征：
- **创建** FActiveGameplayEffect，跟踪开始时间和剩余时间
- BaseValue 由 **Aggregator** 管理，修改时触发 `InternalUpdateNumericalAttribute`
- 支持 **Period** 周期执行
- 支持 **Stacking** 叠加
- 过期后 **自动移除**，Aggregator 自动清理
- 常用于：Buff、Debuff、状态效果

### 5.3.3 Infinite

Infinite GE 与 Duration 类似，但 **没有到期时间**，必须手动调用 `RemoveActiveGameplayEffect` 移除。

关键特征：
- 与 Duration 相同：创建 FActiveGameplayEffect、Aggregator 管理 BaseValue
- 不同点：持续时间设为 `-1`（即 "永久"）
- 典型用途：被动技能（如永久增加攻击力）、条件性效果（离开范围后手动移除）

### Duration 配置参数

```cpp
USTRUCT(BlueprintType)
struct FGameplayEffectDurationDefinition  // GameplayEffectTypes.h 内
{
    UPROPERTY()
    EGameplayEffectDurationType DurationType;  // Instant/Duration/Infinite
    
    // Duration > 0 时使用以下字段
    UPROPERTY()
    float Duration;                            // 持续时间（秒）
    
    UPROPERTY()
    FGameplayEffectModifierMagnitude DurationMagnitude;  // 支持 SetByCaller 动态时长！
};
```

注意 `DurationMagnitude` 是一个 `FGameplayEffectModifierMagnitude`，这意味着持续时间本身也可以用 **ScalableFloat / AttributeBased / SetByCaller** 来动态计算。SetByCaller 的 Duration 在实际项目中很常见——比如"眩晕时间 = 技能等级 * 0.5 秒"。

---

## 5.4 Modifier 体系：GE 最核心的配置

```cpp
// GameplayEffect.h
struct FGameplayModifierInfo
{
    FGameplayAttribute  Attribute;      // 目标属性
    EGameplayModOp::Type ModifierOp;    // 运算方式
    FGameplayEffectModifierMagnitude ModifierMagnitude;  // 数值来源
    FGameplayTagRequirements SourceTags;  // 过滤来源标签
    FGameplayTagRequirements TargetTags;  // 过滤目标标签
    FGameplayModEvaluationChannelSettings EvaluationChannelSettings; // 评估通道
};
```

一个 GE 可以有 **多个 Modifiers**，按数组顺序依次计算。

### 5.4.1 ModifierOp：四种运算

```cpp
// GameplayEffectTypes.h
namespace EGameplayModOp
{
    enum Type
    {
        Additive,       // + (默认)
        Multiplicitive, // *
        Division,       // /
        Override,       // 直接覆盖
        Max
    };
}
```

| ModOp | 含义 | BaseValue 变化 | 典型用途 |
|-------|------|----------------|----------|
| Additive | 加法 | New = Old + Value | 伤害、治疗 |
| Multiplicitive | 乘法 | New = Old * Value | 百分比加成 / 减速 |
| Division | 除法 | New = Old / Value | 伤害减免 |
| Override | 覆盖 | New = Value | 固定值效果 |

**优先级**：Override 最高，Additive 最低。Aggregator 在计算最终 CurrentValue 时，先按优先级聚合所有 GE 的 BaseValue 贡献。

### 5.4.2 ModifierMagnitude：数值从哪里来

```cpp
// GameplayEffectTypes.h
USTRUCT(BlueprintType)
struct FGameplayEffectModifierMagnitude
{
    EGameplayEffectMagnitude MagnitudeCalculationType;
    
    // 四种来源之一：
    FScalableFloat                    ScalableFloatMagnitude;     // 1. 固定值/曲线
    FAttributeBasedFloat              AttributeBasedMagnitude;    // 2. 基于属性
    FGameplayEffectCustomMagnitude    CustomMagnitude;            // 3. 自定义计算
    FSetByCallerFloat                 SetByCallerMagnitude;       // 4. 调用时传入
};
```

#### (1) ScalableFloat — 固定值

```cpp
FScalableFloat ScalableFloatMagnitude;
// Value = 50.0f;  // 固定50点伤害
// 也可以是 CurveTable Row — 按 Level 查表
```

最常用。可以直填一个浮点数，也可以绑定 Curvetable Row 实现按等级缩放。比如 "Level 1 伤害 30, Level 2 伤害 50"。

#### (2) AttributeBased — 基于属性计算

```cpp
USTRUCT(BlueprintType)
struct FAttributeBasedFloat
{
    FScalableFloat                     Coefficient;            // 系数
    FScalableFloat                     PreMultiplyAdditiveValue; // 预加值
    FScalableFloat                     PostMultiplyAdditiveValue;// 后加值
    FGameplayEffectAttributeCaptureDefinition BackingAttribute; // 捕获哪个属性
    FGameplayEffectAttributeCaptureDefinition AttributeCurve;    // 曲线映射属性
    EAttributeBasedMagnitudeCalculation AttributeCalculationType; // 计算类型
};
```

公式：`Magnitude = (BackingAttribute + PreAdd) * Coefficient + PostAdd`

典型例子：**"造成 攻击力 * 1.5 + 20 点伤害"**
- BackingAttribute = 攻击力
- Coefficient = 1.5
- PostMultiplyAdditiveValue = 20

#### (3) CustomCalculationClass — 自定义计算

```cpp
struct FGameplayEffectCustomMagnitude
{
    TSubclassOf<UGameplayModMagnitudeCalculation> CalculationClass;
};
```

指向一个 `UGameplayModMagnitudeCalculation` 子类。跟 ExecutionCalculation 不同，它只算**一个数值**，不直接修改最终输出。适合单一 Modifier 的复杂计算场景。

#### (4) SetByCaller — 调用时传入

```cpp
USTRUCT(BlueprintType)
struct FSetByCallerFloat
{
    FGameplayTag DataTag;   // 调用方用此 Tag 作为 Key 传值
};
```

施加 GE 时，调用方传入一个 `FGameplayTag → float` 的映射：

```cpp
FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GE, Level, Context);
Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage"), 100.0f);
ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
```

动态伤害、动态 Buff 值最常用的方式。

---

## 5.5 Tags 系统：GE 的条件与副作用

GE 的标签系统极其丰富，理解它是"配出正确的 GE"的关键。

### 5.5.1 GE Asset 自身的 Tags

```cpp
// UGameplayEffect 类成员
FGameplayTagContainer GameplayEffectAssetTags;                 // 标记此 GE
FGameplayTagContainer GrantedTags;                              // 施加时授予目标
FGameplayTagContainer RemoveGameplayEffectWithTags;             // 移除匹配 Tag 的 GE
FGameplayTagContainer OngoingTagRequirements.IgnoreTags;        // 持续条件（忽略）
FGameplayTagContainer OngoingTagRequirements.RequireTags;       // 持续条件（需要）
```

| Tag 字段 | 作用 |
|----------|------|
| AssetTags | 标记 GE 自身类型。`GE.Damage.Fire`, `GE.Buff.Speed` |
| GrantedTags | 施加时**授予目标** GameplayTag。`State.Stunned` 会添加到目标 ASC |
| RemoveGameplayEffectWithTags | 施加时**移除目标已有**的匹配 GE。低等级 Buff 被高等级替换 |
| OngoingTagRequirements | 持续条件。目标必须 **一直有** RequireTags 且 **没有** IgnoreTags，否则 GE 失效 |

**OngoingTagRequirements 是 Duration/Infinite GE 的"保鲜"条件**。比如一个 Buff 要求目标必须有 `State.Alive` Tag，目标死亡后 Tag 消失 → Aggregator 自动禁用此 GE → Buff 效果消失 → 复活后 Tag 重新出现 → GE 自动恢复。

### 5.5.2 Application / Removal Tag Requirements

```cpp
// GameplayEffect.h, UGameplayEffect 中
FGameplayTagRequirements ApplicationTagRequirements;   // 施加前检查
FGameplayTagRequirements RemovalTagRequirements;        // 移除前检查
FGameplayEffectTagRequirements OwnedTagRequirements;    // 所有者 Tag 条件
```

- **ApplicationTagRequirements**: 目标必须满足条件才能施加。检查失败 → GE 不会施加。只有 Duration/Infinite 类型的 GE 会检查。
- **RemovalTagRequirements**: 只有当目标 Tag 满足条件时，此 GE 才能被移除。
- **OwnedTagRequirements**: 施加者的 Tag 条件（较少用）。

**注意**：Instant GE 不检查 ApplicationTagRequirements，因为它不创建 FActiveGameplayEffect，没有"持续"的概念。

### 5.5.3 Modifier 级别的 Source/Target Tags

```cpp
// FGameplayModifierInfo 内部
FGameplayTagRequirements SourceTags;   // 来源方必须满足这些 Tag 才计算此 Modifier
FGameplayTagRequirements TargetTags;   // 目标方必须满足这些 Tag 才计算此 Modifier
```

这允许**单个 GE 的不同 Modifier 对不同条件生效**。比如一个"火焰伤害 GE"：
- Modifier[0]：Health.Add(-30)  — 基础伤害
- Modifier[1]：Health.Add(-20) — 仅在目标有 `State.Wet` Tag 时生效（额外伤害）

---

## 5.6 Stacking 机制

多次施加同一 GE 时，"叠加"还是"刷新"？

```cpp
// GameplayEffectTypes.h
UENUM(BlueprintType)
enum class EGameplayEffectStackingType : uint8
{
    None,                   // 不叠加 — 新施加就创建新的独立实例
    AggregateBySource,      // 按来源聚合 — 同一来源只维护一个 Stack
    AggregateByTarget,      // 按目标聚合 — 所有来源共享一个 Stack
};
```

### 5.6.1 AggregateBySource

**同一施法者施加同一 GE → 叠加 Stack 计数，不创建新实例。**

用例：同一个 Boss 对玩家施放 3 次中毒 → 目标身上只有 1 个 FActiveGameplayEffect，stackCount = 3。

### 5.6.2 AggregateByTarget

**任何来源施加同一 GE → 都叠加到同一个 Stack。**

用例：玩家被多个敌人施放中毒 → 目标身上仍只有 1 个 FActiveGameplayEffect，stackCount = 施加次数。

### 5.6.3 Stack 配置细节

```cpp
USTRUCT(BlueprintType)
struct FGameplayEffectStackingConfig
{
    EGameplayEffectStackingType StackingType;
    int32 StackLimitCount;              // 最大层数（0 = 无上限）
    
    // 每层持续时间的处理方式
    EGameplayEffectStackingDurationPolicy StackDurationRefreshPolicy;  // RefreshOnSuccess / NeverRefresh
    EGameplayEffectStackingPeriodPolicy StackPeriodResetPolicy;        // ResetOnSuccess / NeverReset
    
    // 过期策略
    EGameplayEffectStackingExpirationPolicy StackExpirationPolicy;     // ClearEntireStack / RemoveSingleStack / RefreshDuration
};
```

| 策略 | 含义 |
|------|------|
| StackDurationRefreshPolicy: RefreshOnSuccess | 施加新层时刷新所有层的 Duration |
| StackDurationRefreshPolicy: NeverRefresh | 施加新层时各层独立计时 |
| StackExpirationPolicy: ClearEntireStack | 到期时移除整个 Stack |
| StackExpirationPolicy: RemoveSingleStack | 到期时只减少一层 |

---

## 5.7 GameplayEffectComponent 体系

从 UE 5.3 开始，GE 引入模块化组件架构。此前的很多配置（如 BlockedAbilityTags、TargetTagRequirements）被迁移到了独立的 Component 中。

```cpp
// GameplayEffect.h, UGameplayEffect 成员
UPROPERTY(EditDefaultsOnly, Category=Components)
TArray<TObjectPtr<UGameplayEffectComponent>> GameplayEffectComponents;
```

### 5.7.1 常用 GE Component 列表

| Component | 功能 |
|-----------|------|
| `AbilitiesGameplayEffectComponent` | 施加时授予 GA |
| `TargetTagRequirementsGameplayEffectComponent` | 替代旧的 TargetTagRequirements |
| `TargetTagsGameplayEffectComponent` | 处理 Tag 授予 |
| `RemoveOtherGameplayEffectComponent` | 施加时移除其他 GE |
| `BlockedAbilityTagsGameplayEffectComponent` | 阻止特定 GA 激活 |
| `AdditionalEffectsGameplayEffectComponent` | 链式施加其他 GE（OnApplication / OnActive / OnRemoved） |
| `AssetTagsGameplayEffectComponent` | 替代旧的 AssetTags |
| `ChanceToApplyGameplayEffectComponent` | 概率执行 |
| `ImmunityGameplayEffectComponent` | 免疫特定 GE |
| `GameplayCuesGameplayEffectComponent` | 触发 GameplayCue |

### 5.7.2 设计优势

- **模块化**：不再需要 `UGameplayEffect` 基类包含所有可能的功能，按需添加 Component
- **可扩展**：项目可以写自定义 GE Component
- **一致性**：同一 Component 模式贯穿 GA 系统

---

## 5.8 FGameplayEffectSpec：运行时冻结实例

UE 在 `MakeOutgoingSpec` 时，从 UGameplayEffect CDO 复制出一个 `FGameplayEffectSpec`。这个复制过程就是冻结：Spec 携带了施加那一刻的所有运行时上下文。

### 5.8.1 核心结构

```cpp
// GameplayEffect.h
USTRUCT(BlueprintType)
struct GAMEPLAYABILITIES_API FGameplayEffectSpec
{
    UPROPERTY()
    TObjectPtr<const UGameplayEffect>  Def;         // → 原始 GE CDO（只读引用）
    
    TArray<FGameplayEffectModifiedAttribute> ModifiedAttributes;  // 已修改的属性记录
    
    FGameplayEffectAttributeCaptureSpecContainer   CapturedRelevantAttributes;  // 捕获的属性
    FGameplayEffectAttributeCaptureSpecContainer   TargetEffectSpec;
    
    float Duration;     // 冻结快照值（可能来自 SetByCaller）
    float Period;       // 冻结快照值
    float Level;        // GE 等级
    float ChanceToApplyToTarget;
    
    FGameplayEffectContextHandle EffectContext;     // 来源信息
    FGameplayTagContainer CapturedSourceTags;       // 冻结时刻的来源 Tags
    FGameplayTagContainer CapturedTargetTags;       // 冻结时刻的目标 Tags
    FGameplayTagContainer DynamicAssetTags;         // 动态 Asset Tags
    FGameplayTagContainer DynamicGrantedTags;       // 动态 Granted Tags
    
    TMap<FGameplayTag, float> SetByCallerTagMagnitudes;  // SetByCaller 映射
};
```

### 5.8.2 为什么需要"冻结"？

思考一个场景：GE 通过 `AttributeBasedMagnitude` 计算伤害，源属性是"攻击力"。如果 BaseValue 在计算过程中被其他 GE 修改，那么最终伤害就不明确了。

"冻结" 意味着：**在 Spec 创建时刻捕获所有相关属性值，后续计算只用冻结值。**

```cpp
// GameplayEffect.cpp
FGameplayEffectSpecHandle UAbilitySystemComponent::MakeOutgoingSpec(
    TSubclassOf<UGameplayEffect> GameplayEffectClass,
    float Level,
    FGameplayEffectContextHandle Context)
{
    // 1. 从 CDO 复制
    FGameplayEffectSpec* NewSpec = new FGameplayEffectSpec();
    NewSpec->InitializeFromLinkedSpec(CDO);  // 拷贝所有配置字段
    
    // 2. 冻结运行时上下文
    NewSpec->Level = Level;
    NewSpec->EffectContext = Context;
    
    return SpecHandle;
}
```

---

## 5.9 FGameplayEffectContext：来源信息容器

```cpp
// GameplayEffectTypes.h
USTRUCT(BlueprintType)
struct FGameplayEffectContext
{
    // 网络相关
    bool bReplicateInstigator;     // 是否同步 Instigator
    bool bReplicateEffectCauser;   // 是否同步 EffectCauser
    bool bHasWorldOrigin;          // 是否有世界坐标
    
    FGameplayAbilityTargetDataHandle TargetData;  // 所有命中信息
    
    // 来源身份
    TWeakObjectPtr<AActor> Instigator;       // 施法者（通常是 Pawn）
    TWeakObjectPtr<AActor> EffectCauser;     // 效果产生者（可能是 Projectile）
    TWeakObjectPtr<UObject> SourceObject;    // 来源对象（如武器）
    TWeakObjectPtr<UAbilitySystemComponent> InstigatorAbilitySystemComponent; // ASC
    TWeakObjectPtr<UGameplayAbility> Ability; // 来源 GA
    
    // 位置
    FVector WorldOrigin;                     // 效果世界坐标
    
    // Hit Result
    FHitResult HitResult;                    // 物理碰撞信息
};
```

区分 `Instigator` 和 `EffectCauser`：
- **Instigator**: 持有 ASC 的角色（玩家 Pawn）
- **EffectCauser**: 实际造成效果的对象（子弹/投掷物/爆炸物）

这影响后文（第 9 篇）"DamageExecution"中 `Source` 和 `Target` 的 Attribute 捕获来源。

---

## 5.10 设计思考

### 为什么 GE 要作为 Data Asset 而不是代码？

1. **策划友好**：策划不需要打开 C++，直接在编辑器中配伤害、Buff、数值曲线
2. **网络自动同步**：GAS 框架接管同步，不需要手写 RPC
3. **统一模式**：伤害、Buff、被动技能、状态效果 — 全用同一套 GE 框架表达
4. **可组合**：Tags + Stacking + Component 三大系统任意组合

### Spec 模式的代价

"冻结"带来隔离性，但有代价：
- 分配和复制 Spec 有内存开销
- 冻结时刻的属性快照可能与实际应用时不同（这是设计意图：AttributeBased 通常需要冻结源值）
- SetByCaller 无法被模板化，每次都必须传值

### GameplayEffectComponent 的演进

从 UE 4.x 到 5.3+，GE 的配置越来越组件化。基类不应承担所有功能，组合优于继承。

---

## 5.11 总结

| 概念 | 一句话 |
|------|--------|
| Duration Policy | Instant — 直写属性；Duration — 定时自动移除；Infinite — 手动移除 |
| Modifier | Attribute + ModOp + Magnitude，支持 4 种运算、4 种数值来源 |
| Tags | Application/Removal/Ongoing 三道关卡，控制施加/持续/移除 |
| Stacking | None / AggBySrc / AggByTgt 三种策略，控制多次施加行为 |
| GE Component | 插件化功能模块，替代基类字段膨胀 |
| Spec | 冻结的运行时实例，携带 Context + Level + SetByCaller |
| Context | 来源身份链：Instigator → EffectCauser → SourceObject |

> **系列导航**
> 
> | 阶段 | 篇章 | 内容 | 状态 |
> |------|------|------|------|
> | 🟢 基础 | 01 | GAS 总览与核心架构 | ✅ |
> | | 02 | ASC — 核心调度器 | ✅ |
> | | 03 | GameplayTags — 通用语言 | ✅ |
> | | 04 | AttributeSet — 属性定义与复制 | ✅ |
> | 🔵 核心 | **05** | **GameplayEffect — 效果与计算 (上)** | ✅ |
> | | 06 | GameplayEffect — 效果与计算 (下) | ✅ |
> | | 07 | GameplayAbility — 技能激活与核心框架 (上) | ✅ |
> | | 08 | GameplayAbility — Task/输入/预测 (下) | ✅ |
> | | 09 | GameplayCue — 表现层触发机制 | 📝 |
> | 🔴 高级 | 10 | Prediction — 预测与回滚 | 📝 |
> | | 11 | GE Components — 组件化架构演进 | 📝 |
> | | 12 | Network & Serial — 网络序列化 | 📝 |
> | | 13 | Targeting — 瞄准系统 | 📝 |
> | | 14 | Debug & Optimization — 调试与优化 | 📝 |
> | | 15 | 终篇回顾 — 全景复习 | 📝 |
