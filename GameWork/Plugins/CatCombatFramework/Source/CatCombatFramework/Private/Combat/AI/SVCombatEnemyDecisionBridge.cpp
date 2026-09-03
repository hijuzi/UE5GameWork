// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/AI/SVCombatEnemyDecisionBridge.h"

#include "Combat/AI/SVCombatEnemyInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Character.h"
#include "CatCombatLog.h"

bool USVCombatEnemyDecisionBridge::RequestEnemyAction(ACharacter* Enemy, FActionRequest& OutRequest)
{
	if (!IsValid(Enemy))
	{
		return false;
	}

	const FGameplayTag SelectedTag = SelectAttackTag(Enemy);
	if (!SelectedTag.IsValid())
	{
		UE_LOG(LogCatCombatAI, Warning, TEXT("EnemyDecisionBridge: 敌人 %s 未选出有效攻击标签"), *Enemy->GetName());
		return false;
	}

	// 目标经敌人接口获取
	AActor* Target = nullptr;
	if (Enemy->Implements<USVCombatEnemyInterface>())
	{
		Target = ISVCombatEnemyInterface::Execute_GetAttackTargetActor(Enemy);
	}
	if (!Target)
	{
		UE_LOG(LogCatCombatAI, Warning, TEXT("EnemyDecisionBridge: 敌人 %s 无攻击目标"), *Enemy->GetName());
		return false;
	}

	OutRequest.AbilityTag = SelectedTag;
	OutRequest.Target = Target;
	return true;
}

FGameplayTag USVCombatEnemyDecisionBridge::SelectAttackTag(ACharacter* Enemy)
{
	if (!Enemy)
	{
		return FGameplayTag();
	}

	TArray<FGameplayTag>& ReleasedPool = ReleasedPools.FindOrAdd(Enemy);

	// 收集候选并累计总权重。bIgnoreCooldown 用于未释放技能全部处于冷却时的兜底（保证敌人仍能行动）
	auto CollectCandidates = [&](TArray<const FEnemyAttackTagWeight*>& OutCandidates, int32& OutTotalWeight, const bool bIgnoreCooldown)
	{
		OutCandidates.Reset();
		OutTotalWeight = 0;
		for (const FEnemyAttackTagWeight& Entry : AttackTagWeights)
		{
			if (!Entry.AttackTag.IsValid()) continue;
			if (ReleasedPool.Contains(Entry.AttackTag)) continue;                             // 已释放过，本轮不再选
			if (!bIgnoreCooldown && IsAttackTagOnCooldown(Entry.AttackTag, Enemy)) continue; // 在冷却中，权重视为0
			const int32 Weight = FMath::Max(0, Entry.Weight);
			if (Weight <= 0) continue;
			OutCandidates.Add(&Entry);
			OutTotalWeight += Weight;
		}
	};

	// 未释放池是否已空（不考虑冷却）：用于判断是否需要重置已释放池开始新一轮
	auto IsUnreleasedPoolEmpty = [&]() -> bool
	{
		for (const FEnemyAttackTagWeight& Entry : AttackTagWeights)
		{
			if (!Entry.AttackTag.IsValid()) continue;
			if (FMath::Max(0, Entry.Weight) <= 0) continue;
			if (!ReleasedPool.Contains(Entry.AttackTag))
			{
				return false;
			}
		}
		return true;
	};

	TArray<const FEnemyAttackTagWeight*> Candidates;
	int32 TotalWeight = 0;
	CollectCandidates(Candidates, TotalWeight, false);

	// 未释放池已空 → 所有技能都释放过一轮，清空已释放池重新开始
	if (TotalWeight <= 0 && IsUnreleasedPoolEmpty())
	{
		ReleasedPool.Reset();
		CollectCandidates(Candidates, TotalWeight, false);
	}

	// 仍无候选：未释放技能全部处于冷却中，兜底忽略冷却再选一次，保证敌人能攻击
	if (TotalWeight <= 0)
	{
		CollectCandidates(Candidates, TotalWeight, true);
	}

	if (TotalWeight <= 0)
	{
		return FGameplayTag();
	}

	int32 RandomWeight = FMath::RandRange(1, TotalWeight);
	for (const FEnemyAttackTagWeight* Entry : Candidates)
	{
		RandomWeight -= FMath::Max(0, Entry->Weight);
		if (RandomWeight <= 0)
		{
			ReleasedPool.AddUnique(Entry->AttackTag); // 移入已释放池
			return Entry->AttackTag;
		}
	}

	return FGameplayTag();
}

bool USVCombatEnemyDecisionBridge::IsAttackTagOnCooldown(const FGameplayTag& AttackTag, ACharacter* Enemy) const
{
	if (!AttackTag.IsValid() || !Enemy)
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Enemy);
	if (!ASC)
	{
		return false;
	}

	// 遍历可激活能力，找到 AssetTags 命中 AttackTag 的能力，检查其冷却 Tag 是否命中
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (!Spec.Ability)
		{
			continue;
		}
		if (!Spec.Ability->GetAssetTags().HasTagExact(AttackTag))
		{
			continue;
		}
		const FGameplayTagContainer* CooldownTags = Spec.Ability->GetCooldownTags();
		if (CooldownTags && !CooldownTags->IsEmpty())
		{
			return ASC->HasAnyMatchingGameplayTags(*CooldownTags);
		}
		return false;
	}
	return false;
}
