# 08 | GameplayAbility — Task/输入/预测 (下)

> **本篇**：GA 的运行时子系统 —— AbilityTask 工厂模式与委托、输入路由、TargetActor 瞄准、TargetData 序列化与 `FPredictionKey` 预测

> **系列**: 《Inside GAS》— UE5 GameplayAbilitySystem 源码深度分析  
> **难度**: 🔴 源码  
> **字数**: ~6000  
> **前置**: 07-GameplayAbility  
> **源码路径**: `Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/Abilities/Tasks/AbilityTask.h`

---

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
> | | 07 | GameplayAbility — 技能激活与核心框架 (上) | ✅ |
> | | **08** | **GameplayAbility — Task/输入/预测 (下)** | ✅ |
> | | 09 | GameplayCue — 表现层触发机制 | 📝 |
> | 🔴 高级 | 10 | Prediction — 预测与回滚 | 📝 |
> | | 11 | GE Components — 组件化架构演进 | 📝 |
> | | 12 | Network & Serial — 网络序列化 | 📝 |
> | | 13 | Targeting — 瞄准系统 | 📝 |
> | | 14 | Debug & Optimization — 调试与优化 | 📝 |
> | | 15 | 终篇回顾 — 全景复习 | 📝 |

---

## 一、问题驱动：GA 的"长时异步"难题

上一篇讲完了激活链路。但一个技能被激活后，往往要**跨越好几个帧甚至好几秒**才能结束：

- 施法要等 1.5 秒（延迟）；
- 冲锋要播完整段动画（Montage 播完）；
- 蓄力技能要等玩家**松开按键**（输入释放）；
- 范围技能要等玩家**确认目标**（瞄准确认）。

这些都不是"调用一个函数、下一行继续"的同步逻辑，而是**异步事件流**。如果用裸 Timer + 裸委托硬写，代码会散落在各处、难以管理、难以在技能结束时统一清理。

`UAbilityTask`（AbilityTask）就是 GAS 为这个难题提供的答案：**把一段异步等待封装成一个可激活、可结束、可被技能统一管理的对象**。本篇拆解它的体系，以及围绕它运转的三个子系统——输入、瞄准、预测。

---

## 二、AbilityTask 体系

### 2.1 基类 UAbilityTask

`UAbilityTask` 定义在 `AbilityTask.h`，继承自 `UGameplayTask`（位于 `Engine/Source/Runtime/GameplayTasks/Classes/GameplayTask.h`）。它的声明有几个**很容易被写错**的细节：

```cpp
// AbilityTask.h:21
UCLASS(Abstract, MinimalAPI)
class UAbilityTask : public UGameplayTask
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TObjectPtr<UGameplayAbility> Ability;

    UPROPERTY()
    TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;   // 注意：弱引用

    // ... 大量辅助函数
    virtual void OnDestroy(bool bInOwnerFinished) override;   // 参数名是 bInOwnerFinished
    virtual void BeginDestroy() override;
    // ...
};
```

几个**关键纠正**：

1. **`UCLASS` 是 `Abstract, MinimalAPI`，不是 `BlueprintType, meta = (ExposedAsyncProxy = AsyncTask)`**。AbilityTask 不是蓝图类型——你从不直接 `new` 一个 Task，而是通过工厂函数创建。
2. **`AbilitySystemComponent` 是 `TWeakObjectPtr`**，不是 `TObjectPtr`。Task 持有弱引用，避免与 ASC 形成引用环，也避免 ASC 销毁后 Task 还抓着悬垂指针。
3. **`UAbilityTask` 基类并没有重新声明 `Activate()`**。`Activate()` 是 `UGameplayTask` 的 protected 虚函数（`GameplayTask.h:162`），默认实现为空。UAbilityTask 不重写它，由**具体子类**按需 override（例如 `UAbilityTask_WaitTargetData` 就重写了 `Activate()`）。
4. **`OnDestroy` 的参数名是 `bInOwnerFinished`**，不是 `bAbilityEnded`。这个参数表示"Task 因 owner（技能）结束而被销毁"。

### 2.2 工厂模式与激活机制

Task 的创建走**静态工厂函数**，而不是 `NewObject`。基类提供一个模板工厂：

```cpp
// AbilityTask.h:135
template <class T>
static T* NewAbilityTask(UGameplayAbility* ThisAbility, FName InstanceName = FName())
{
    T* MyObj = NewObject<T>();
    MyObj->InitTask(*ThisAbility, ThisAbility->GetGameplayTaskDefaultPriority());
    MyObj->InstanceName = InstanceName;
    return MyObj;
}
```

