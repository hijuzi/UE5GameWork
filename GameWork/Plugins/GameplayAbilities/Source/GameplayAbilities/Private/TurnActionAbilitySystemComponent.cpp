// Copyright Epic Games, Inc. All Rights Reserved.

#include "TurnActionAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilityTimerManager.h"
#include "AbilityTimingTags.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"

//=====================================================================
// ===== [GAS_MOD_16] START=====
// 新增文件：回合制专用 ASC 子类实现（TurnAction）
// 推进入口为 UTurnActionAbilitySystemComponent::TickTimeline（唯一入口，见头文件说明）。
//=====================================================================

UTurnActionAbilitySystemComponent::UTurnActionAbilitySystemComponent()
{
	// 「类型即语义」：使用本类即表示该 ASC 走回合制（Duration / Period 按时机刻度推进）
	bTurnBased = true;

	// 刻度副本需要复制到客户端，显式声明一次（避免依赖基类的默认值）
	SetIsReplicatedByDefault(true);
}

UTurnActionAbilitySystemComponent* UTurnActionAbilitySystemComponent::GetTurnActionAbilitySystemComponent(AActor* Actor)
{
	// 等价于 Cast<UTurnActionAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))：
	// 直接走 UAbilitySystemGlobals 的查找（IAbilitySystemInterface → 组件搜索），少一次蓝图库跳转。
	return Cast<UTurnActionAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor));
}

void UTurnActionAbilitySystemComponent::OnRegister()
{
	Super::OnRegister();

	// OnRegister 可能被多次调用（重注册），避免重复绑定。
	// 注意：以下两个原生委托都是 public 成员/访问器，绑定无需改动引擎源码。
	if (bBoundDelegates)
	{
		return;
	}
	bBoundDelegates = true;

	// GE 挂载：该回调在「抑制恢复」时也会触发，由 HandleActiveEffectAdded 内部按 Handle 去重
	OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &UTurnActionAbilitySystemComponent::HandleActiveEffectAdded);

	// GE 卸载：一次性绑定（广播发生在 Timer 清理之前，仍可查剩余刻数以判定「提前移除」）
	OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &UTurnActionAbilitySystemComponent::HandleActiveEffectRemoved);
}

void UTurnActionAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 刻度副本：所有能看到本 ASC 的客户端都需要（UI 显示「第几回合 / 还剩几回合」）
	DOREPLIFETIME(UTurnActionAbilitySystemComponent, TimelineCounters);
}

// ---------------------------------------------------------------
// 推进入口（唯一）
// ---------------------------------------------------------------

void UTurnActionAbilitySystemComponent::TickTimeline(FGameplayTag Timing, int32 Delta)
{
	if (!Timing.IsValid() || Delta <= 0)
	{
		return;
	}

	FAbilityTimerManager& TimerManager = UAbilitySystemGlobals::Get().GetAbilityTimerManager();

	// ① 真正推进。Timing 可能同时命中多条轴（父级聚合），例如 Action.Attack 会同时推进
	//    Action.Attack 轴与父级 Action 轴。
	TimerManager.TickTimeline(this, Timing, Delta);

	// ② 客户端：本次推进的可见性完全由 OnRep_TimelineCounters 提供
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	// ③ 确保被推进的时机在副本里有条目（首次推进该轴时），否则客户端看不到这条轴
	FindOrAddTimelineCounter(Timing);

	// ④ 逐条命中轴同步刻度并广播（与 FAbilityTimerManager 的匹配规则保持一致）
	for (FTurnTimelineCounter& Entry : TimelineCounters)
	{
		if (!Timing.MatchesTag(Entry.Timing))
		{
			continue;
		}

		const int32 NewCounter = TimerManager.GetTimelineCounter(this, Entry.Timing);
		Entry.Counter = NewCounter;

		OnTimelineTicked.Broadcast(this, Entry.Timing, NewCounter);
	}
}

// ---------------------------------------------------------------
// 查询
// ---------------------------------------------------------------

