// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CatCombatFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AI/CatAISkillDecisionData.h"
#include "CatCombatGameplayTags.h"
#include "CatCombatLog.h"
#include "Combat/Actor/CatCombatScenePoint.h"
#include "Combat/Component/CatCharacterTurnComponent.h"
#include "Combat/CatCombatDataStore.h"
#include "Combat/CatCombatDataTable.h"
#include "Combat/CatCombatManagerSubsystem.h"
#include "GameFramework/Character.h"
#include "Combat/CatCombatHUDHandler.h"
#include "Combat/CatCombatRoundTimer.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

bool UCatCombatFunctionLibrary::StopPlayerAction()
{
	// 源项目依赖 SvPlayerController::CombatRoundController 停止玩家行动，框架层已解耦。
	// 此处返回 false 占位，新项目需停止玩家行动时可通过扩展点接入。
	return false;
}

bool UCatCombatFunctionLibrary::TryStartBattle(const UObject* WorldContextObject, const FName CombatConfigRowName)
{
	// 预检：行名对应的场景点已注册且配置了有效数据行，才允许开始战斗
	//（双方角色生成统一在 DataStore::CreateCombatData 内按配置完成）
	if (!GetCombatConfigRowFromWorld(WorldContextObject, CombatConfigRowName))
	{
		return false;
	}

	if (UCatCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject))
	{
		Subsystem->StartBattle(CombatConfigRowName);
		return true;
	}
	return false;
}

TArray<FName> UCatCombatFunctionLibrary::GetCombatConfigRowNames()
{
	return ACatCombatScenePoint::GetCombatConfigRowNames();
}

const FCatCombatDataTableRow* UCatCombatFunctionLibrary::GetCombatConfigRowFromWorld(const UObject* WorldContextObject, const FName CombatConfigRowName)
{
	UCatCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject);
	if (!Subsystem)
	{
		return nullptr;
	}

	ACatCombatScenePoint* ScenePoint = Subsystem->GetScenePoint(CombatConfigRowName);
	if (!ScenePoint)
	{
		UE_LOG(LogCatCombatManager, Warning, TEXT("GetCombatConfigRowFromWorld: 未找到配置行名 [%s] 对应的场景点"), *CombatConfigRowName.ToString());
		return nullptr;
	}

	const FCatCombatDataTableRow* Row = ScenePoint->GetCombatConfigRow();
	if (!Row)
	{
		UE_LOG(LogCatCombatManager, Warning, TEXT("GetCombatConfigRowFromWorld: 场景点 %s 未配置有效的战斗数据行"), *ScenePoint->GetName());
		return nullptr;
	}
	return Row;
}

bool UCatCombatFunctionLibrary::TryEndBattle(const UObject* WorldContextObject, bool bClearTeam)
{
	if (UCatCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject))
	{
		return Subsystem->EndBattle(bClearTeam);
	}
	return false;
}

bool UCatCombatFunctionLibrary::IsInCombat(const UObject* WorldContextObject)
{
	if (UCatCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject))
	{
		if (UCatCombatDataStore* DataStore = Subsystem->GetDataStore())
		{
			return DataStore->IsInCombat();
		}
	}
	return false;
}

void UCatCombatFunctionLibrary::SetBattleHUDVisible(const UObject* WorldContextObject, bool bVisible, const FCatCombatHUDVisibilityParams& Params)
{
	if (UCatCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject))
	{
		if (UCatCombatHUDHandler* HUDHandler = Subsystem->GetHUDHandler())
		{
			HUDHandler->SetVisible(bVisible, Params);
		}

		// 自动联动暂停/恢复战斗回合（显示时恢复，隐藏时暂停）
		if (Params.bAutoPauseCombatRound)
		{
			if (UCatCombatRoundTimer* RoundTimer = Subsystem->GetRoundTimer())
			{
				RoundTimer->SetPaused(!bVisible);
			}
		}
	}
}

void UCatCombatFunctionLibrary::SetCombatRoundPaused(const UObject* WorldContextObject, bool bPaused)
{
	if (UCatCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject))
	{
		if (UCatCombatRoundTimer* RoundTimer = Subsystem->GetRoundTimer())
		{
			RoundTimer->SetPaused(bPaused);
		}
	}
}

void UCatCombatFunctionLibrary::AddCombatCharacter(const UObject* WorldContextObject, ECombatTeamType TeamType, ACharacter* Character)
{
	if (UCatCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject))
	{
		if (UCatCombatDataStore* DataStore = Subsystem->GetDataStore())
		{
			DataStore->AddCharacter(TeamType, Character);
		}
		// 输入映射由 Subsystem 监听 OnCombatCharacterChanged 自动处理
	}
}

