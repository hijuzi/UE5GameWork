// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Combat/SVCombatTypes.h"
#include "SVCombatPlayerInputBridge.generated.h"

class ACharacter;

/**
 * 玩家输入桥接（适配层）。
 * 把玩家输入产出包装成 FActionRequest，交由 USVCombatFunctionLibrary::RequestAction 统一激活。
 *
 * 移植说明：源项目依赖 ASvCharacter / UCombatMouseComponent 作为决策源，
 * 此处玩家类型抽象为 ACharacter，输入决策源由外部注入（迁移期实现，当前为占位）。
 */
UCLASS()
class CATCOMBATFRAMEWORK_API USVCombatPlayerInputBridge : public UObject
{
	GENERATED_BODY()

public:
	/** 为指定玩家角色产出一个行动请求，返回是否成功产出 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Input")
	bool RequestPlayerAction(ACharacter* Player, FActionRequest& OutRequest);
};
