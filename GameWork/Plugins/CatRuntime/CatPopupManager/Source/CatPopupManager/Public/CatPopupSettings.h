// Copyright SegameVictory Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "CatPopupSettings.generated.h"

class UTexture2D;

/**
 * Cat浮字插槽类型
 */
UENUM(BlueprintType)
enum class ECatFloatingTextSlotType : uint8
{
	/** Character头部 */
	CharacterHead	UMETA(DisplayName = "Character头部"),

	/** UIBuff插槽 */
	UIBuffSlot		UMETA(DisplayName = "UIBuff插槽"),
};

/**
 * Cat浮字文本样式配置
 * 定义浮字显示的视觉样式参数。
 */
USTRUCT(BlueprintType)
struct CATPOPUPMANAGER_API FCatFloatingTextStyle
{
	GENERATED_BODY()

	/** 是否覆盖基础样式 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat Popup Manager|Style")
	bool bOverrideBaseStyle = true;

	/** 主文本字号 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat Popup Manager|Style")
	float FontSize = 64;

	/** 文本主要颜色（如果渐变，则为 Color_Bottom 属性）*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat Popup Manager|Style")
	FLinearColor PrimaryColor = FLinearColor::Red;

	/** 文本附加颜色（如果渐变，则为 Color_Top 属性）*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat Popup Manager|Style")
	FLinearColor SecondaryColor = FLinearColor::Red;

	/** 文本描边粗细 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat Popup Manager|Style")
	int32 OutlineThickness = 2;

	/** 文本描边颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat Popup Manager|Style")
	FLinearColor OutlineColor = FLinearColor::Black;

	/** 显示图标（如暴击标志、属性图标等） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat Popup Manager|Style")
	TObjectPtr<UTexture2D> DisplayIcon;

	/** 额外文本（如"暴击"、"格挡"等后缀文本） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat Popup Manager|Style")
	FText ExtraText;

	/** 额外文本字号 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat Popup Manager|Style")
	float ExtraFontSize = 48;

	/** 飘字插槽类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cat Popup Manager|Style")
	ECatFloatingTextSlotType SlotType = ECatFloatingTextSlotType::CharacterHead;
};

/**
 * Cat弹窗全局设置
 * 可在 项目设置 -> Cat Popup Manager 面板中配置
 */
UCLASS(config = CatPopupManager, defaultconfig, meta = (DisplayName = "Cat Popup Manager"))
class CATPOPUPMANAGER_API UCatPopupSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	/** 获取 CatPopupSettings 单例 */
	UFUNCTION(BlueprintCallable, Category = "Cat Popup Manager")
	static UCatPopupSettings* Get()
	{
		return GetMutableDefault<UCatPopupSettings>();
	}

	// TODO: 在此添加弹窗相关配置项
};
