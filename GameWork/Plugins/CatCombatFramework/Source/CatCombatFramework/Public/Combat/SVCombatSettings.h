// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SVCombatSettings.generated.h"

class UDataTable;
class USVCombatHUDLayout;

/**
 * 战斗系统配置（Project Settings -> CatCombatFramework -> Combat Settings）。
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Combat Settings"))
class CATCOMBATFRAMEWORK_API USVCombatSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FName(TEXT("CatCombatFramework")); }

	/** 战斗配置数据表（行类型 FSVCombatDataTableRow，按 CombatConfigId 检索整场战斗配置） */
	UPROPERTY(Config, EditAnywhere, Category = "Combat Config", meta = (RowType = "FSVCombatDataTableRow"))
	TSoftObjectPtr<UDataTable> CombatConfigTable;

	/** 战斗 HUD 布局类 */
	UPROPERTY(Config, EditAnywhere, Category = "Combat HUD")
	TSoftClassPtr<USVCombatHUDLayout> CombatHUDLayoutClass;

	/** 最大回合时间（秒） */
	UPROPERTY(Config, EditAnywhere, Category = "Combat Round", meta = (ClampMin = "0.1"))
	float MaxRoundTime = 25.0f;
};
