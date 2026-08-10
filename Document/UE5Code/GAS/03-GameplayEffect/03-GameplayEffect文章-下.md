# 深入浅出UE5 GAS（四）：GameplayEffect（下）—— Modifier、Stacking与Periodic

## 系列目录

1. [AbilitySystemComponent —— GAS的心脏](../01-ASC/01-ASC文章.md)
2. [AttributeSet —— 数值系统的基石](../02-AttributeSet/02-AttributeSet文章.md)
3. [GameplayEffect（上）—— 从定义到应用](./03-GameplayEffect文章-上.md)
4. **（本文）GameplayEffect（下）—— Modifier、Stacking与Periodic**
5. [ExecutionCalculation —— 自定义伤害公式](../04-ExecutionCalculation/04-ExecutionCalculation文章.md)
6. [GameplayAbility —— 技能的诞生与消亡](../05-GameplayAbility/05-GameplayAbility文章.md)
7. [AbilityTask —— 异步编程的艺术](../06-AbilityTask/06-AbilityTask文章.md)
8. [TargetData —— 索敌与数据传递](../07-TargetData/07-TargetData文章.md)
9. [GameplayCue —— 技能反馈的表现层](../08-GameplayCue/08-GameplayCue文章.md)
10. [网络同步 —— 复制、预测与回滚](../09-Network/09-Network文章.md)
11. [GE Components —— UE 5.3 模块化新范式](../10-GEComponents/10-GEComponents文章.md)
12. [实战全景 —— 调试、陷阱与工程化模式](../11-Practice/11-Practice文章.md)

> 📎 [附录 —— FGameplayAbilitySpec 详解](../12-Others/12-Others文章.md)

---

## 写在前面

上篇我们分析了 GE 的 CDO→Spec→ActiveEffect 三阶段生命周期、三种 Duration Policy 和效果上下文。但上篇有意回避了一个核心问题：**GE 到底怎么"修改属性值"？**

这就是本篇的主题——GE 的数值修改系统。我们将深入四个子模块：

1. **FGameplayModifierInfo**：Modifier 的配置结构
2. **Magnitude 计算**：四种"数值来自哪里"的方式
3. **Stacking**：多个同类 GE 如何叠加
4. **Periodic Effects**：周期性效果的实现机制

最后会给出一个 ExecCalc 的快速指引（详细分析见第 5 篇）。

---

## 一、FGameplayModifierInfo —— Modifier 的配置结构

每个 GE 的 Modifiers 数组元素是 `FGameplayModifierInfo`：

```cpp:200:229:E:\EpicGames\UE_5.8\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Public\GameplayEffect.h
USTRUCT(BlueprintType)
struct GAMEPLAYABILITIES_API FGameplayModifierInfo
{
    GENERATED_USTRUCT_BODY()

    /** 修改哪个属性？ */
    UPROPERTY(EditDefaultsOnly, Category = Modifier)
    FGameplayAttribute Attribute;

    /** 怎么修改？Add / Multiply / Divide / Override */
    UPROPERTY(EditDefaultsOnly, Category = Modifier)
    TEnumAsByte<EGameplayModOp::Type> ModifierOp;

    /** 修改多少？(Magnitude 来源) */
    UPROPERTY(EditDefaultsOnly, Category = Modifier)
    FGameplayEffectModifierMagnitude ModifierMagnitude;

    /** 评估通道（默认 Channel 0） */
    UPROPERTY(EditDefaultsOnly, Category = Modifier)
    FGameplayModEvaluationChannelSettings EvaluationChannelSettings;

    /** 来源 Tags 条件 */
    UPROPERTY(EditDefaultsOnly, Category = Modifier)
    FGameplayTagRequirements SourceTags;

    /** 目标 Tags 条件 */
    UPROPERTY(EditDefaultsOnly, Category = Modifier)
    FGameplayTagRequirements TargetTags;
};
```

这个结构回答了三个核心问题：

| 问题 | 字段 |
|------|------|
| **改什么** | `Attribute`（指向 `FGameplayAttribute`，通过 `FProperty*` 关联到 AttributeSet 的具体属性） |
| **怎么改** | `ModifierOp`（Add / Multiply / Divide / Override） |
| **改多少** | `ModifierMagnitude`（四种计算方式，见下一节） |

一个 GE 可以有**多个** Modifier，例如一个"狂战士之怒"技能可以同时：
- `Additive +20` 攻击力
- `Multiplicitive ×1.15` 移动速度
- `Additive +50` 最大生命值