void UCatCombatFunctionLibrary::RemoveCombatCharacter(const UObject* WorldContextObject, ECombatTeamType TeamType, ACharacter* Character)
{
	if (UCatCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject))
	{
		if (UCatCombatDataStore* DataStore = Subsystem->GetDataStore())
		{
			DataStore->RemoveCharacter(TeamType, Character);
			// 输入映射由 Subsystem 监听 OnCombatCharacterChanged 自动处理
		}
	}
}

TArray<ACharacter*> UCatCombatFunctionLibrary::GetCombatCharacterList(const UObject* WorldContextObject, ECombatTeamType TeamType)
{
	if (UCatCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(WorldContextObject))
	{
		if (UCatCombatDataStore* DataStore = Subsystem->GetDataStore())
		{
			return DataStore->GetCharacters(TeamType);
		}
	}
	return TArray<ACharacter*>();
}

ACharacter* UCatCombatFunctionLibrary::GetMainPlayerCombatCharacter(const UObject* WorldContextObject)
{
	const TArray<ACharacter*> PlayerCharacters = GetCombatCharacterList(WorldContextObject, ECombatTeamType::Player);
	return PlayerCharacters.Num() > 0 ? PlayerCharacters[0] : nullptr;
}

ACharacter* UCatCombatFunctionLibrary::GetMainEnemyCombatCharacter(const UObject* WorldContextObject)
{
	const TArray<ACharacter*> EnemyCharacters = GetCombatCharacterList(WorldContextObject, ECombatTeamType::Enemy);
	return EnemyCharacters.Num() > 0 ? EnemyCharacters[0] : nullptr;
}

UCatCombatManagerSubsystem* UCatCombatFunctionLibrary::GetCombatManagerSubsystem(const UObject* WorldContextObject)
{
	if (const UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		return GI->GetSubsystem<UCatCombatManagerSubsystem>();
	}
	return nullptr;
}

bool UCatCombatFunctionLibrary::RequestAction(ACharacter* Actor, const FActionRequest& Request)
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

bool UCatCombatFunctionLibrary::GetCharacterTeam(ACharacter* Character, ECombatTeamType& OutTeamType)
{
	if (!IsValid(Character))
	{
		return false;
	}

	UCatCombatManagerSubsystem* Subsystem = GetCombatManagerSubsystem(Character);
	UCatCombatDataStore* DataStore = Subsystem ? Subsystem->GetDataStore() : nullptr;
	if (!DataStore)
	{
		return false;
	}

	// 从数据层反查 Character 真实所属队伍（而非协调器的当前行动队伍）
	// 移植说明：源数据层提供 FindTeamByCharacter 反查；本框架数据层未提供该方法，改为遍历双方队伍列表反查（回合制低频调用，开销可接受）
	if (DataStore->GetCharacters(ECombatTeamType::Player).Contains(Character))
	{
		OutTeamType = ECombatTeamType::Player;
		return true;
	}
	if (DataStore->GetCharacters(ECombatTeamType::Enemy).Contains(Character))
	{
		OutTeamType = ECombatTeamType::Enemy;
		return true;
	}
	return false;
}

ECombatTeamType UCatCombatFunctionLibrary::GetOpponentTeam(ECombatTeamType TeamType)
{
	return TeamType == ECombatTeamType::Player ? ECombatTeamType::Enemy : ECombatTeamType::Player;
}

TArray<ACharacter*> UCatCombatFunctionLibrary::GetOpponentCombatCharacterList(ACharacter* Character)
{
	// 反查 Character 所属队伍 → 取对方阵营 → 返回对方角色列表
	ECombatTeamType MyTeam = ECombatTeamType::Player;
	if (!GetCharacterTeam(Character, MyTeam))
	{
		return TArray<ACharacter*>();
	}

	return GetCombatCharacterList(Character, GetOpponentTeam(MyTeam));
}

ACharacter* UCatCombatFunctionLibrary::GetMainOpponentCombatCharacter(ACharacter* Character)
{
	// 反查 Character 所属队伍 → 敌方阵营 → 敌方主战斗角色（列表中首个）
	ECombatTeamType MyTeam = ECombatTeamType::Player;
	if (!GetCharacterTeam(Character, MyTeam))
	{
		return nullptr;
	}

	const TArray<ACharacter*> Opponents = GetCombatCharacterList(Character, GetOpponentTeam(MyTeam));
	return Opponents.Num() > 0 ? Opponents[0] : nullptr;
}

bool UCatCombatFunctionLibrary::CanActivateTurnAbilityByTag(ACharacter* Character, const FGameplayTag& Tag, ECombatTurnRole Role)
{
	const UCatCharacterTurnComponent* TurnComp = UCatCharacterTurnComponent::GetCatCharacterTurnComponent(Character);
	if (!TurnComp)
	{
		return false;
	}

	return TurnComp->CanActivateAbilityByTag(Tag, Role);
}

