// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "GameplayTagContainer.h"
#include "BTDecorator_Cat_IsRole.generated.h"

/**
 * 攻防分支装饰器：查询角色 ASC 状态 Tag（State.Turn.Selecting / State.Turn.Defending）判定。
 */
UCLASS(Category = "Cat AI", meta = (DisplayName = "Cat Is Role"))
class CATCOMBATFRAMEWORK_API UBTDecorator_Cat_IsRole : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_Cat_IsRole();

	virtual FString GetStaticDescription() const override;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	/** 期望的角色阶段状态 Tag（如 State.Turn.Selecting / State.Turn.Defending） */
	UPROPERTY(EditAnywhere, Category = "Condition")
	FGameplayTag ExpectedStateTag;
};
