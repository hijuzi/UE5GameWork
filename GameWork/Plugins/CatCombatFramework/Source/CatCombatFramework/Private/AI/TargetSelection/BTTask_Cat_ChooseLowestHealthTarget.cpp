// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/TargetSelection/BTTask_Cat_ChooseLowestHealthTarget.h"

#include "GameFramework/Character.h"

#include "Combat/SVCombatFunctionLibrary.h"

UBTTask_Cat_ChooseLowestHealthTarget::UBTTask_Cat_ChooseLowestHealthTarget()
{
	NodeName = TEXT("Cat Choose Lowest Health Target");
}

FString UBTTask_Cat_ChooseLowestHealthTarget::GetStaticDescription() const
{
	return FString::Printf(TEXT("选血量最低目标 | 血量属性: %s"),
		HealthAttribute.IsValid() ? *HealthAttribute.GetName() : TEXT("未配置"));
}

EBTNodeResult::Type UBTTask_Cat_ChooseLowestHealthTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
	ACharacter* Owner = GetOwnerCharacter(OwnerComp);
	if (!Owner)
	{
		return EBTNodeResult::Failed;
	}

	// 攻击目标 = Owner 所在阵营的对方阵营角色（阵营判定已封装在 USVCombatFunctionLibrary::GetOpponentCombatCharacterList）
	const TArray<ACharacter*> Targets = USVCombatFunctionLibrary::GetOpponentCombatCharacterList(Owner);

	ACharacter* Best = nullptr;
	float BestHealth = TNumericLimits<float>::Max();
	for (ACharacter* Target : Targets)
	{
		if (!IsValid(Target))
		{
			continue;
		}

		// 移植说明：源项目读 USVHealthComponent，本框架改读可配置 GAS 属性；取不到血量数据的目标不参与血量最低筛选
		float Health = 0.f;
		if (!GetHealthValue(Target, HealthAttribute, Health))
		{
			continue;
		}

		if (Health < BestHealth)
		{
			BestHealth = Health;
			Best = Target;
		}
	}

	if (!Best)
	{
		return EBTNodeResult::Failed;
	}

	WriteTargetActor(OwnerComp, Best);
	return EBTNodeResult::Succeeded;
}
