// Copyright SegameVictory Team. All Rights Reserved.

#include "SVFloatingTextComponent_WidgetText.h"
#include "SVFloatingTextWidgetTextDataAsset.h"
#include "SVFloatingTextWidget.h"
#include "GameplayTags/CommonGameplayTags.h"
#include "SVFloatingTextSettings.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"

USVFloatingTextComponent_WidgetText::USVFloatingTextComponent_WidgetText()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USVFloatingTextComponent_WidgetText::BeginPlay()
{
	Super::BeginPlay();

	// 每秒 Tick 检查超时回收
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TickTimerHandle,
			this,
			&USVFloatingTextComponent_WidgetText::TickFloatingWidgets,
			0.2f,
			true);
	}
}

void USVFloatingTextComponent_WidgetText::PopFloatingText(const FSVFloatingPopRequest& Request)
{
	if (!Request.SourceActor)
	{
		UE_LOG(LogSVFloatingTextComponent, Warning, TEXT("[PopFloatingText] SourceActor 为空，跳过"));
		return;
	}

	const USVFloatingTextSettings* Settings = USVFloatingTextSettings::Get();
	if (!Settings || !Settings->bEnableFloatingText)
	{
		return;
	}

	// 根据 Tag 匹配样式
	FSVFloatingTextStyle MatchedStyle;
	if (!GetFloatingTextStyle(Request, MatchedStyle))
	{
		UE_LOG(LogSVFloatingTextComponent, Verbose, TEXT("[PopFloatingText] 未匹配到样式 Tag=%s"), *Request.OriginalTag.ToString());
		return;
	}

	// 根据槽位类型选择 Widget 类
	TSubclassOf<USVFloatingTextWidget> WidgetClass = nullptr;
	switch (MatchedStyle.SlotType)
	{
	case ESVFloatingTextSlotType::Damage:
	case ESVFloatingTextSlotType::Crit:
		WidgetClass = Settings->DamageFloatingTextWidgetClass.LoadSynchronous();
		break;
	case ESVFloatingTextSlotType::Heal:
		WidgetClass = Settings->HealingFloatingTextWidgetClass.LoadSynchronous();
		break;
	case ESVFloatingTextSlotType::Status:
	case ESVFloatingTextSlotType::Default:
	default:
		WidgetClass = Settings->StatusFloatingTextWidgetClass.LoadSynchronous();
		break;
	}

	if (!WidgetClass)
	{
		UE_LOG(LogSVFloatingTextComponent, Warning, TEXT("[PopFloatingText] SlotType=%d 的 Widget 类未配置"), static_cast<int32>(MatchedStyle.SlotType));
		return;
	}

	// 检查是否超过最大同时显示数
	// 按槽位类型统计当前活跃数
	int32 CurrentSlotCount = 0;
	for (const FLiveFloatingTextEntry& Entry : LiveEntries)
	{
		if (Entry.Widget)
		{
			// 检查该 Widget 的样式是否属于同一槽位
			// 如果超过 MaxSimultaneous，移除最早的条目
			CurrentSlotCount++;
		}
	}

	if (CurrentSlotCount >= MatchedStyle.MaxSimultaneous)
	{
		// 移除最早的条目（回收）
		for (int32 i = 0; i < LiveEntries.Num(); ++i)
		{
			if (LiveEntries[i].Widget)
			{
				LiveEntries[i].Widget->Hide();
				ReturnWidgetToPool(LiveEntries[i].Widget, WidgetClass);
				LiveEntries.RemoveAt(i);
				break;
			}
		}
	}

	// 获取或创建 Widget
	USVFloatingTextWidget* Widget = GetOrCreateWidget(WidgetClass);
	if (!Widget)
	{
		UE_LOG(LogSVFloatingTextComponent, Error, TEXT("[PopFloatingText] 无法获取或创建 Widget"));
		return;
	}

	// 推入活跃列表
	PushActiveWidget(Widget, Request);
}

