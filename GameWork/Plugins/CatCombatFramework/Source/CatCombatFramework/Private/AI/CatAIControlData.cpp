// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/CatAIControlData.h"

#include "BehaviorTree/BehaviorTree.h"

UBehaviorTree* UCatAIControlData::GetSubTree(ECatAISubTreeType SubTreeType) const
{
	switch (SubTreeType)
	{
	case ECatAISubTreeType::TargetSelection:
		return TargetSelectionTree;
	case ECatAISubTreeType::SkillSelection:
		return AttackSkillSelectionTree;
	case ECatAISubTreeType::DefenseDecision:
		return DefenseSkillDecisionTree;
	default:
		return nullptr;
	}
}
