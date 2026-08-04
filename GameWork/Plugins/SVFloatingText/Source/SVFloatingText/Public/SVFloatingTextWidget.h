// Copyright SegameVictory Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "SVFloatingTextSettings.h"
#include "SVFloatingTextComponent.h"
#include "SVFloatingTextWidget.generated.h"

/**
 * 浮字显示 Widget 基类（抽象，蓝图继承）
 * 每个实例对应一个浮动文本条目，由 SVFloatingTextComponent 管理生命周期。
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class SVFLOATINGTEXT_API USVFloatingTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 显示 Widget */
	UFUNCTION(BlueprintImplementableEvent, Category = "Floating Text")
	void Show();

	/** 隐藏 Widget */
	UFUNCTION(BlueprintImplementableEvent, Category = "Floating Text")
	void Hide();

	/** 播放入场动画 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Floating Text")
	void OnPlayInAnim();

	/** 播放出场动画 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Floating Text")
	void OnPlayOutAnim();

	/** 更新飘字文本内容 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Floating Text")
	void OnUpdateText(const FText& Text);

	/** 更新飘字数据（数字、类型等） */
	UFUNCTION(BlueprintImplementableEvent, Category = "Floating Text")
	void OnUpdateData(const FSVFloatingPopRequest& Request);

	/** 初始化 Widget（替代原 USVUserWidget::InitWidget） */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Floating Text")
	void InitWidget();

	// -- 子 Widget 绑定 --
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> HealingText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> BuffFloatingText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusFloatingText;

	// -- 当前数据 --
	UPROPERTY(BlueprintReadOnly, Category = "Floating Text")
	FSVFloatingPopRequest CurrentRequest;
};
