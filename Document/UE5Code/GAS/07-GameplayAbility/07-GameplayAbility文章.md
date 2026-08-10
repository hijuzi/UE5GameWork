> **系列导航**
> 
> | 阶段 | 篇章 | 内容 | 状态 |
> |------|------|------|------|
> | 🟢 基础 | 01 | GAS 总览与核心架构 | ✅ |
> | | 02 | ASC — 核心调度器 | ✅ |
> | | 03 | GameplayTags — 通用语言 | ✅ |
> | | 04 | AttributeSet — 属性定义与复制 | ✅ |
> | 🔵 核心 | 05 | GameplayEffect — 效果与计算 (上) | ✅ |
> | | 06 | GameplayEffect — 效果与计算 (下) | ✅ |
> | | **07** | **GameplayAbility — 技能激活与核心框架 (上)** | ✅ |
> | | 08 | GameplayAbility — Task/输入/预测 (下) | ✅ |
> | | 09 | GameplayCue — 表现层触发机制 | 📝 |
> | 🔴 高级 | 10 | Prediction — 预测与回滚 | 📝 |
> | | 11 | GE Components — 组件化架构演进 | 📝 |
> | | 12 | Network & Serial — 网络序列化 | 📝 |
> | | 13 | Targeting — 瞄准系统 | 📝 |
> | | 14 | Debug & Optimization — 调试与优化 | 📝 |
> | | 15 | 终篇回顾 — 全景复习 | 📝 |

---

# 07 GameplayAbility — 技能激活与核心框架 (上)

## 7.1 问题驱动

前面花了六篇把 GameplayEffect 讲透了。GE 是 GAS "数据驱动的效果系统"，它定义了 *什么会发生*。但实战中还有另一半：*谁来做、什么时候做、怎么做*。

这就是 GameplayAbility 的领域。

一个典型的 GAS 技能全流程：

```
玩家按下按键 → ASC 查找对应 Ability → 网络验证 → 实例化 → 
CanActivate 检查 → Activate（蓝图 C++ 混合）→ 播放动画/等待事件 → 
提交消耗 → 施加 GE → 结束
```

中间每一步都值得追问：

1. **按键怎么映射到 Ability？** ASC 如何知道哪个按键对应哪个技能？
2. **实例化策略**：技能对象什么时刻创建？每个 Actor 一份还是每次激活一份？
3. **网络执行策略**：哪些技能只在客户端跑？哪些必须在服务器跑？预测怎么工作？
4. **激活链路**：`TryActivateAbility` → `InternalTryActivateAbility` → `CanActivateAbility` → `PreActivate` → `CallActivateAbility` → `ActivateAbility`，每步做什么？
5. **提交与消耗**：`CommitAbility` 做了什么？消耗 Cost GE 和 Cooldown GE 是什么时机？
6. **结束与取消**：谁决定技能结束？客户端和服务器如何同步结束状态？
7. **Block & Cancel**：一个技能如何阻止另一个技能？标签驱动的互斥是怎么实现的？

本篇（上）聚焦 1-5，下篇聚焦 AbilityTask、输入绑定、瞄准系统和网络预测。

---

## 7.2 核心概念速览

在深入源码之前，快速建立心智模型。

| 概念 | 角色 | 类比 |
|------|------|------|
| **UGameplayAbility** | 技能的逻辑载体，定义[激活 → 执行 → 结束]的行为 | "技能的蓝图" |
| **FGameplayAbilitySpec** | ASC 中存储的"技能槽位"，持有 Level、InputID、ActiveCount 等运行时状态 | "技能卡牌" |
| **FGameplayAbilitySpecHandle** | Spec 的轻量句柄，用于在 ASC 内部索引技能 | "卡牌编号" |
| **InstancingPolicy** | 控制技能对象何时创建、如何共享 | "演员管理" |
| **NetExecutionPolicy** | 控制技能在网络上的执行位置 | "执行权限" |
| **AbilityTask** | 在技能生命周期内运行的异步任务（等待、监听、动画） | "协程" |

关键关系：

