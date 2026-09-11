// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "Combat/SVCombatTypes.h"
#include "UBTTask_CatBase.generated.h"

class ACharacter;
class UBlackboardComponent;
class UCatAIControlData;
class USVCharacterTurnComponent;

/**
 * Cat 行为树 Task 公共基类：提供 Owner / 回合组件 / 黑板 / 目标 / 技能 Tag 等读写辅助。
 *
 * 移植说明：
 *  - 源项目基类面向 ASVCharacterBase 强类型角色，本框架抽象为 ACharacter，
 *    角色能力/状态查询统一走角色 ASC 与 USVCharacterTurnComponent（与框架其它部分一致）；
 *  - AI 控制数据由角色扩展组件（USVCharacterExtensionComponent）改为 AI 控制器（ACatAIControllerBase）持有；
 *  - 目标血量由源项目 USVHealthComponent 改为按可配置的 GAS 属性读取（框架层不绑定具体 AttributeSet）。
 */
UCLASS(Abstract)
class CATCOMBATFRAMEWORK_API UBTTask_CatBase : public UBTTaskNode
{
	GENERATED_BODY()

public:
	/** 取 Owner 角色（AI 控制器的 Pawn） */
	static ACharacter* GetOwnerCharacter(UBehaviorTreeComponent& OwnerComp);

	/** 取角色回合组件 */
	static USVCharacterTurnComponent* GetTurnComponent(UBehaviorTreeComponent& OwnerComp);

	/** 取黑板组件 */
	static UBlackboardComponent* GetBlackboard(UBehaviorTreeComponent& OwnerComp);

	/** 读黑板 TargetActor 返回角色（无则 nullptr） */
	static ACharacter* GetTargetActor(UBehaviorTreeComponent& OwnerComp);

	/** 写黑板 TargetActor */
	static void WriteTargetActor(UBehaviorTreeComponent& OwnerComp, AActor* Target);

	/** 写黑板 SelectedAbilityTag（Name 键，存 TagName） */
	static void WriteSelectedAbilityTag(UBehaviorTreeComponent& OwnerComp, const FGameplayTag& Tag);

	/** 从 Tag 容器中取第一个「当前职责可激活」的 Tag（成功返回 true 并输出） */
	static bool PickFirstActivatableTag(UBehaviorTreeComponent& OwnerComp, ECombatTurnRole Role, const FGameplayTagContainer& Tags, FGameplayTag& OutTag);

	/** 取 AI 控制数据：从 AI 控制器（ACatAIControllerBase）读取；控制器类型不符或未配置时返回 nullptr */
	static UCatAIControlData* GetAIControlData(UBehaviorTreeComponent& OwnerComp);

	/** 读 Actor 的 ASC 上是否持有指定 GameplayTag（无 ASC 返回 false） */
	static bool HasMatchingTag(AActor* Actor, const FGameplayTag& Tag);

	/** 按属性读取 Actor 的血量值（属性未配置 / 无 ASC / 无该属性集时返回 false） */
	static bool GetHealthValue(AActor* Actor, const FGameplayAttribute& HealthAttribute, float& OutHealth);

	/** 按属性读取 Actor 的血量比例（属性未配置 / 无 ASC / 最大值为 0 时返回 false） */
	static bool GetHealthNormalized(AActor* Actor, const FGameplayAttribute& HealthAttribute, const FGameplayAttribute& MaxHealthAttribute, float& OutNormalized);
};
