// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/SVCombatFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CatCombatLog.h"
#include "Combat/Actor/SVCombatScenePoint.h"
#include "Combat/SVCombatDataStore.h"
#include "Combat/SVCombatDataTable.h"
#include "Combat/SVCombatManagerSubsystem.h"
#include "GameFramework/Character.h"
#include "Combat/SVCombatHUDHandler.h"
#include "Combat/SVCombatRoundTimer.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

bool USVCombatFunctionLibrary::StopPlayerAction()
{
	// 源项目依赖 SvPlayerController::CombatRoundController 停止玩家行动，框架层已解耦。
	// 此处返回 false 占位，新项目需停止玩家行动时可通过扩展点接入。
	return false;
}

bool USVCombatFunctionLibrary::TryStartBattle(const UObject* WorldContextObject, const FName CombatConfigRowName)
{
	// 预检：行名对应的场景点已注册且配置了有效数据行，才允许开始战斗
	//（双方角色生成统一在 DataStore::CreateCombatData 内按配置完成）
	if (!GetCombatConfigRowFromWorld(WorldContextObject, CombatConfigRowName))
	{
		return false;
	}

	if (USVCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject))
	{
		Subsystem->StartBattle(CombatConfigRowName);
		return true;
	}
	return false;
}

TArray<FName> USVCombatFunctionLibrary::GetCombatConfigRowNames()
{
	return ASVCombatScenePoint::GetCombatConfigRowNames();
}

const FSVCombatDataTableRow* USVCombatFunctionLibrary::GetCombatConfigRowFromWorld(const UObject* WorldContextObject, const FName CombatConfigRowName)
{
	USVCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject);
	if (!Subsystem)
	{
		return nullptr;
	}

	ASVCombatScenePoint* ScenePoint = Subsystem->GetScenePoint(CombatConfigRowName);
	if (!ScenePoint)
	{
		UE_LOG(LogCatCombatManager, Warning, TEXT("GetCombatConfigRowFromWorld: 未找到配置行名 [%s] 对应的场景点"), *CombatConfigRowName.ToString());
		return nullptr;
	}

	const FSVCombatDataTableRow* Row = ScenePoint->GetCombatConfigRow();
	if (!Row)
	{
		UE_LOG(LogCatCombatManager, Warning, TEXT("GetCombatConfigRowFromWorld: 场景点 %s 未配置有效的战斗数据行"), *ScenePoint->GetName());
		return nullptr;
	}
	return Row;
}

bool USVCombatFunctionLibrary::TryEndBattle(const UObject* WorldContextObject, bool bClearTeam)
{
	if (USVCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject))
	{
		return Subsystem->EndBattle(bClearTeam);
	}
	return false;
}

bool USVCombatFunctionLibrary::IsInCombat(const UObject* WorldContextObject)
{
	if (USVCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject))
	{
		if (USVCombatDataStore* DataStore = Subsystem->GetDataStore())
		{
			return DataStore->IsInCombat();
		}
	}
	return false;
}

void USVCombatFunctionLibrary::SetBattleHUDVisible(const UObject* WorldContextObject, bool bVisible, const FSVCombatHUDVisibilityParams& Params)
{
	if (USVCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject))
	{
		if (USVCombatHUDHandler* HUDHandler = Subsystem->GetHUDHandler())
		{
			HUDHandler->SetVisible(bVisible, Params);
		}

		// 自动联动暂停/恢复战斗回合（显示时恢复，隐藏时暂停）
		if (Params.bAutoPauseCombatRound)
		{
			if (USVCombatRoundTimer* RoundTimer = Subsystem->GetRoundTimer())
			{
				RoundTimer->SetPaused(!bVisible);
			}
		}
	}
}

void USVCombatFunctionLibrary::SetCombatRoundPaused(const UObject* WorldContextObject, bool bPaused)
{
	if (USVCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject))
	{
		if (USVCombatRoundTimer* RoundTimer = Subsystem->GetRoundTimer())
		{
			RoundTimer->SetPaused(bPaused);
		}
	}
}

