> **系列导航**
> 
> | 阶段 | 篇章 | 内容 | 状态 |
> |------|------|------|------|
> | 🟢 基础 | 01 | GAS 总览与核心架构 | 📝 |
> | | 02 | ASC — 核心调度器 | 📝 |
> | | 03 | GameplayTags — 通用语言 | 📝 |
> | | 04 | AttributeSet — 属性定义与复制 | 📝 |
> | 🔵 核心 | 05 | GameplayEffect — 效果与计算 (上) | 📝 |
> | | 06 | GameplayEffect — 效果与计算 (下) | 📝 |
> | | 07 | GameplayAbility — 技能激活与核心框架 (上) | 📝 |
> | | **08** | **GameplayAbility — Task/输入/预测 (下)** | ✅ |
> | | 09 | GameplayCue — 表现层触发机制 | 📝 |
> | 🔴 高级 | 10 | Prediction — 预测与回滚 | 📝 |
> | | 11 | GE Components — 组件化架构演进 | 📝 |
> | | 12 | Network & Serial — 网络序列化 | 📝 |
> | | 13 | Targeting — 瞄准系统 | 📝 |
> | | 14 | Debug & Optimization — 调试与优化 | 📝 |
> | | 15 | 终篇回顾 — 全景复习 | 📝 |

---

# 08 GameplayAbility — Task、输入与预测 (下)

## 8.1 问题驱动

上篇讲到 `ActivateAbility` 被调用，你的蓝图事件触发了。然后呢？

`ActivateAbility` 只是一个入口点，真正让技能 "活起来" 的是 `UAbilityTask`。一个技能通常需要：

1. **异步等待** — 播放动画、等待输入释放、等一段时间后自动结束
2. **输入响应** — 按住按键蓄力、释放按键发射、按键确认目标
3. **目标选择** — 鼠标点击、准星瞄准、范围扫描、目标确认/取消
4. **网络预测** — 本地立即执行，服务器验证回滚

这些需求如果直接写在 `ActivateAbility` 里，将是一团难以维护的回调嵌套。AbilityTask 解决了这个问题。

本篇逐一拆解这四个子系统。

---

## 8.2 AbilityTask 体系

### 8.2.1 基类设计

`UAbilityTask` 继承自 `UGameplayTask`，是 GAS 中 "技能生命周期内的异步任务" 的基类。

```cpp
// AbilityTask.h:107
UCLASS(Abstract, BlueprintType, meta = (ExposedAsyncProxy = AsyncTask))
class GAMEPLAYABILITIES_API UAbilityTask : public UGameplayTask
{
    GENERATED_BODY()
public:
    UPROPERTY()
    TObjectPtr<UGameplayAbility> Ability;

    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    /** 任务是否在 EndAbility 时自动结束 */
    virtual bool IsWaitingOnRemotePlayerdata() const { return false; }

    /** 蓝图覆写——任务激活 */
    virtual void Activate() override;

protected:
    /** AbilityEnding 参数表示是否因技能结束而销毁 */
    virtual void OnDestroy(bool bAbilityEnded) override;
};
```

关键机制：

1. **Outer = UGameplayAbility 实例** — Task 的生命周期绑定到 Ability 实例。`EndAbility` 时会倒序遍历 `ActiveTasks`，逐一调用 `EndTask()`
2. **`Activate()`** — 不是构造函数调用的，而是 Task 创建后你手动调用的。在调用前，Task 处于 "准备就绪" 状态
3. **`OnDestroy(bool bAbilityEnded)`** — 销毁时回调，`bAbilityEnded` 告诉你是因为技能结束 (`true`) 还是任务自己完成 (`false`)

### 8.2.2 工厂模式 + 委托

所有 AbilityTask 子类都遵循统一的创建模式：

```
静态工厂函数创建 Task → 绑定委托 → Activate()
```

以 `UAbilityTask_WaitDelay` 为例：

```cpp
// AbilityTask_WaitDelay.h
static UAbilityTask_WaitDelay* WaitDelay(
    UGameplayAbility* OwningAbility,
    float Time);

UPROPERTY(BlueprintAssignable)
FWaitDelayDelegate OnFinish;
```

蓝图使用：

