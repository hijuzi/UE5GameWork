// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/SubTree/BTTask_CatSubTree.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

#include "AI/UBTTask_CatBase.h"

UBTTask_CatSubTree::UBTTask_CatSubTree(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Cat SubTree");
}

EBTNodeResult::Type UBTTask_CatSubTree::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 从 AI 控制数据读取对应子树资产，命中则覆盖默认子树
	if (UCatAIControlData* AIControlData = UBTTask_CatBase::GetAIControlData(OwnerComp))
	{
		if (UBehaviorTree* SubTree = AIControlData->GetSubTree(SubTreeType))
		{
			SetBehaviorAsset(SubTree);
		}
	}

	return Super::ExecuteTask(OwnerComp, NodeMemory);
}

FString UBTTask_CatSubTree::GetStaticDescription() const
{
	const UEnum* Enum = StaticEnum<ECatAISubTreeType>();
	const FString TypeName = Enum ? Enum->GetDisplayNameTextByValue(static_cast<int64>(SubTreeType)).ToString() : TEXT("未知");
	return FString::Printf(TEXT("子树类型: %s"), *TypeName);
}
