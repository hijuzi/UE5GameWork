#include "SGASGameplayEffectNodeBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilityTimerManager.h"
#include "AbilityTimingTags.h"
#include "AttributeSet.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "SGASAttachEditor"

FGASGameplayEffectNode::~FGASGameplayEffectNode()
{
}

TSharedRef<FGASGameplayEffectNode> FGASGameplayEffectNode::Create(TWeakObjectPtr<UAbilitySystemComponent> InASComponent, const UWorld* InWorld, const FActiveGameplayEffect& InGameplayEffect)
{
	return MakeShareable(new FGASGameplayEffectNode(InASComponent, InWorld, InGameplayEffect));
}

FText FGASGameplayEffectNode::GetDurationText() const
{
	FText DurationText;

	// 子行（Modifier）：Time 列显示该 Modifier 的求值结果
	if (!World)
	{
		if (ModSpec && ModInfo)
		{
			UEnum* e = StaticEnum<EGameplayModOp::Type>();
			FString ModifierOpStr = e->GetNameStringByValue(ModInfo->ModifierOp);
			DurationText = FText::Format(LOCTEXT("GameplayEffectMod", "Mod: {0}, Value: {1}"), FText::FromString(ModifierOpStr), ModSpec->GetEvaluatedMagnitude());
		}

		return DurationText;
	}

	// 无限时长：不区分回合制 / 实时制
	if (GameplayEffect.GetDuration() <= 0.f)
	{
		return LOCTEXT("GameplayEffectInfiniteDurationText", "Infinite Duration");
	}

	FNumberFormattingOptions NumberFormatOptions;
	NumberFormatOptions.MaximumFractionalDigits = 2;

	// 回合制：Duration 语义为「时机刻度」（回合数 / 出手次数），剩余量从 FAbilityTimerManager 的时机轴查询
	if (IsTurnBased())
	{
		const float Remaining = QueryTurnRemaining();
		return Remaining >= 0.f
			? FText::Format(LOCTEXT("GameplayEffectTurnDuration", "[Turn] Duration: {0}, Remaining: {1}"),
				FText::AsNumber(GameplayEffect.GetDuration(), &NumberFormatOptions),
				FText::AsNumber(Remaining, &NumberFormatOptions))
			: FText::Format(LOCTEXT("GameplayEffectTurnDurationNoTimer", "[Turn] Duration: {0}, Remaining: -"),
				FText::AsNumber(GameplayEffect.GetDuration(), &NumberFormatOptions));
	}

	// 非回合制：世界时间口径（秒）
	DurationText = FText::Format(LOCTEXT("GameplayEffectDurationStr", "Duration: {0},Remaining: {1} (Start: {2} / {3} / {4})"),
		FText::AsNumber(GameplayEffect.GetDuration(), &NumberFormatOptions),
		FText::AsNumber(GameplayEffect.GetTimeRemaining(World->GetTimeSeconds()), &NumberFormatOptions),
		FText::AsNumber(GameplayEffect.StartServerWorldTime, &NumberFormatOptions),
		FText::AsNumber(GameplayEffect.CachedStartServerWorldTime, &NumberFormatOptions),
		FText::AsNumber(GameplayEffect.StartWorldTime, &NumberFormatOptions));

	return DurationText;
}

FText FGASGameplayEffectNode::GetPeriodText() const
{
	// 子行（Modifier）不显示周期
	if (!World)
	{
		return FText::GetEmpty();
	}

	const float Period = GameplayEffect.GetPeriod();
	if (Period <= 0.f)
	{
		return LOCTEXT("GameplayEffectNoPeriod", "-");
	}

	FNumberFormattingOptions NumberFormatOptions;
	NumberFormatOptions.MaximumFractionalDigits = 2;

	// 回合制：Period 语义为「每 N 个刻度触发一次」；非回合制：秒
	return IsTurnBased()
		? FText::Format(LOCTEXT("GameplayEffectPeriodTurn", "{0} Turn"), FText::AsNumber(Period, &NumberFormatOptions))
		: FText::Format(LOCTEXT("GameplayEffectPeriodSecond", "{0} s"), FText::AsNumber(Period, &NumberFormatOptions));
}

FText FGASGameplayEffectNode::GetTimingText() const
{
	// 子行（Modifier）不显示时机
	if (!World)
	{
		return FText::GetEmpty();
	}

	const UGameplayEffect* Def = GameplayEffect.Spec.Def;
	if (!Def)
	{
		return LOCTEXT("GameplayEffectNoTiming", "-");
	}

	// Timing 为 GE 上的配置（未配置时回落默认时机 TimeAxis.Round.End），仅回合制下参与推进
	return FText::FromString(AbilityTimingTags::ResolveOrDefault(Def->Timing).ToString());
}

