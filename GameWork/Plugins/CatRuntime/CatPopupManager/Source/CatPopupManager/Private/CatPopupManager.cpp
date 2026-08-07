// Copyright SegameVictory Team. All Rights Reserved.

#include "CatPopupManager.h"
#include "Engine/GameInstance.h"

void UCatPopupManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UCatPopupManager::Deinitialize()
{
	Super::Deinitialize();
}

UCatPopupManager* UCatPopupManager::GetInstance(const UObject* WorldContextObject)
{
	if (const UGameInstance* GameInstance = UGameInstance::GetGameInstance(WorldContextObject))
	{
		return GameInstance->GetSubsystem<UCatPopupManager>();
	}
	return nullptr;
}
