// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Actor/SVCombatScenePoint.h"

#include "CatCombatLog.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"
#include "Combat/DataAsset/SVCombatCameraDataAsset.h"
#include "Combat/DataAsset/SVCombatCharacterDataAsset.h"
#include "Combat/SVCombatDataTable.h"
#include "Combat/SVCombatSettings.h"
#include "Combat/SVCombatManagerSubsystem.h"
#include "Components/BillboardComponent.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/UnrealType.h"

ASVCombatScenePoint::ASVCombatScenePoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	Billboard->SetupAttachment(SceneRoot);
	Billboard->SetHiddenInGame(true);
}

void ASVCombatScenePoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	// 预览仅在编辑器未播放时生效，运行时（游戏世界）不重建
	if (const UWorld* World = GetWorld())
	{
		if (!World->IsGameWorld())
		{
			RebuildPreviewActors();
		}
	}
#endif
}

void ASVCombatScenePoint::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USVCombatManagerSubsystem* CombatManager = GameInstance->GetSubsystem<USVCombatManagerSubsystem>())
		{
			CombatManager->RegisterScenePoint(this);
		}
	}
}

void ASVCombatScenePoint::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USVCombatManagerSubsystem* CombatManager = GameInstance->GetSubsystem<USVCombatManagerSubsystem>())
		{
			CombatManager->UnregisterScenePoint(this);
		}
	}

#if WITH_EDITOR
	// 清理编辑器预览角色与场景相机，避免场景点销毁后残留
	ClearPreviewActors(PlayerPreviewActors);
	ClearPreviewActors(EnemyPreviewActors);
	ClearCameraPreview();
#endif

	Super::EndPlay(EndPlayReason);
}

void ASVCombatScenePoint::Destroyed()
{
#if WITH_EDITOR
	// 编辑器删除场景点时不触发 EndPlay（编辑器世界无 BeginPlay），在此清理全部子 Actor（预览角色 + 场景相机），避免残留
	if (const UWorld* World = GetWorld())
	{
		if (!World->IsGameWorld())
		{
			ClearPreviewActors(PlayerPreviewActors);
			ClearPreviewActors(EnemyPreviewActors);
			ClearCameraPreview();
		}
	}
#endif
	Super::Destroyed();
}

#if WITH_EDITOR
void ASVCombatScenePoint::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 关联配置Id（CombatConfigRowName）或显隐开关（bShowCharacterPreview）变化时，一律重建预览角色
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	const FName MemberPropertyName = PropertyChangedEvent.GetMemberPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASVCombatScenePoint, CombatConfigRowName) ||
		MemberPropertyName == GET_MEMBER_NAME_CHECKED(ASVCombatScenePoint, CombatConfigRowName) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(ASVCombatScenePoint, bShowCharacterPreview) ||
		MemberPropertyName == GET_MEMBER_NAME_CHECKED(ASVCombatScenePoint, bShowCharacterPreview))
	{
		if (const UWorld* World = GetWorld())
		{
			if (!World->IsGameWorld())
			{
				RebuildPreviewActors();
			}
		}
	}
}

#endif

void ASVCombatScenePoint::UpdateCombatData()
{
#if WITH_EDITOR
	// 仅在编辑器未播放时刷新
	if (const UWorld* World = GetWorld())
	{
		if (!World->IsGameWorld())
		{
			RebuildPreviewActors();   // 仅重建预览角色（相机独立管理）
			RebuildSceneCamera();     // 独立重建相机（正常生成、随关卡保存）
		}
	}
#endif
}

#if WITH_EDITOR
void ASVCombatScenePoint::RebuildPreviewActors()
{
	// 清理旧的预览角色（相机由 RebuildSceneCamera 独立管理，此处不处理）
	ClearPreviewActors(PlayerPreviewActors);
	ClearPreviewActors(EnemyPreviewActors);

	// 未配置有效数据表行时不生成预览（角色类与站位均来自数据表行阵容资产）
	if (const FSVCombatDataTableRow* Row = GetCombatConfigRow())
	{
		BuildPreviewActors(ECombatTeamType::Player, Row->PlayerTeams, PlayerPreviewActors);
		BuildPreviewActors(ECombatTeamType::Enemy, Row->EnemyTeams, EnemyPreviewActors);
	}
}

void ASVCombatScenePoint::ClearPreviewActors(TArray<TObjectPtr<AActor>>& PreviewActors)
{
	ClearPreviewActorsImpl(PreviewActors);
}

