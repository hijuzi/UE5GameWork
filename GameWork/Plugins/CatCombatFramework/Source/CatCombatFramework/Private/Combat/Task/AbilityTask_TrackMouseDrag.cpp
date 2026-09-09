// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/Task/AbilityTask_TrackMouseDrag.h"

#include "Combat/SVCombatCoreInterface.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "InputAction.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"
#include "CatCombatLog.h"

namespace
{
	// 鼠标拖拽 Debug 可视化开关（运行时通过控制台命令动态切换）：
	//   控制台输入  cat.TrackMouseDrag.Debug 1  开启
	//   控制台输入  cat.TrackMouseDrag.Debug 0  关闭
	static TAutoConsoleVariable<bool> CVarTrackMouseDragDebug(
		TEXT("cat.TrackMouseDrag.Debug"),
		false,
		TEXT("是否开启鼠标拖动任务的 Debug 可视化（0=关，1=开）"),
		ECVF_Default);

	FVector2D SwipeDirectionToVector(ESwipeDirection Direction)
	{
		switch (Direction)
		{
			case ESwipeDirection::Left:  return FVector2D(-1.0f, 0.0f);
			case ESwipeDirection::Right: return FVector2D(1.0f, 0.0f);
			case ESwipeDirection::Up:    return FVector2D(0.0f, -1.0f);
			case ESwipeDirection::Down:  return FVector2D(0.0f, 1.0f);
			default:                     return FVector2D::ZeroVector;
		}
	}

	FVector ScreenToWorld(const APlayerController* PC, const FVector2D& ScreenPos, float Depth)
	{
		FVector WorldLocation, WorldDirection;
		if (PC && PC->DeprojectScreenPositionToWorld(ScreenPos.X, ScreenPos.Y, WorldLocation, WorldDirection))
		{
			return WorldLocation + WorldDirection * Depth;
		}
		return FVector::ZeroVector;
	}

	// 点到线段 [A, B] 的最近距离平方（用于线段与球相交检测，避免高速滑动穿过球体）
	float DistSquaredPointToSegment(const FVector& A, const FVector& B, const FVector& Point)
	{
		const FVector AB = B - A;
		const float LenSq = AB.SizeSquared();
		if (LenSq <= KINDA_SMALL_NUMBER)
		{
			return FVector::DistSquared(Point, A);
		}
		const float T = FMath::Clamp(FVector::DotProduct(Point - A, AB) / LenSq, 0.0f, 1.0f);
		const FVector Closest = A + AB * T;
		return FVector::DistSquared(Point, Closest);
	}
}

// ==================== 基类 ====================

UAbilityTask_TrackMouseDrag::UAbilityTask_TrackMouseDrag(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
}

void UAbilityTask_TrackMouseDrag::SetInputAction(UInputAction* InInputAction)
{
	InputAction = InInputAction;
}

UEnhancedInputComponent* UAbilityTask_TrackMouseDrag::GetAvatarEnhancedInputComponent() const
{
	if (!Ability)
	{
		return nullptr;
	}

	const AActor* Avatar = Ability->GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return nullptr;
	}

	return Cast<UEnhancedInputComponent>(Avatar->InputComponent);
}

const APlayerController* UAbilityTask_TrackMouseDrag::GetPlayerController()
{
	// 缓存仍有效则直接返回，避免每帧重复查找
	if (CachedPlayerController.IsValid())
	{
		return CachedPlayerController.Get();
	}

	APlayerController* PC = nullptr;
	if (Ability && Ability->GetCurrentActorInfo())
	{
		PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	}
	if (!PC)
	{
		PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	}

	CachedPlayerController = PC;
	return CachedPlayerController.Get();
}

