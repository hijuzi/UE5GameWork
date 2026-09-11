// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UBTTask_CatBase.h"
#include "BTTask_Cat_ChooseAoEAbility.generated.h"

/** 技能选择：AOE 能力可激活时选 AOE 写 SelectedAbilityTag（多目标判定后续接入） */
UCLASS(Category = "Cat AI", meta = (DisplayName = "Cat Choose AoE Ability"))
class CATCOMBATFRAMEWORK_API UBTTask_Cat_ChooseAoEAbility : public UBTTask_CatBase
{
	GENERATED_BODY()

public:
	UBTTask_Cat_ChooseAoEAbility();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** AOE 能力 Tag 候选（取第一个可激活的） */
	UPROPERTY(EditAnywhere, Category = "Cat|Ability")
	FGameplayTagContainer AoEAbilityTags;
};