```
FGameplayAbilitySpec (槽位)
  ├── Handle: FGameplayAbilitySpecHandle (索引)
  ├── Ability: UGameplayAbility* (CDO / CDO的实例)
  ├── Level / InputID / ActiveCount (运行时状态)
  └── ReplicatedInstances (网络实例)

UGameplayAbility (逻辑载体)
  ├── AbilityTags / CancelAbilitiesWithTag / BlockAbilitiesWithTag
  ├── InstancingPolicy / NetExecutionPolicy
  ├── CooldownGameplayEffectClass / CostGameplayEffectClass
  ├── ActivateAbility() / EndAbility()
  └── OnGameplayAbilityEnded (委托)
```

---

## 7.3 UGameplayAbility：三层策略

打开 `GameplayAbility.h`，第一眼看到的是三个枚举，它们共同决定了技能的行为模式。这三层策略必须一起理解。

### 7.3.1 NetExecutionPolicy：在哪执行？

```cpp
// GameplayAbility.h:205-217
UENUM(BlueprintType)
enum class EGameplayAbilityNetExecutionPolicy : uint8
{
    LocalPredicted  UMETA(DisplayName = "Local Predicted"),
    LocalOnly       UMETA(DisplayName = "Local Only"),
    ServerInitiated UMETA(DisplayName = "Server Initiated"),
    ServerOnly      UMETA(DisplayName = "Server Only")
};
```

四种策略的差别很关键：

| 策略 | 客户端执行 | 服务器执行 | 典型场景 |
|------|-----------|-----------|---------|
| `LocalPredicted` | ✅ 预测执行 | ✅ 验证执行 | 移动、跳跃、射击（立即响应） |
| `LocalOnly` | ✅ 执行 | ❌ 不执行 | 本地纯表现技能（UI动画、相机震动） |
| `ServerInitiated` | ❌ 不执行 | ✅ 执行，然后复制到客户端 | AI 技能、GM命令 |
| `ServerOnly` | ❌ 不执行 | ✅ 执行，不复制 | 服务器逻辑专用 |

`InternalTryActivateAbility` 中可以看到这些策略如何被检查：

```cpp
// AbilitySystemComponent_Abilities.cpp:1764-1793
// LocalPredicted/LocalOnly 需要 bIsLocal 为 true
if (!bIsLocal)
{
    if (Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalOnly
        || (Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted
            && !InPredictionKey.IsValidKey()))
    {
        // 不允许执行：不是本地客户端，或者没有有效预测Key
        return false;
    }
}

// ServerOnly/ServerInitiated 需要 ROLE_Authority
if (NetMode != ROLE_Authority
    && (Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::ServerOnly
        || Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::ServerInitiated))
{
    return false;
}
```

**设计要点**：`LocalPredicted` + 有效 `PredictionKey` 时，即使在 `ROLE_SimulatedProxy` 上也能执行——因为客户端合法地预测了自己的技能。

### 7.3.2 InstancingPolicy：何时创建实例？

```cpp
// GameplayAbility.h:233-240
UENUM(BlueprintType)
enum class EGameplayAbilityInstancingPolicy : uint8
{
    NonInstanced                    UMETA(DisplayName = "Non-Instanced"),
    InstancedPerActor               UMETA(DisplayName = "Instanced Per Actor"),
    InstancedPerExecution           UMETA(DisplayName = "Instanced Per Execution"),
};
```

这是最容易踩坑的策略。

| 策略 | 实例行为 | 内存 | 适用场景 | 风险 |
|------|---------|------|---------|------|
| `NonInstanced` | 无实例，直接在 CDO 上调用 `ActivateAbility` | 零开销 | 纯函数式技能（跳跃、奔跑） | ❌ 不能使用 AbilityTask（Task 需要对象实例做 RootObject） |
| `InstancedPerActor` | 每个 Actor 创建一个实例，复用 | 每技能一份 | 有状态的技能（连招、蓄力） | 首次激活时实例化，后面复用 |
| `InstancedPerExecution` | 每次激活创建新实例 | 每次激活一份 | 需要干净状态的技能 | 注意 GC 和性能 |

