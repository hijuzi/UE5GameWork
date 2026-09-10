# GameplayAbilities 插件：项目定制与 UE 5.8 引擎源码对比

> 本文对比项目内插件 `GameWork\Plugins\GameplayAbilities` 与引擎源码
> `E:\EpicGames\UE_5.8\Engine\Plugins\Runtime\GameplayAbilities` 的全部差异。
> 对比基于逐文件字节级 Hash 校验与源码级 `git diff`，而非仅看文件是否同名。
>
> ⚠️ **本文为早期快照**：描述的是 `GAS_MOD_01~04` 时期的状态（当时为自研 `ERoundType` 回合制）。
> 之后插件又经历了 **回合制改造**（`GAS_MOD_05~09`，见 `GameWork/Plugins/GameplayAbilities/回合制改造方案.md`）
> 与 **多时间轴（时机 Tag）改造**（`GAS_MOD_10~12`，见 `GameWork/Plugins/GameplayAbilities/回合制改造方案.md`）。
> 现行实现为 `bTurnBased` + `FAbilityTimerManager`（按 `TimeAxis.*` 时机分轴），
> 完整改动前/后对照以 `GameWork/Plugins/GameplayAbilities/源码修改记录.md` 为准。

## 1. 结论速览

该项目**直接复制了引擎 GameplayAbilities 插件源码到项目内进行二次开发**，然后做了两类改动：

1. **自研"回合制 Duration 效果"系统**（主要改动，围绕 `ERoundType` / `ERoundNum` / `ERoundApplyType`）。
2. **若干 BUG 修复 / 体验优化**：
   - 修复 GameplayCue `N*N` 重复触发（带作者注释 `[LiuYang] 2026/07/28`）。
   - 修复 StackOverflow 无 StackLimit 时被误判为已达上限的问题。
   - 修复 Stack 满后 `OverflowEffects` 多次应用的问题。
   - 屏蔽若干 UE5.8 废弃警告。
   - 新增一个等待 GameplayTag 添加/移除并回调具体标签的 Async Action。

差异**只集中在运行时模块 `GameplayAbilities`**。编辑器模块 `GameplayAbilitiesEditor`、插件描述文件、所有 `Build.cs`、`Config/*.ini`、`README.md` 均与引擎逐字节一致。

---

## 2. 文件级差异（目录结构 / 数量）

| 项目 | 引擎 | 项目 | 说明 |
|------|------|------|------|
| 运行时模块 `.h/.cpp/.cs` | 286 | 288 | 项目**净增 2 个文件** |
| 编辑器模块 `.h/.cpp/.cs` | 50 | 50 | 完全一致 |
| `GameplayAbilities.uplugin` | — | — | 字节级相同（SAME） |
| `README.md` | — | — | 字节级相同（SAME） |
| `Config\DefaultGameplayAbilities.ini` | — | — | 字节级相同（SAME） |
| `Config\Input.ini` | — | — | 字节级相同（SAME） |
| `Source\*\*.Build.cs` | — | — | 字节级相同（SAME） |
| `Content\` | — | — | 未发现额外资源（运行时模块级纯代码改动） |

### 2.1 运行时模块新增文件（项目独有，引擎不存在）

| 文件 | 类型 | 作用 |
|------|------|------|
| `Public\Abilities\Async\AbilityAsync_WaitGameplayTagNotify.h` | 头文件 | 定义两个新的 Async Action 类 |
| `Private\Abilities\Async\AbilityAsync_WaitGameplayTagNotify.cpp` | 源文件 | 实现 |

> 其余 286 个运行时模块文件与引擎**同名**，其中 280 个内容完全一致、仅 7 个内容被改动（见下表）。

### 2.2 内容被改动的同名文件（7 个，全部在运行时模块）

| 文件 | 改动规模 | 改动主题 |
|------|---------|---------|
| `Public\GameplayEffect.h` | 大 | 新增 `ERoundType`/`ERoundApplyType` 枚举、回合字段（GE、Spec、ActiveGE 容器） |
| `Private\GameplayEffect.cpp` | 大（+167/-31） | 回合字段拷贝/初始化、`TickTurn`、`GetRemainingTurns`、StackOverflow 修复、Cue N*N 修复 |
| `Public\AbilitySystemComponent.h` | 中 | 回合相关 UPROPERTY/UFUNCTION、回合委托、K2 版 GetGameplayEffectCDO |
| `Private\AbilitySystemComponent.cpp` | 中 | 实现回合接口、应用时长效果时广播回合委托 |
| `Public\AbilitySystemBlueprintLibrary.h` | 极小 | 仅 1 行空白 |
| `Private\AbilitySystemBlueprintLibrary.cpp` | 小 | 屏蔽 `GetInstancingPolicy()` 的废弃警告 |
| `Private\GameplayEffectAggregator.cpp` | 极小 | 仅 1 行空白 |

---

## 3. 逐项差异详解

### 3.1 回合制 Duration 效果系统（项目核心定制）

这是整套改动的**主线**，目标是在原生"无限时长(Periodic)+手动移除"的时长效果基础上，支持**按回合结算并自动到期**的 Buff/Debuff。

#### (a) 新增枚举（`GameplayEffect.h`）

```cpp
//回合类型
UENUM(BlueprintType)
enum class ERoundType : uint8
{
    None = 0,
    TurnStart = (1 << 4),     // 开始结算
    TurnEnd  = (1 << 4) | 1,  // 结束结算
};

