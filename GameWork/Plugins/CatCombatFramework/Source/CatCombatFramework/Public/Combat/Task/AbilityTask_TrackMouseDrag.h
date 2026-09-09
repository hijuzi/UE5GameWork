// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_TrackMouseDrag.generated.h"

class UInputAction;
class UEnhancedInputComponent;
class APlayerController;
class ACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTrackMouseDragDelegate, FVector2D, SuccessVector, bool, bSucceeded, FVector2D, LiveVector);

/** 2D 滑动方向 */
UENUM(BlueprintType)
enum class ESwipeDirection : uint8
{
	None  UMETA(DisplayName = "无"),
	Up    UMETA(DisplayName = "上"),
	Down  UMETA(DisplayName = "下"),
	Left  UMETA(DisplayName = "左"),
	Right UMETA(DisplayName = "右"),
};


/**
 * 追踪鼠标拖动手势的能力任务基类（抽象）。
 * 从激活起持续追踪鼠标位置，结束时广播净位移向量（终点 - 起点，屏幕像素）。
 * 松开按键始终作为兜底结束；具体的"滑动类型"提前结束条件由子类 CheckSuccessConditions 实现。
 */
UCLASS(Abstract)
class CATCOMBATFRAMEWORK_API UAbilityTask_TrackMouseDrag : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAbilityTask_TrackMouseDrag(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 命中/滑动成功（由"未成功"转为"成功"）瞬间广播；参数为成功位移向量、是否成功、实时位移向量 */
	UPROPERTY(BlueprintAssignable)
	FTrackMouseDragDelegate OnHitSucceeded;

	/** 任务正常结束时广播，参数为成功位移向量（终点 - 起点，屏幕像素）、是否成功、实时位移向量 */
	UPROPERTY(BlueprintAssignable)
	FTrackMouseDragDelegate OnDragEnded;

	/** 所属 GA 中途被打断/结束时广播（参数为成功位移向量、是否成功、实时位移向量） */
	UPROPERTY(BlueprintAssignable)
	FTrackMouseDragDelegate OnDragCancelled;

	/** 拖拽异常退出广播（无鼠标输入或 InputAction 无输入时广播；参数同上） */
	UPROPERTY(BlueprintAssignable)
	FTrackMouseDragDelegate OnDragAborted;

	/** 每帧追踪鼠标时广播（参数为成功位移向量、是否成功、实时位移向量 = CurrentMousePos - StartMousePos） */
	UPROPERTY(BlueprintAssignable)
	FTrackMouseDragDelegate OnTick;

	/** 滑动起点（屏幕像素，蓝图只读） */
	UPROPERTY(BlueprintReadOnly, Category = "Swipe")
	FVector2D StartMousePos = FVector2D::ZeroVector;

	/** 当前鼠标滑动点（屏幕像素，每帧更新，蓝图只读） */
	UPROPERTY(BlueprintReadOnly, Category = "Swipe")
	FVector2D CurrentMousePos = FVector2D::ZeroVector;

	/** 满足成功条件时记录的拖拽位移向量（屏幕像素，终点 - 起点，蓝图只读） */
	UPROPERTY(BlueprintReadOnly, Category = "Swipe")
	FVector2D SuccessDragVector = FVector2D::ZeroVector;

	/** 是否开启 Debug 可视化（总开关，与运行时 CVar cat.TrackMouseDrag.Debug 共同生效） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swipe|Debug")
	bool bDebug = false;

	/** 是否已满足滑动成功条件（方向滑动达到距离 / 目标框滑动命中；满足时置为 true，蓝图只读） */
	UPROPERTY(BlueprintReadOnly, Category = "Swipe")
	bool bHasSucceeded = false;

	/** 输入动作是否已结束（检测到松开/取消时置为 true，蓝图只读，仅作状态标记）：
	 *  松手收尾路径为 true；命中 + 提前退出路径保持 false，可据此区分两种结束方式。任务重新激活时重置为 false */
	UPROPERTY(BlueprintReadOnly, Category = "Swipe")
	bool bHasInputCompleted = false;

protected:
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
	virtual void TickTask(float DeltaTime) override;

	/** 子类实现：判断是否满足提前结束条件（PrevPos 为上一帧鼠标屏幕位置，供线段相交检测避免高速穿过） */
	virtual bool CheckSuccessConditions(const FVector2D& CurrentPos, const FVector2D& PrevPos) PURE_VIRTUAL(UAbilityTask_TrackMouseDrag::CheckSuccessConditions, return false;);

	/** 供子类工厂函数设置监听松开事件的输入动作 */
	void SetInputAction(UInputAction* InInputAction);

	/** Debug 可视化（子类可扩展绘制各自的元素） */
	virtual void DrawDebug(const FVector2D& CurrentPos, const FVector2D& PrevPos);

	/** 满足成功条件后的 Debug 可视化：在 SuccessDragVector 终点画大球，起点画绿色箭头 */
	void DrawSuccessDebug();

	// 获取玩家角色（AvatarActor）头上的 EnhancedInputComponent
	UEnhancedInputComponent* GetAvatarEnhancedInputComponent() const;

	// 获取并缓存 PlayerController（Activate 时首次解析，Tick 复用缓存，避免每帧重复查找）
	const APlayerController* GetPlayerController();

	// Debug 绘制持续的时间（秒），用于形成轨迹
	float DebugDrawDuration = 1.0f;

	// Debug 投影深度（屏幕坐标投影到世界空间的深度）
	// 值越小越贴近镜头；150 让 debug 图形贴在镜头前，接近屏幕 2D 效果
	float DebugDrawDepth = 150.0f;

	// 满足成功条件（达到方向距离 / 命中目标）后是否立即结束任务（由子类工厂函数传入；true 则成功即结束，不等待鼠标松开）
	bool bEndOnSuccess = false;

private:
	/** 输入动作结束（Completed / Canceled 均可触发）时的内部回调：置输入完成标记并正常结束任务 */
	void HandleInputCompleted(const struct FInputActionValue& Value);
	void FinishTask(bool bCancelled);

	/** 异常退出：广播 OnDragAborted 并结束任务（不广播 OnDragCancelled） */
	void AbortTask();

	/** 判断当前是否使用鼠标键盘输入（手柄/触摸等非鼠标键盘输入返回 false） */
	bool IsUsingMouseAndKeyboard();

	/** 判断 InputAction 当前是否处于按下状态（未按下/无输入返回 false） */
	bool IsInputActionPressed();

	UPROPERTY()
	TObjectPtr<UInputAction> InputAction;

	// 缓存 PlayerController，Activate 时解析一次，Tick 期间直接复用
	TWeakObjectPtr<APlayerController> CachedPlayerController;

	// 保存绑定 handle（Completed + Canceled），用于 OnDestroy 时主动解绑
	uint32 InputCompletedBindHandle = 0;
	uint32 InputCanceledBindHandle = 0;
	bool bBound = false;

	// 当前净位移向量（当前 - 起点），每帧在 Tick 中更新
	FVector2D NetDelta = FVector2D::ZeroVector;

	bool bHasValidMousePos = false;
	bool bFinished = false;
};