int32 UTurnActionAbilitySystemComponent::GetTimelineRound(FGameplayTag Timing) const
{
	if (!Timing.IsValid())
	{
		return 0;
	}

	// 权威端：刻度由 FAbilityTimerManager 维护，直接查最准
	if (IsOwnerActorAuthoritative())
	{
		return UAbilitySystemGlobals::Get().GetAbilityTimerManager().GetTimelineCounter(
			const_cast<UTurnActionAbilitySystemComponent*>(this), Timing);
	}

	// 客户端：读复制副本
	for (const FTurnTimelineCounter& Entry : TimelineCounters)
	{
		if (Entry.Timing == Timing)
		{
			return Entry.Counter;
		}
	}

	return 0;
}

FGameplayTag UTurnActionAbilitySystemComponent::GetGameplayEffectTiming(FActiveGameplayEffectHandle Handle) const
{
	const FActiveGameplayEffect* ActiveGE = GetActiveGameplayEffect(Handle);
	if (!ActiveGE || !ActiveGE->Spec.Def)
	{
		return FGameplayTag();
	}

	return AbilityTimingTags::ResolveOrDefault(ActiveGE->Spec.Def->Timing);
}

int32 UTurnActionAbilitySystemComponent::GetGameplayEffectRemainingTicks(FActiveGameplayEffectHandle Handle) const
{
	const FActiveGameplayEffect* ActiveGE = GetActiveGameplayEffect(Handle);
	if (!ActiveGE || !ActiveGE->Spec.Def)
	{
		return -1;
	}

	const FGameplayTag Timing = AbilityTimingTags::ResolveOrDefault(ActiveGE->Spec.Def->Timing);

	// 权威端：直接问管理器
	if (IsOwnerActorAuthoritative())
	{
		// 没有 Duration Timer ⇒ 无限期（Infinite）
		if (!ActiveGE->DurationHandle.IsValid())
		{
			return -1;
		}

		FTimerHandle DurationHandle = ActiveGE->DurationHandle;
		const float Remaining = UAbilitySystemGlobals::Get().GetAbilityTimerManager().GetAbilityTimerRemaining(
			const_cast<UTurnActionAbilitySystemComponent*>(this), Timing, DurationHandle);

		return FMath::Max(0, FMath::CeilToInt(Remaining));
	}

	// 客户端：总刻数（回合制下 Spec.GetDuration() 即刻度数）- 已存活刻数
	const int32* StartCounter = EffectStartCounters.Find(Handle);
	if (!StartCounter)
	{
		return -1;
	}

	const int32 TotalTicks = FMath::RoundToInt(ActiveGE->Spec.GetDuration());
	if (TotalTicks <= 0)
	{
		return -1;	// Infinite(-1) 或无效
	}

	return FMath::Max(0, TotalTicks - (GetTimelineRound(Timing) - *StartCounter));
}

int32 UTurnActionAbilitySystemComponent::GetGameplayEffectElapsedTicks(FActiveGameplayEffectHandle Handle) const
{
	const int32* StartCounter = EffectStartCounters.Find(Handle);
	if (!StartCounter)
	{
		return -1;
	}

	const FGameplayTag Timing = GetGameplayEffectTiming(Handle);
	if (!Timing.IsValid())
	{
		return -1;
	}

	return FMath::Max(0, GetTimelineRound(Timing) - *StartCounter);
}

// ---------------------------------------------------------------
// 原生委托 → 蓝图事件
// ---------------------------------------------------------------

void UTurnActionAbilitySystemComponent::HandleActiveEffectAdded(UAbilitySystemComponent* SourceASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle)
{
	if (!Spec.Def)
	{
		return;
	}

	const FGameplayTag Timing = AbilityTimingTags::ResolveOrDefault(Spec.Def->Timing);

	// 该原生回调在「抑制恢复」时也会被调用（新 ActiveGE 初始 bIsInhibited = true，
	// 随后由 SetActiveGameplayEffectInhibit 激活），因此按 Handle 去重：
	//   首次 = 挂载；后续 = 重新激活。
	const bool bIsReactivation = KnownEffectHandles.Contains(Handle);
	KnownEffectHandles.Add(Handle);

	if (!bIsReactivation)
	{
		EffectStartCounters.Add(Handle, GetTimelineRound(Timing));

		// 权威端把该轴登记进复制副本；客户端不需要（由复制得到）
		if (IsOwnerActorAuthoritative())
		{
			FindOrAddTimelineCounter(Timing);
		}
	}

	FTurnBasedEffectEvent Event;
	Event.Handle = Handle;
	Event.GameplayEffectClass = TSubclassOf<UGameplayEffect>(Spec.Def->GetClass());
	Event.Timing = Timing;
	Event.TimelineCounter = GetTimelineRound(Timing);
	Event.StartCounter = EffectStartCounters.FindRef(Handle);
	Event.bIsReactivation = bIsReactivation;

	OnTurnBasedEffectAdded.Broadcast(this, Event);
}

