// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CatCombatSettings.generated.h"

class UDataTable;
class UCatCombatHUDLayout;

/**
 * 战斗系统配置（Project Settings -> CatCombatFramework -> Combat Settings）。
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Combat Settings"))
class CATCOMBATFRAMEWORK_API UCatCombatSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FName(TEXT("CatCombatFramework")); }

	/** 战斗配置数据表（行类型 FCatCombatDataTableRow，按 CombatConfigId 检索整场战斗配置） */
	UPROPERTY(Config, EditAnywhere, Category = "Combat Config", meta = (RowType = "FCatCombatDataTableRow"))
	TSoftObjectPtr<UDataTable> CombatConfigTable;

	/** 战斗 HUD 布局类 */
	UPROPERTY(Config, EditAnywhere, Category = "Combat HUD")
	TSoftClassPtr<UCatCombatHUDLayout> CombatHUDLayoutClass;

	/** 最大回合时间（秒） */
	UPROPERTY(Config, EditAnywhere, Category = "Combat Round", meta = (ClampMin = "0.1"))
	float MaxRoundTime = 25.0f;
};