注意：`NewAbilityTask` 只做"创建 + 初始化"，**并不激活**。以 `WaitDelay` 为例（`AbilityTask_WaitDelay.cpp`）：

```cpp
UAbilityTask_WaitDelay* UAbilityTask_WaitDelay::WaitDelay(UGameplayAbility* OwningAbility, float Time)
{
    UAbilityTask_WaitDelay* MyObj = NewAbilityTask<UAbilityTask_WaitDelay>(OwningAbility);
    MyObj->Time = Time;
    return MyObj;   // 工厂不激活，只设参数
}

void UAbilityTask_WaitDelay::Activate()   // 真正启动逻辑的地方
{
    UWorld* World = GetWorld();
    TimeStarted = World->GetTimeSeconds();
    World->GetTimerManager().SetTimer(TimerHandle, this, &UAbilityTask_WaitDelay::OnTimeFinish, Time, false);
}
```

那么 `Activate()` 是谁调用的？答案是 `UGameplayTask::ReadyForActivation()`（`GameplayTask.h:157`）：

```cpp
/** Called to trigger the actual task once the delegates have been set up */
UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Gameplay Tasks")
void ReadyForActivation();
```

`ReadyForActivation()` 是 public 的蓝图可见入口，它在内部做状态检查后，调用 protected 的 `Activate()`。整条链路是：

```
静态工厂（如 WaitDelay）→ NewAbilityTask（创建 + InitTask）
  → 蓝图节点绑定输出委托
  → ReadyForActivation()（public，蓝图内部调用 / C++ 手动调用）
      → Activate()（protected 虚函数，子类 override，真正启动）
```

**在 C++ 里手动使用 Task** 时，正确姿势是调用 `ReadyForActivation()`，而不是 `Activate()`（后者是 protected，外部无法调用）：

```cpp
UAbilityTask_WaitDelay* Task = UAbilityTask_WaitDelay::WaitDelay(this, 1.5f);
Task->OnFinish.AddDynamic(this, &AMyActor::OnDelayFinished);
Task->ReadyForActivation();   // 手动激活，不能写 Task->Activate()
```

### 2.3 委托广播与自动清理

每个 Task 都声明一组 `FMulticastDelegate`（如 `OnFinish`、`OnTrigger`、`OnBlendOut`）。Task 在满足条件后广播委托，然后调用 `EndTask()` 自毁。

而上一篇文章提到的 `EndAbility` 里的 `TaskOwnerEnded()` 清理，正是 Task 体系的另一半：**技能结束时，倒序遍历 `ActiveTasks` 并调用 `TaskOwnerEnded()`**。所以 Task 无需你手动追踪生命周期——它注册进技能的 `ActiveTasks`，技能结束时被统一清理。

---

## 三、关键 Task 深入

GAS 内置了十几个 Task，这里挑几个最常用的拆开看。

### 3.1 WaitDelay —— 最简单的延时

上面已经拆过。它只是 `SetTimer` 等一段时间，然后广播 `OnFinish` 并 `EndTask`。注意它的工厂名就叫 `WaitDelay`（`AbilityTask_WaitDelay.h`），不是 `WaitForDelay`。

### 3.2 PlayMontageAndWait —— 播动画 + 分段回调

工厂名是 **`CreatePlayMontageAndWaitProxy`**（`AbilityTask_PlayMontageAndWait.h:66`）。它封装了 `UAnimMontage` 的播放，并暴露多个关键委托：

| 委托 | 触发时机 |
|------|---------|
| `OnCompleted` | Montage 完整播完 |
| `OnBlendOut` | 进入 BlendOut |
| `OnInterrupted` | 被打断 |
| `OnCancelled` | 被取消 |

这四个回调覆盖了动画的四种结局，是"冲锋后接攻击""翻滚无敌帧结束"这类逻辑的标准写法。

### 3.3 WaitGameplayEvent —— 等一个 GameplayEvent

工厂名 `WaitGameplayEvent`（`AbilityTask_WaitGameplayEvent.h:31`）。它监听一个 `FGameplayTag` 对应的事件，常用于"等另一个技能/系统发来某个事件再继续"。

### 3.4 WaitForAttributeChange —— 等属性变化

工厂名是 **`WaitForAttributeChange`**（`AbilityTask_WaitAttributeChange.h:50`）——注意是 `WaitFor` 开头，很多文章写成 `WaitAttributeChange` 是错的。它监听某个 Attribute 的变化，变化时广播 `OnAttributeChange`，回调里能拿到 `OldValue` 和 `NewValue`。这是"受伤触发被动""血量低于 30% 触发狂暴"的实现基础。

