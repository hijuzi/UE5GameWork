// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/SVCombatTypes.h"
#include "Engine/DataTable.h"
#include "SVCombatDataTable.generated.h"

class USVCombatCameraDataAsset;
class USVCombatCharacterDataAsset;

/**
 * 战斗数据表行：配置一场战斗的完整数据。
 * 按战斗配置Id 检索，关联 ScenePoint 场景点，并指定相机与双方阵容。
 */
USTRUCT(BlueprintType)
struct CATCOMBATFRAMEWORK_API FSVCombatDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 战斗配置Id */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatConfig")
	int32 CombatConfigId = 0;

	/** 配置说明 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatConfig")
	FText Description;

	/** 回合开始时的队伍（先手阵营），默认玩家先手 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatConfig")
	ECombatTeamType StartingTeam = ECombatTeamType::Player;

	/** 场景点相机配置数据 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatConfig")
	TObjectPtr<USVCombatCameraDataAsset> ScenePointCameraData;

	/** 玩家阵容配置数据 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatConfig")
	TArray<TObjectPtr<USVCombatCharacterDataAsset>> PlayerTeams;

	/** 敌人阵容配置数据 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatConfig")
	TArray<TObjectPtr<USVCombatCharacterDataAsset>> EnemyTeams;
};