```
ActivateAbility:
  ├─ WaitDelay(Task, 2.0)
  │   └─ OnFinish → EndAbility
  └─ (其他 Task...)
```

C++ 使用：

```cpp
void UMySkill::ActivateAbility(...)
{
    UAbilityTask_WaitDelay* WaitTask = 
        UAbilityTask_WaitDelay::WaitDelay(this, 2.0f);
    WaitTask->OnFinish.AddDynamic(this, &UMySkill::OnDurationComplete);
    WaitTask->ReadyForActivation();
}
```

**三个关键约定**：
1. 静态工厂函数名通常与类名一致（`WaitDelay`、`PlayMontageAndWait`）
2. 委托前缀：`OnCompleted`（正常结束）、`OnFailed`（失败）、`OnCancelled`（取消）
3. `ReadyForActivation()` 等同于 `Activate()`——调用后任务开始运行

---

## 8.3 关键 Task 深入

### 8.3.1 WaitDelay：最简单的异步等待

```cpp
// 纯定时器，到期后广播 OnFinish
UAbilityTask_WaitDelay* Task = UAbilityTask_WaitDelay::WaitDelay(this, 3.0f);
Task->OnFinish.AddDynamic(this, &UMyAbility::OnWaitComplete);
Task->Activate(); // 内部启动 FTimerHandle
```

内部实现极简——一个 `GetWorld()->GetTimerManager().SetTimer()` + 广播委托。但正是 "简单" 让它成为最常用的 Task。

### 8.3.2 PlayMontageAndWait：动画驱动技能

这是 GAS 中最复杂的 Task 之一，因为它需要同时处理 **5 个回调**：

| 委托 | 触发时机 | 使用场景 |
|------|---------|---------|
| `OnBlendedIn` | 动画完全混合进入 | 开始检测伤害窗口 |
| `OnBlendOut` | 动画开始混合退出 | 停止伤害检测，允许移动到下一段 |
| `OnCompleted` | 动画完全播放结束 | 正常结束技能 |
| `OnInterrupted` | 被其他蒙太奇覆盖 | 技能被中断 |
| `OnCancelled` | 技能被取消 | 显式取消 |

```cpp
// AbilityTask_PlayMontageAndWait.h
UFUNCTION(BlueprintCallable, meta = (DisplayName = "PlayMontageAndWait", ...))
static UAbilityTask_PlayMontageAndWait* CreatePlayMontageAndWaitProxy(
    UGameplayAbility* OwningAbility,
    FName TaskInstanceName,
    UAnimMontage* MontageToPlay,
    float Rate = 1.f,
    FName StartSection = NAME_None,
    bool bStopWhenAbilityEnds = true,
    float AnimRootMotionTranslationScale = 1.f,
    float StartTimeSeconds = 0.f,
    bool bAllowInterruptAfterBlendOut = false);
```

**连招技能的典型使用**：

```cpp
void UComboAbility::ActivateAbility(...)
{
    PlaySection(CurrentSection);
}

void UComboAbility::PlaySection(FName SectionName)
{
    UAbilityTask_PlayMontageAndWait* Task = 
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, TEXT("Combo"), ComboMontage, 1.0f, SectionName);
    Task->OnBlendedIn.AddDynamic(this, &UComboAbility::OnBlendedIn);
    Task->OnBlendOut.AddDynamic(this, &UComboAbility::OnBlendOut);
    Task->OnCompleted.AddDynamic(this, &UComboAbility::OnMontageCompleted);
    Task->OnCancelled.AddDynamic(this, &UComboAbility::OnMontageCancelled);
    Task->Activate();
}

void UComboAbility::OnBlendedIn()
{
    // 动画已进入，开始检测碰撞（伤害窗口）
    bCanDealDamage = true;
}

void UComboAbility::OnBlendOut()
{
    // 混合退出，关闭伤害窗口
    bCanDealDamage = false;

    // 如果在窗口期间收到了下一次输入，继续连招
    if (bReceivedNextInput && NextSection != NAME_None)
    {
        PlaySection(NextSection);
    }
}
```

**`bStopWhenAbilityEnds` 的陷阱**：设为 `true`（默认值）时，`EndAbility` 会自动停掉蒙太奇。如果你希望在技能结束后动画继续播放（如死亡动画），需要设为 `false`。

