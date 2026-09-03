// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Combat/SVCombatTypes.h"
#include "SVCombatScenePoint.generated.h"

class UBillboardComponent;
class UDataTable;
class USceneComponent;
class UWorld;
class USVCombatCharacterDataAsset;
class USVCombatCameraDataAsset;
class USVCombatManagerSubsystem;
class ACameraActor;
struct FSVCombatDataTableRow;
struct FPropertyChangedEvent;

/**
 * 战斗场景点：回合战斗场景的配置容器 + 编辑器站位预览。
 *
 * - 作为配置容器，通过 CombatConfigRowName 关联战斗数据表行（FSVCombatDataTableRow），
 *   玩家/敌方站位与角色类、相机均由数据表行阵容资产携带，
 *   供战斗管理器在运行时读取并生成回合战斗。
 * - 作为编辑器预览，仅在编辑器未播放（非游戏世界）时，遍历数据表行阵容资产，
 *   按其 CharacterClass 与站位变换（相对/绝对）动态生成角色 Actor 可视化双方出生点；
 *   场景相机由“刷新数据”按钮（UpdateCombatData）按相机资产生成，为正常生成、随关卡保存的持久 Actor；
 *   预览角色与相机均挂载到场景点 Actor 下，删除场景点时一并清理；
 *   正常运行（游戏世界）时不会生成，预览角色不可见。
 */
UCLASS()
class CATCOMBATFRAMEWORK_API ASVCombatScenePoint : public AActor
{
	GENERATED_BODY()

public:
	ASVCombatScenePoint();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Actor 被销毁时（含编辑器删除场景点）清理全部子 Actor（预览角色 + 场景相机），避免残留 */
	virtual void Destroyed() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/** 获取关联的战斗配置数据表行名（未配置时返回 NAME_None） */
	UFUNCTION(BlueprintPure, Category = "CombatScene")
	FName GetCombatConfigRowName() const;

	/** 获取关联的战斗数据表行（未配置或行不存在时返回 nullptr） */
	const FSVCombatDataTableRow* GetCombatConfigRow() const;

	/** 编辑器中点击：按当前数据表配置更新战斗场景数据（重建预览角色与场景相机；相机为正常生成的关卡 Actor，保存关卡后常驻） */
	UFUNCTION(CallInEditor, Category = "CombatScene|Config")
	void UpdateCombatData();

	/** 获取主要场景相机 Actor（SceneCameraActors 中第一个有效项；未生成或全部失效时返回 nullptr） */
	UFUNCTION(BlueprintPure, Category = "CombatScene")
	ACameraActor* GetMainCameraActor() const;

	/** 供编辑器下拉框使用的选项：返回 USVCombatSettings::CombatConfigTable 的全部行名 */
	UFUNCTION()
	static TArray<FName> GetCombatConfigRowNames();

private:
	/** 全局战斗配置表（软引用，首次访问时同步加载并缓存，避免每次查询都 LoadSynchronous；弱引用，表被替换/卸载时自动失效） */
	mutable TWeakObjectPtr<UDataTable> CachedConfigTable;

	// ---- 战斗配置（数据表） ----
	/** 关联的战斗配置数据表行名（选项仅来自 USVCombatSettings::CombatConfigTable 的行名，编辑器中渲染为单选下拉框）。
	 *  运行时通过 GetCombatConfigRow() 从全局配置表读取对应行。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CombatScene|Config", meta = (GetOptions = "GetCombatConfigRowNames", AllowPrivateAccess = "true"))
	FName CombatConfigRowName;

	// ---- 编辑器预览 ----
	/** 是否在编辑器中显示战斗站位预览角色（按数据表行阵容资产 CharacterClass 生成的预览角色 Actor）。默认开启。
	 *  相机为正常生成的关卡持久 Actor，不受此开关控制。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CombatScene|Config", meta = (AllowPrivateAccess = "true"))
	bool bShowCharacterPreview = true;

	/** 获取指定队伍在世界坐标系下的全部站位变换 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CombatScene", meta = (AllowPrivateAccess = "true"))
	TArray<FTransform> GetSpawnTransforms(ECombatTeamType TeamType) const;

	/** 获取玩家方世界坐标系站位变换 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CombatScene", meta = (AllowPrivateAccess = "true"))
	TArray<FTransform> GetPlayerSpawnTransforms() const;

	/** 获取敌方世界坐标系站位变换 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CombatScene", meta = (AllowPrivateAccess = "true"))
	TArray<FTransform> GetEnemySpawnTransforms() const;

	UPROPERTY(VisibleAnywhere, Category = "CombatScene", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "CombatScene", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBillboardComponent> Billboard;

	/** 场景相机（按相机资产生成的 ACameraActor，正常生成并随关卡保存，刷新数据时重建；挂载到场景点下，删除场景点时一并清理；打包运行时可读取） */
	UPROPERTY(VisibleAnywhere, Category = "CombatScene|Preview", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<ACameraActor>> SceneCameraActors;

#if WITH_EDITORONLY_DATA
	/** 玩家方编辑器预览角色（仅编辑器生成，正常运行不可见；RF_Transient + bIsEditorPreviewActor，不参与序列化与关卡保存） */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> PlayerPreviewActors;

	/** 敌方编辑器预览角色（仅编辑器生成，正常运行不可见；RF_Transient + bIsEditorPreviewActor，不参与序列化与关卡保存） */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> EnemyPreviewActors;
#endif

#if WITH_EDITOR
	/** 按当前数据表行阵容重建编辑器预览角色（bShowCharacterPreview 关闭时直接不生成） */
	void RebuildPreviewActors();

	/** 销毁并清空指定的预览 Actor 数组（预览角色/场景相机通用；不标记关卡脏） */
	template <typename TActorType>
	void ClearPreviewActorsImpl(TArray<TObjectPtr<TActorType>>& PreviewActors)
	{
		if (UWorld* World = GetWorld())
		{
			for (TObjectPtr<TActorType>& PreviewActor : PreviewActors)
			{
				if (PreviewActor)
				{
					World->DestroyActor(PreviewActor, false, false);
				}
			}
		}
		PreviewActors.Reset();
	}

	/** 清理指定阵营的全部预览角色并清空数组 */
	void ClearPreviewActors(TArray<TObjectPtr<AActor>>& PreviewActors);

	/** 按阵容资产生成预览角色 Actor：角色类与站位均取自资产本身（bShowCharacterPreview 关闭时直接不生成；角色命名：<配置行名>_<队伍类别>_<阵容索引>） */
	void BuildPreviewActors(ECombatTeamType TeamType, const TArray<TObjectPtr<USVCombatCharacterDataAsset>>& Teams, TArray<TObjectPtr<AActor>>& OutPreviewActors);

	/** 按相机资产生成场景相机 Actor（无相机配置或没有有效世界时不生成） */
	void BuildCameraPreview(const USVCombatCameraDataAsset* CameraData);

	/** 清理全部场景相机 Actor 并清空数组 */
	void ClearCameraPreview();

	/** 重建场景相机：清理旧的并重新按数据表行相机资产生成 */
	void RebuildSceneCamera();
#endif
};
