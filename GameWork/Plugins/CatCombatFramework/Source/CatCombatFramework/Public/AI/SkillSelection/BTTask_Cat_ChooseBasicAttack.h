// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UBTTask_CatBase.h"
#include "BTTask_Cat_ChooseBasicAttack.generated.h"

/** 技能选择：普攻兜底，普攻可激活时写 SelectedAbilityTag */
UCLASS(Category = "Cat AI", meta = (DisplayName = "Cat Choose Basic Attack"))
class CATCOMBATFRAMEWORK_API UBTTask_Cat_ChooseBasicAttack : public UBTTask_CatBase
{
	GENERATED_BODY()

public:
	UBTTask_Cat_ChooseBasicAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** 普攻 Tag 候选（取第一个可激活的） */
	UPROPERTY(EditAnywhere, Category = "Cat|Ability")
	FGameplayTagContainer BasicAttackTags;
};