void ASVCombatScenePoint::BuildPreviewActors(ECombatTeamType TeamType, const TArray<TObjectPtr<USVCombatCharacterDataAsset>>& Teams, TArray<TObjectPtr<AActor>>& OutPreviewActors)
{
	// 关闭预览角色开关时不生成任何角色
	if (!bShowCharacterPreview)
	{
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const FString TeamTypeName = TeamType == ECombatTeamType::Player ? TEXT("PlayerTeam") : TEXT("EnemyTeam");
	for (int32 Index = 0; Index < Teams.Num(); ++Index)
	{
		const TObjectPtr<USVCombatCharacterDataAsset>& TeamAsset = Teams[Index];
		if (!TeamAsset || !TeamAsset->CharacterClass)
		{
			continue;
		}
		// 站位变换：相对模式按资产内局部变换换算为世界坐标（Local * ActorTransform）；绝对模式直接使用绝对变换
		const FTransform SpawnTransform = TeamAsset->bUseRelativeTransform
			? TeamAsset->RelativeTransform * GetActorTransform()
			: TeamAsset->AbsoluteTransform;

		FActorSpawnParameters SpawnParams;
		SpawnParams.ObjectFlags = RF_Transient; // 不参与序列化/关卡保存
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.bNoFail = true;
		AActor* PreviewActor = World->SpawnActor<AActor>(TeamAsset->CharacterClass, SpawnTransform, SpawnParams);
		if (!PreviewActor)
		{
			continue;
		}
		// 挂载到场景点 Actor 下（保持生成时的世界变换，焊接模拟体）
		FAttachmentTransformRules AttachmentTransform = FAttachmentTransformRules::KeepWorldTransform;
		AttachmentTransform.bWeldSimulatedBodies = true;
		PreviewActor->AttachToActor(this, AttachmentTransform);
		// 角色命名：<配置行名>_<队伍类别>_<阵容索引>，便于编辑器中定位（同名冲突时引擎自动追加后缀）
		PreviewActor->SetActorLabel(FString::Printf(TEXT("%s_%s_%d"), *CombatConfigRowName.ToString(), *TeamTypeName, Index));
		// 标记为编辑器预览 Actor：Outliner 归入 Preview 分组，保存关卡时一并跳过
		PreviewActor->bIsEditorPreviewActor = true;
		OutPreviewActors.Add(PreviewActor);
	}
}
#endif

#if WITH_EDITOR
void ASVCombatScenePoint::BuildCameraPreview(const USVCombatCameraDataAsset* CameraData)
{
	// 无相机配置或没有有效世界时不再生成
	UWorld* World = GetWorld();
	if (!CameraData || !CameraData->CameraClass || !World)
	{
		UE_LOG(LogCatCombatScenePoint, Warning, TEXT("ASVCombatScenePoint::BuildCameraPreview: 相机未生成（数据表行 %s，CameraData=%s，CameraClass=%s）"),
			*CombatConfigRowName.ToString(),
			CameraData ? *CameraData->GetName() : TEXT("NULL"),
			(CameraData && CameraData->CameraClass) ? *CameraData->CameraClass->GetName() : TEXT("NULL"));
		return;
	}

	// 相机正常生成：按资产相机类与世界变换生成 ACameraActor，并应用资产 FOV
	// 相对模式：资产内相对场景点的变换换算为世界坐标；绝对模式：直接使用绝对变换
	const FTransform CameraWorldTransform = CameraData->bUseRelativeTransform
		? CameraData->RelativeTransform * GetActorTransform()
		: CameraData->AbsoluteTransform;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.bNoFail = true;
	ACameraActor* CameraActor = World->SpawnActor<ACameraActor>(
		CameraData->CameraClass, CameraWorldTransform, SpawnParams);
	if (!CameraActor)
	{
		UE_LOG(LogCatCombatScenePoint, Warning, TEXT("ASVCombatScenePoint::BuildCameraPreview: 相机生成失败（类 %s）"), *CameraData->CameraClass->GetName());
		return;
	}
	if (!CameraActor->GetCameraComponent())
	{
		UE_LOG(LogCatCombatScenePoint, Warning, TEXT("ASVCombatScenePoint::BuildCameraPreview: 相机 %s 缺少 CameraComponent，已销毁"), *CameraActor->GetName());
		World->DestroyActor(CameraActor, false, false);
		return;
	}
	// 挂载到场景点 Actor 下（保持生成时的世界变换，焊接模拟体）；随关卡保存后父级关系一并持久化
	FAttachmentTransformRules AttachmentTransform = FAttachmentTransformRules::KeepWorldTransform;
	AttachmentTransform.bWeldSimulatedBodies = true;
	CameraActor->AttachToActor(this, AttachmentTransform);
	// 相机命名：<配置行名>_Camera_Main（主相机；同名冲突时引擎自动追加后缀）
	CameraActor->SetActorLabel(FString::Printf(TEXT("%s_Camera_Main"), *CombatConfigRowName.ToString()));
	CameraActor->GetCameraComponent()->SetFieldOfView(CameraData->FOV);
	SceneCameraActors.Add(CameraActor);
	// 相机为正常生成的关卡 Actor：标记关卡包为脏，保存关卡后相机常驻
	if (ULevel* Level = World->GetCurrentLevel())
	{
		Level->MarkPackageDirty();
	}
}

void ASVCombatScenePoint::ClearCameraPreview()
{
	ClearPreviewActorsImpl(SceneCameraActors);
}

void ASVCombatScenePoint::RebuildSceneCamera()
{
	// 清理旧的相机 Actor（若已保存进关卡，需保存关卡后才会从磁盘移除）
	ClearCameraPreview();

	// 未配置有效数据表行时不再生成
	const FSVCombatDataTableRow* Row = GetCombatConfigRow();
	if (!Row)
	{
		UE_LOG(LogCatCombatScenePoint, Warning, TEXT("ASVCombatScenePoint::RebuildSceneCamera: 数据表行 %s 不存在或未配置，无法生成相机"), *CombatConfigRowName.ToString());
		return;
	}
	BuildCameraPreview(Row->ScenePointCameraData);
}
#endif

TArray<FTransform> ASVCombatScenePoint::GetSpawnTransforms(ECombatTeamType TeamType) const
{
	TArray<FTransform> Result;
	const FSVCombatDataTableRow* Row = GetCombatConfigRow();
	if (!Row)
	{
		return Result;
	}
	// 按阵营选择阵容资产（显式 switch，避免枚举扩展时静默归入敌方）
	const TArray<TObjectPtr<USVCombatCharacterDataAsset>>* Teams = nullptr;
	switch (TeamType)
	{
	case ECombatTeamType::Player:
		Teams = &Row->PlayerTeams;
		break;
	case ECombatTeamType::Enemy:
		Teams = &Row->EnemyTeams;
		break;
	default:
		checkNoEntry();
		break;
	}
	if (!Teams)
	{
		return Result;
	}
	for (const TObjectPtr<USVCombatCharacterDataAsset>& TeamAsset : *Teams)
	{
		if (!TeamAsset)
		{
			continue;
		}
		// 站位变换：相对模式按资产内局部变换换算为世界坐标（Local * ActorTransform）；绝对模式直接使用绝对变换
		Result.Add(TeamAsset->bUseRelativeTransform
			? TeamAsset->RelativeTransform * GetActorTransform()
			: TeamAsset->AbsoluteTransform);
	}
	return Result;
}

TArray<FTransform> ASVCombatScenePoint::GetPlayerSpawnTransforms() const
{
	return GetSpawnTransforms(ECombatTeamType::Player);
}

TArray<FTransform> ASVCombatScenePoint::GetEnemySpawnTransforms() const
{
	return GetSpawnTransforms(ECombatTeamType::Enemy);
}

FName ASVCombatScenePoint::GetCombatConfigRowName() const
{
	return CombatConfigRowName;
}

ACameraActor* ASVCombatScenePoint::GetMainCameraActor() const
{
	// 取第一个有效的场景相机；已销毁/失效的引用直接跳过
	for (const TObjectPtr<ACameraActor>& CameraActor : SceneCameraActors)
	{
		if (IsValid(CameraActor))
		{
			return CameraActor;
		}
	}
	return nullptr;
}

TArray<FName> ASVCombatScenePoint::GetCombatConfigRowNames()
{
	TArray<FName> RowNames;
	if (const USVCombatSettings* Settings = GetDefault<USVCombatSettings>())
	{
		// 表已加载时直接复用，避免属性面板每次刷新下拉选项都同步加载（仅首次未加载时才 LoadSynchronous）
		const UDataTable* Table = Settings->CombatConfigTable.Get();
		if (!Table)
		{
			Table = Settings->CombatConfigTable.LoadSynchronous();
		}
		if (Table)
		{
			RowNames = Table->GetRowNames();
		}
	}
	return RowNames;
}

const FSVCombatDataTableRow* ASVCombatScenePoint::GetCombatConfigRow() const
{
	if (CombatConfigRowName.IsNone())
	{
		return nullptr;
	}
	const USVCombatSettings* Settings = GetDefault<USVCombatSettings>();
	if (!Settings)
	{
		return nullptr;
	}
	// 优先使用配置软引用当前指向的表（Project Settings 更换配置表后立即生效）；
	// 表尚未加载时回退到缓存（仅当缓存仍指向同一资产时复用），仍无效则首次同步加载并缓存
	UDataTable* Table = Settings->CombatConfigTable.Get();
	if (!Table)
	{
		// 用 ToSoftObjectPath 比对软引用当前指向的资产，避免依赖各引擎版本可用的 GetAssetPath API
		const FSoftObjectPath TablePath = Settings->CombatConfigTable.ToSoftObjectPath();
		if (CachedConfigTable.IsValid() && CachedConfigTable->GetPathName() == TablePath.ToString())
		{
			Table = CachedConfigTable.Get();
		}
		else
		{
			Table = Settings->CombatConfigTable.LoadSynchronous();
		}
		CachedConfigTable = Table;
	}
	if (Table)
	{
		return Table->FindRow<FSVCombatDataTableRow>(CombatConfigRowName, TEXT("ASVCombatScenePoint::GetCombatConfigRow"));
	}
	return nullptr;
}
