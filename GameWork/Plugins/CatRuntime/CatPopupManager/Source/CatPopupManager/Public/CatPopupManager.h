// Copyright SegameVictory Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CatPopupManager.generated.h"

/**
 * Cat弹窗管理器（GameInstanceSubsystem）
 * 负责游戏内所有弹窗的统一调度，包括优先级、队列管理、显示/隐藏生命周期
 */
UCLASS()
class CATPOPUPMANAGER_API UCatPopupManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 获取 CatPopupManager 实例 */
	UFUNCTION(BlueprintCallable, Category = "Cat Popup Manager")
	static UCatPopupManager* GetInstance(const UObject* WorldContextObject);

	// UGameInstanceSubsystem 接口
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