void UAbilityTask_TrackMouseDrag::Activate()
{
	Super::Activate();

	// 每次激活时重置状态，避免任务复用残留上次的结果
	bHasSucceeded = false;
	bHasInputCompleted = false;
	bFinished = false;

	if (!Ability || !AbilitySystemComponent.IsValid())
	{
		UE_LOG(LogCatCombatInput, Warning, TEXT("Activate: Ability 或 AbilitySystemComponent 无效，拖拽异常退出"));
		AbortTask();
		return;
	}

	// 无鼠标键盘输入（手柄/触摸）时，拖拽无意义，异常退出
	if (!IsUsingMouseAndKeyboard())
	{
		UE_LOG(LogCatCombatInput, Warning, TEXT("Activate: 无鼠标键盘输入，拖拽异常退出"));
		AbortTask();
		return;
	}

	// InputAction 无输入时，松开事件永不触发，异常退出
	if (!IsInputActionPressed())
	{
		UE_LOG(LogCatCombatInput, Warning, TEXT("Activate: InputAction 无输入，拖拽异常退出"));
		AbortTask();
		return;
	}

	const APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		UE_LOG(LogCatCombatInput, Warning, TEXT("Activate: 未获取到 PlayerController"));
	}

	// 记录滑动起点
	if (PC && PC->GetMousePosition(StartMousePos.X, StartMousePos.Y))
	{
		CurrentMousePos = StartMousePos;
		bHasValidMousePos = true;
		UE_LOG(LogCatCombatInput, Log, TEXT("Activate: 滑动起点记录成功 StartMousePos=(%.1f, %.1f)"), StartMousePos.X, StartMousePos.Y);
	}
	else
	{
		UE_LOG(LogCatCombatInput, Warning, TEXT("Activate: 获取鼠标起点位置失败"));
	}

	// 绑定输入动作 Completed / Canceled（松开按键，两条路径都作为兜底结束条件），并保存 handle 以便 OnDestroy 时主动解绑
	// 注意：输入绑定挂在玩家角色（AvatarActor）的 InputComponent 上，而非 PlayerController
	if (InputAction)
	{
		if (UEnhancedInputComponent* EIC = GetAvatarEnhancedInputComponent())
		{
			InputCompletedBindHandle = EIC->BindAction(
				InputAction, ETriggerEvent::Completed,
				this, &UAbilityTask_TrackMouseDrag::HandleInputCompleted).GetHandle();
			InputCanceledBindHandle = EIC->BindAction(
				InputAction, ETriggerEvent::Canceled,
				this, &UAbilityTask_TrackMouseDrag::HandleInputCompleted).GetHandle();
			bBound = true;
			UE_LOG(LogCatCombatInput, Log, TEXT("Activate: 输入绑定成功 InputAction=%s CompletedHandle=%u CanceledHandle=%u"),
				*InputAction->GetName(), InputCompletedBindHandle, InputCanceledBindHandle);
		}
		else
		{
			UE_LOG(LogCatCombatInput, Warning, TEXT("Activate: 未获取到 Avatar 的 EnhancedInputComponent，输入未绑定"));
		}
	}
	else
	{
		UE_LOG(LogCatCombatInput, Warning, TEXT("Activate: InputAction 为空，无法绑定松开事件"));
	}
}

void UAbilityTask_TrackMouseDrag::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (bFinished || !bHasValidMousePos)
	{
		return;
	}

	const APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return;
	}

	FVector2D CurPos = FVector2D::ZeroVector;
	if (!PC->GetMousePosition(CurPos.X, CurPos.Y))
	{
		UE_LOG(LogCatCombatInput, Warning, TEXT("TickTask: 获取鼠标位置失败"));
		return;
	}

	const FVector2D PrevMousePos = CurrentMousePos;  // 上一帧位置（用于画轨迹）
	CurrentMousePos = CurPos;

	// 更新当前净位移（当前 - 起点）
	NetDelta = CurrentMousePos - StartMousePos;

	// 每帧广播给蓝图
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnTick.Broadcast(SuccessDragVector, bHasSucceeded, NetDelta);
	}

	// Debug 可视化（蓝图 bDebug 或 运行时 CVar cat.TrackMouseDrag.Debug 任一开启即可）
	if (bDebug || CVarTrackMouseDragDebug.GetValueOnGameThread())
	{
		DrawDebug(CurPos, PrevMousePos);
	}

	const bool bPrevSucceeded = bHasSucceeded; // 进入命中检测前的状态快照
	if (CheckSuccessConditions(CurPos, PrevMousePos))
	{
		// 由"未成功"转为"成功"（首次满足成功条件）的瞬间广播成功事件；
		// 成功后状态持续保持，靠 bPrevSucceeded 边沿检测避免每帧重复广播
		if (!bPrevSucceeded && ShouldBroadcastAbilityTaskDelegates())
		{
			OnHitSucceeded.Broadcast(SuccessDragVector, bHasSucceeded, NetDelta);
		}

		UE_LOG(LogCatCombatInput, Log, TEXT("TickTask: 满足结束条件 NetDelta=(%.1f, %.1f) bEndOnSuccess=%d"), NetDelta.X, NetDelta.Y, bEndOnSuccess);
		if (bEndOnSuccess)
		{
			FinishTask(false);
		}
	}
}

