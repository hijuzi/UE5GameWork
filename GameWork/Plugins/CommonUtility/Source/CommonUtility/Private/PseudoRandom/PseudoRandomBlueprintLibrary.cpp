// Copyright xiele. All Rights Reserved.

#include "PseudoRandom/PseudoRandomBlueprintLibrary.h"
#include "PseudoRandom/PRBPseudoRandomSubsystem.h"
#include "PseudoRandom/PseudoRandomLog.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UPRBPseudoRandomSubsystem* UPseudoRandomBlueprintLibrary::GetPseudoRandomSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogPseudoRandom, Warning, TEXT("[PseudoRandomBlueprintLibrary] WorldContextObject is null"));
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogPseudoRandom, Warning, TEXT("[PseudoRandomBlueprintLibrary] Failed to get World from context"));
		return nullptr;
	}

	const UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogPseudoRandom, Warning, TEXT("[PseudoRandomBlueprintLibrary] Failed to get GameInstance"));
		return nullptr;
	}

	return GameInstance->GetSubsystem<UPRBPseudoRandomSubsystem>();
}

// =============================================================
//  UObject Key API
// =============================================================

void UPseudoRandomBlueprintLibrary::PseudoRandomSetExpectedProbability(const UObject* WorldContextObject, UObject* Object, float InProbability)
{
	if (UPRBPseudoRandomSubsystem* Subsystem = GetPseudoRandomSubsystem(WorldContextObject))
	{
		Subsystem->SetExpectedProbability(Object, InProbability);
	}
}

bool UPseudoRandomBlueprintLibrary::PseudoRandomRoll(const UObject* WorldContextObject, UObject* Object)
{
	if (UPRBPseudoRandomSubsystem* Subsystem = GetPseudoRandomSubsystem(WorldContextObject))
	{
		return Subsystem->Roll(Object);
	}
	return false;
}

void UPseudoRandomBlueprintLibrary::PseudoRandomReset(const UObject* WorldContextObject, UObject* Object)
{
	if (UPRBPseudoRandomSubsystem* Subsystem = GetPseudoRandomSubsystem(WorldContextObject))
	{
		Subsystem->Reset(Object);
	}
}

float UPseudoRandomBlueprintLibrary::PseudoRandomGetCurrentProb(const UObject* WorldContextObject, const UObject* Object)
{
	if (UPRBPseudoRandomSubsystem* Subsystem = GetPseudoRandomSubsystem(WorldContextObject))
	{
		return Subsystem->GetCurrentProb(Object);
	}
	return 0.0f;
}

int32 UPseudoRandomBlueprintLibrary::PseudoRandomGetFailCount(const UObject* WorldContextObject, const UObject* Object)
{
	if (UPRBPseudoRandomSubsystem* Subsystem = GetPseudoRandomSubsystem(WorldContextObject))
	{
		return Subsystem->GetFailCount(Object);
	}
	return 0;
}

void UPseudoRandomBlueprintLibrary::PseudoRandomSetSeed(const UObject* WorldContextObject, UObject* Object, int32 Seed)
{
	if (UPRBPseudoRandomSubsystem* Subsystem = GetPseudoRandomSubsystem(WorldContextObject))
	{
		Subsystem->SetSeed(Object, Seed);
	}
}

void UPseudoRandomBlueprintLibrary::PseudoRandomRemoveObject(const UObject* WorldContextObject, UObject* Object)
{
	if (UPRBPseudoRandomSubsystem* Subsystem = GetPseudoRandomSubsystem(WorldContextObject))
	{
		Subsystem->RemoveObject(Object);
	}
}

// =============================================================
//  GameplayTag Key API
// =============================================================

void UPseudoRandomBlueprintLibrary::PseudoRandomSetExpectedProbabilityForTag(const UObject* WorldContextObject, FGameplayTag Tag, float InProbability)
{
	if (UPRBPseudoRandomSubsystem* Subsystem = GetPseudoRandomSubsystem(WorldContextObject))
	{
		Subsystem->SetExpectedProbabilityForTag(Tag, InProbability);
	}
}

bool UPseudoRandomBlueprintLibrary::PseudoRandomRollForTag(const UObject* WorldContextObject, FGameplayTag Tag)
{
	if (UPRBPseudoRandomSubsystem* Subsystem = GetPseudoRandomSubsystem(WorldContextObject))
	{
		return Subsystem->RollForTag(Tag);
	}
	return false;
}

void UPseudoRandomBlueprintLibrary::PseudoRandomResetForTag(const UObject* WorldContextObject, FGameplayTag Tag)
{
	if (UPRBPseudoRandomSubsystem* Subsystem = GetPseudoRandomSubsystem(WorldContextObject))
	{
		Subsystem->ResetForTag(Tag);
	}
}

float UPseudoRandomBlueprintLibrary::PseudoRandomGetCurrentProbForTag(const UObject* WorldContextObject, FGameplayTag Tag)
{
	if (UPRBPseudoRandomSubsystem* Subsystem = GetPseudoRandomSubsystem(WorldContextObject))
	{
		return Subsystem->GetCurrentProbForTag(Tag);
	}
	return 0.0f;
}

int32 UPseudoRandomBlueprintLibrary::PseudoRandomGetFailCountForTag(const UObject* WorldContextObject, FGameplayTag Tag)
{
	if (UPRBPseudoRandomSubsystem* Subsystem = GetPseudoRandomSubsystem(WorldContextObject))
	{
		return Subsystem->GetFailCountForTag(Tag);
	}
	return 0;
}

void UPseudoRandomBlueprintLibrary::PseudoRandomSetSeedForTag(const UObject* WorldContextObject, FGameplayTag Tag, int32 Seed)
{
	if (UPRBPseudoRandomSubsystem* Subsystem = GetPseudoRandomSubsystem(WorldContextObject))
	{
		Subsystem->SetSeedForTag(Tag, Seed);
	}
}

void UPseudoRandomBlueprintLibrary::PseudoRandomRemoveTag(const UObject* WorldContextObject, FGameplayTag Tag)
{
	if (UPRBPseudoRandomSubsystem* Subsystem = GetPseudoRandomSubsystem(WorldContextObject))
	{
		Subsystem->RemoveTag(Tag);
	}
}