/**
 * 方向滑动 Task：沿指定方向累计达到指定距离提前结束（Case1）。
 */
UCLASS()
class CATCOMBATFRAMEWORK_API UAbilityTask_TrackMouseDragDirection : public UAbilityTask_TrackMouseDrag
{
	GENERATED_BODY()

public:
	UAbilityTask_TrackMouseDragDirection(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", ToolTip = "追踪鼠标拖拽方向的任务，沿指定方向累计达到指定距离即结束"))
	static UAbilityTask_TrackMouseDragDirection* TrackMouseDragDirection(
		/** 所属的 GameplayAbility（自动取自身，蓝图隐藏该引脚） */
		UGameplayAbility* OwningAbility,
		/** 用于监听"松开按键"的输入动作，松开时作为兜底结束条件 */
		UInputAction* InInputAction,
		/** 满足滑动成功条件（沿目标方向达到要求距离）后是否立即结束任务（true 则成功即结束，false 则继续拖拽直到鼠标松开） */
		bool InEndOnSuccess = false,
		/** 要求的目标滑动方向（None 表示不判定方向） */
		ESwipeDirection InRequiredDirection = ESwipeDirection::None,
		/** 沿目标方向需要达到的最小像素距离，达到即视为成功 */
		float InRequiredDistance = 0.0f);

protected:
	virtual bool CheckSuccessConditions(const FVector2D& CurrentPos, const FVector2D& PrevPos) override;
	virtual void DrawDebug(const FVector2D& CurrentPos, const FVector2D& PrevPos) override;

private:
	// 目标滑动方向（None 表示不判定方向）
	ESwipeDirection RequiredDirection = ESwipeDirection::None;

	// 沿 RequiredDirection 方向需要达到的最小像素距离
	float RequiredDistance = 0.0f;
};

/**
 * 目标框滑动 Task：鼠标滑入目标框提前结束（Case2）。
 */
UCLASS()
class CATCOMBATFRAMEWORK_API UAbilityTask_TrackMouseDragTargetBox : public UAbilityTask_TrackMouseDrag
{
	GENERATED_BODY()

public:
	UAbilityTask_TrackMouseDragTargetBox(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", ToolTip = "追踪鼠标拖拽到目标角色的任务，滑入目标主要受击范围即结束"))
	static UAbilityTask_TrackMouseDragTargetBox* TrackMouseDragTargetBox(
		/** 所属的 GameplayAbility（自动取自身，蓝图隐藏该引脚） */
		UGameplayAbility* OwningAbility,
		/** 用于监听"松开按键"的输入动作，松开时作为兜底结束条件 */
		UInputAction* InInputAction,
		/** 待检测的目标角色列表（遍历每个目标，通过 GetMainHitReactionRange 获取其受击范围判定） */
		const TArray<ACharacter*>& InTargetCharacters,
		/** 命中目标成功后是否立即结束任务（true 则命中即结束，false 则继续收集命中目标直到鼠标松开） */
		bool InEndOnSuccess = false);

	/** 获取已命中的目标角色列表（只读，返回副本） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Swipe")
	TArray<ACharacter*> GetHitCharacters() const;

	/** 获取第一个命中的目标角色（按命中顺序取第一个非空角色，无命中时返回 nullptr） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Swipe")
	ACharacter* GetFirstHitCharacter() const;

protected:
	virtual bool CheckSuccessConditions(const FVector2D& CurrentPos, const FVector2D& PrevPos) override;
	virtual void DrawDebug(const FVector2D& CurrentPos, const FVector2D& PrevPos) override;

private:
	// 将屏幕坐标反投影到「经过 PlanePoint、法线为 PlaneNormal」的平面，返回是否成功并输出交点世界坐标
	bool ProjectScreenPosToPlane(const APlayerController* PC, const FVector2D& ScreenPos, const FVector& PlaneNormal, const FVector& PlanePoint, FVector& OutWorldPos) const;

	// 待检测的目标角色列表（由工厂函数传入，遍历通过 GetMainHitReactionRange 获取受击范围）
	UPROPERTY()
	TArray<ACharacter*> TargetCharacters;

	// 已命中的目标角色列表（跨帧去重累积：收集整个拖拽过程命中的所有目标）
	UPROPERTY()
	TArray<ACharacter*> HitCharacters;
};