### 8.3.3 WaitGameplayEvent：事件驱动技能

```cpp
// AbilityTask_WaitGameplayEvent.h
UFUNCTION(BlueprintCallable, ...)
static UAbilityTask_WaitGameplayEvent* WaitGameplayEvent(
    UGameplayAbility* OwningAbility,
    FGameplayTag EventTag,
    AActor* OptionalExternalTarget = nullptr,
    bool OnlyTriggerOnce = false,
    bool OnlyMatchExact = true);
```

这个 Task 监听 GameplayTag 事件（通过 `UAbilitySystemComponent::AddGameplayEventTagContainerDelegate`），当目标 Tag 被发送事件时触发。

**关键参数**：
- `OnlyTriggerOnce` — false 时持续监听（可用于多次事件驱动的技能）
- `OnlyMatchExact` — false 时子 Tag 也会匹配（如 `Damage.Physical` 也会响应 `Damage` 事件）
- `OptionalExternalTarget` — 监听其他 Actor 的事件（默认监听自己）

**典型用法**：
- 动画通知发送 Tag 事件 → `WaitGameplayEvent` 接收 → 施加 GE
- 武器碰撞发送 `Damage.PostApply` → 技能响应 → 触发额外效果

### 8.3.4 WaitAttributeChange：属性监听

```cpp
UAbilityTask_WaitAttributeChange* Task = 
    UAbilityTask_WaitAttributeChange::WaitForAttributeChange(this, HealthAttribute);
Task->OnAttributeChange.AddDynamic(this, &UMyAbility::OnHealthChanged);
```

监听某个属性的值变化。常用于 "生命值低于阈值时触发" 或 "法力值变化时更新 UI" 的被动技能。

注意事项：属性变化非常频繁——如果你的 Task 一直存活，`OnAttributeChange` 会被高频调用。确保在不需要时及时 `EndTask()`。

### 8.3.5 WaitOverlap：碰撞等待

```cpp
UAbilityTask_WaitOverlap* Task = 
    UAbilityTask_WaitOverlap::WaitForOverlap(this);
Task->OnOverlap.AddDynamic(this, &UMeleeAbility::OnTargetHit);
```

等待与某个碰撞体的重叠事件。近战攻击技能的标配——在动画的特定阶段激活碰撞体，等待 `OnOverlap` 对目标施加 GE。

### 8.3.6 WaitConfirmCancel：确认/取消输入

```cpp
UAbilityTask_WaitConfirmCancel* Task = 
    UAbilityTask_WaitConfirmCancel::WaitConfirmCancel(this);
Task->OnConfirm.AddDynamic(this, &UMyAbility::OnTargetConfirmed);
Task->OnCancel.AddDynamic(this, &UMyAbility::OnTargetCancelled);
```

用于技能瞄准阶段——等待玩家确认目标（左键）或取消瞄准（右键）。与 `WaitTargetData` 配合使用构成完整的目标选择流。

---

## 8.4 技能输入绑定

### 8.4.1 AbilityInputID 与输入映射

技能输入绑定的核心是 `FGameplayAbilitySpec::InputID`：

```cpp
// GameplayAbilitySpec.h
UPROPERTY()
int32 InputID = INDEX_NONE;
```

`GiveAbility` 时指定 `InputID`，然后 ASC 的 `AbilityInputCache` 在每帧处理输入：

```cpp
// AbilitySystemComponent.h
/** 将按键事件映射到技能 */
void AbilityInputCachePressed(int32 InputID);
void AbilityInputCacheReleased(int32 InputID);

/** 每帧处理缓存的输入事件 */
void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
```

**GAS + EnhancedInput 的完整链路**：

```
EnhancedInput Action
  └─ InputAction 触发
      └─ InputMappingContext 映射
          └─ C++ 回调：AbilitySystemComponent->AbilityInputCachePressed(InputID)
              └─ ASC::ProcessAbilityInput()
                  └─ 遍历 ActivateAbilities，匹配 InputID
                      └─ Spec->InputPressed = true
                      └─ TryActivateAbility(Handle)
```

一个具体的绑定示例：