void UAbilityTask_TrackMouseDrag::DrawDebug(const FVector2D& CurrentPos, const FVector2D& PrevPos)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return;
	}

	const FVector StartWorld = ScreenToWorld(PC, StartMousePos, DebugDrawDepth);
	const FVector CurWorld = ScreenToWorld(PC, CurrentPos, DebugDrawDepth);

	// 鼠标起点：方块（深度 150 贴脸，尺寸相应缩小）
	DrawDebugBox(World, StartWorld, FVector(0.8f), FColor::Yellow, false, DebugDrawDuration);
	// 鼠标实时位置：球（满足成功条件为绿色，否则为红色）
	const FColor CursorColor = bHasSucceeded ? FColor::Green : FColor::Red;
	DrawDebugSphere(World, CurWorld, 0.6f, 12, CursorColor, false, DebugDrawDuration);
}

void UAbilityTask_TrackMouseDrag::DrawSuccessDebug()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return;
	}

	// 终点 = 起点 + SuccessDragVector（屏幕坐标）
	const FVector2D EndScreen = StartMousePos + SuccessDragVector;
	const FVector StartWorld = ScreenToWorld(PC, StartMousePos, DebugDrawDepth);
	const FVector EndWorld = ScreenToWorld(PC, EndScreen, DebugDrawDepth);

	// 在 SuccessDragVector 终点绘制一个稍大的球（绿色）
	DrawDebugSphere(World, EndWorld, 1.6f, 16, FColor::Green, false, DebugDrawDuration);

	// 在起始位置绘制一个绿色箭头（指向 SuccessDragVector 方向）
	DrawDebugDirectionalArrow(World, StartWorld, EndWorld, 1.0f, FColor::Green, false, DebugDrawDuration, 0, 0.4f);
}

void UAbilityTask_TrackMouseDrag::HandleInputCompleted(const FInputActionValue& Value)
{
	UE_LOG(LogCatCombatInput, Log, TEXT("HandleInputCompleted: 检测到输入松开，正常结束任务 NetDelta=(%.1f, %.1f)"), NetDelta.X, NetDelta.Y);

	// 标记输入已完成，供蓝图区分"松手收尾"(true) 与"命中提前退出"(false)
	bHasInputCompleted = true;

	FinishTask(false);
}

void UAbilityTask_TrackMouseDrag::FinishTask(bool bCancelled)
{
	if (bFinished)
	{
		return;
	}
	bFinished = true;

	UE_LOG(LogCatCombatInput, Log, TEXT("FinishTask: 结束任务 bCancelled=%d NetDelta=(%.1f, %.1f)"), bCancelled, NetDelta.X, NetDelta.Y);

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (bCancelled)
		{
			OnDragCancelled.Broadcast(SuccessDragVector, bHasSucceeded, NetDelta);
		}
		else
		{
			OnDragEnded.Broadcast(SuccessDragVector, bHasSucceeded, NetDelta);
		}
	}

	// 正常完成且已满足成功条件时，绘制一次完成态 Debug（大球 + 绿色箭头）
	if (!bCancelled && bHasSucceeded && (bDebug || CVarTrackMouseDragDebug.GetValueOnGameThread()))
	{
		DrawSuccessDebug();
	}

	EndTask();
}