FORCEINLINE bool IsRoundTurn(ERoundType Type)
{
    return (static_cast<uint8>(Type) & 0xF0) == 0x10;
}

UENUM()
enum class ERoundApplyType : uint8
{
    Once = 0,                    // 只用一次
    EveryTurn = (1 << 4),        // 每次结算使用
    EveryTurnSkipFirst = (1 << 4) | 1, // 每次结算使用，但第一次不用
};

FORCEINLINE bool IsEveryTurn(ERoundApplyType Type)
{
    return (static_cast<uint8>(Type) & 0xF0) == 0x10;
}
```

> 用高位 `0x10` 位掩码区分"回合类型/是否每回合"，低位记录具体变体——类似 UE 用 UHT 标志位的习惯写法。

#### (b) `UGameplayEffect` 数据资产上新增 3 个可配置字段

```cpp
//回合类型
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Duration|Round",
          meta=(EditCondition="DurationPolicy == EGameplayEffectDurationType::Infinite", EditConditionHides))
ERoundType RoundType = ERoundType::None;

//回合数
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Duration|Round",
          meta=(EditCondition="RoundType != ERoundType::None", EditConditionHides))
int32 RoundNum = 0;

//是否每次结算都应用
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Duration|Round",
          meta=(EditCondition="RoundType != ERoundType::None", EditConditionHides))
ERoundApplyType RoundApplyType = ERoundApplyType::Once;
```

- 仅在 `DurationPolicy == Infinite` 时显示（`EditCondition`），即"回合效果"建模为无限时长效果。
- `RoundType` 决定它在 `TurnStart` 还是 `TurnEnd` 时被结算。

#### (c) `FGameplayEffectSpec` 增加字段并在构造/赋值/初始化时同步

```cpp
UPROPERTY()
ERoundType RoundType = ERoundType::None;
UPROPERTY()
int32 RoundNum = 0;
UPROPERTY()
ERoundApplyType RoundApplyType = ERoundApplyType::Once;
```

在以下 4 处补充了这 3 个字段的传递，保证 Spec 被 Move/拷贝时回合信息不丢失：
- 移动构造函数 `FGameplayEffectSpec(FGameplayEffectSpec&&)`
- 移动赋值 `operator=(FGameplayEffectSpec&&)`
- 拷贝赋值 `operator=(const FGameplayEffectSpec&)`
- `Initialize(const UGameplayEffect* InDef, ...)`：从 `Def` 上拷贝 `RoundType/RoundNum/RoundApplyType`

#### (d) `FActiveGameplayEffectsContainer` 新增两个回合调度接口

```cpp
UE_API void TickTurn(const ERoundType Round);            // 某一回合来临时统一结算
UE_API int32 GetRemainingTurns(TSubclassOf<UGameplayEffect> EffectClass);

