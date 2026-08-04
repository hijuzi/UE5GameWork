// Copyright SegameVictory Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SVFloatingTextSettings.h"
#include "SVFloatingTextWidgetTextDataAsset.generated.h"

/**
 * 飘字样式数据资产——定义不同 GameplayCue 的飘字外观。
 * 配置在 Content Browser 中，一个资产对应一组飘字配置。
 */
UCLASS(BlueprintType)
class SVFLOATINGTEXT_API USVFloatingTextWidgetTextDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 伤害飘字基础样式（默认匹配 GameplayCue.FloatingText.DamageTaken） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floating Text")
	FSVFloatingTextStyle DamageBaseStyle;

	/** 治疗飘字基础样式（默认匹配 GameplayCue.FloatingText.Healing） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floating Text")
	FSVFloatingTextStyle HealingBaseStyle;

	/** 状态/效果飘字基础样式 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floating Text")
	FSVFloatingTextStyle StatusBaseStyle;

	/** 自定义飘字样式列表（按 GameplayTag 匹配，覆盖基础样式） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floating Text")
	TArray<FSVFloatingTextStyle> CustomStyles;
};