void USVCombatFunctionLibrary::AddCombatCharacter(const UObject* WorldContextObject, ECombatTeamType TeamType, ACharacter* Character)
{
	if (USVCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject))
	{
		if (USVCombatDataStore* DataStore = Subsystem->GetDataStore())
		{
			DataStore->AddCharacter(TeamType, Character);
		}
		// 输入映射由 Subsystem 监听 OnCombatCharacterChanged 自动处理
	}
}

void USVCombatFunctionLibrary::RemoveCombatCharacter(const UObject* WorldContextObject, ECombatTeamType TeamType, ACharacter* Character)
{
	if (USVCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject))
	{
		if (USVCombatDataStore* DataStore = Subsystem->GetDataStore())
		{
			DataStore->RemoveCharacter(TeamType, Character);
			// 输入映射由 Subsystem 监听 OnCombatCharacterChanged 自动处理
		}
	}
}

TArray<ACharacter*> USVCombatFunctionLibrary::GetCombatCharacterList(const UObject* WorldContextObject, ECombatTeamType TeamType)
{
	if (USVCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject))
	{
		if (USVCombatDataStore* DataStore = Subsystem->GetDataStore())
		{
			return DataStore->GetCharacters(TeamType);
		}
	}
	return TArray<ACharacter*>();
}

ACharacter* USVCombatFunctionLibrary::GetMainPlayerCombatCharacter(const UObject* WorldContextObject)
{
	const TArray<ACharacter*> PlayerCharacters = GetCombatCharacterList(WorldContextObject, ECombatTeamType::Player);
	return PlayerCharacters.Num() > 0 ? PlayerCharacters[0] : nullptr;
}

ACharacter* USVCombatFunctionLibrary::GetMainEnemyCombatCharacter(const UObject* WorldContextObject)
{
	const TArray<ACharacter*> EnemyCharacters = GetCombatCharacterList(WorldContextObject, ECombatTeamType::Enemy);
	return EnemyCharacters.Num() > 0 ? EnemyCharacters[0] : nullptr;
}

USVCombatManagerSubsystem* USVCombatFunctionLibrary::GetCombatManagerSubsystem(const UObject* WorldContextObject)
{
	if (const UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		return GI->GetSubsystem<USVCombatManagerSubsystem>();
	}
	return nullptr;
}

bool USVCombatFunctionLibrary::RequestAction(ACharacter* Actor, const FActionRequest& Request)
{
	if (!IsValid(Actor))
	{
		UE_LOG(LogCatCombatManager, Warning, TEXT("RequestAction: 行动者为空"));
		return false;
	}

	if (!Request.AbilityTag.IsValid())
	{
		UE_LOG(LogCatCombatManager, Warning, TEXT("RequestAction: 行动者 %s 的 AbilityTag 无效"), *Actor->GetName());
		return false;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
	if (!ASC)
	{
		UE_LOG(LogCatCombatManager, Warning, TEXT("RequestAction: 行动者 %s 无有效 ASC"), *Actor->GetName());
		return false;
	}

	// 遍历可激活能力，找到 AssetTags 含指定 AbilityTag 的并激活。
	// 目标不在此处传递：由输入/AI 桥接在激活前写入角色攻击目标，
	// 能力激活后自行从角色读取目标，与项目现有结算链一致。
	bool bSuccess = false;
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetAssetTags().HasTag(Request.AbilityTag))
		{
			bSuccess = ASC->TryActivateAbility(Spec.Handle, true);
			break;
		}
	}

	UE_LOG(LogCatCombatManager, Log, TEXT("RequestAction: 行动者=%s, Tag=%s, 激活=%s"),
		*Actor->GetName(), *Request.AbilityTag.ToString(), bSuccess ? TEXT("成功") : TEXT("失败"));

	return bSuccess;
}
