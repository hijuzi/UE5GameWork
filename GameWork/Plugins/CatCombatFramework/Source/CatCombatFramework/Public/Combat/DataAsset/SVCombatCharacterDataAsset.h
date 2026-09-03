// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Character.h"
#include "SVCombatCharacterDataAsset.generated.h"

class UGameplayAbility;

/**
 * 战斗角色资产：描述一个出战角色的配置数据。
 *
 * 后续在此补充角色的属性、技能、动画、AI 等配置。
 */
UCLASS()
class CATCOMBATFRAMEWORK_API USVCombatCharacterDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 该资产对应的战斗角色类 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CombatCharacter")
	TSubclassOf<ACharacter> CharacterClass;

	/** 该角色使用的能力列表（生成后授予到角色 ASC；若角色实现 ISVCombatAbilityGranter，则改由其自行授予） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CombatCharacter")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

	/** 是否使用相对场景点的变换（true=相对，false=绝对） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CombatCharacter")
	bool bUseRelativeTransform = true;

	/** 角色相对场景点的站位变换（局部坐标，运行时换算为世界坐标） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CombatCharacter", meta = (EditCondition = "bUseRelativeTransform", EditConditionHides))
	FTransform RelativeTransform = FTransform(FRotator(0.f, 40.8f, 0.f), FVector(-49.3f, 0.f, 0.f), FVector::OneVector);

	/** 角色绝对站位变换（直接使用该世界坐标，不随场景点变换） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CombatCharacter", meta = (EditCondition = "!bUseRelativeTransform", EditConditionHides))
	FTransform AbsoluteTransform = FTransform(FRotator(0.f, 40.8f, 0.f), FVector(0.f, 0.f, 0.f), FVector::OneVector);
};