void UTurnActionAbilitySystemComponent::HandleActiveEffectRemoved(const FActiveGameplayEffect& Effect)
{
	if (!Effect.Spec.Def)
	{
		return;
	}

	const FActiveGameplayEffectHandle Handle = Effect.Handle;
	const FGameplayTag Timing = AbilityTimingTags::ResolveOrDefault(Effect.Spec.Def->Timing);

	FTurnBasedEffectEvent Event;
	Event.Handle = Handle;
	Event.GameplayEffectClass = TSubclassOf<UGameplayEffect>(Effect.Spec.Def->GetClass());
	Event.Timing = Timing;
	Event.TimelineCounter = GetTimelineRound(Timing);
	Event.StartCounter = EffectStartCounters.FindRef(Handle);

	// 本回调在 Timer 清理之前触发（见 InternalRemoveActiveGameplayEffect → InternalOnActiveGameplayEffectRemoved），
	// 因此此刻仍可查剩余刻数：remaining > 0 即「未到期就被移除」= 提前移除。
	// 注意：此处不能走 GetGameplayEffectRemainingTicks —— 它内部用 GetActiveGameplayEffect 查表，
	//       而此刻 Effect 已被标记 IsPendingRemove，会被跳过而返回 -1。
	//       另外客户端拿不到刻度，bPrematureRemoval 恒为 false。
	if (IsOwnerActorAuthoritative() && Effect.DurationHandle.IsValid())
	{
		FTimerHandle DurationHandle = Effect.DurationHandle;
		const float Remaining = UAbilitySystemGlobals::Get().GetAbilityTimerManager().GetAbilityTimerRemaining(
			this, Timing, DurationHandle);

		Event.bPrematureRemoval = (Remaining > 0.f);
	}

	OnTurnBasedEffectRemoved.Broadcast(this, Event);

	// 登记清理（FActiveGameplayEffect 会被容器原地复用，必须清干净）
	KnownEffectHandles.Remove(Handle);
	EffectStartCounters.Remove(Handle);
}

// ---------------------------------------------------------------
// 内部工具 / 复制回调
// ---------------------------------------------------------------

FTurnTimelineCounter& UTurnActionAbilitySystemComponent::FindOrAddTimelineCounter(FGameplayTag Timing)
{
	for (FTurnTimelineCounter& Entry : TimelineCounters)
	{
		if (Entry.Timing == Timing)
		{
			return Entry;
		}
	}

	FTurnTimelineCounter& NewEntry = TimelineCounters.AddDefaulted_GetRef();
	NewEntry.Timing = Timing;
	NewEntry.Counter = 0;
	return NewEntry;
}

void UTurnActionAbilitySystemComponent::OnRep_TimelineCounters()
{
	// 首次收到（加入 / 初始复制）只登记镜像，不广播「推进」事件——那不是真的推进。
	if (!bReceivedInitialCounters)
	{
		bReceivedInitialCounters = true;
		LastReplicatedCounters.Reset();
		for (const FTurnTimelineCounter& Entry : TimelineCounters)
		{
			LastReplicatedCounters.Add(Entry.Timing, Entry.Counter);
		}
		return;
	}

	// diff 出被推进的轴并广播，使客户端行为与权威端一致
	for (const FTurnTimelineCounter& Entry : TimelineCounters)
	{
		const int32* PreviousCounter = LastReplicatedCounters.Find(Entry.Timing);
		if (!PreviousCounter || *PreviousCounter != Entry.Counter)
		{
			LastReplicatedCounters.Add(Entry.Timing, Entry.Counter);
			OnTimelineTicked.Broadcast(this, Entry.Timing, Entry.Counter);
		}
	}
}

// ===== [GAS_MOD_16] END =====
//=====================================================================
