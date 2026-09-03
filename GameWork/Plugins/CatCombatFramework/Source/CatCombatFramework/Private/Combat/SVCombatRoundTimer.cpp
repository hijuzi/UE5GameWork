// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/SVCombatRoundTimer.h"

#include "Combat/SVCombatManagerSubsystem.h"
#include "Combat/SVCombatSettings.h"
#include "Kismet/GameplayStatics.h"
#include "CatCombatLog.h"

UWorld* USVCombatRoundTimer::GetWorld() const
{
	return OwnerSubsystem.IsValid() ? OwnerSubsystem->GetWorld() : nullptr;
}

void USVCombatRoundTimer::Initialize(USVCombatManagerSubsystem* InOwner)
{
	OwnerSubsystem = InOwner;
	// 缓存单回合最大时长，避免每帧 GetDefault<USVCombatSettings>() 查询 CDO
	MaxRoundTime = GetDefault<USVCombatSettings>()->MaxRoundTime;
}

void USVCombatRoundTimer::Shutdown()
{
	RemoveTick();
	OwnerSubsystem = nullptr;
}

void USVCombatRoundTimer::StartRoundTiming()
{
	ResetRoundTime();
	bPaused = false;
	RegisterTick();
}

void USVCombatRoundTimer::StopRoundTiming()
{
	RemoveTick();
}

void USVCombatRoundTimer::ResetRoundTime()
{
	CurrentRoundTime = 0.0f;
	bTimeExpired = false;
	// 保留 bPaused，暂停态由外部显式控制
}

void USVCombatRoundTimer::SetPaused(bool bInPaused)
{
	bPaused = bInPaused;
}

float USVCombatRoundTimer::GetProgress() const
{
	if (bTimeExpired || MaxRoundTime <= 0.0f)
	{
		return 1.0f;
	}
	return FMath::Min(CurrentRoundTime / MaxRoundTime, 1.0f);
}

void USVCombatRoundTimer::RegisterTick()
{
	if (!TickHandle.IsValid())
	{
		TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &USVCombatRoundTimer::Tick));
	}
}

void USVCombatRoundTimer::RemoveTick()
{
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}
}

bool USVCombatRoundTimer::Tick(float DeltaTime)
{
	// 已超时：不再累计时间，但保持 Ticker 存活（切回合并重置 bTimeExpired 后自动恢复计时）
	if (bTimeExpired)
	{
		return true;
	}

	// 暂停：返回 true 继续 tick（暂停是临时状态，恢复后继续计时）
	if (bPaused)
	{
		return true;
	}

	if (UGameplayStatics::IsGamePaused(GetWorld()))
	{
		return true;
	}

	CurrentRoundTime += DeltaTime;
	CheckTimeFinished();
	return true;
}

void USVCombatRoundTimer::CheckTimeFinished()
{
	if (bTimeExpired)
	{
		return;
	}

	if (CurrentRoundTime >= MaxRoundTime)
	{
		bTimeExpired = true;
		OnRoundTimeExpired.Broadcast();
		UE_LOG(LogCatCombatManager, Log, TEXT("CombatRoundTimer: Round time expired, timer invalidated"));
	}
}