`InternalTryActivateAbility` 中的实例化逻辑：

```cpp
// AbilitySystemComponent_Abilities.cpp:1851-1889
bool bIsInstancedPerExecution = (Ability->GetInstancingPolicy() 
    == EGameplayAbilityInstancingPolicy::InstancedPerExecution);

// InstancedPerActor -> 检查是否已有实例
if (Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerActor)
{
    if (Spec->IsActive())
    {
        if (Ability->bRetriggerInstancedAbility && InstancedAbility)
        {
            // 重触发：先结束现有实例，再激活新的
            InstancedAbility->EndAbility(Handle, ActorInfo, ActivationInfo,
                true /*bReplicateEndAbility*/, false /*bWasCancelled*/);
        }
        else
        {
            return false; // 已在激活状态，不允许
        }
    }
}

// 检测到需要实例化
if (bIsInstancedPerExecution || !InstancedAbility)
{
    InstancedAbility = Ability->CreateInstance(*this);
}

Spec->AddInstance(InstancedAbility);
```

**三个关键细节**：
1. `NonInstanced` 不能使用 Task，因为 `UAbilityTask::Activate()` 依赖 Outer 对象的生命周期
2. `InstancedPerActor` 设置 `bRetriggerInstancedAbility = true` 后会先 End 再重新 Activate
3. `InstancedPerExecution` 每次 new 一个，要留意 `EndAbility` 清理

### 7.3.3 ReplicationPolicy：实例如何复制？

```cpp
// GameplayAbility.h:242-248
UENUM(BlueprintType)
enum class EGameplayAbilityReplicationPolicy : uint8
{
    ReplicateNo                    UMETA(DisplayName = "Do Not Replicate"),
    ReplicateYes                   UMETA(DisplayName = "Replicate"),
};
```

这个枚举只有两个值，但决策点不在枚举本身，而在 **什么时候需要用 `ReplicateYes`**：

- `NonInstanced` + `ReplicateNo`：完全不复制（常见组合）
- `InstancedPerActor` / `InstancedPerExecution` + `ReplicateYes`：实例状态需要在网络上同步时使用（如当前 `ActivationInfo`、`MontageRepData`）

---

## 7.4 FGameplayAbilitySpec：ASC 中的技能槽位

`FGameplayAbilitySpec` 是 ASC 存储技能的状态结构，不是技能逻辑本身。

```cpp
// GameplayAbilitySpec.h:40-118
USTRUCT(BlueprintType)
struct FGameplayAbilitySpec : public FFastArraySerializerItem
{
    GENERATED_USTRUCT_BODY()

    // 索引句柄——外部持有这个值来引用技能
    UPROPERTY()
    FGameplayAbilitySpecHandle Handle;

    // 技能 CDO（NonInstanced）或指向 CDO（Instanced）
    UPROPERTY()
    TObjectPtr<UGameplayAbility> Ability;

    // 技能等级（影响 GE 数值计算、Cost/Cooldown 强度等）
    UPROPERTY()
    int32 Level;

    // 输入绑定 ID（EnhancedInput 体系下的映射键）
    UPROPERTY()
    int32 InputID;

    // 技能来源对象（GiveAbility 时传入的 SourceObject）
    UPROPERTY()
    TObjectPtr<UObject> SourceObject;

    // 当前激活计数（同 Spec 可多次激活，如 InstancedPerExecution）
    UPROPERTY()
    uint8 ActiveCount;

    // 是否因网络而来（远端激活的标记）
    UPROPERTY()
    uint8 InputPressed : 1;

    // 标记为待移除（在下一帧安全清理）
    UPROPERTY()
    uint8 RemoveAfterActivation : 1;

    // 实例化技能的复制容器（InstancedPerActor/InstancedPerExecution）
    UPROPERTY()
    TArray<TObjectPtr<UGameplayAbility>> ReplicatedInstances;
};
```

**Handle 的轻量特性**：

