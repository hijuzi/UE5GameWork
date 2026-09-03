// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Camera/CameraActor.h"
#include "SVCombatCameraDataAsset.generated.h"

/**
 * 战斗相机资产：描述一个战斗镜头的配置数据。
 *
 * 承载相机类与相机参数（相对场景点的偏移/朝向/视野），供战斗场景生成战斗相机。
 * 后续可在此补充镜头运动、切换规则等配置。
 */
UCLASS()
class CATCOMBATFRAMEWORK_API USVCombatCameraDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 该资产对应的战斗相机类（使用引擎原生 ACameraActor 或其派生类） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CombatCamera")
	TSubclassOf<ACameraActor> CameraClass;

	/** 是否使用相对场景点的变换（true=相对，false=绝对） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CombatCamera")
	bool bUseRelativeTransform = true;

	/** 相机相对场景点的变换（局部坐标，运行时换算为世界坐标） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CombatCamera", meta = (EditCondition = "bUseRelativeTransform", EditConditionHides))
	FTransform RelativeTransform = FTransform(FRotator(0.f, 40.8f, 0.f), FVector(-183.6f, -163.6f, 70.0f), FVector::OneVector);

	/** 相机绝对变换（直接使用该世界坐标，不随场景点变换） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CombatCamera", meta = (EditCondition = "!bUseRelativeTransform", EditConditionHides))
	FTransform AbsoluteTransform = FTransform(FRotator(0.f, 40.8f, 0.f), FVector(0.f, 0.f, 100.f), FVector::OneVector);

	/** 视野角度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CombatCamera", meta = (ClampMin = "30.0", ClampMax = "120.0"))
	float FOV = 40.f;
};