### 3.5 WaitForOverlap —— 等一次碰撞

工厂名 **`WaitForOverlap`**（`AbilityTask_WaitOverlap.h:36`）。它监听 Owner 的 Overlap 事件，常用于"近战挥砍命中判定""地面践踏"这类技能。

### 3.6 WaitConfirmCancel —— 等确认 / 取消输入

工厂名 `WaitConfirmCancel`（`AbilityTask_WaitConfirmCancel.h:41`）。它同时监听"确认"和"取消"两个通用输入，是瞄准类技能的收尾环节（下一节结合 `WaitTargetData` 一起看）。

---

## 四、技能输入绑定：路由的真相

### 4.1 输入 ID 与绑定

GA 的输入不是直接绑定按键，而是绑定一个抽象的 **InputID**。ASC 里维护 `InputID → 技能` 的映射，玩家按下一个绑定键，引擎把"按键"翻译成"某个 InputID 被按下"，再路由到对应技能。

ASC 提供的真实接口是（`AbilitySystemComponent.h:1363-1364`）：

```cpp
virtual void AbilityLocalInputPressed(int32 InputID);
virtual void AbilityLocalInputReleased(int32 InputID);
```

**注意**：这两个才是真实的输入路由 API。很多资料臆造了 `AbilityInputCachePressed` / `AbilityInputCacheReleased` / `ProcessAbilityInput` 这样的函数，源码里**并不存在**。

`AbilityLocalInputPressed` 内部会根据 InputID 找到对应的 `FGameplayAbilitySpec`，设置 `Spec->InputPressed = true`（上一篇提到的位域字段），并尝试激活技能。`AbilityLocalInputReleased` 则置为 `false`。

### 4.2 两个输入 Task

- **`WaitInputPress`**（`AbilityTask_WaitInputPress.h:32`）：等下一次输入按下。蓄力/吟唱技能常用它监听"再按一下触发二段"。
- **`WaitInputRelease`**（`AbilityTask_WaitInputRelease.h:32`）：等当前按键释放。蓄力技能释放、跳跃蓄力等场景。

它们和 `AbilityLocalInputPressed/Released` 是**上下游关系**：前者是 ASC 的路由入口，后者是 Task 层对路由的封装——Task 订阅输入事件，转化为可等待的异步委托。

---

## 五、瞄准系统：TargetActor + WaitTargetData + TargetData

### 5.1 AGameplayAbilityTargetActor

范围技能需要玩家先"选个地方 / 选个目标"，这就靠 `AGameplayAbilityTargetActor`（`GameplayAbilityTargetActor.h`）。它的声明：

```cpp
// GameplayAbilityTargetActor.h:26
UCLASS(Blueprintable, abstract, notplaceable, MinimalAPI)
class AGameplayAbilityTargetActor : public AActor
{
    // ...
};
```

**关键纠正**：很多文章给 TargetActor 安上一个 `TargetConfirmation` 成员，源码里**没有这个成员**。瞄准的"确认类型"（Instant / UserConfirmed）是作为参数传给 `WaitTargetData` 工厂的，不是 TargetActor 的属性。

TargetActor 的真实关键成员与委托：

```cpp
// 网络相关：是否在服务器上产生目标数据
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Confirmation")
bool ShouldProduceTargetDataOnServer = false;

// 委托：目标数据就绪 / 瞄准被取消
FAbilityTargetData TargetDataReadyDelegate;   // 不是 FGameplayAbilityTargetDataHandle！
FAbilityTargetData CanceledDelegate;          // 不是 CancelledDelegateDelegate！

// 关联的技能
UPROPERTY()
TObjectPtr<UGameplayAbility> OwningAbility;
```

三个纠正：`TargetDataReadyDelegate` / `CanceledDelegate` 是 **`FAbilityTargetData` 委托类型**，不是 `FGameplayAbilityTargetDataHandle`；TargetActor 持有的是 `OwningAbility`（`UGameplayAbility`），不是 `OwningAbilitySystemComponent`。

关键方法：

| 方法 | 作用 |
|------|------|
| `StartTargeting` | 开始瞄准 |
| `ConfirmTargeting` | 确认当前瞄准 |
| `CancelTargeting` | 取消瞄准 |
| `ConfirmTargetingAndContinue` | 确认并继续 |
| `IsConfirmTargetingAllowed` | 是否允许确认 |

### 5.2 EGameplayTargetingConfirmation：确认类型