void UAbilityTask_TrackMouseDrag::AbortTask()
{
	if (bFinished)
	{
		return;
	}
	bFinished = true;

	// 异常退出只广播 OnDragAborted，不广播 OnDragCancelled
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnDragAborted.Broadcast(SuccessDragVector, bHasSucceeded, NetDelta);
	}

	EndTask();
}

bool UAbilityTask_TrackMouseDrag::IsUsingMouseAndKeyboard()
{
	const APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return false;
	}

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)
	{
		return false;
	}

	const UCommonInputSubsystem* CommonInput = UCommonInputSubsystem::Get(LP);
	if (!CommonInput)
	{
		return false;
	}

	return CommonInput->GetCurrentInputType() == ECommonInputType::MouseAndKeyboard;
}

bool UAbilityTask_TrackMouseDrag::IsInputActionPressed()
{
	if (!InputAction)
	{
		return false;
	}

	const APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return false;
	}

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)
	{
		return false;
	}

	const UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem)
	{
		return false;
	}

	const UEnhancedPlayerInput* PlayerInput = Subsystem->GetPlayerInput();
	if (!PlayerInput)
	{
		return false;
	}

	const FInputActionInstance* Instance = PlayerInput->FindActionInstanceData(InputAction);
	return Instance && Instance->GetTriggerEvent() != ETriggerEvent::None;
}

void UAbilityTask_TrackMouseDrag::OnDestroy(bool bInOwnerFinished)
{
	UE_LOG(LogCatCombatInput, Log, TEXT("OnDestroy: 任务销毁 bInOwnerFinished=%d bFinished=%d NetDelta=(%.1f, %.1f)"), bInOwnerFinished, bFinished, NetDelta.X, NetDelta.Y);

	// 若 GA 结束导致 Task 被强制销毁，且尚未广播过结果，按"取消"处理
	if (bInOwnerFinished && !bFinished && ShouldBroadcastAbilityTaskDelegates())
	{
		bFinished = true;
		OnDragCancelled.Broadcast(SuccessDragVector, bHasSucceeded, NetDelta);
		UE_LOG(LogCatCombatInput, Log, TEXT("OnDestroy: 任务被强制销毁，按取消处理并广播 OnDragCancelled"));
	}

	// 主动解绑，避免重复激活 GA 时绑定数组累积
	if (bBound)
	{
		if (UEnhancedInputComponent* EIC = GetAvatarEnhancedInputComponent())
		{
			if (InputCompletedBindHandle != 0)
			{
				EIC->RemoveBindingByHandle(InputCompletedBindHandle);
			}
			if (InputCanceledBindHandle != 0)
			{
				EIC->RemoveBindingByHandle(InputCanceledBindHandle);
			}
			UE_LOG(LogCatCombatInput, Log, TEXT("OnDestroy: 已解绑输入 CompletedHandle=%u CanceledHandle=%u"), InputCompletedBindHandle, InputCanceledBindHandle);
		}
		bBound = false;
		InputCompletedBindHandle = 0;
		InputCanceledBindHandle = 0;
	}

	Super::OnDestroy(bInOwnerFinished);
}

// ==================== 方向滑动子类（Case1） ====================

UAbilityTask_TrackMouseDragDirection::UAbilityTask_TrackMouseDragDirection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAbilityTask_TrackMouseDragDirection* UAbilityTask_TrackMouseDragDirection::TrackMouseDragDirection(
	UGameplayAbility* OwningAbility,
	UInputAction* InInputAction,
	bool InEndOnSuccess,
	ESwipeDirection InRequiredDirection,
	float InRequiredDistance)
{
	UAbilityTask_TrackMouseDragDirection* Task = NewAbilityTask<UAbilityTask_TrackMouseDragDirection>(OwningAbility);
	if (Task)
	{
		Task->SetInputAction(InInputAction);
		Task->bEndOnSuccess = InEndOnSuccess;
		Task->RequiredDirection = InRequiredDirection;
		Task->RequiredDistance = InRequiredDistance;
	}
	return Task;
}