// 并把 FindStackableActiveGameplayEffect 从 private 挪到 public
UE_API FActiveGameplayEffect* FindStackableActiveGameplayEffect(const FGameplayEffectSpec& Spec);
```

`TickTurn` 核心逻辑（见 `GameplayEffect.cpp`）：

```cpp
void FActiveGameplayEffectsContainer::TickTurn(const ERoundType Round)
{
    TArray<FActiveGameplayEffectHandle> RemoveHandles;
    for (FActiveGameplayEffect& Effect : this)
    {
        if (Effect.Spec.RoundType == Round)          // 只结算匹配该回合类型的效果
        {
            Effect.Spec.RoundNum -= 1;               // 剩余回合 -1
            if (Owner) Owner->OnTurnNumChange.Broadcast(Effect.Handle, Effect.Spec.RoundNum);
            if (IsEveryTurn(Effect.Spec.RoundApplyType))
                ExecuteActiveEffectsFrom(Effect.Spec); // EveryTurn 类每回合重新执行一次 Modifier/副作用
            if (Effect.Spec.RoundNum <= 0)
                RemoveHandles.Add(Effect.Handle);    // 到期移除
        }
    }
    for (auto RemoveHandle : RemoveHandles)
        RemoveActiveGameplayEffect(RemoveHandle, -1);
}
```

`GetRemainingTurns` 遍历容器按 `Def == EffectClass.GetDefaultObject()` 命中返回剩余回合。

#### (e) 与 Stack / Modifier 注册的联动

- `AddActiveGameplayEffectGrantedTagsAndModifiers`：当效果为 `RoundType` 且 `RoundApplyType == EveryTurnSkipFirst` 时，**首次应用不注册 Modifier**（只在后续每回合结算时生效）。
- `InternalExecutePeriodicGameplayEffect` 附近的 StackCount 变更处：`EveryTurnSkipFirst` 的效果跳过 `OnStackCountChange` 回调。
- 二者配合实现"首回合空过、之后每回合生效"的持续伤害/持续增益语义。

### 3.2 `UAbilitySystemComponent` 侧回合接口与委托

新增公开接口（`AbilitySystemComponent.h`）：

```cpp
// 回合相关（BlueprintAssignable 委托）
UPROPERTY(BlueprintAssignable)
FOnTurnNumChangeDelegate OnTurnNumChange;          // 回合数变化
UPROPERTY(BlueprintAssignable)
FOnApplyDurationEffectDelegate OnApplyDurationEffect; // 应用时长效果

UFUNCTION(BlueprintCallable) ERoundType GetRoundTypeByEffectHandle(FActiveGameplayEffectHandle Handle);
UFUNCTION(BlueprintCallable) static ERoundType GetRoundTypeByEffectClass(TSubclassOf<UGameplayEffect> EffectClass);
UFUNCTION(BlueprintCallable) void TickTurnEffect(const ERoundType Round);   // 转调容器 TickTurn
UFUNCTION(BlueprintCallable) int32 GetRemainingTurns(TSubclassOf<UGameplayEffect> EffectClass);
UFUNCTION(BlueprintCallable) int32 GetRemainingTurnsByHandle(FActiveGameplayEffectHandle Handle);
UFUNCTION(BlueprintCallable) void AddRemainingTurnsByHandle(FActiveGameplayEffectHandle Handle, int32 AddTurns = 1);
```

配套新委托声明（放在文件顶部现有委托声明旁）：

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTurnNumChangeDelegate, FActiveGameplayEffectHandle, Handle, int32, Num);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnApplyDurationEffectDelegate, FActiveGameplayEffectHandle, Handle, ERoundType, RoundType, int32, RoundNum);
```

`ApplyGameplayEffectSpecToSelf` 内在成功应用一个效果后新增广播：

```cpp
if (IsRoundTurn(AppliedEffect->Spec.RoundType))
    OnTurnNumChange.Broadcast(AppliedEffect->Handle, AppliedEffect->Spec.RoundNum);
OnApplyDurationEffect.Broadcast(AppliedEffect->Handle, AppliedEffect->Spec.RoundType, AppliedEffect->Spec.RoundNum);
```

`AbilitySystemComponent.cpp` 还顺带删掉了一个无用的 include：`#include "UObject/UObjectMigrationContext.h"`。

> 这些接口纯属给游戏侧（蓝图层）驱动的"回合系统逻辑"，与原生 GAS 无关，需由外部（如回合管理器）在每个回合开始/结束时调用 `TickTurnEffect`。

### 3.3 额外暴露的 Blueprint 便利函数

在 `AbilitySystemComponent.h` 新增一个 K2 版查询（引擎只有 C++ 版）：

```cpp
UFUNCTION(BlueprintCallable, DisplayName = "GetGameplayEffect")
UE_API const UGameplayEffect* K2_GetGameplayEffectCDO(const FActiveGameplayEffectHandle Handle) const;
// 实现即转发原生 GetGameplayEffectCDO
```

### 3.4 BUG 修复：GameplayCue `N*N` 重复触发（作者 `[LiuYang] 2026/07/28`）

**位置**：`GameplayEffect.cpp` 两处——`AddActiveGameplayEffectGrantedTagsAndModifiers` 内 `OnActive/WhileActive` 的预测路径，以及移除时的 `Removed` 路径。