bool UCatCombatFunctionLibrary::CanActivateTurnAbilityByTagAnyRole(ACharacter* Character, const FGameplayTag& Tag)
{
	const UCatCharacterTurnComponent* TurnComp = UCatCharacterTurnComponent::GetCatCharacterTurnComponent(Character);
	if (!TurnComp)
	{
		return false;
	}

	// 攻击方或防守方任意职责可激活即返回 true
	return TurnComp->CanActivateAbilityByTag(Tag, ECombatTurnRole::Attacker)
		|| TurnComp->CanActivateAbilityByTag(Tag, ECombatTurnRole::Defender);
}

bool UCatCombatFunctionLibrary::CanActivateTurnAbilityByTagWithCurrentRole(ACharacter* Character, const FGameplayTag& Tag)
{
	const UCatCharacterTurnComponent* TurnComp = UCatCharacterTurnComponent::GetCatCharacterTurnComponent(Character);
	if (!TurnComp)
	{
		return false;
	}

	// Role 从回合组件当前职责（TurnRole）取，调用方无需感知当前是攻还是防
	return TurnComp->CanActivateAbilityByTag(Tag, TurnComp->GetTurnRole());
}

bool UCatCombatFunctionLibrary::IsEmptyActionTag(const FGameplayTag& SkillTag)
{
	// 家族匹配：等于 Ability.TurnAction.EmptyAction 本身，或为其子 Tag（未来细分变体）
	static const FGameplayTag EmptyActionTag = CatCombatGameplayTags::TAG_ABILITY_TURNACTION_EMPTYACTION;
	return SkillTag.IsValid()
		&& (SkillTag == EmptyActionTag || EmptyActionTag.MatchesTag(SkillTag));
}

void UCatCombatFunctionLibrary::CollectSkillCandidates(const TArray<FAISkillWeight>& Source, ACharacter* Character,
	UCatCharacterTurnComponent* TurnComp, ECombatTurnRole Role, TArray<FAISkillWeight>& OutCandidates)
{
	const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);

	for (const FAISkillWeight& Item : Source)
	{
		if (!Item.SkillTag.IsValid())
		{
			continue;
		}

		// 状态匹配：条目未指定状态 Tag（留空则忽略判定），或角色当前持有该状态 Tag（如 Status.Health.Low）
		const bool bStateMatch = !Item.RequiredStateTag.IsValid()
			|| (ASC && ASC->HasMatchingGameplayTag(Item.RequiredStateTag));
		if (!bStateMatch)
		{
			continue;
		}

		// 空技能：纯占位（Ability.TurnAction.EmptyAction 家族），不校验是否可激活，直接入候选
		if (IsEmptyActionTag(Item.SkillTag))
		{
			OutCandidates.Add(Item);
			continue;
		}

		// 可激活过滤
		if (TurnComp && !TurnComp->CanActivateAbilityByTag(Item.SkillTag, Role))
		{
			continue;
		}

		OutCandidates.Add(Item);
	}
}

bool UCatCombatFunctionLibrary::PickWeightedSkill(const TArray<FAISkillWeight>& Candidates, FGameplayTag& OutTag)
{
	if (Candidates.Num() == 0)
	{
		return false;
	}

	float TotalWeight = 0.f;
	for (const FAISkillWeight& Item : Candidates)
	{
		TotalWeight += FMath::Max(0.f, Item.Weight);
	}

	// 全 0 权重：退化为等概率随机
	if (TotalWeight <= 0.f)
	{
		const int32 Index = FMath::RandRange(0, Candidates.Num() - 1);
		OutTag = Candidates[Index].SkillTag;
		return true;
	}

	const float Roll = FMath::FRandRange(0.f, TotalWeight);
	float Accum = 0.f;
	for (const FAISkillWeight& Item : Candidates)
	{
		Accum += FMath::Max(0.f, Item.Weight);
		if (Roll <= Accum)
		{
			OutTag = Item.SkillTag;
			return true;
		}
	}

	// 浮点精度兜底
	OutTag = Candidates.Last().SkillTag;
	return true;
}

bool UCatCombatFunctionLibrary::PickSkillFromWeightTable(ACharacter* Character, ECombatTurnRole Role,
	const TArray<FAISkillWeight>& SkillTable, FGameplayTag& OutTag)
{
	UCatCharacterTurnComponent* TurnComp = UCatCharacterTurnComponent::GetCatCharacterTurnComponent(Character);
	if (!TurnComp)
	{
		return false;
	}

	TArray<FAISkillWeight> Candidates;
	CollectSkillCandidates(SkillTable, Character, TurnComp, Role, Candidates);
	return PickWeightedSkill(Candidates, OutTag);
}