### ModifierOp 的聚合语义

四种操作符在 Aggregator 中的合并规则：

```cpp
enum class EGameplayModOp : uint8
{
    Additive,       // 直接加到 BaseValue 上: Final = Base + Sum(AddMods)
    Multiplicitive, // 累乘: Final = Base * Product(MultMods)
    Division,       // 累除: Final = Base / Product(DivMods)
    Override,       // 覆盖 BaseValue: Final = LastOverride (只有最后一个生效)
};
```

**关键**：同一属性的多个 GE 的 Modifier 会被 Aggregator 统一计算。对于 Additive 类型的 Modifier 是**求和**，对于 Multiplicitive 是**求积**。这保证了十个 Buff（每个 +10% 攻击力）能正确地叠加而非覆盖。

`SourceTags` / `TargetTags` 提供细粒度的条件控制——只有当 Source（施放者）或 Target（目标）拥有特定 Tag 时，这个 Modifier 才生效。这给了策划"条件修改"的能力，无需编写代码。

### Aggregator 的内部机制

每个属性在 ASC 内部对应一个 `FAggregator` 实例。当一个 Duration GE 应用时，它的 Modifier 被**注册**到该属性的 Aggregator 中（而非直接修改属性值）。属性的"当前值" = `BaseValue + Aggregator.Evaluate()`。

这意味着：
- GE 移除时，只需从 Aggregator 中注销 Modifier，Aggregator 自动重新计算
- 属性复制的是 BaseValue，Modifier 在客户端本地 Aggregator 中独立计算
- 多个 GE 可以同时修改同一个属性而不冲突

---

## 二、Magnitude 计算：四种"数值来自哪里"

GAS 提供了四种 Magnitude 计算方式：

```cpp:344:351:E:\EpicGames\UE_5.8\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Public\GameplayEffect.h
UENUM()
enum class EGameplayEffectMagnitudeCalculation : uint8
{
    ScalableFloat,            // 1. 可缩放浮点数（支持曲线表）
    AttributeBased,           // 2. 基于另一个属性的值
    CustomCalculationClass,   // 3. MMC：自定义蓝图/C++计算
    SetByCaller              // 4. 运行时在Spec创建时设定
};
```

### 2.1 ScalableFloat —— 最简单的数据驱动

直接配置一个数值，通过 UE 的 CurveTable 按技能等级缩放：

```cpp
UPROPERTY(EditDefaultsOnly)
FScalableFloat ScalableFloatMagnitude;
// → 在 CurveTable 中: Lv.1=10, Lv.2=15, Lv.3=22...
```

适合不需要动态计算、策划可以直接填表配置的场景。

### 2.2 AttributeBased —— 单向属性依赖

让 Magnitude 依赖于**另一个属性**的值：

```cpp
struct FAttributeBasedFloat
{
    FGameplayEffectAttributeCaptureDefinition BackingAttribute; // 从哪个属性取值？
    FScalableFloat Coefficient;                                  // 属性系数
    FScalableFloat PreMultiplyAdditiveValue;                     // 前加数
    FScalableFloat PostMultiplyAdditiveValue;                    // 后加数
    EGameplayEffectAttributeCaptureSource AttributeCaptureSource; // Source 还是 Target？
};
```

等价公式：`Magnitude = (BackingAttrValue + PreAdd) * Coefficient + PostAdd`

例如：伤害 = 来源的 AttackPower * 1.5 + 10。

### 2.3 CustomCalculationClass（MMC）

当 ScalableFloat 和 AttributeBased 不够用时，使用 `UGameplayModMagnitudeCalculation`：

```cpp:394:404:E:\EpicGames\UE_5.8\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Public\GameplayEffectCalculation.h
UCLASS(BlueprintType, Blueprintable, Abstract, MinimalAPI)
class UGameplayModMagnitudeCalculation : public UGameplayEffectCalculation
{
    GENERATED_UCLASS_BODY()

public:
    /** 在蓝图中重写的计算逻辑 */
    UFUNCTION(BlueprintNativeEvent, Category = "Calculation")
    float CalculateBaseMagnitude(const FGameplayEffectSpec& Spec) const;
};
```

MMC 适合"单个属性 × 自定义公式"的计算，如果公式只涉及单个属性的 Magnitude 就可以用它。但 MMC 只能返回**一个 float**——不能同时修改多个属性。

