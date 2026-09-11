#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SListView.h"

static FName NAME_GAGameplayEffectName(TEXT("GameplayEffectName"));
static FName NAME_GAGameplayEffectDuration(TEXT("GameplayEffectDuration"));
static FName NAME_GAGameplayEffectPeriod(TEXT("GameplayEffectPeriod"));
static FName NAME_GAGameplayEffectTiming(TEXT("GameplayEffectTiming"));
static FName NAME_GAGameplayEffectStack(TEXT("GameplayEffectStack"));
static FName NAME_GAGameplayEffectLevel(TEXT("GameplayEffectLevel"));
static FName NAME_GAGameplayEffectPrediction(TEXT("GameplayEffectPrediction"));
static FName NAME_GAGameplayEffectGrantedTags(TEXT("GameplayEffectGrantedTags"));

class UAbilitySystemComponent;

class FGASGameplayEffectNodeBase
{
public:

	virtual ~FGASGameplayEffectNodeBase(){};

public:

	// 当前GA名字
	virtual FName GetGAName() const = 0;

	// 当前时间（回合制下为回合 / 出手刻度信息，非回合制为世界时间）
	virtual FText GetDurationText() const = 0;

	// 当前周期信息（回合制下为「每 N 个刻度」，非回合制为秒）
	virtual FText GetPeriodText() const = 0;

	// 当前时机信息（GE 配置的 Timing Tag，未配置显示默认时机）
	virtual FText GetTimingText() const = 0;

	// 当前堆信息
	virtual FText GetStackText() const = 0;

	// 当前等级信息
	virtual FName GetLevelStr() const = 0;

	// 当前预触发信息
	virtual FText GetPredictedText() const = 0;

	// 当前含有的Tag信息
	virtual FName GetGrantedTagsName() const = 0;

public:
	// 将给定节点添加到此小部件的子级列表中（此节点将保留对实例的强引用）
	void AddChildNode(TSharedRef<FGASGameplayEffectNodeBase> InChildNode);

	// 返回子条目的数组
	const TArray<TSharedRef<FGASGameplayEffectNodeBase>>& GetChildNodes() const;

protected:

	FGASGameplayEffectNodeBase(){};

protected:
	/** 子级列表 */
	TArray<TSharedRef<FGASGameplayEffectNodeBase>> ChildNodes;
};


class SGASGameplayEffectTreeItem : public SMultiColumnTableRow<TSharedRef<FGASGameplayEffectNodeBase>>
{
public:

	SLATE_BEGIN_ARGS(SGASGameplayEffectTreeItem)
		: _WidgetInfoToVisualize()
	{}
	SLATE_ARGUMENT(TSharedPtr<FGASGameplayEffectNodeBase>, WidgetInfoToVisualize)
		SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

public:

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

protected:
	/** 关于我们正在可视化的小部件的信息 */
	TSharedPtr<FGASGameplayEffectNodeBase> WidgetInfo;

	FName GAName;
	FText DurationText;
	FText PeriodText;
	FText TimingText;
	FText StackText;
	FName LevelStr;
	FName GrantedTagsName;
};

class FGASGameplayEffectNode : public FGASGameplayEffectNodeBase
{
public:
	virtual ~FGASGameplayEffectNode() override;

	static TSharedRef<FGASGameplayEffectNode> Create(TWeakObjectPtr<UAbilitySystemComponent> InASComponent, const UWorld* World, const FActiveGameplayEffect& InGameplayEffect);


public:

	virtual FName GetGAName() const override;

	// Time 列
	virtual FText GetDurationText() const override;

	// Period 列
	virtual FText GetPeriodText() const override;

	// Timing 列
	virtual FText GetTimingText() const override;

	virtual FText GetStackText() const override;

	virtual FName GetLevelStr() const override;

	virtual FText GetPredictedText() const override;

	virtual FName GetGrantedTagsName() const override;

private:

	explicit FGASGameplayEffectNode(TWeakObjectPtr<UAbilitySystemComponent> InASComponent, const UWorld* World, const FActiveGameplayEffect& InGameplayEffect);

	explicit FGASGameplayEffectNode(const FModifierSpec* ModSpec,const FGameplayModifierInfo* ModInfo);

protected:

	void CreateChild();

	/** 该 GE 所在 ASC 是否已启用回合制（ASC 失效时返回 false） */
	bool IsTurnBased() const;

	/** 查询本 GE 在当前 Timing 轴上的剩余刻度；查不到返回 -1（UI 显示 "-"） */
	float QueryTurnRemaining() const;

protected:
	const UWorld* World;

	TWeakObjectPtr<UAbilitySystemComponent> ASComponent;

	FActiveGameplayEffect GameplayEffect;

	const FModifierSpec* ModSpec;
	const FGameplayModifierInfo* ModInfo;
};