void USVFloatingTextComponent_WidgetText::PopTextFloatingText(const FSVFloatingPopRequest& Request, const FText& Text)
{
	FSVFloatingPopRequest TextRequest = Request;
	PopFloatingText(TextRequest);

	// 在最新的活跃条目上设置文本
	if (LiveEntries.Num() > 0)
	{
		FLiveFloatingTextEntry& LastEntry = LiveEntries.Last();
		if (LastEntry.Widget)
		{
			LastEntry.Widget->OnUpdateText(Text);
		}
	}
}

void USVFloatingTextComponent_WidgetText::ClearAllFloatingText()
{
	for (FLiveFloatingTextEntry& Entry : LiveEntries)
	{
		if (Entry.Widget)
		{
			Entry.Widget->Hide();
		}
	}

	// 清空对象池中的 Widget
	for (auto& PoolPair : WidgetPool)
	{
		for (USVFloatingTextWidget* Widget : PoolPair.Value.AvailableWidgets)
		{
			if (Widget)
			{
				Widget->RemoveFromParent();
			}
		}
	}

	WidgetPool.Empty();
	LiveEntries.Empty();
}

void USVFloatingTextComponent_WidgetText::SetTextScaleFactor(float InScale)
{
	TextScaleFactor = FMath::Max(InScale, 0.1f);
}

USVFloatingTextWidget* USVFloatingTextComponent_WidgetText::GetOrCreateWidget(TSubclassOf<USVFloatingTextWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		return nullptr;
	}

	FPooledWidgetList& Pool = WidgetPool.FindOrAdd(WidgetClass);

	// 尝试从池中取出
	if (Pool.AvailableWidgets.Num() > 0)
	{
		USVFloatingTextWidget* Widget = Pool.AvailableWidgets.Pop();
		if (IsValid(Widget))
		{
			return Widget;
		}
	}

	// 创建新 Widget
	APlayerController* PC = GetLocalPlayerController();
	if (!PC)
	{
		return nullptr;
	}

	USVFloatingTextWidget* NewWidget = CreateWidget<USVFloatingTextWidget>(PC, WidgetClass);
	if (NewWidget)
	{
		NewWidget->AddToViewport(100);
	}

	return NewWidget;
}

void USVFloatingTextComponent_WidgetText::ReturnWidgetToPool(USVFloatingTextWidget* Widget, TSubclassOf<USVFloatingTextWidget> WidgetClass)
{
	if (!Widget || !WidgetClass)
	{
		return;
	}

	Widget->Hide();
	FPooledWidgetList& Pool = WidgetPool.FindOrAdd(WidgetClass);
	Pool.AvailableWidgets.Add(Widget);
}

bool USVFloatingTextComponent_WidgetText::GetFloatingTextStyle(const FSVFloatingPopRequest& Request, FSVFloatingTextStyle& OutStyle) const
{
	if (!ConfigDataAsset)
	{
		UE_LOG(LogSVFloatingTextComponent, Warning, TEXT("[GetFloatingTextStyle] ConfigDataAsset 为空！"));
		return false;
	}

	// 优先查找 CustomStyles 中的匹配项
	for (const FSVFloatingTextStyle& Style : ConfigDataAsset->CustomStyles)
	{
		if (Request.OriginalTag.MatchesTag(Style.MatchingTag))
		{
			OutStyle = Style;
			UE_LOG(LogSVFloatingTextComponent, Verbose, TEXT("[GetFloatingTextStyle] 匹配自定义样式 Tag=%s"), *Style.MatchingTag.ToString());
			return true;
		}
	}

	// 未找到或未覆盖基础样式，根据 Tag 返回伤害或治疗基础样式
	if (Request.OriginalTag.MatchesTag(CommonGameplayTags::TAG_GAMEPLAYCUE_FLOATING_TEXT_HEALING))
	{
		OutStyle = ConfigDataAsset->HealingBaseStyle;
		UE_LOG(LogSVFloatingTextComponent, Verbose, TEXT("[GetFloatingTextStyle] 匹配治疗标签, 使用治疗基础样式"));
		return true;
	}
	if (Request.OriginalTag.MatchesTag(CommonGameplayTags::TAG_GAMEPLAYCUE_FLOATING_TEXT_DAMAGETAKEN))
	{
		OutStyle = ConfigDataAsset->DamageBaseStyle;
		UE_LOG(LogSVFloatingTextComponent, Verbose, TEXT("[GetFloatingTextStyle] 匹配伤害标签, 使用伤害基础样式"));
		return true;
	}

	// 默认使用 Status 基础样式
	OutStyle = ConfigDataAsset->StatusBaseStyle;
	UE_LOG(LogSVFloatingTextComponent, Verbose, TEXT("[GetFloatingTextStyle] 使用默认状态基础样式"));
	return true;
}