### 2.4 SetByCaller —— 运行时动态注入

允许在创建 Spec 时通过 Tag 传递数值：

```cpp
// 创建 Spec 时设置
FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GE_Class, Level, ContextHandle);
SpecHandle.Data.Get()->SetSetByCallerMagnitude(
    FGameplayTag::RequestGameplayTag("Data.Damage"), 50.0f);
```

然后在 GE 的 Modifier 中，将 Magnitude Calculation Type 设为 `SetByCaller`，Data Tag 设为 `Data.Damage`——运行时 GE 会自动读取这个值。

SetByCaller 解决了"同一个 GE CDO，不同调用方传入不同数值"的问题。例如，`GE_FireballDamage` 是同一个 CDO，但不同等级的火球术传不同的 Damage 值——不需要创建 5 个 GE 资产。

### 快速决策表

| 场景 | 选择 |
|------|------|
| 固定数值或从曲线表读值 | `ScalableFloat` |
| 数值依赖另一个属性（如 "伤害 = 攻击力 * 1.5"） | `AttributeBased` |
| 需要自定义计算但只输出一个数值 | `MMC` (CustomCalculationClass) |
| 同一个 GE 资产，调用方传不同数值 | `SetByCaller` |
| 多属性输入 + 复杂公式 + 多属性输出 | `ExecCalc`（见第 5 篇） |

---

## 三、Stacking —— 叠加机制

### 3.1 StackingPolicy 三种模式

```cpp
enum class EGameplayEffectStackingPolicy : uint8
{
    None,                   // 不叠加：每个 GE 独立存在
    AggregateBySource,      // 按来源聚合：同一来源的多个 GE 合并
    AggregateByTarget       // 按目标聚合：忽略来源，只合并到目标
};
```

| 模式 | 行为 | 典型场景 |
|------|------|---------|
| `None` | 每次应用都创建独立的 ActiveGE，互不影响 | 不同 Buff 同时存在 |
| `AggregateBySource` | 同一来源再次应用 → 增加 StackCount | 同一个敌人多次攻击的 DoT 叠加 |
| `AggregateByTarget` | 任何来源再次应用 → 增加 StackCount | 多个队友的加速 Buff 不叠加 |

### 3.2 Stacking 配置详解

```cpp
struct FGameplayEffectStackingPolicy
{
    EGameplayEffectStackingPolicy StackingPolicy;
    int32 StackLimitCount;                                    // 最大层数（0=无上限）
    EGameplayEffectStackingDurationPolicy StackDurationRefreshPolicy; // Duration刷新策略
    EGameplayEffectStackingPeriodPolicy StackPeriodResetPolicy;        // Period重置策略
    EGameplayEffectStackingExpirationPolicy StackExpirationPolicy;    // 过期策略
};
```

**Duration 刷新策略**（控制叠加时 Duration 的行为）：
- `RefreshDuration`：重置为完整 Duration（适合"重新中毒，DOT 时间刷新"）
- `NeverRefresh`：保持原到期时间（适合"叠加层数但不延长持续时间"）

**Expiration 策略**（StackCount 减少到 0 时的行为）：
- `ClearEntireStack`：一次性清空所有层（默认行为）
- `RemoveSingleStackAndRefreshDuration`：逐层移除
- `RefreshDuration`：重置 Duration 但不移除层

### 3.3 Stacking 的工作原理

当 ApplyGE 检测到 StackingPolicy 不是 `None`，系统会：

1. 查询目标的 ActiveGEs 中是否有匹配类型的 GE
2. 如果找到，不创建新的 ActiveGE，而是调用现有 GE 的 `IncrementStackCount()`
3. 根据 `StackDurationRefreshPolicy` 决定是否刷新 Duration
4. 根据 `StackExpirationPolicy` 决定过期行为
5. Aggregator 中的 Modifier 自动根据新的 StackCount 调整 Magnitude

这意味着 Aggregator 中的 Modifier 是从 `FActiveGameplayEffect.StackCount` 获取其 Magnitude——Aggregator 本身不存储 StackCount。

---

## 四、Periodic Effects —— 周期性效果

### 4.1 核心配置

Periodic GE 通过将 `Period` 设为大于 0 来实现周期性执行：

| 字段 | 作用 |
|------|------|
| `Period` | 周期时间（秒） |
| `bExecutePeriodicEffectOnApplication` | 应用时是否立即执行一次 |

### 4.2 执行循环

每次 Period 到期时：

