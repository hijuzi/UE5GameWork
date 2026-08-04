// Copyright SegameVictory Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SVFloatingTextComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSVFloatingTextComponent, Log, All);

class USVFloatingTextWidgetTextDataAsset;

/**
 * 单次飘字弹出请求（用于排队或广播）
 */
USTRUCT(BlueprintType)
struct FSVFloatingPopRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Floating Text")
	FGameplayTag OriginalTag;

	UPROPERTY(BlueprintReadWrite, Category = "Floating Text")
	int32 NumberToDisplay = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Floating Text")
	AActor* SourceActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Floating Text")
	bool bIsCritical = false;

	UPROPERTY(BlueprintReadWrite, Category = "Floating Text")
	FVector2D OverrideScreenOffset = FVector2D::ZeroVector;
};

/**
 * 基础飘字组件（挂载在 Controller 上管理该角色的所有飘字）。
 * 具体实现由蓝图或 C++ 子类（如 USVFloatingTextComponent_WidgetText）提供。
 */
UCLASS(Abstract, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SVFLOATINGTEXT_API USVFloatingTextComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USVFloatingTextComponent();

	/** 弹出一个数字飘字（子类实现） */
	UFUNCTION(BlueprintCallable, Category = "Floating Text")
	virtual void PopFloatingText(const FSVFloatingPopRequest& Request);

	/** 弹出一个文本飘字（子类实现） */
	UFUNCTION(BlueprintCallable, Category = "Floating Text")
	virtual void PopTextFloatingText(const FSVFloatingPopRequest& Request, const FText& Text);

	/** 清除所有活跃飘字 */
	UFUNCTION(BlueprintCallable, Category = "Floating Text")
	virtual void ClearAllFloatingText();

	/** 设置字体缩放因子（子类可重写） */
	UFUNCTION(BlueprintCallable, Category = "Floating Text")
	virtual void SetTextScaleFactor(float InScale);

protected:
	/** 获取世界空间中的飘字位置（基于 Owner Actor 的骨骼） */
	UFUNCTION(BlueprintCallable, Category = "Floating Text")
	FVector GetFloatingTextPosition(AActor* TargetActor) const;

	/** 获取本地 PlayerController */
	APlayerController* GetLocalPlayerController() const;

	/** 飘字配置数据资产 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floating Text")
	TObjectPtr<USVFloatingTextWidgetTextDataAsset> ConfigDataAsset;
};
