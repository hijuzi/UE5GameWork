// Copyright SegameVictory Team. All Rights Reserved.

#include "SVFloatingTextComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogSVFloatingTextComponent);

USVFloatingTextComponent::USVFloatingTextComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USVFloatingTextComponent::PopFloatingText(const FSVFloatingPopRequest& Request)
{
	// 基类空实现，子类重写
}

void USVFloatingTextComponent::PopTextFloatingText(const FSVFloatingPopRequest& Request, const FText& Text)
{
	// 基类空实现，子类重写
}

void USVFloatingTextComponent::ClearAllFloatingText()
{
	// 基类空实现，子类重写
}

void USVFloatingTextComponent::SetTextScaleFactor(float InScale)
{
	// 基类空实现，子类重写
}

FVector USVFloatingTextComponent::GetFloatingTextPosition(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	}

	// 优先取骨骼网格体的头部骨骼位置，否则用 Actor 位置加高度偏移
	if (ACharacter* Character = Cast<ACharacter>(TargetActor))
	{
		if (USkeletalMeshComponent* SkelMesh = Character->GetMesh())
		{
			return SkelMesh->GetSocketLocation(FName("head")) + FVector(0, 0, 60.0f);
		}
	}

	return TargetActor->GetActorLocation() + FVector(0, 0, TargetActor->GetSimpleCollisionHalfHeight() + 40.0f);
}

APlayerController* USVFloatingTextComponent::GetLocalPlayerController() const
{
	if (const UWorld* World = GetWorld())
	{
		return World->GetFirstPlayerController();
	}
	return nullptr;
}