```
1. 检查 OngoingTagRequirements 是否还满足
   ↓ 不满足 → 移除 GE
2. 重新执行 Modifiers / ExecCalc
   → 属性值被重新修改（新值覆盖旧值）
3. 触发 GameplayCue（如果有周期表现）
4. 设置下一次定时器
5. （可选）触发 Conditional GE
```

### 4.3 Periodic 引擎的实现

Periodic 的执行引擎在 `FActiveGameplayEffectsContainer` 中。其定时器通过 FTimerManager 管理——这是 UE 的标准定时器系统（非 GAS 特有）。每个周期 GE 的定时器会在 `PeriodHandle` 中记录。

**模式区别**：
- **Period + Simple Modifier**：每个周期重新执行 Modifier 计算，结果**覆盖**上一周期的修改。适合"流血 DoT：每 2 秒造成 10 点伤害"
- **Period + ExecCalc**：每个周期执行完整的 ExecCalc 逻辑。适合"复杂的周期性效果：每 3 秒检查目标属性，根据护盾值、抗性等计算实际伤害"

**一个关键细节**：对于 Periodic GE，如果 Modifier 是 `AttributeBased` 且 `bSnapshot = true`，那么每次周期使用的都是创建时捕获的属性快照值——即使源属性中途变化也不会受影响。这也是为什么 DoT 的"首次伤害"和"后续伤害"可以使用相同的伤害公式。

---

## 五、ExecutionCalculation 快速指引

当 Modifiers 列表不够用时（需要多属性输入 + 复杂公式 + 多属性输出），使用 ExecCalc。

ExecCalc 的核心接口：

```cpp:433:436:E:\EpicGames\UE_5.8\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\Public\GameplayEffectExecutionCalculation.h
UFUNCTION(BlueprintNativeEvent, Category = "Calculation")
void Execute(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
             FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const;
```

与 MMC 的关键区别：

| | MMC | ExecCalc |
|---|---|---|
| **输出** | 一个 `float` | 任意数量的 `FGameplayModifierEvaluatedData` |
| **属性捕获** | `RelevantAttributesToCapture` | `RelevantAttributesToCapture` + `AttemptGatherAttributeMods()` |
| **蓝图支持** | 是 | 是（但生产环境推荐 C++） |
| **是否可在客户端执行** | 是（作为预测的一部分） | **否**（仅服务器执行） |

ExecCalc 的详细分析（属性捕获宏、Scoped Modifiers、AttemptGatherAttributeMods、实战示例）见 **第 5 篇：ExecutionCalculation —— 自定义伤害公式**。

---

## 六、下篇总结

本篇覆盖了 GE 的数值修改系统：

| 概念 | 关键类/结构 | 核心作用 |
|------|-----------|---------|
| Modifier 配置 | `FGameplayModifierInfo` | 描述"改什么、怎么改、改多少" |
| 运算符 | `EGameplayModOp` | Add/Multiply/Divide/Override |
| Magnitude 来源 | ScalableFloat / AttributeBased / MMC / SetByCaller | 四种数值计算方式 |
| 叠加 | `FGameplayEffectStackingPolicy` | 控制同类 GE 的合并 |
| 周期执行 | `Period` + Timer | 实现 DoT/HoT |

**核心设计思想**：
1. `ModifierInfo.Attribute` 通过 `FProperty*` 与 AttributeSet 关联——编译时安全、反射友好
2. Modifier 不直接修改属性值，而是注册到 Aggregator —— 支持多 GE 协作修改同一属性
3. Stacking 本质上是"复用已存在的 ActiveGE 而非创建新的"——节省内存和网络带宽
4. 四种 Magnitude 来源 + ExecCalc 构成了从"极简"到"极灵活"的完整梯度
5. Periodic 的每次执行都重新计算 Magnitude——允许来源属性在 tick 间变化

**结合上下两篇**，GameplayEffect 完整覆盖了"效果是什么 → 如何定义 → 怎么修改属性 → 持续多久 → 怎么叠加 → 如何周期执行"的全链路。

**下一篇预告**：当 Modifier 的计算不够灵活时，如何使用 ExecutionCalculation 实现"攻击力 - 防御力 × 暴击倍率"这样的复杂伤害公式？属性捕获的 `bSnapshot` 模式如何影响计算？Scoped Modifiers 如何实现数据驱动的公式预处理？

---

*本系列文章基于 UE 5.8 源码分析。源码路径：`Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/`*