```cpp
// GameplayAbilitySpecHandle.h
USTRUCT(BlueprintType)
struct FGameplayAbilitySpecHandle
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    int32 Handle;

    bool IsValid() const    { return Handle != INDEX_NONE; }
    void GenerateNewHandle() { static int32 GHandle = 1; Handle = GHandle++; }
};
```

Handle 就是一个 `int32`，ASC 用它通过 `FindAbilitySpecFromHandle()` 查找 `FGameplayAbilitySpec`。整个 GAS 系统都用 Handle 做技能引用，而不是直接持有指针。

**ActiveCount 的含义**：`InstancedPerExecution` 下，同一个 Spec 可以同时有多个活跃实例。`ActiveCount` 追踪这个数字。当 `ActiveCount` 归零时，说明所有实例都已经 `EndAbility`。

---

## 7.5 技能激活完整链路

这是本篇最核心的部分。从 `TryActivateAbility` 到 `ActivateAbility`，经历五层检查。

### 7.5.1 TryActivateAbility：入口

```cpp
// AbilitySystemComponent.h:1040
bool TryActivateAbility(FGameplayAbilitySpecHandle AbilityToActivate, 
    bool bAllowRemoteActivation = true);
```

这是公开入口，做三件事：
1. 如果当前是 Authority，直接调 `InternalTryActivateAbility`
2. 如果不是 Authority 但 `bAllowRemoteActivation = true`，走 RPC `ServerTryActivateAbility`
3. 如果也不行，返回 false

```cpp
// AbilitySystemComponent_Abilities.cpp:1678-1682
bool UAbilitySystemComponent::TryActivateAbility(
    FGameplayAbilitySpecHandle AbilityToActivate, bool bAllowRemoteActivation)
{
    // ... 权限检查 ...
    return InternalTryActivateAbility(AbilityToActivate);
}
```

### 7.5.2 InternalTryActivateAbility：六层关卡

`InternalTryActivateAbility` 是整个激活流程的核心。它像安检流水线，任何一层失败都会中断并通知 `AbilityFailed`。

`InternalTryActivateAbilityFailureTags` 是静态成员，记录最近一次失败的原因。

```cpp
// AbilitySystemComponent_Abilities.cpp:1704
bool UAbilitySystemComponent::InternalTryActivateAbility(
    FGameplayAbilitySpecHandle Handle, FPredictionKey InPredictionKey,
    UGameplayAbility** OutInstancedAbility,
    FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
    const FGameplayEventData* TriggerEventData)
{
    InternalTryActivateAbilityFailureTags.Reset();
```

**关卡 1：Handle 有效性和 Spec 存在性**

```cpp
    if (Handle.IsValid() == false) { return false; }

    FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
    if (!Spec) { return false; }

    // 锁定列表，防止激活期间 Spec 被销毁
    ABILITYLIST_SCOPE_LOCK();
```

**关卡 2：ActorInfo 有效性**

```cpp
    const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
    if (ActorInfo == nullptr || !ActorInfo->OwnerActor.IsValid()
                            || !ActorInfo->AvatarActor.IsValid())
    {
        return false;
    }
```

**关卡 3：网络执行策略检查**（7.3.1 已详述）

**关卡 4：事件触发检查（TriggerEventData）**

```cpp
    if (TriggerEventData)
    {
        if (!AbilitySource->ShouldAbilityRespondToEvent(ActorInfo, TriggerEventData))
        {
            NotifyAbilityFailed(Handle, AbilitySource,
                InternalTryActivateAbilityFailureTags);
            return false;
        }
    }
```

`TriggerEventData` 由事件驱动技能时传入（如被 GE 触发的技能）。此时要多做一层检查：技能是否应该响应这个事件？

**关卡 5：CanActivateAbility**