bool UAbilityTask_TrackMouseDragDirection::CheckSuccessConditions(const FVector2D& CurrentPos, const FVector2D& PrevPos)
{
	// 净位移向量 = 终点 - 起点
	const FVector2D SwipeDelta = CurrentPos - StartMousePos;

	if (RequiredDirection != ESwipeDirection::None && RequiredDistance > 0.0f)
	{
		const FVector2D DirectionVec = SwipeDirectionToVector(RequiredDirection);
		// 净位移在目标方向上的投影距离
		const float DirDist = FVector2D::DotProduct(SwipeDelta, DirectionVec);

		if (DirDist >= RequiredDistance)
		{
			// 保存"已满足目标方向距离"的状态
			bHasSucceeded = true;
			// 记录成功位移向量为目标方向的向量（长度 = 实际滑动距离 DirDist）
			SuccessDragVector = DirectionVec * DirDist;
			return true;
		}
	}

	return false;
}

void UAbilityTask_TrackMouseDragDirection::DrawDebug(const FVector2D& CurrentPos, const FVector2D& PrevPos)
{
	Super::DrawDebug(CurrentPos, PrevPos);

	if (RequiredDirection == ESwipeDirection::None)
	{
		return;
	}

	UWorld* World = GetWorld();
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	const FVector2D DirectionVec = SwipeDirectionToVector(RequiredDirection);
	const FVector2D EndScreen = StartMousePos + DirectionVec * RequiredDistance;

	const FVector StartWorld = ScreenToWorld(PC, StartMousePos, DebugDrawDepth);
	const FVector EndWorld = ScreenToWorld(PC, EndScreen, DebugDrawDepth);

	// 常驻方向箭头（从起点沿目标方向画 RequiredDistance 长度）
	DrawDebugDirectionalArrow(World, StartWorld, EndWorld, 1.5f, FColor::Yellow, false, DebugDrawDuration, 0, 0.6f);
}

// ==================== 目标框滑动子类（Case2） ====================

UAbilityTask_TrackMouseDragTargetBox::UAbilityTask_TrackMouseDragTargetBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HitCharacters.Empty();
}

UAbilityTask_TrackMouseDragTargetBox* UAbilityTask_TrackMouseDragTargetBox::TrackMouseDragTargetBox(
	UGameplayAbility* OwningAbility,
	UInputAction* InInputAction,
	const TArray<ACharacter*>& InTargetCharacters,
	bool InEndOnSuccess)
{
	UAbilityTask_TrackMouseDragTargetBox* Task = NewAbilityTask<UAbilityTask_TrackMouseDragTargetBox>(OwningAbility);
	if (Task)
	{
		Task->SetInputAction(InInputAction);
		Task->TargetCharacters = InTargetCharacters;
		Task->HitCharacters.Empty();
		Task->bEndOnSuccess = InEndOnSuccess;
	}
	return Task;
}

TArray<ACharacter*> UAbilityTask_TrackMouseDragTargetBox::GetHitCharacters() const
{
	return HitCharacters;
}

ACharacter* UAbilityTask_TrackMouseDragTargetBox::GetFirstHitCharacter() const
{
	// 仅在成功命中后才有有效结果
	if (!bHasSucceeded)
	{
		return nullptr;
	}

	// 按命中顺序取第一个非空角色（HitCharacters 通过 AddUnique 追加，顺序即命中顺序）
	for (ACharacter* Character : HitCharacters)
	{
		if (Character)
		{
			return Character;
		}
	}
	return nullptr;
}