它也是 `UENUM + namespace + enum Type : int` 的形式（`GameplayAbilityTargetTypes.h`），**不是 `enum class`**：

```cpp
UENUM(BlueprintType)
namespace EGameplayTargetingConfirmation
{
    enum Type : int
    {
        Instant,         // 立即瞄准，无需确认
        UserConfirmed,   // 需要玩家确认
        Custom,          // 自定义
        CustomMulti,     // 自定义多目标
    };
}
```

`Instant` 适合"点击即释放"的瞬发技能；`UserConfirmed` 适合"瞄准 → 显示指示器 → 玩家确认 → 释放"的流程。

### 5.3 WaitTargetData：把瞄准变成异步等待

工厂 `WaitTargetData` / `WaitTargetDataUsingActor`（`AbilityTask_WaitTargetData.h:52`）把整个瞄准流程封装成 Task。蓝图里的标准三连：

```
WaitTargetData（确认类型=UserConfirmed，TargetActor=某瞄准 Actor）
  └─ OnTargetDataReady → 拿到 FGameplayAbilityTargetDataHandle → 释放技能
  └─ OnCancelled → 取消
```

配合 `WaitConfirmCancel`，就实现了"瞄准 → 等确认 → 释放"的完整循环。

### 5.4 FGameplayAbilityTargetData：目标数据的真实形态

`FGameplayAbilityTargetData` 是目标数据的**抽象基类**（`GameplayAbilityTargetTypes.h`）。它定义了一套多态接口：

- `GetActors()` —— 返回命中的 Actor 数组；
- `HasOrigin()` / `GetOrigin()` —— 是否有 / 获取瞄准原点；
- `GetScriptStruct()` —— 返回具体子类的 `UScriptStruct`（用于多态序列化的关键）；
- `ShouldCheckForTargetActorSwap()` / `GetTargetingTransform()` —— 瞄准相关。

**一个关键纠正**：`FGameplayAbilityTargetData` **基类本身没有 `NetSerialize` 虚方法**。网络序列化是**具体子类**负责的——例如 `FGameplayAbilityTargetData_SingleTargetHit` 才有自己的 `NetSerialize`。基类只通过 `GetScriptStruct()` 配合反射做通用序列化。很多文章把 `NetSerialize` 当成基类接口，是错的。

具体子类（如 `_SingleTargetHit`、`_ActorArray`、`_LocationInfo`、`_SourceLocation`）通过 `GetScriptStruct()` 实现多态，这套机制支撑了"目标数据跨网络传递"——预测技能里，客户端选的目标要传给服务器，靠的就是这个多态序列化。

---

## 六、网络预测：FPredictionKey 的真实结构

上一篇多次提到"预测"，这里把它的"钥匙"——`FPredictionKey` 拆开。

### 6.1 真实字段

`FPredictionKey` 定义在 `GameplayPrediction.h`（结构体从 295 行左右开始）。它的核心字段：

```cpp
struct FPredictionKey
{
    typedef int16 KeyType;   // 注意：是 int16，不是 int32

    UPROPERTY()
    int16 Current = 0;       // 当前预测键

    UPROPERTY(NotReplicated)
    int16 Base = 0;          // 基础键（链式预测时用）

    UPROPERTY()
    bool bIsServerInitiated = false;   // 是否由服务器发起
};
```

三个**关键纠正**：

1. **`Current` / `Base` 是 `int16`，不是 `int32`**。预测键用 16 位整数，足够区分并发的预测序列，且更省带宽。
2. **没有 `bIsStale` 这个字段**。很多资料写 `FPredictionKey` 有 `bIsStale` 标志"预测已过期"，源码里并不存在。过期判断靠的是键值本身和 ASC 维护的"已确认预测"集合。
3. `bIsServerInitiated` 才是指示预测来源的字段——由服务器发起的预测（如服务器触发的范围技能）置为 true。

### 6.2 关键方法

```cpp
bool IsValidKey() const;          // Current > 0
bool IsLocalClientKey() const;    // 本地客户端产生的预测
bool IsServerInitiatedKey() const;// 服务器发起的预测
bool WasReceived() const;         // 是否已收到（非本地产生）
bool WasLocallyGenerated() const; // 是否本地生成
bool IsValidForMorePrediction() const;
```

以及创建方法 `CreateNewPredictionKey`、`CreateNewServerInitiatedKey`、`GenerateDependentPredictionKey`。

### 6.3 预测的本质（预告）

`FPredictionKey` 是"客户端预测 + 服务器验证"的对齐令牌：