**现象（原引擎代码）**：外层已 `for` 遍历每个 Cue 并 `InvokeGameplayCueEvent(Effect.Spec, ...)`，而 `InvokeGameplayCueEvent(Spec)` 内部又会遍历 Spec 上的**全部 Cue**，导致 N 个 Cue 被触发 N×N 次。

**修复（项目代码）**：不再调用 `InvokeGameplayCueEvent`，改为**为当前单个 Cue** 手工构建 `FGameplayCueParameters` 后直接 `CueManager->HandleGameplayCues(...)`：

```cpp
// [LiuYang] 2026/07/28 修复 GameplayCue N*N 重复触发：
// 外层循环已遍历每个 Cue，改为直接为当前 Cue 构建参数调用 HandleGameplayCues，
// 避免 InvokeGameplayCueEvent(Spec) 内部再次遍历全部 Cue 导致 N*N 次触发。
FGameplayCueParameters CueParameters(Effect.Spec);
if (Cue.MagnitudeAttribute.IsValid()) {
    if (const FGameplayEffectModifiedAttribute* ModifiedAttribute = Effect.Spec.GetModifiedAttribute(Cue.MagnitudeAttribute))
        CueParameters.RawMagnitude = ModifiedAttribute->TotalMagnitude;
    else CueParameters.RawMagnitude = 0.0f;
} else CueParameters.RawMagnitude = 0.0f;
// 计算 NormalizedMagnitude
CueParameters.NormalizedMagnitude = (Range > KINDA_SMALL_NUMBER) ? ((Level - Cue.MinLevel) / Range) : 1.f;
// 设置 bGameplayEffectActive
FGameplayEffectQuery EffectQuery;
EffectQuery.EffectDefinition = Effect.Spec.Def->GetClass();
CueParameters.bGameplayEffectActive = Effect.Spec.Def->DurationPolicy == EGameplayEffectDurationType::Instant || GetActiveEffectCount(EffectQuery) > 0;
if (AActor* ActorAvatar = Owner->AbilityActorInfo.IsValid() ? Owner->AbilityActorInfo->AvatarActor.Get() : nullptr) {
    UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager();
    CueManager->HandleGameplayCues(ActorAvatar, Cue.GameplayCueTags, EGameplayCueEvent::Removed, CueParameters);
}
```

> 附带保证了 `MagnitudeAttribute`、`bGameplayEffectActive` 等参数被正确填充（原生 `InvokeGameplayCueEvent` 版本可能缺失这些信息）。

### 3.5 BUG 修复：StackOverflow 边界判定

**位置**：`GameplayEffect.cpp` → `HandleActiveGameplayEffectStackOverflow`。

原引擎逻辑（在无 StackLimit 即 `StackLimitCount == 0` 时，`bAtStackLimit` 会被误判为 `true`，导致"刷新到上限/走 Overflow 分支"的错误）：

```cpp
// 引擎
const bool bAtStackLimit = OldSpec.GetStackCount() == StackedGE->StackLimitCount;
```

修复为：先判断是否存在 Stack 上限，再判断是否达到上限；且 **OverflowEffects 只在真正到达 StackLimit 时才应用**（引擎在 `!bAtStackLimit && !bClearStackOnOverflow` 的刷新路径下也会应用，属越界触发）：

```cpp
const bool bHasStackLimit = StackedGE->StackLimitCount > 0;
const bool bAtStackLimit = bHasStackLimit && OldSpec.GetStackCount() >= StackedGE->StackLimitCount;
const bool bRefreshToLimit = !bAtStackLimit && !StackedGE->bClearStackOnOverflow && ...;
if (bAtStackLimit) {
    for (TSubclassOf<UGameplayEffect> OverflowEffect : StackedGE->OverflowEffects) { ... }
}
```

### 3.6 屏蔽 UE5.8 废弃警告

`AbilitySystemBlueprintLibrary.cpp` 中 `HasAnyAbilitiesByPredicate` 对调用 `GetInstancingPolicy()` 的代码用 `PRAGMA_DISABLE_DEPRECATION_WARNINGS` / `PRAGMA_ENABLE_DEPRECATION_WARNINGS` 包裹（规避 UE5.8 对该方法的废弃标记，保持向后兼容）。

### 3.7 纯空白差异（无逻辑影响）

- `AbilitySystemBlueprintLibrary.h`：类末尾新增 1 个空行。
- `GameplayEffectAggregator.cpp`：`AddAggregatorMod` 内删/增 1 个空行。

### 3.8 新增 Async Action：等待 GameplayTag 增删