bool UAbilityTask_TrackMouseDragTargetBox::CheckSuccessConditions(const FVector2D& CurrentPos, const FVector2D& PrevPos)
{
	if (TargetCharacters.Num() == 0)
	{
		return false;
	}

	const APlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return false;
	}

	// 统一投影平面：以屏幕中心点的反投影方向为法线（近似视线方向），
	// 保证上一帧点与当前帧点投影到同一平面内，便于做 2D 线段-范围相交检测
	int32 ViewportX = 0, ViewportY = 0;
	PC->GetViewportSize(ViewportX, ViewportY);
	FVector CenterLoc, CenterDir;
	if (!PC->DeprojectScreenPositionToWorld(ViewportX * 0.5f, ViewportY * 0.5f, CenterLoc, CenterDir))
	{
		return false;
	}
	const FVector PlaneNormal = CenterDir.GetSafeNormal();

	// 遍历目标列表，通过接口获取每个目标的世界空间受击范围 FBox，判定鼠标滑动线段是否与之相交
	for (ACharacter* Target : TargetCharacters)
	{
		if (!Target || !Target->Implements<USVCombatCoreInterface>())
		{
			continue;
		}

		FBox HitBox;
		if (!ISVCombatCoreInterface::Execute_GetMainHitReactionRange(Target, HitBox))
		{
			continue;
		}

		// 以受击范围中心作为投影平面锚点，把上一帧/当前帧屏幕点投影到同一平面
		const FVector BoxCenter = HitBox.GetCenter();

		// 上一帧鼠标世界点
		FVector PrevWorldPos;
		const bool bHasPrev = ProjectScreenPosToPlane(PC, PrevPos, PlaneNormal, BoxCenter, PrevWorldPos);

		// 当前帧鼠标世界点
		FVector CurWorldPos;
		if (!ProjectScreenPosToPlane(PC, CurrentPos, PlaneNormal, BoxCenter, CurWorldPos))
		{
			continue;
		}

		// 检测「上一帧点到当前点」的线段是否与受击范围 FBox 相交（避免高速滑动穿过范围而漏判）
		const FVector SegmentStart = bHasPrev ? PrevWorldPos : CurWorldPos;
		const FVector SegmentDir = CurWorldPos - SegmentStart;
		if (FMath::LineBoxIntersection(HitBox, SegmentStart, CurWorldPos, SegmentDir))
		{
			// 将命中目标添加到命中目标列表（去重）
			HitCharacters.AddUnique(Target);

			// 保存"已命中目标"的状态
			bHasSucceeded = true;
			// 直接记录成功位移向量（从起点到命中点）
			SuccessDragVector = CurrentPos - StartMousePos;

			// 仅在需要提前退出时，命中即结束；否则继续遍历完所有目标，收集全部命中目标
			if (bEndOnSuccess)
			{
				return true;
			}
		}
	}

	return bHasSucceeded;
}

bool UAbilityTask_TrackMouseDragTargetBox::ProjectScreenPosToPlane(const APlayerController* PC, const FVector2D& ScreenPos, const FVector& PlaneNormal, const FVector& PlanePoint, FVector& OutWorldPos) const
{
	if (!PC)
	{
		return false;
	}

	FVector WorldLoc, WorldDir;
	if (!PC->DeprojectScreenPositionToWorld(ScreenPos.X, ScreenPos.Y, WorldLoc, WorldDir))
	{
		return false;
	}

	// 求屏幕射线与「经过 PlanePoint、法线为 PlaneNormal」的平面的交点
	const float Denom = FVector::DotProduct(WorldDir, PlaneNormal);
	if (FMath::IsNearlyZero(Denom))
	{
		return false;
	}
	const float T = FVector::DotProduct(PlanePoint - WorldLoc, PlaneNormal) / Denom;
	if (T <= 0.0f)
	{
		return false;
	}

	OutWorldPos = WorldLoc + WorldDir * T;
	return true;
}

void UAbilityTask_TrackMouseDragTargetBox::DrawDebug(const FVector2D& CurrentPos, const FVector2D& PrevPos)
{
	Super::DrawDebug(CurrentPos, PrevPos);

	if (TargetCharacters.Num() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 遍历目标列表，绘制每个目标的世界空间受击范围 FBox
	for (ACharacter* Target : TargetCharacters)
	{
		if (!Target || !Target->Implements<USVCombatCoreInterface>())
		{
			continue;
		}

		FBox HitBox;
		if (ISVCombatCoreInterface::Execute_GetMainHitReactionRange(Target, HitBox))
		{
			DrawDebugBox(World, HitBox.GetCenter(), HitBox.GetExtent(), FColor::Yellow, false, DebugDrawDuration);
		}
	}
}
