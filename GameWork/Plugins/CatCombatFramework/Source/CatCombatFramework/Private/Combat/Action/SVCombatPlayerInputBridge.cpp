// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/Action/SVCombatPlayerInputBridge.h"

#include "GameFramework/Character.h"
#include "CatCombatLog.h"

bool USVCombatPlayerInputBridge::RequestPlayerAction(ACharacter* Player, FActionRequest& OutRequest)
{
	if (!IsValid(Player))
	{
		return false;
	}

	// 玩家输入决策源在源项目为 UCombatMouseComponent（鼠标交互选目标/选技能）。
	// 框架层已解耦，具体「输入交互结果 → FActionRequest」的转换在迁移期接入。
	// 此处暂返回 false 占位，待新项目接入输入决策源。
	UE_LOG(LogCatCombatInput, Warning, TEXT("PlayerInputBridge: 玩家 %s 输入桥接尚未接入（迁移期实现）"), *Player->GetName());
	return false;
}