**文件**：`Public\Abilities\Async\AbilityAsync_WaitGameplayTagNotify.h` / `.cpp`（引擎无此文件）。

实现两个派生自 `UAbilityAsync` 的类，可直接放进 Ability 蓝图异步等待标签变化，且像其他 `UAbilityAsync` 一样在 Ability 结束后仍保持存活：

| 类 | 静态工厂 | 触发事件 | 说明 |
|----|---------|---------|------|
| `UAbilityAsync_WaitGameplayTagRemovedNotify` | `WaitGameplayTagRemoveNotifyFromActor(Target, Tag, bOnlyTriggerOnce)` | `OnRemoved(Tag, Count)` | 标签移除时回调；开始时若标签不存在则立即回调 |
| `UAbilityAsync_WaitGameplayTagAddedNotify` | `WaitGameplayTagAddNotifyToActor(Target, Tag, bOnlyTriggerOnce)` | `OnAdded(Tag, Count)` | 标签添加时回调；开始时若标签已存在则立即回调 |

实现要点（注释里写得很清楚）：
- 用 `ASC->RegisterGenericGameplayTagEvent()` 监听**全部标签变化**，再用 `MatchesTag(Tag)` 过滤——以便捕获"注册父标签但实际被移除/添加的是其具体子标签"的情况。
- **移除侧**：GAS 移除标签会对"自身+所有父级"各广播一次（顺序为最深优先），通过 `LastProcessedTag` 记录上一次广播，若本次 `InTag` 是上次的父级则视为同一移除链条的连带广播而跳过，保证一个标签只触发一次。
- **添加侧**：通过 `ASC->GetOwnedGameplayTags().HasTagExact(InTag)` 判断是否为真正显式添加的标签（父级不会进 ExplicitTags），避免父级连带广播误触发。
- `EndAction()` 正确 `Remove` 掉监听句柄。

> 注意：这两类只依赖引擎已有 API（`RegisterGenericGameplayTagEvent` / `GetOwnedGameplayTags`），是**纯新增**，不改动引擎任何既有文件，属于低侵入扩展。

---

## 4. 引擎侧其他需留意的点

- 引擎源码中 `GameplayEffect.h` 的 `FindStackableActiveGameplayEffect` 声明在 `private:` 段，项目把它上移到了 `public:` 段（供外部查询 Stackable GE）。除该声明位置外，该方法签名与实现未变。
- 项目运行时模块中，`#include "UObject/UObjectMigrationContext.h"` 被移除（该头在 UE5.8 属重构遗留），属于版本兼容清理。

---

## 5. 工程/升级影响提示

1. **回合字段改动横跨 `UGameplayEffect` → `FGameplayEffectSpec` → `FActiveGameplayEffect`**，若要升级引擎或合并上游，需同步这三处 + 拷贝/移动构造 + `Initialize`，极易遗漏，建议以本文件 3.1(c) 清单为准。
2. `TickTurn` 迭代 `FActiveGameplayEffect` 时先收集再统一 `RemoveActiveGameplayEffect`，避免边遍历边删除导致迭代器失效。
3. Cue N*N 与 StackOverflow 两处修复**语义与上游不同**，若拿引擎源码回刷会被覆盖，需保留本地 patch。
4. 整个插件通过 git 处于 Untracked（`GameWork/Plugins/GameplayAbilities/`），建议纳入版本管理以便跟踪后续上游合并差异。

---

## 6. 涉及文件速查

| 文本中简称 | 项目相对路径（相对 `GameWork\Plugins\GameplayAbilities\`） |
|------|---------|
| `GameplayEffect.h/.cpp` | `Source\GameplayAbilities\Public\GameplayEffect.h` / `Private\GameplayEffect.cpp` |
| `AbilitySystemComponent.h/.cpp` | `Source\GameplayAbilities\Public\AbilitySystemComponent.h` / `Private\AbilitySystemComponent.cpp` |
| `AbilitySystemBlueprintLibrary.h/.cpp` | `Source\GameplayAbilities\Public\AbilitySystemBlueprintLibrary.h` / `Private\AbilitySystemBlueprintLibrary.cpp` |
| `GameplayEffectAggregator.cpp` | `Source\GameplayAbilities\Private\GameplayEffectAggregator.cpp` |
| `AbilityAsync_WaitGameplayTagNotify.h/.cpp` | `Source\GameplayAbilities\Public\Abilities\Async\AbilityAsync_WaitGameplayTagNotify.h` / `Private\Abilities\Async\AbilityAsync_WaitGameplayTagNotify.cpp` |