```cpp
// 玩家控制器中绑定 EnhancedInput
void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInput = 
        Cast<UEnhancedInputComponent>(InputComponent))
    {
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started,
            this, &AMyPlayerController::OnJumpStarted);
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed,
            this, &AMyPlayerController::OnJumpCompleted);
    }
}

void AMyPlayerController::OnJumpStarted()
{
    AbilitySystemComponent->AbilityInputCachePressed(JumpInputID);
}

void AMyPlayerController::OnJumpCompleted()
{
    AbilitySystemComponent->AbilityInputCacheReleased(JumpInputID);
}
```

### 8.4.2 WaitInputPress / WaitInputRelease

在技能内部，你可以用 Task 直接等待输入：

```cpp
UAbilityTask_WaitInputPress* PressTask = 
    UAbilityTask_WaitInputPress::WaitInputPress(this);
PressTask->OnPress.AddDynamic(this, &UChargeAbility::OnButtonPress);
PressTask->Activate();
```

以及 `WaitInputRelease`：

```cpp
UAbilityTask_WaitInputRelease* ReleaseTask = 
    UAbilityTask_WaitInputRelease::WaitInputRelease(this);
ReleaseTask->OnRelease.AddDynamic(this, &UChargeAbility::OnButtonRelease);
ReleaseTask->Activate();
```

**蓄力技能的典型模式**：

```
ActivateAbility:
  └─ WaitInputRelease → OnRelease:
      ├─ 施加蓄力等级对应的 GE
      ├─ CommitAbility（消耗 + 冷却）
      └─ EndAbility
```

### 8.4.3 InputPressed / InputReleased 路由

除了 Task 等待输入，`UGameplayAbility` 还提供了直接覆写的入口：

```cpp
// GameplayAbility.h
virtual void InputPressed(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo);

virtual void InputReleased(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo);
```

这两个方法在技能激活期间被调用——当 ASC `ProcessAbilityInput` 匹配到已激活技能的 `InputID` 时，不会重新激活技能，而是调用 `InputPressed`/`InputReleased`。适用于 "技能激活中需要响应按键事件" 的场景——连招输入的下一段、蓄力释放等。

---

## 8.5 瞄准系统

### 8.5.1 TargetActor 架构

GAS 的瞄准系统不直接写在 `UGameplayAbility` 里，而是通过 `AGameplayAbilityTargetActor` 实现：

```cpp
// GameplayAbilityTargetActor.h
UCLASS(Blueprintable, abstract, notplaceable)
class GAMEPLAYABILITIES_API AGameplayAbilityTargetActor : public AActor
{
    // 瞄准确认类型
    TEnumAsByte<EGameplayTargetingConfirmation::Type> TargetConfirmation;

    // 来源 ASC（技能拥有者）
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> OwningAbilitySystemComponent;

    // 目标数据（瞄准结果）
    FGameplayAbilityTargetDataHandle TargetDataReadyDelegate;
    FGameplayAbilityTargetDataHandle CancelledDelegateDelegate;

    // 开始瞄准
    virtual void StartTargeting(UGameplayAbility* Ability);

    // 确认目标
    virtual void ConfirmTargeting();

    // 取消瞄准
    virtual void CancelTargeting();
};
```

`EGameplayTargetingConfirmation::Type` 决定了瞄准的行为模式：

| 类型 | 含义 | 典型 TargetActor |
|------|------|-----------------|
| `Instant` | 自动确认，不需要玩家输入 | 圆形范围扫描（`AGameplayAbilityTargetActor_Radius`） |
| `UserConfirmed` | 等待玩家按键确认 | 地面指示器（`AGameplayAbilityTargetActor_GroundTrace`） |
| `Custom` | 由 TargetActor 自行决定 | 自定义瞄准 UI |
| `CustomMulti` | 多次确认（多个目标） | 多目标锁定 |

### 8.5.2 WaitTargetData 流程

`WaitTargetData` 是瞄准系统与技能之间的桥梁 Task：

```cpp
// AbilityTask_WaitTargetData.h
UFUNCTION(BlueprintCallable, ...)
static UAbilityTask_WaitTargetData* WaitTargetData(
    UGameplayAbility* OwningAbility,
    FName TaskInstanceName,
    TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType,
    TSubclassOf<AGameplayAbilityTargetActor> Class);

// 使用已有的 TargetActor（而不是生成新的）
static UAbilityTask_WaitTargetData* WaitTargetDataUsingActor(
    UGameplayAbility* OwningAbility,
    FName TaskInstanceName,
    TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType,
    AGameplayAbilityTargetActor* TargetActor);
```

