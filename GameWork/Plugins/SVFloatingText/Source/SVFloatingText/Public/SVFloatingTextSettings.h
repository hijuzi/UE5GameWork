// Copyright SegameVictory Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "GameplayTagContainer.h"
#include "SVFloatingTextSettings.generated.h"

class USVFloatingTextWidget;

/**
 * 飘字槽位类型（枚举，供配置使用）
 */
UENUM(BlueprintType)
enum class ESVFloatingTextSlotType : uint8
{
	Default			UMETA(DisplayName = "默认"),
	Damage			UMETA(DisplayName = "伤害"),
	Heal			UMETA(DisplayName = "治疗"),
	Status			UMETA(DisplayName = "状态/效果"),
	Crit			UMETA(DisplayName = "暴击"),
};

/**
 * 飘字样式配置（一种漂浮文本的视觉效果）
 */
USTRUCT(BlueprintType)
struct FSVFloatingTextStyle
{
	GENERATED_BODY()

	/** 此样式对应的槽位类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floating Text")
	ESVFloatingTextSlotType SlotType = ESVFloatingTextSlotType::Default;

	/** 匹配 GameplayTag（1个：要应用此样式，OriginalTag 必须匹配此 Tag） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floating Text", meta = (Categories = "GameplayCue.FloatingText"))
	FGameplayTag MatchingTag;

	/** 此槽位最多同时存在的飘字数量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floating Text", meta = (ClampMin = "1"))
	int32 MaxSimultaneous = 5;
};

/**
 * 飘字系统全局设置（Project Settings > Game > Floating Text）
 */
UCLASS(config = FloatingText, defaultconfig, meta = (DisplayName = "Floating Text"))
class SVFLOATINGTEXT_API USVFloatingTextSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	USVFloatingTextSettings();

	static const USVFloatingTextSettings* Get();

	/** 伤害飘字 Widget 类 */
	UPROPERTY(config, EditAnywhere, Category = "Floating Text", meta = (MetaClass = "/Script/SVFloatingText.SVFloatingTextWidget"))
	TSoftClassPtr<USVFloatingTextWidget> DamageFloatingTextWidgetClass;

	/** 治疗飘字 Widget 类 */
	UPROPERTY(config, EditAnywhere, Category = "Floating Text", meta = (MetaClass = "/Script/SVFloatingText.SVFloatingTextWidget"))
	TSoftClassPtr<USVFloatingTextWidget> HealingFloatingTextWidgetClass;

	/** Buff/状态飘字 Widget 类 */
	UPROPERTY(config, EditAnywhere, Category = "Floating Text", meta = (MetaClass = "/Script/SVFloatingText.SVFloatingTextWidget"))
	TSoftClassPtr<USVFloatingTextWidget> StatusFloatingTextWidgetClass;

	/** 全局飘字显示开关（关闭后所有飘字消失） */
	UPROPERTY(config, EditAnywhere, Category = "Floating Text")
	bool bEnableFloatingText = true;

	/** 飘字系统最大活跃 Widget 数量 */
	UPROPERTY(config, EditAnywhere, Category = "Floating Text", meta = (ClampMin = "1"))
	int32 MaxActiveWidgets = 64;
};
