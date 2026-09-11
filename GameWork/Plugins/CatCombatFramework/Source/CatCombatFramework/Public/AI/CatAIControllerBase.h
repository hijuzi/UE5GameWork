// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CatAIControllerBase.generated.h"

class UCatAIControlData;

/**
 * AI 控制器基类。
 *
 * 沉淀 AI 控制器通用逻辑：行为树/决策启动所需的 AI 控制数据（黑板书、主行为树、决策子树、技能决策配置）的持有与访问。
 *
 * 移植说明：
 *  - 源项目 ASVAIControllerBase 的飘字组件（USVFloatingTextComponent）依赖 Feedback 表现层，本框架未包含该层，已移除；
 *  - 源项目 AI 控制数据挂在角色扩展组件（USVCharacterExtensionComponent）上，本框架无该组件，
 *    改由 AI 控制器持有（UCatAIControlData 为本框架的数据资产类型），行为树任务经 UBTTask_CatBase::GetAIControlData 读取。
 */
UCLASS(Blueprintable, ClassGroup = AI)
class CATCOMBATFRAMEWORK_API ACatAIControllerBase : public AAIController
{
	GENERATED_BODY()

public:
	ACatAIControllerBase();

	/** 获取 AI 控制数据资产（可能为空，调用方需判空） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AI")
	UCatAIControlData* GetAIControlData() const { return AIControlData; }

	/** 设置 AI 控制数据资产 */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetAIControlData(UCatAIControlData* InAIControlData) { AIControlData = InAIControlData; }

protected:
	/** AI 控制数据：为空表示该控制器不走配置化行为树 AI（决策任务将直接失败）。敌人在各自 BP 子类里配置 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UCatAIControlData> AIControlData;
};