核心流程：

```
WaitTargetData::Activate()
  ├─ ShouldSpawnTargetActor?
  │   ├─ Yes → BeginSpawningActor → SpawnActorDeferred
  │   │       → InitializeTargetActor → FinishSpawningActor
  │   └─ No  → 使用已有 TargetActor
  │
  ├─ RegisterTargetDataCallbacks()
  │   ├─ TargetDataReadyDelegate → OnTargetDataReadyCallback
  │   │   └─ Broadcast ValidData(FGameplayAbilityTargetDataHandle)
  │   └─ CancelledDelegate → OnTargetDataCancelledCallback
  │       └─ Broadcast Cancelled
  │
  └─ TargetActor->StartTargeting(Ability)
```

**带确认的地面瞄准技能**：

```
ActivateAbility:
  └─ WaitTargetData(..., ConfirmationType=UserConfirmed, Class=GroundTrace)
      └─ ValidData → OnTargetReady(TargetData)
          ├─ 从 TargetData 提取目标位置/方向
          ├─ CommitAbility
          ├─ 施加 GE 到目标
          └─ EndAbility
      └─ Cancelled → EndAbility
```

### 8.5.3 TargetData 网络复制

`FGameplayAbilityTargetDataHandle` 在网络间使用 `FGameplayAbilityTargetData` 结构复制：

```cpp
// GameplayAbilityTargetTypes.h
struct FGameplayAbilityTargetData
{
    virtual TArray<TWeakObjectPtr<AActor>> GetActors() const;
    virtual bool HasOrigin() const;
    virtual FTransform GetOrigin() const;
    virtual UScriptStruct* GetScriptStruct() const;
    virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);
};
```

网络流程：

1. **客户端**：`WaitTargetData` 在客户端收集瞄准信息（位置、方向、目标 Actor）
2. **发送到服务器**：`TargetData` 通过 RPC 发送到服务器
3. **服务器验证**：服务器端 `WaitTargetData` 重新运行（如果需要），或直接使用客户端数据
4. **`ValidData` 调用**：服务器收到 `TargetData` 后，触发 `ValidData` 委托，技能继续执行

这个流程的关键在于 **`TargetActor` 的 `ShouldProduceTargetDataOnServer` 属性**——如果设为 `true`，服务器端会重新运行瞄准逻辑，而不是信任客户端数据。

---

## 8.6 网络预测

### 8.6.1 PredictionKey 机制

GAS 的预测系统围绕 `FPredictionKey` 展开：

```cpp
// GameplayPrediction.h
struct FPredictionKey
{
    int32 Current;
    int32 Base;
    bool bIsStale;
    bool bIsServerInitiated;

    bool IsValidKey() const;
    bool IsStale() const;
};
```

**预测窗口**：ASC 维护一个 "预测 Key 的窗口" ——一个有效 Key 的集合。当客户端预测一个技能时，它生成一个 `PredictionKey` 并加入窗口。服务器处理该技能时，用同一个 `PredictionKey` 标记所有副作用。客户端收到服务器的确认后，将 Key 标记为 "已确认"。

### 8.6.2 激活时的预测交互

`InternalTryActivateAbility` 中的预测检查：

```cpp
// AbilitySystemComponent_Abilities.cpp:1766
if (!bIsLocal)
{
    if (Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted
        && !InPredictionKey.IsValidKey())
    {
        // LocalPredicted 技能在非本地执行时，必须有有效 PredictionKey
        return false;
    }
}
```

**预测的常用模式**：

```
客户端（LocalPredicted 技能）：
  ├─ 生成 PredictionKey
  ├─ 立即 ActivateAbility（本地预测执行）
  │   ├─ 播放动画 / 特效（客户端立即看到）
  │   └─ 生成 TargetData（客户端瞄准结果）
  │
  ├─ ServerTryActivateAbility RPC（携带 PredictionKey）
  │   └─ 服务器 InternalTryActivateAbility(InPredictionKey)
  │       ├─ CanActivateAbility（服务器端验证）
  │       ├─ ActivateAbility（服务器端执行）
  │       └─ 所有 GE 应用都带上 PredictionKey
  │
  └─ 服务器复制 GE 到客户端
      └─ 客户端：PredictionKey 匹配 → 不再重复应用 GE
```

