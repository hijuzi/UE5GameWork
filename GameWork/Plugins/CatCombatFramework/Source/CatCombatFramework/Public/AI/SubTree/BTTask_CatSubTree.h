// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_RunBehaviorDynamic.h"
#include "AI/CatAIControlData.h"
#include "BTTask_CatSubTree.generated.h"

/**
 * 动态子树 Task：运行时从 UCatAIControlData 按子树类型读取对应子树资产并执行。
 * 槽位为空时回退默认子树（DefaultBehaviorAsset）。
 */
UCLASS(Category = "Cat AI", meta = (DisplayName = "Cat SubTree"))
class CATCOMBATFRAMEWORK_API UBTTask_CatSubTree : public UBTTask_RunBehaviorDynamic
{
	GENERATED_BODY()

public:
	UBTTask_CatSubTree(const FObjectInitializer& ObjectInitializer);

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** 标识要读取 UCatAIControlData 中的哪个子树槽位 */
	UPROPERTY(EditAnywhere, Category = "Cat SubTree")
	ECatAISubTreeType SubTreeType = ECatAISubTreeType::SkillSelection;
};