```cpp
    {
        const FGameplayTagContainer* SourceTags = TriggerEventData
            ? &TriggerEventData->InstigatorTags : nullptr;
        const FGameplayTagContainer* TargetTags = TriggerEventData
            ? &TriggerEventData->TargetTags : nullptr;

        FScopedCanActivateAbilityLogEnabler LogEnabler;
        if (!AbilitySource->CanActivateAbility(Handle, ActorInfo,
                SourceTags, TargetTags, &InternalTryActivateAbilityFailureTags))
        {
            if (InternalTryActivateAbilityFailureTags.IsEmpty())
            {
                InternalTryActivateAbilityFailureTags.AddTag(
                    GetDefault<UGameplayAbilitiesDeveloperSettings>()
                        ->ActivateFailCanActivateAbilityTag);
            }
            NotifyAbilityFailed(Handle, AbilitySource,
                InternalTryActivateAbilityFailureTags);
            return false;
        }
    }
```

这里有几个微妙点：
- `SourceTags` 和 `TargetTags` 来自事件驱动——非事件激活时两个都是 nullptr
- `FScopedCanActivateAbilityLogEnabler` 是 RAII，启用日志输出
- 如果 `CanActivateAbility` 返回 false 却没有填充 FailureTags，引擎会补一个默认 tag

**关卡 6：InstancedPerActor 重复激活检查**（7.3.2 已详述）

全部通过后，进入实例化阶段和下一环节。

### 7.5.3 CanActivateAbility：我们能激活吗？

这是蓝图可以覆写的验证函数。GAS 为你做了基础的 Tag 检查，你只需要补充业务逻辑。

```cpp
// GameplayAbility.cpp
virtual bool CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags,
    FGameplayTagContainer* OptionalRelevantTags) const;
```

默认实现检查：
1. **`ActivationRequiredTags`** — Actor 必须拥有这些 Tags
2. **`ActivationBlockedTags`** — Actor 不能有这些 Tags
3. **SourceTags / TargetTags** — 事件驱动时的额外条件（`SourceTagsMustHave`、`TargetTagsMustHave`、`SourceTagsMustNotHave`、`TargetTagsMustNotHave`）

蓝图覆写最常见的模式：

```cpp
// 蓝图 C++ 混合覆写
bool UMyAbility::CanActivateAbility(...) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags,
            TargetTags, OptionalRelevantTags))
    {
        return false;
    }
    // 自定义条件：有武器吗？有弹药吗？
    return MyCheckHasWeapon() && MyCheckHasAmmo();
}
```

### 7.5.4 PreActivate 与 CallActivateAbility

通过所有检查后，进入激活执行阶段。

```cpp
// AbilitySystemComponent_Abilities.cpp（InternalTryActivateAbility 尾部）
    // 通知：技能即将激活
    NotifyAbilityCommit(Handle);

    // 某些场景下需要复制 InputPressed 状态
    if (!bIsLocal && Ability->GetReplicationPolicy()
        == EGameplayAbilityReplicationPolicy::ReplicateNo)
    {
        Ability->CallActivateAbility(Handle, ActorInfo,
            Spec->ActivationInfo, OnGameplayAbilityEndedDelegate,
            TriggerEventData);
    }
    else
    {
        // 打印激活信息以便调试
        UE_LOGF(LogAbilitySystem, Verbose, TEXT("%s: Activating %s"),
            *GetNameSafe(GetOwner()), *Ability->GetName());
    }

    // PreActivate — 实例化后、ActivateAbility 前的钩子
    InstancedAbility->PreActivate(Handle, ActorInfo,
        Spec->ActivationInfo, SpecInput, TriggerEventData);
```

`PreActivate` 是 5.4 新加的钩子，在主 `ActivateAbility` 之前调用，适合做初始化但还不该"正式激活"的准备工作。

```cpp
// CallActivateAbility
void UGameplayAbility::CallActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate = nullptr,
    const FGameplayEventData* TriggerEventData = nullptr)
{
    // 绑定结束回调
    if (OnGameplayAbilityEndedDelegate)
    {
        OnGameplayAbilityEnded.Add(*OnGameplayAbilityEndedDelegate);
    }

    // 调用蓝图事件或 C++ 覆写
    ActivateAbility(Handle, ActorInfo, ActivationInfo,
        TriggerEventData);
}
```

### 7.5.5 ActivateAbility：你的蓝图入口