bool FGASGameplayEffectNode::IsTurnBased() const
{
	return ASComponent.IsValid() && ASComponent->IsTurnBased();
}

float FGASGameplayEffectNode::QueryTurnRemaining() const
{
	UAbilitySystemComponent* ASC = ASComponent.Get();
	if (!ASC)
	{
		return -1.f;
	}

	// 与 GameplayEffect.cpp 注册 Timer 时保持一致：未配置 Timing 回落默认时机
	const FGameplayTag Timing = GameplayEffect.Spec.Def
		? AbilityTimingTags::ResolveOrDefault(GameplayEffect.Spec.Def->Timing)
		: AbilityTimingTags::TAG_TIMEAXIS_ROUND_END.GetTag();

	// 句柄按值拷贝：查询接口接收非 const 引用，节点内保存的是 GE 的展示快照
	FTimerHandle DurationHandle = GameplayEffect.DurationHandle;
	const float Remaining = UAbilitySystemGlobals::Get().GetAbilityTimerManager().GetAbilityTimerRemaining(ASC, Timing, DurationHandle);

	// 查不到（接口返回 0）视为不可用，UI 显示 "-"
	return Remaining > 0.f ? Remaining : -1.f;
}

FText FGASGameplayEffectNode::GetStackText() const
{
	FText StackText;

	if (World && GameplayEffect.Spec.GetStackCount() > 1)
	{
		if (GameplayEffect.Spec.Def->GetStackingType() == EGameplayEffectStackingType::AggregateBySource)
		{
			StackText =  FText::Format(LOCTEXT("GameplayEffectStacksForm", "Stacks: {0},From: {1}"), GameplayEffect.Spec.GetStackCount(), FText::FromString(GetNameSafe(GameplayEffect.Spec.GetContext().GetInstigatorAbilitySystemComponent()->GetAvatarActor_Direct())));
		}
		else
		{
			StackText =  FText::Format(LOCTEXT("GameplayEffectStacks", "Stacks: {0}"), GameplayEffect.Spec.GetStackCount());
		}
	}

	return StackText;
}

FName FGASGameplayEffectNode::GetLevelStr() const
{
	if (!World) return FName();

	return *LexToSanitizedString(GameplayEffect.Spec.GetLevel());
}

FText FGASGameplayEffectNode::GetPredictedText() const
{
	FText PredictionText;

	if (World && GameplayEffect.PredictionKey.IsValidKey())
	{
		if (GameplayEffect.PredictionKey.WasLocallyGenerated())
		{
			PredictionText =  LOCTEXT("GameplayEffectPredictedWaiting", "Predicted and Waiting");
		}
		else
		{
			PredictionText = LOCTEXT("GameplayEffectPredictedCaught", "Predicted and Caught Up");
		}
	}

	return PredictionText;
}

FName FGASGameplayEffectNode::GetGrantedTagsName() const
{
	if (!World) return FName();

	FGameplayTagContainer GrantedTags;
	GameplayEffect.Spec.GetAllGrantedTags(GrantedTags);

	return *GrantedTags.ToStringSimple();
}

FName FGASGameplayEffectNode::GetGAName() const
{
	if (ModSpec && ModInfo)
	{
		return *ModInfo->Attribute.GetName();
	}

	return *GetNameSafe(GameplayEffect.Spec.Def);
}

FGASGameplayEffectNode::FGASGameplayEffectNode(TWeakObjectPtr<UAbilitySystemComponent> InASComponent, const UWorld* InWorld, const FActiveGameplayEffect& InGameplayEffect)
{
	ASComponent = InASComponent;
	World = InWorld;
	ModInfo = nullptr;
	ModSpec = nullptr;

	// 只逐字段复制 UI 展示所需的数据。
	// 不能整体拷贝 FActiveGameplayEffect：其 EventSet（FActiveGameplayEffectEvents，含 4 个 multicast delegate）
	// 会被一并拷贝，副本析构时在 delegate 的 invocation list 析构处崩溃
	// （EXCEPTION_ACCESS_VIOLATION reading 0xa，栈：FActiveGameplayEffectEvents::~FActiveGameplayEffectEvents
	//   -> DestructItems<TDelegateBase<FNotThreadSafeNotCheckedDelegateMode>>）。
	GameplayEffect.Handle = InGameplayEffect.Handle;
	GameplayEffect.Spec = InGameplayEffect.Spec;
	GameplayEffect.PredictionKey = InGameplayEffect.PredictionKey;
	GameplayEffect.StartServerWorldTime = InGameplayEffect.StartServerWorldTime;
	GameplayEffect.CachedStartServerWorldTime = InGameplayEffect.CachedStartServerWorldTime;
	GameplayEffect.StartWorldTime = InGameplayEffect.StartWorldTime;
	GameplayEffect.bIsInhibited = InGameplayEffect.bIsInhibited;
	GameplayEffect.ClientCachedStackCount = InGameplayEffect.ClientCachedStackCount;
	// Timer 句柄必须一并拷贝：回合制下 Time 列的 Remaining 依赖该句柄到 FAbilityTimerManager 的时机轴上查询
	// （漏拷会得到默认无效句柄，查询永远失败 → Remaining 显示 "-"）
	GameplayEffect.DurationHandle = InGameplayEffect.DurationHandle;
	GameplayEffect.PeriodHandle = InGameplayEffect.PeriodHandle;

	CreateChild();
}

