// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UBTTask_CatBase.h"
#include "BTTask_Cat_ChooseLowestHealthTarget.generated.h"

/** 目标选择：选敌方角色中血量最低者，写 TargetActor */
UCLASS(Category = "Cat AI", meta = (DisplayName = "Cat Choose Lowest Health Target"))
class CATCOMBATFRAMEWORK_API UBTTask_Cat_ChooseLowestHealthTarget : public UBTTask_CatBase
{
	GENERATED_BODY()

public:
	UBTTask_Cat_ChooseLowestHealthTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	/**
	 * 参与比较的血量属性（读取角色 ASC，如自定义 AttributeSet 的 Health）。
	 * 移植说明：源项目读 USVHealthComponent，本框架不绑定具体 AttributeSet，改为可配置属性；未配置时所有目标都不参与筛选（任务失败）。
	 */
	UPROPERTY(EditAnywhere, Category = "Cat|Health")
	FGameplayAttribute HealthAttribute;
};
