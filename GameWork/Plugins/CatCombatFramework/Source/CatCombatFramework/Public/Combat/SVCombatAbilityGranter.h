// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SVCombatAbilityGranter.generated.h"

class UAbilitySystemComponent;

/**
 * 战斗能力授予接口（扩展点）。
 *
 * 替代源项目对 USVAbilitySet / USVAbilitySystemComponent 的强依赖：
 * 战斗角色在生成后，由数据层 USVCombatDataStore 调用此接口，将技能/属性集/效果
 * 授予到角色自身的 UAbilitySystemComponent。具体授予逻辑由角色类实现（C++ 或蓝图）。
 */
UINTERFACE(MinimalAPI, Blueprintable)
class USVCombatAbilityGranter : public UInterface
{
	GENERATED_BODY()
};

class CATCOMBATFRAMEWORK_API ISVCombatAbilityGranter
{
	GENERATED_BODY()

public:
	/** 授予战斗能力：由具体角色实现，向传入的 ASC 授予技能/属性集/效果 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Ability")
	void GrantCombatAbilities(UAbilitySystemComponent* ASC);
};