```cpp
// GameplayAbility.h
UFUNCTION(BlueprintImplementableEvent, Category = "Ability")
void K2_ActivateAbility();

UFUNCTION(BlueprintImplementableEvent, Category = "Ability")
void K2_ActivateAbilityFromEvent(const FGameplayEventData& EventData);

virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData);
```

蓝图实现：
- 按键激活 → `K2_ActivateAbility`
- 事件激活 → `K2_ActivateAbilityFromEvent`

C++ 覆写：
- 直接覆写 `ActivateAbility`，在里面启动 `UAbilityTask`：

```cpp
void UMyAbility::ActivateAbility(...)
{
    // 播放蒙太奇并等待通知
    UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, TEXT("PlayAttack"), AttackMontage);
    Task->OnCompleted.AddDynamic(this, &UMyAbility::OnMontageCompleted);
    Task->Activate(); // ReadyForActivation → Active

    // 等待输入释放（蓄力技能）
    UAbilityTask_WaitInputRelease* WaitRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
    WaitRelease->OnRelease.AddDynamic(this, &UMyAbility::OnInputReleased);
    WaitRelease->Activate();
}
```

> 图注：GA激活链路全景
>
> ```
> TryActivateAbility(Handle)
>   │
>   ├─ Authority?
>   │   ├─ Yes → InternalTryActivateAbility(Handle)
>   │   └─ No  → ServerTryActivateAbility RPC → (Server) InternalTryActivateAbility
>   │
> InternalTryActivateAbility:
>   ├─ [关卡1] Handle 有效？→ Spec 存在？
>   ├─ [关卡2] ActorInfo 有效？
>   ├─ [关卡3] NetExecutionPolicy 匹配当前 NetMode？
>   ├─ [关卡4] TriggerEventData → ShouldAbilityRespondToEvent？
>   ├─ [关卡5] CanActivateAbility → Tags / 蓝图覆写？
>   ├─ [关卡6] InstancedPerActor 重复激活检查？
>   │
>   ├─ 实例化（如果需要）
>   ├─ Spec->ActiveCount++
>   ├─ PreActivate(Handle, ActorInfo, ...)
>   └─ CallActivateAbility(Handle, ActorInfo, ...)
>       └─ ActivateAbility(Handle, ActorInfo, ...)
>           ├─ K2_ActivateAbility() [蓝图]
>           └─ AbilityTask::Activate() [C++ Task]
> ```

---

## 7.6 CommitAbility：资源消耗与冷却

技能激活后，在合适的时候调用 `CommitAbility`，它会完成两件事：

```cpp
// GameplayAbility.cpp
virtual void CommitAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    FGameplayTagContainer* OptionalRelevantTags = nullptr);

void UGameplayAbility::CommitAbility(...)
{
    // 1. 施加 Cost GE（消耗法力、体力等）
    ApplyCost(Handle, ActorInfo, ActivationInfo);

    // 2. 施加 Cooldown GE（进入冷却）
    ApplyCooldown(Handle, ActorInfo, ActivationInfo);
}
```

`CostGameplayEffectClass` 和 `CooldownGameplayEffectClass` 都定义在 `UGameplayAbility` 上：

```cpp
// GameplayAbility.h
UPROPERTY(EditDefaultsOnly, Category = "Costs")
TSubclassOf<UGameplayEffect> CostGameplayEffectClass;

UPROPERTY(EditDefaultsOnly, Category = "Cooldowns")
TSubclassOf<UGameplayEffect> CooldownGameplayEffectClass;
```

**为什么是 GE 而不是直接修改属性？** 这样 Cost 和 Cooldown 可以复用现有 GE 框架的所有能力：Modifier 计算（基于属性、基于Tag计数等）、Stacking、Duration Policy、网络复制……一个 `CooldownGE` 就是一个 Duration GE，它的 `DurationPolicy` 决定冷却时长，GE 过期后自动移除，冷却结束。

