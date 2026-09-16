// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/CatCombatHUDHandler.h"

#include "Combat/CatCombatManagerSubsystem.h"
#include "Combat/CatCombatSettings.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Blueprint/UserWidget.h"
#include "CatCombatLog.h"

UWorld* UCatCombatHUDHandler::GetWorld() const
{
	return OwnerSubsystem.IsValid() ? OwnerSubsystem->GetWorld() : nullptr;
}

void UCatCombatHUDHandler::Initialize(UCatCombatManagerSubsystem* InOwner)
{
	OwnerSubsystem = InOwner;
}

void UCatCombatHUDHandler::Shutdown()
{
	DestroyHUD();
	OwnerSubsystem = nullptr;
}

void UCatCombatHUDHandler::CreateHUD()
{
	if (StreamingHandle.IsValid())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const TSoftClassPtr<UCatCombatHUDLayout> HUDClass = GetDefault<UCatCombatSettings>()->CombatHUDLayoutClass;
	if (HUDClass.IsNull())
	{
		UE_LOG(LogCatCombatHUD, Warning, TEXT("CreateCombatHUD: CombatHUDLayoutClass 未配置，无法创建战斗 HUD"));
		return;
	}

	TWeakObjectPtr<UCatCombatHUDHandler> WeakThis(this);
	StreamingHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		HUDClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateWeakLambda(this, [WeakThis, HUDClass]()
		{
			UCatCombatHUDHandler* ThisPtr = WeakThis.Get();
			if (!ThisPtr)
			{
				return;
			}
			ThisPtr->StreamingHandle.Reset();

			UWorld* World = ThisPtr->GetWorld();
			if (!World)
			{
				return;
			}

			UClass* LoadedClass = HUDClass.Get();
			if (!LoadedClass)
			{
				UE_LOG(LogCatCombatHUD, Warning, TEXT("CreateCombatHUD: CombatHUDLayoutClass 加载失败"));
				return;
			}

			UCatCombatHUDLayout* Widget = CreateWidget<UCatCombatHUDLayout>(World->GetFirstPlayerController(), LoadedClass);
			if (!Widget)
			{
				UE_LOG(LogCatCombatHUD, Warning, TEXT("CreateCombatHUD: 创建 CombatHUD Widget 失败"));
				return;
			}

			Widget->AddToViewport(0);
			Widget->SetVisibility(ESlateVisibility::Collapsed);
			ThisPtr->SpawnedHUD = Widget;
			UE_LOG(LogCatCombatHUD, Log, TEXT("CreateCombatHUD: CombatHUD 已创建并加入 Viewport（隐藏）"));
		}));
}

void UCatCombatHUDHandler::DestroyHUD()
{
	if (StreamingHandle.IsValid())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}

	if (SpawnedHUD)
	{
		SpawnedHUD->RemoveFromParent();
		SpawnedHUD = nullptr;
		UE_LOG(LogCatCombatHUD, Log, TEXT("DestroyCombatHUD: CombatHUD removed"));
	}
}

void UCatCombatHUDHandler::SetVisible(const bool bVisible, const FCatCombatHUDVisibilityParams& Params) const
{
	if (!SpawnedHUD)
	{
		return;
	}

	if (IsHUDActive() == bVisible)
	{
		return;
	}

	if (bVisible)
	{
		if (Params.bPlayLoadUnloadAnimation)
		{
			// 进场动画
			SpawnedHUD->PlayLoadAnimation();
		}
		if (Params.bPlayVisibilityAnimation)
		{
			SpawnedHUD->PlayShowAnimation();
		}
		if (!Params.bPlayLoadUnloadAnimation && !Params.bPlayVisibilityAnimation)
		{
			SpawnedHUD->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
	else
	{
		if (Params.bPlayLoadUnloadAnimation)
		{
			// 出场动画
			SpawnedHUD->PlayUnloadAnimation();
		}
		if (Params.bPlayVisibilityAnimation)
		{
			SpawnedHUD->PlayHideAnimation();
		}
		if (!Params.bPlayLoadUnloadAnimation && !Params.bPlayVisibilityAnimation)
		{
			SpawnedHUD->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	UE_LOG(LogCatCombatHUD, Log, TEXT("SetBattleHUDVisible: %s"), bVisible ? TEXT("Visible") : TEXT("Hidden"));
}

bool UCatCombatHUDHandler::IsHUDActive() const
{
	return SpawnedHUD != nullptr &&
		SpawnedHUD->GetVisibility() != ESlateVisibility::Collapsed &&
		SpawnedHUD->GetVisibility() != ESlateVisibility::Hidden;
}

void UCatCombatHUDHandler::ShowHUD()
{
	FCatCombatHUDVisibilityParams Params;
	Params.bPlayLoadUnloadAnimation = false;
	Params.bAutoPauseCombatRound = false;
	SetVisible(true, Params);
}

void UCatCombatHUDHandler::HideHUD()
{
	FCatCombatHUDVisibilityParams Params;
	Params.bPlayLoadUnloadAnimation = false;
	Params.bAutoPauseCombatRound = false;
	SetVisible(false, Params);
}