1. 客户端技能激活时，本地生成一个 `FPredictionKey`，**立即**用预测实例执行技能逻辑；
2. 技能涉及的网络调用（GE 应用、目标数据）都带上这个 Key；
3. 服务器用权威实例重放技能，用同一个 Key 对齐；
4. 客户端等服务器的确认结果——通过 `NewRejectedDelegate` / `NewCaughtUpDelegate` 感知预测被拒还是已追上。

这套机制的完整展开（回滚、依赖预测、`bIsServerInitiated` 的处理）放到第 10 篇专门讲，这里只需记住：**`FPredictionKey` 是 int16 的轻量令牌，没有 `bIsStale`，它是预测链路里"对齐客户端与服务器"的凭证。**

---

## 七、设计思考：Task 为什么是"对象"而不是"回调"？

回顾 Task 体系，你会发现一个贯穿始终的设计选择：**把异步等待建模成对象，而不是回调函数**。

- 回调是"匿名"的：你不知道当前有多少个回调在等、什么时候该清理它们；
- Task 是"具名"的：每个 Task 有类型、有状态、注册在技能的 `ActiveTasks` 数组里，可以被统一追踪、统一清理。

这正是 `EndAbility` 里 `ActiveTasks.Reset()` 能"一把清空所有挂起异步"的前提——如果没有 Task 这个对象层，技能结束时你要逐个取消 Timer、解绑委托、销毁监听，漏一个就是悬垂回调 bug。

另一个设计点是**工厂 + 模板**的创建范式：`NewAbilityTask<T>()` 统一处理 `NewObject` + `InitTask`，每个 Task 只需写一个静态工厂设自己的参数。这既保证了创建的规范性（不会漏 InitTask），又把"怎么创建"和"怎么运行"分开——工厂设参，`Activate()` 启动，`EndTask()` 收尾，生命周期清晰可预测。

最后是**输入与瞄准的统一抽象**：InputID 把"具体按键"和"技能"解耦（换键位不用改技能代码）；TargetData 把"选了什么目标"抽象成多态对象（技能不关心目标怎么选的，只关心拿到了什么）。这两层解耦让技能逻辑能脱离"具体输入设备"和"具体瞄准 UI"而独立存在，这是 GAS 能被复用到各种项目（RPG、MOBA、FPS）的底层原因。

---

## 八、总结

本篇从 Task 出发，串联了输入、瞄准、预测三个子系统：

| 主题 | 关键点 |
|------|--------|
| **UAbilityTask 基类** | `UCLASS(Abstract, MinimalAPI)`；`AbilitySystemComponent` 是 `TWeakObjectPtr`；**不重新声明 `Activate()`**；`OnDestroy(bool bInOwnerFinished)` |
| **激活机制** | 静态工厂 `NewAbilityTask` 只创建不激活 → `ReadyForActivation()`（public）→ `Activate()`（protected，子类 override） |
| **工厂命名** | `CreatePlayMontageAndWaitProxy` / `WaitForAttributeChange`（不是 `WaitAttributeChange`）/ `WaitForOverlap` / `WaitConfirmCancel` |
| **输入路由** | 真实 API 是 `AbilityLocalInputPressed(int32)` / `AbilityLocalInputReleased(int32)`，**不存在** `AbilityInputCache*` |
| **TargetActor** | `UCLASS(Blueprintable, abstract, notplaceable, MinimalAPI)`；**无 `TargetConfirmation` 成员**；委托是 `FAbilityTargetData` |
| **确认类型** | `EGameplayTargetingConfirmation` 是 `UENUM + namespace + enum Type : int`，值 `Instant/UserConfirmed/Custom/CustomMulti` |
| **TargetData** | 基类**无 `NetSerialize`**，靠 `GetScriptStruct()` 多态序列化，`NetSerialize` 在具体子类 |
| **FPredictionKey** | `Current`/`Base` 是 **`int16`**；**无 `bIsStale`**；有 `bIsServerInitiated` |

至此，GA 的"激活"（上篇）与"运行时"（本篇）都已拆完。下一篇进入表现层——GameplayCue 如何把"技能发生了什么"变成屏幕上的特效与音效。

**上一篇**：[07 | GameplayAbility — 技能激活与核心框架 (上)](../07-GameplayAbility/07-GameplayAbility文章.md)

**下一篇**：[09 | GameplayCue — 表现层触发机制](../09-GameplayCue/09-GameplayCue文章.md) —— 拆解 `GameplayCue` 的 Tag 路由、即时/持续两类表现与网络同步。

---

*本文基于 UE 5.8 源码分析。系列文章会继续按模块拆解，从基础到高级，从 API 到设计哲学。*
