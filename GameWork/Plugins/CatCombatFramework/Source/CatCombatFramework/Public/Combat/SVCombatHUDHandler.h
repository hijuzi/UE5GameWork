// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Combat/Widget/SVCombatHUDLayout.h"
#include "SVCombatHUDHandler.generated.h"

class USVCombatManagerSubsystem;
struct FStreamableHandle;

/**
 * 战斗 HUD 显隐参数。
 * 由调用方决定是否播放动画、是否自动联动暂停/恢复战斗回合。
 */
USTRUCT(BlueprintType)
struct FSVCombatHUDVisibilityParams
{
	GENERATED_BODY()

	/** 是否播放显隐动画（PlayShowAnimation / PlayHideAnimation） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|HUD")
	bool bPlayVisibilityAnimation = true;

	/** 是否播放加载/卸载动画（进场/出场动画，PlayLoadAnimation / PlayUnloadAnimation） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|HUD")
	bool bPlayLoadUnloadAnimation = false;

	/** 是否自动暂停/恢复战斗回合（显示时恢复，隐藏时暂停） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|HUD")
	bool bAutoPauseCombatRound = true;
};

/**
 * 战斗 HUD 处理器（执行层，非决策层）。
 * 仅负责 HUD 的创建、销毁、显隐与动画切换。
 * 不负责决定"何时"显示/隐藏 —— 该决策由 USVCombatManagerSubsystem 或外部调用方承担。
 *
 * 移植说明：原依赖 GameUIFramework（UPrimaryGameUILayout）与 GameplayMessageRuntime
 * （UGameplayMessageSubsystem），此处改为直接 CreateWidget + AddToViewport，
 * 并通过 ShowHUD()/HideHUD() 暴露显隐入口（替代原消息广播）。
 */
UCLASS(BlueprintType, Blueprintable)
class CATCOMBATFRAMEWORK_API USVCombatHUDHandler : public UObject
{
	GENERATED_BODY()

public:
	/** 初始化：记录持有者 */
	void Initialize(USVCombatManagerSubsystem* InOwner);

	/** 反初始化：销毁 HUD */
	void Shutdown();

	/** 异步创建并推入 HUD 到 Viewport（初始隐藏） */
	UFUNCTION(BlueprintCallable, Category = "Combat|HUD")
	void CreateHUD();

	/** 从 Viewport 移除 HUD 并取消异步加载 */
	UFUNCTION(BlueprintCallable, Category = "Combat|HUD")
	void DestroyHUD();

	/** 控制 HUD 可见性（被动执行，不判断业务状态） */
	void SetVisible(bool bVisible, const FSVCombatHUDVisibilityParams& Params = FSVCombatHUDVisibilityParams()) const;

	/** 显示 HUD（扩展点：外部直接调用，替代原 GameplayMessage 广播） */
	UFUNCTION(BlueprintCallable, Category = "Combat|HUD")
	void ShowHUD();

	/** 隐藏 HUD（扩展点：外部直接调用，替代原 GameplayMessage 广播） */
	UFUNCTION(BlueprintCallable, Category = "Combat|HUD")
	void HideHUD();

	/** HUD 是否已生成且可见 */
	bool IsHUDActive() const;

	/** 提供 World 上下文（Outer 为 CombatManagerSubsystem） */
	virtual UWorld* GetWorld() const override;

private:
	/** 持有者（弱引用，避免强引用环） */
	TWeakObjectPtr<USVCombatManagerSubsystem> OwnerSubsystem;

	/** 已生成的 HUD 实例（生命周期由 Viewport 管理） */
	UPROPERTY(Transient)
	TObjectPtr<USVCombatHUDLayout> SpawnedHUD;

	/** 异步加载句柄 */
	TSharedPtr<FStreamableHandle> StreamingHandle;
};
