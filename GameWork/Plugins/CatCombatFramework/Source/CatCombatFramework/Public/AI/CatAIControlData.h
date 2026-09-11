// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CatAIControlData.generated.h"

class UBehaviorTree;
class UBlackboardData;
class UAISkillDecisionData;

/** 子树类型：标识 UCatAIControlData 中的哪个子树槽位 */
UENUM(BlueprintType)
enum class ECatAISubTreeType : uint8
{
	TargetSelection UMETA(DisplayName = "目标选择"),
	SkillSelection  UMETA(DisplayName = "技能选择"),
	DefenseDecision UMETA(DisplayName = "防御决策"),
};

/**
 * AI 控制数据：一份资产收敛一个敌人的整套 AI 配置。
 *
 * 黑板书 + 主行为树（路由/激活） + 可替换决策子树。
 *
 * 移植说明：源项目挂在敌我通用的角色扩展组件（USVCharacterExtensionComponent）上，通过 GetAIControlData() 读取；
 * 本框架无该组件，改由 AI 控制器（ACatAIControllerBase）持有，行为树任务经 UBTTask_CatBase::GetAIControlData() 读取。
 */
UCLASS(BlueprintType, Blueprintable)
class CATCOMBATFRAMEWORK_API UCatAIControlData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 行为树使用的黑板书资产（初始化 BB 键用）。为空时回退行为树自带的 BlackboardAsset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Asset")
	TObjectPtr<UBlackboardData> BlackboardData;

	/** 主行为树资产（一次性决策执行，见 01 文档方案 A） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Asset")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	/** 目标选择子树：空则回退默认目标选择子树 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|SubTree")
	TObjectPtr<UBehaviorTree> TargetSelectionTree;

	/** 攻击技能选择决策树：空则回退默认技能选择子树 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|SubTree")
	TObjectPtr<UBehaviorTree> AttackSkillSelectionTree;

	/** 防御技能决策树：空则回退默认防御决策子树 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|SubTree")
	TObjectPtr<UBehaviorTree> DefenseSkillDecisionTree;

	/** AI 驱动数据（技能决策配置）：基类指针，可挂普通 Boss 基类资产或子类资产，运行时 Cast 读取子类专属字段 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Asset")
	TObjectPtr<UAISkillDecisionData> SkillDecisionData;

	/** 攻击决策思考延迟（秒）：Selecting 阶段先停该时长再跑行为树，模拟思考；0=不等待 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Timing", meta = (ClampMin = "0.0"))
	float AttackThinkDelay = 0.f;

	/** 按子树类型返回对应子树资产（未配置返回 nullptr，调用方回退默认） */
	UBehaviorTree* GetSubTree(ECatAISubTreeType SubTreeType) const;
};