FGASGameplayEffectNode::FGASGameplayEffectNode(const FModifierSpec* InModSpec,const FGameplayModifierInfo* InModInfo)
{
	World = nullptr;
	ModInfo = InModInfo;
	ModSpec = InModSpec;
}

void FGASGameplayEffectNode::CreateChild()
{
	if (!GameplayEffect.Spec.Def)
	{
		return;
	}

	// 两个数组长度取小：Spec.Modifiers 由 Def->Modifiers 求值而来，但动态构造的 Spec 可能存在数量不一致，避免越界
	const int32 ModCount = FMath::Min(GameplayEffect.Spec.Modifiers.Num(), GameplayEffect.Spec.Def->Modifiers.Num());
	for (int32 ModIdx = 0; ModIdx < ModCount; ++ModIdx)
	{
		AddChildNode(MakeShareable(new FGASGameplayEffectNode(&GameplayEffect.Spec.Modifiers[ModIdx], &GameplayEffect.Spec.Def->Modifiers[ModIdx])));
	}
}

void SGASGameplayEffectTreeItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	this->WidgetInfo = InArgs._WidgetInfoToVisualize;
	this->SetPadding(0);

	check(WidgetInfo.IsValid());

	GAName = WidgetInfo->GetGAName();
	DurationText = WidgetInfo->GetDurationText();
	PeriodText = WidgetInfo->GetPeriodText();
	TimingText = WidgetInfo->GetTimingText();
	StackText = WidgetInfo->GetStackText();
	LevelStr = WidgetInfo->GetLevelStr();
	GrantedTagsName = WidgetInfo->GetGrantedTagsName();
	
	SMultiColumnTableRow< TSharedRef<FGASGameplayEffectNodeBase> >::Construct(SMultiColumnTableRow< TSharedRef<FGASGameplayEffectNodeBase> >::FArguments().Padding(0), InOwnerTableView);
}

TSharedRef<SWidget> SGASGameplayEffectTreeItem::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (NAME_GAGameplayEffectName == ColumnName)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SExpanderArrow, SharedThis(this))
				.IndentAmount(16)
				.ShouldDrawWires(true)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(FMargin(2.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text(FText::FromName(GAName))
					.Justification(ETextJustify::Center)
				]
			];
	}
	else if (NAME_GAGameplayEffectDuration == ColumnName)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(DurationText)
				.Justification(ETextJustify::Center)
			];
	}
	else if (NAME_GAGameplayEffectPeriod == ColumnName)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(PeriodText)
				.Justification(ETextJustify::Center)
			];
	}
	else if (NAME_GAGameplayEffectTiming == ColumnName)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(TimingText)
				.Justification(ETextJustify::Center)
				.ToolTipText(TimingText)
			];
	}
	else if (NAME_GAGameplayEffectStack == ColumnName)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(StackText)
				.Justification(ETextJustify::Center)
			];
	}
	else if (NAME_GAGameplayEffectLevel == ColumnName)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromName(LevelStr))
				.Justification(ETextJustify::Center)
			];
	}
	else if (NAME_GAGameplayEffectGrantedTags == ColumnName)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromName(GrantedTagsName))
				.Justification(ETextJustify::Center)
				.ToolTipText(FText::FromName(GrantedTagsName))
			];
	}

	return SNullWidget::NullWidget;
}


void FGASGameplayEffectNodeBase::AddChildNode(TSharedRef<FGASGameplayEffectNodeBase> InChildNode)
{
	ChildNodes.Add(MoveTemp(InChildNode));
}

const TArray<TSharedRef<FGASGameplayEffectNodeBase>>& FGASGameplayEffectNodeBase::GetChildNodes() const
{
	return ChildNodes;
}

#undef LOCTEXT_NAMESPACE