### 8.6.3 预测的回滚与确认

当服务器拒绝技能时：

```
服务器 CanActivateAbility 返回 false
  └─ ServerAbilityFailed RPC → 客户端
      ├─ 客户端收到 PredictionKey 的回滚通知
      ├─ 取消 ActivateAbility
      ├─ 回滚动画状态
      ├─ 回滚属性变化（通过 PredictionKey 标识）
      └─ NotifyAbilityFailed
```

客户端通过 `PredictionKey` 追踪所有它预测创建的 GE 和状态变化——当服务器确认或拒绝时，客户端能精确定位哪些变化需要保留，哪些需要回滚。

---

## 8.7 设计思考

### 为什么 AbilityTask 使用 "代理" 模式？

`AbilityTask` 的工厂函数命名透着设计意图：`CreatePlayMontageAndWaitProxy`（"创建代理"）、`WaitDelay`（"等待"）、`WaitGameplayEvent`（"等待"）。

这不是命名上的巧合——每个 AbilityTask 都是 **技能主逻辑的代理**。技能 `ActivateAbility` 不做具体工作，只创建代理并委托回调：

```
技能职责：编排 (Orchestration)
  ├─ 定义 "什么时机做什么"
  └─ 不直接操作底层 API

Task 职责：执行 (Execution)
  ├─ 封装具体的异步操作
  └─ 完成后通过委托通知技能
```

这种分离让技能逻辑清晰——你看到的是 "先等 0.5s，播放动画，等动画混入后开启伤害窗口，动画结束时关闭伤害窗口，提交技能"，而不是嵌套的 Timer 和 `AnimInstance` 回调。

### 为什么输入不走 Component 而走 ASC？

很多游戏框架把输入绑定在 PlayerController 或 Pawn 上。GAS 把输入路由到 ASC，背后有一个关键原因：

**网络抽象**。在客户端预测执行时，`WaitInputPress`/`WaitInputRelease` 不需要区分 "这是本地输入" 还是 "这是服务器收到的远程输入"——ASC 统一处理。当服务器通过 RPC 执行技能时，它也能触发同样的 `InputPressed`/`InputReleased` 路径。

### 瞄准系统的网络信任模型

`ShouldProduceTargetDataOnServer` 是一个容易被忽视的战场——它直接关系到反作弊和安全：

- `false`（默认）：服务器信任客户端的瞄准数据——低延迟、对玩家友好，但容易被作弊利用
- `true`：服务器重新运行瞄准逻辑——更安全，但可能因为延迟差异导致玩家看到 "我瞄准了但没打中"

大多数 PvE 游戏用 `false`，竞技类游戏用 `true`。没有绝对正确的答案，但 Epic 把选择权留给了开发者。

---

## 8.8 总结

本篇覆盖了 GameplayAbility 的运行时子系统：

| 主题 | 关键点 |
|------|--------|
| **AbilityTask 体系** | 工厂模式 + 委托 + Activate()；9 个最常用 Task 覆盖动画、事件、属性、输入、碰撞、瞄准 |
| **PlayMontageAndWait** | 5 个回调（BlendedIn/BlandOut/Completed/Interrupted/Cancelled），连招在 BlendOut 中编排 |
| **WaitGameplayEvent** | 事件驱动——动画通知、GE 副作用都可以通过 Tag 事件触发技能逻辑 |
| **输入绑定** | EnhancedInput → ASC::AbilityInputCachePressed(InputID) → ProcessAbilityInput → TryActivateAbility；激活中的输入走 `InputPressed`/`InputReleased` |
| **瞄准系统** | TargetActor 架构，WaitTargetData 桥接技能与瞄准；`ShouldProduceTargetDataOnServer` 决定服务器信任策略 |
| **网络预测** | PredictionKey 标识预测行为；客户端预测执行 + 服务器验证 + Key 匹配确认/回滚 |

这篇结束，GAS 的核心四大组件（ASC / Tags / AttributeSet / GameplayEffect / GameplayAbility）已全部覆盖。下一篇进入表现层——GameplayCue。

[返回系列目录](../README.md)
