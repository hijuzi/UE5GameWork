// Copyright SegameVictory Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SVFloatingTextComponent.h"
#include "SVFloatingTextSettings.h"
#include "SVFloatingTextComponent_WidgetText.generated.h"

class USVFloatingTextWidget;
class USVFloatingTextWidgetTextDataAsset;

/**
 * 单个对象池条目
 */
USTRUCT()
struct FPooledWidgetList
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<USVFloatingTextWidget>> AvailableWidgets;
};

/**
 * 活跃飘字条目（已显示在屏幕上的）
 */
USTRUCT()
struct FLiveFloatingTextEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USVFloatingTextWidget> Widget;

	int32 SlotIndex = 0;

	FVector WorldOffset = FVector::ZeroVector;
};

/**
 * Widget 模式实现——使用 UMG 控件池管理飘字生命周期。
 * 复用 USVFloatingTextWidget 实例，按槽位堆叠排布。
 */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SVFLOATINGTEXT_API USVFloatingTextComponent_WidgetText : public USVFloatingTextComponent
{
	GENERATED_BODY()

public:
	USVFloatingTextComponent_WidgetText();

	virtual void BeginPlay() override;

	// -- 重写 --
	virtual void PopFloatingText(const FSVFloatingPopRequest& Request) override;
	virtual void PopTextFloatingText(const FSVFloatingPopRequest& Request, const FText& Text) override;
	virtual void ClearAllFloatingText() override;
	virtual void SetTextScaleFactor(float InScale) override;

protected:
	/**
	 * 从对象池获取或创建指定类型的 Widget
	 * @param WidgetClass Widget 类型
	 * @return 可用 Widget 实例
	 */
	USVFloatingTextWidget* GetOrCreateWidget(TSubclassOf<USVFloatingTextWidget> WidgetClass);

	/**
	 * 将 Widget 归还到对象池
	 */
	void ReturnWidgetToPool(USVFloatingTextWidget* Widget, TSubclassOf<USVFloatingTextWidget> WidgetClass);

	/**
	 * 根据请求匹配飘字样式
	 * @return true 表示找到有效样式
	 */
	bool GetFloatingTextStyle(const FSVFloatingPopRequest& Request, FSVFloatingTextStyle& OutStyle) const;

	/**
	 * 根据请求确定要显示的实际文本
	 */
	FText FormatDisplayText(const FSVFloatingPopRequest& Request) const;

	/**
	 * 推入一个活跃飘字（排入布局）
	 */
	void PushActiveWidget(USVFloatingTextWidget* Widget, const FSVFloatingPopRequest& Request);

	/** 回收超时的飘字并重新布局 */
	void TickFloatingWidgets();

	/** 重新布局所有活跃飘字 */
	void RelayoutAll();

	// -- 配置 --

	/** 单个飘字的默认存活时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floating Text")
	float FloatingTextLifetime = 1.5f;

	/** 垂直方向堆叠间距 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floating Text")
	float VerticalSpacing = 30.0f;

	/** 字体缩放因子 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floating Text")
	float TextScaleFactor = 1.0f;

private:
	// -- 对象池：WidgetClass -> FPooledWidgetList --
	UPROPERTY()
	TMap<TSubclassOf<USVFloatingTextWidget>, FPooledWidgetList> WidgetPool;

	// -- 活跃中的飘字列表 --
	UPROPERTY()
	TArray<FLiveFloatingTextEntry> LiveEntries;

	// -- 定时器句柄 --
	FTimerHandle TickTimerHandle;
};
