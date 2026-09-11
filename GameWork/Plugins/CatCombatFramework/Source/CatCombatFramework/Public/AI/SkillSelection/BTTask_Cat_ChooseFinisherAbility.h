// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/UBTTask_CatBase.h"
#include "BTTask_Cat_ChooseFinisherAbility.generated.h"

/** 技能选择：目标血量低于阈值且终结技可激活时，选终结技写 SelectedAbilityTag */
UCLASS(Category = "Cat AI", meta = (DisplayName = "Cat Choose Finisher Ability"))
class CATCOMBATFRAMEWORK_API UBTTask_Cat_ChooseFinisherAbility : public UBTTask_CatBase
{
	GENERATED_BODY()

public:
	UBTTask_Cat_ChooseFinisherAbility();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** 终结技 Tag 候选（取第一个可激活的） */
	UPROPERTY(EditAnywhere, Category = "Cat|Ability")
	FGameplayTagContainer FinisherAbilityTags;

	/** 目标血量阈值：目标血量比例低于该值才考虑终结技 */
	UPROPERTY(EditAnywhere, Category = "Cat|Ability", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FinisherHealthThreshold = 0.3f;

	/**
	 * 目标血量 / 最大血量属性（读取目标 ASC）。
	 * 移植说明：源项目读 USVHealthComponent 的 GetHealthNormalized，本框架不绑定具体 AttributeSet，改为可配置属性；
	 * 未配置或目标无该属性时不做血量拦截（与源项目「无血量组件则直接尝试终结技」一致）。
	 */
	UPROPERTY(EditAnywhere, Category = "Cat|Health")
	FGameplayAttribute HealthAttribute;

	/** 目标最大血量属性（与 HealthAttribute 配对使用） */
	UPROPERTY(EditAnywhere, Category = "Cat|Health")
	FGameplayAttribute MaxHealthAttribute;
};