**CommitAbility 的设计约定**：
- 它不自动调用——你必须在 `ActivateAbility` 中显式调用
- 一般调用位置：确认消耗后（蓄力技能在释放时确认）；立即消耗（瞬时技能在 ActivateAbility 开头）
- 它不验证是否有足够资源——你需要先用 `CheckCost` 验证（返回 false 则不能 Commit）
- CooldownGE 施加到外层的 ASC 上，CostGE 施加到自身

---

## 7.7 EndAbility：技能结束

```cpp
// GameplayAbility.h
void K2_EndAbility();
void K2_OnEndAbility(bool bWasCancelled);

void UGameplayAbility::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    // 1. 清理所有 AbilityTask
    for (int32 TaskIdx = ActiveTasks.Num() - 1; TaskIdx >= 0; --TaskIdx)
    {
        if (UAbilityTask* Task = ActiveTasks[TaskIdx])
        {
            Task->EndTask();
        }
        ActiveTasks.RemoveAt(TaskIdx);
    }

    // 2. 广播委托
    OnGameplayAbilityEnded.Broadcast(this);

    // 3. 通知 ASC
    if (AbilitySystemComponent.IsValid())
    {
        AbilitySystemComponent->NotifyAbilityEnded(Handle, this, bWasCancelled);
    }

    // 4. 蓝图回调
    K2_OnEndAbility(bWasCancelled);

    // 5. 网络复制
    if (bReplicateEndAbility && AbilitySystemComponent.IsValid())
    {
        AbilitySystemComponent->ReplicateEndAbility(Handle, 
            GetCurrentAbilitySpecHandle(), ActivationInfo);
    }
}
```

`EndAbility` 的核心约定：
1. **Task 清理** — 所有未完成的 Task 被 `EndTask()` 终止
2. **委托广播** — `OnGameplayAbilityEnded` 通知所有监听者（包括 ASC）
3. **ASC 通知** — `NotifyAbilityEnded` 递减 `ActiveCount`
4. **蓝图回调** — `K2_OnEndAbility(bWasCancelled)` —— 一个参数告诉你为什么结束

`bReplicateEndAbility` 参数控制是否将结束通知发送到客户端。大多数情况用 `true`。

---

## 7.8 Block & Cancel：标签驱动的技能互斥

GAS 用 GameplayTags 实现技能之间的互斥和取消，不需要硬编码的引用关系。

**取消关系 (`CancelAbilitiesWithTag`)**：
当一个技能激活时，所有匹配 Tag 的已激活技能被 `CancelAbility`。

```cpp
// GameplayAbility.h
UPROPERTY(EditDefaultsOnly, Category = "Advanced", meta = (Categories = "AbilityTagCategory"))
FGameplayTagContainer CancelAbilitiesWithTag;
```

**阻塞关系 (`BlockAbilitiesWithTag`)**：
当一个技能激活期间，所有匹配 Tag 的待激活技能被阻止（`CanActivateAbility` 返回 false）。

```cpp
// GameplayAbility.h
UPROPERTY(EditDefaultsOnly, Category = "Advanced", meta = (Categories = "AbilityTagCategory"))
FGameplayTagContainer BlockAbilitiesWithTag;
```

**激活阻塞 (`ActivationBlockedTags`)**：
当 Actor 拥有这些 Tags 时，技能无法激活（如"眩晕" Tag 阻塞所有主动技能）。

```cpp
// GameplayAbility.h
UPROPERTY(EditDefaultsOnly, Category = "Tags", meta = (Categories = "ActivationTagCategory"))
FGameplayTagContainer ActivationBlockedTags;
```

**实际互斥场景示例**：

```
技能A (近战攻击)：
  BlockAbilitiesWithTag = ["Ability.Skill"]

技能B (大招)：
  AbilityTags = ["Ability.Skill"]
  ActivationBlockedTags = ["State.Stunned"]

技能C (被眩晕GE)：
  施加 "State.Stunned" Tag → 拥有 Ability.Skill 标签的技能全部阻塞
```

**关键洞察**：Block 和 Cancel 都通过 Tag 匹配实现，这意味着：
- 新的 GE 可以间接阻塞技能（通过施加 Tag）
- 技能之间的互斥关系由数据配置，不由代码硬编码
- 一个 Tag 能同时影响多个技能