FText USVFloatingTextComponent_WidgetText::FormatDisplayText(const FSVFloatingPopRequest& Request) const
{
	if (!ConfigDataAsset)
	{
		return FText::FromString(FString::FromInt(Request.NumberToDisplay));
	}

	// 通过 OriginalTag 匹配是否为治疗标签
	if (Request.OriginalTag.MatchesTag(CommonGameplayTags::TAG_GAMEPLAYCUE_FLOATING_TEXT_HEALING))
	{
		return FText::FromString(FString::Printf(TEXT("+%d"), Request.NumberToDisplay));
	}

	return FText::FromString(FString::FromInt(Request.NumberToDisplay));
}

void USVFloatingTextComponent_WidgetText::PushActiveWidget(USVFloatingTextWidget* Widget, const FSVFloatingPopRequest& Request)
{
	if (!Widget || !Request.SourceActor)
	{
		return;
	}

	FVector WorldPos = GetFloatingTextPosition(Request.SourceActor);
	FText DisplayText = FormatDisplayText(Request);

	FLiveFloatingTextEntry NewEntry;
	NewEntry.Widget = Widget;
	NewEntry.WorldOffset = WorldPos;

	Widget->CurrentRequest = Request;
	Widget->InitWidget();
	Widget->OnUpdateData(Request);
	Widget->OnUpdateText(DisplayText);
	Widget->Show();
	Widget->OnPlayInAnim();

	LiveEntries.Add(NewEntry);
	RelayoutAll();
}

void USVFloatingTextComponent_WidgetText::TickFloatingWidgets()
{
	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	for (int32 i = LiveEntries.Num() - 1; i >= 0; --i)
	{
		FLiveFloatingTextEntry& Entry = LiveEntries[i];
		// 简化：不做超时检查，由外部 ClearAllFloatingText 或手动隐藏管理
	}

	RelayoutAll();
}

void USVFloatingTextComponent_WidgetText::RelayoutAll()
{
	APlayerController* PC = GetLocalPlayerController();
	if (!PC)
	{
		return;
	}

	int32 ViewportX, ViewportY;
	PC->GetViewportSize(ViewportX, ViewportY);

	for (int32 i = 0; i < LiveEntries.Num(); ++i)
	{
		FLiveFloatingTextEntry& Entry = LiveEntries[i];
		if (!Entry.Widget)
		{
			continue;
		}

		FVector2D ScreenPos;
		if (PC->ProjectWorldLocationToScreen(Entry.WorldOffset, ScreenPos, false))
		{
			// 垂直堆叠
			ScreenPos.Y -= i * VerticalSpacing * TextScaleFactor;

			if (UWidgetLayoutLibrary::SlotAsCanvasSlot(Entry.Widget))
			{
				UWidgetLayoutLibrary::SlotAsCanvasSlot(Entry.Widget)->SetPosition(ScreenPos);
			}
		}
	}
}