`InternalTryActivateAbility` 中的阻塞检查：

```cpp
// AbilitySystemComponent_Abilities.cpp
// CanActivateAbility 内部会执行：
const FGameplayTagContainer& OwnedTags = ActorInfo->AbilitySystemComponent->
    GetOwnedGameplayTags();
if (OwnedTags.HasAny(ActivationBlockedTags))
{
    return false; // 拥有阻塞标签
}
```

以及跨技能阻塞：

```cpp
// Check if any active ability blocks this one
for (const FGameplayAbilitySpec& ActiveSpec : GetActivatableAbilities())
{
    if (ActiveSpec.IsActive())
    {
        if (Ability->GetAssetTags().HasAny(
            ActiveSpec.Ability->BlockAbilitiesWithTag))
        {
            return false; // 某个活跃技能阻塞了这个技能
        }
    }
}
```

---

## 7.9 设计思考

### 为什么用 "三层策略" 而不是一个统一的枚举？

Epic 把 NetExecutionPolicy、InstancingPolicy、ReplicationPolicy 拆成三个独立枚举，这是深思熟虑的结果：

**正交设计**：三种策略的决策是独立的。
- `NonInstanced` 的技能也可以是 `ServerOnly`（如服务器端的积分结算技能）
- `InstancedPerExecution` 的技能可以是 `LocalOnly`（如客户端纯表现技能）
- 不合理的组合（如 `NonInstanced` + `ReplicateYes`）在运行时会被发现

如果把这三个维度合并成一个枚举，会出现 4×3×2 = 24 种组合的 "超级策略枚举"，反而难以理解。

### 为什么 Cost 和 Cooldown 是 GE 而不是直接属性修改？

这是一种"组合优于继承"的设计。如果 Cost 和 Cooldown 是独立的属性系统，它们需要：
- 独立的数值计算逻辑
- 独立的网络复制
- 独立的 Stacking / Duration 逻辑

复用 GE 框架后，Cost 和 Cooldown 天然获得：
- `ModifierMagnitudeCalculation` — 自定义计算公式
- `DurationPolicy` — 冷却时长即 GE 时长
- `Stacking` — 多层冷却的概念直接可用
- 网络复制 — GE 的复制机制直接承担

这是一种"不做不必要的抽象"的务实设计。

### 为什么激活失败用 Tag 而不是 ErrorCode？

`InternalTryActivateAbilityFailureTags` 是 `FGameplayTagContainer` 而不是 `int32 ErrorCode`。这又是 Tag 驱动哲学的体现：

- 不同的失败原因可以组合（"网络错误 + 冷却中"是两个 Tag）
- 蓝图可以直接用 Tag 查询 "哪种失败" 并展示不同 UI
- 新增失败原因不需要修改枚举，只需加 Tag

---

## 7.10 总结

本篇覆盖了 GameplayAbility 的核心框架：

| 主题 | 关键点 |
|------|--------|
| **三层策略** | NetExecutionPolicy 决定在哪执行，InstancingPolicy 决定何时创建实例，ReplicationPolicy 决定是否复制 |
| **FGameplayAbilitySpec** | ASC 中的"技能槽位"，持有 Level、InputID、ActiveCount 等运行时状态 |
| **激活链路** | TryActivateAbility → InternalTryActivateAbility（6层关卡）→ PreActivate → CallActivateAbility → ActivateAbility |
| **CommitAbility** | 施加 CostGE + CooldownGE，复用现有 GE 框架 |
| **EndAbility** | 清理 Task、广播委托、通知 ASC、蓝图回调 |
| **Block & Cancel** | Tag 驱动的互斥——配置即可实现任意复杂的互斥关系 |

**下篇预告**：`ActivateAbility` 之后发生了什么？走进 AbilityTask 的世界——WaitDelay、PlayMontageAndWait、WaitGameplayEvent、WaitAttributeChange，以及输入绑定、瞄准系统和网络预测机制。

[返回系列目录](../README.md)
