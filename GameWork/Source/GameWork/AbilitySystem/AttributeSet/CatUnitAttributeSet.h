// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CatAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "CatUnitAttributeSet.generated.h"

/** 生命值变化（供血条 UI 监听） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCatHealthChangedDelegate, float, NewHealth, float, MaxHealth);

/** 进出「倒地」状态 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCatOutOfHealthDelegate, bool, bOutOfHealth);

/** 护盾层数变化（供血条上的盾牌图标 UI 监听） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCatShieldLayersChangedDelegate, float, NewLayers, float, MaxLayers);

/** 护盾被打破（由有盾变为 0 层） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCatShieldBrokenDelegate);

/** 速度变化（供行动顺序条 / 出手次数 UI 监听） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCatSpeedChangedDelegate, float, NewSpeed);

/**
 * 单位战斗数值集（L3：战斗实时属性，**玩家 + 敌人共用**）。
 *
 * 合并来源：原 `UCatVitalAttributeSet` + `UCatCombatAttributeSet`。
 * 合并依据：二者的**挂载对象（角色）、阵营（双方）、持久化与重置时机（战斗即弃、不重置）三条完全相同**，
 *          合并零代价；且合并后伤害结算的「输出端 + 承伤端」在同一个集里，读属性不必跨集跳转。
 *
 * 文件内按**分组**组织，回调也只做分发，具体逻辑进各自的私有函数
 * （这是把「单文件属性变多」的认知成本压住的关键）：
 *   - 生命组：Health / MaxHealth          → ClampVital() / ResolveDamage() / ResolveHealing()
 *   - 护盾组：ShieldLayers / MaxShieldLayers → ClampVital() / ResolveShieldBreak()
 *   - 进攻组：Attack / CritRate / CritDamage / DamageMultiplier / BreakPower → ClampOffense()
 *   - 防御组：DamageReduction / ParryEfficiency / DodgeEfficiency           → ClampDefense()
 *   - 行动组：Speed
 *
 * 官方对照（Game8 / Gamer Guides）：
 *   - 五个次级数值 = Health / Attack Power / Speed / Defense / Critical Rate，本集覆盖其中全部。
 *   - 暴击基础游戏固定 +50% 伤害 → `CritDamage` 默认 1.5。
 *   - 护盾是**层数**（血条盾牌图标），**有盾完全免伤**，且**只能用瞄准射击（Aim Shot）削减**，
 *     与 Break 是两套独立系统（见 `UCatPostureAttributeSet`）。
 *   - `DamageMultiplier` / `BreakPower` / `ParryEfficiency` / `DodgeEfficiency` 在官方 UI 无对应项，
 *     属本项目**实现层系数**。
 */
UCLASS()
class GAMEWORK_API UCatUnitAttributeSet : public UCatAttributeSetBase
{
	GENERATED_BODY()

public:
	UCatUnitAttributeSet();

	// ==================================================================
	// 生命组
	// ==================================================================

	/** 当前生命 [0, MaxHealth] */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Cat|Unit|Vital")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, Health);

	/** 最大生命（只保下限，不设上限） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Cat|Unit|Vital")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, MaxHealth);

	/** 元属性：受伤量。仅 execute 瞬间存在，结算后清零，不复制 */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Vital")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, Damage);

	/** 元属性：治疗量。仅 execute 瞬间存在，结算后清零，不复制 */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Vital")
	FGameplayAttributeData Healing;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, Healing);

	/** 是否处于倒地状态（Health <= 0）。由 PostAttributeChange 派生，服务器与客户端一致，无需复制 */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Cat|Unit|Vital")
	bool bOutOfHealth = false;

	// ==================================================================
	// 护盾组
	// ==================================================================

	/** 当前护盾层数（1 层 = 血条上 1 个盾牌图标） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ShieldLayers, Category = "Cat|Unit|Vital")
	FGameplayAttributeData ShieldLayers;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, ShieldLayers);

	/** 护盾层数上限（0 = 无护盾能力） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxShieldLayers, Category = "Cat|Unit|Vital")
	FGameplayAttributeData MaxShieldLayers;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, MaxShieldLayers);

	/** 元属性：破盾量（层）。**仅由瞄准射击（Aim Shot）命中写入**，结算后清零，不复制 */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Vital")
	FGameplayAttributeData ShieldBreak;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, ShieldBreak);

	// ==================================================================
	// 进攻组
	// ==================================================================

	/** 基础攻击力（官方次级数值 Attack Power，由力量 / 武器派生） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Attack, Category = "Cat|Unit|Offense")
	FGameplayAttributeData Attack;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, Attack);

	/** 暴击率 [0, 1]（官方次级数值 Critical Rate） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CritRate, Category = "Cat|Unit|Offense")
	FGameplayAttributeData CritRate;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, CritRate);

	/** 暴击伤害倍率（>= 1，官方基础游戏固定 +50% → 默认 1.5） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CritDamage, Category = "Cat|Unit|Offense")
	FGameplayAttributeData CritDamage;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, CritDamage);

	/** 全局伤害倍率（实现层系数；对应 Mark / Berserk 等状态，默认 1.0） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageMultiplier, Category = "Cat|Unit|Offense")
	FGameplayAttributeData DamageMultiplier;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, DamageMultiplier);

	/** Break 积累效率（实现层系数，默认 1.0） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BreakPower, Category = "Cat|Unit|Offense")
	FGameplayAttributeData BreakPower;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, BreakPower);

	// ==================================================================
	// 防御组
	// ==================================================================

	/** 减伤率 [0, 1]（官方次级数值 Defense 的结算端表达） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageReduction, Category = "Cat|Unit|Defense")
	FGameplayAttributeData DamageReduction;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, DamageReduction);

	/** 格挡收益（实现层系数，影响反击强度，默认 1.0） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ParryEfficiency, Category = "Cat|Unit|Defense")
	FGameplayAttributeData ParryEfficiency;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, ParryEfficiency);

	/** 闪避收益（实现层系数，默认 1.0） */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DodgeEfficiency, Category = "Cat|Unit|Defense")
	FGameplayAttributeData DodgeEfficiency;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, DodgeEfficiency);

	// ==================================================================
	// 行动组
	// ==================================================================

	/**
	 * 速度（官方次级数值 Speed，由 Agility 主 + Luck 次派生）。
	 *
	 * 作用存在来源分歧（已如实记录）：
	 *   - Game8《Speed Guide》称：速度决定**每回合出手次数**，回合顺序由**队伍排列**决定；
	 *   - 玩家实测反驳：队伍排列只决定第一轮，速度影响后续轮次的行动频率，甚至可连续行动。
	 * 本项目按「速度参与行动顺序计算」处理（`USVCombatTurnCoordinator` 排 Timeline 要读它）。
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Speed, Category = "Cat|Unit|Speed")
	FGameplayAttributeData Speed;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, Speed);

	// ==================================================================
	// 委托
	// ==================================================================

	/** 生命变化广播 */
	UPROPERTY(BlueprintAssignable, Category = "Cat|Unit")
	FCatHealthChangedDelegate OnHealthChanged;

	/** 倒地 / 复活广播 */
	UPROPERTY(BlueprintAssignable, Category = "Cat|Unit")
	FCatOutOfHealthDelegate OnOutOfHealth;

	/** 护盾层数变化广播 */
	UPROPERTY(BlueprintAssignable, Category = "Cat|Unit")
	FCatShieldLayersChangedDelegate OnShieldLayersChanged;

	/** 破盾广播（层数由 >0 变为 0） */
	UPROPERTY(BlueprintAssignable, Category = "Cat|Unit")
	FCatShieldBrokenDelegate OnShieldBroken;

	/** 速度变化广播 */
	UPROPERTY(BlueprintAssignable, Category = "Cat|Unit")
	FCatSpeedChangedDelegate OnSpeedChanged;

	// ==================================================================
	// 回调
	// ==================================================================

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// ~begin UCatAttributeSetBase
	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const override;
	// ~end UCatAttributeSetBase

	/** 生命 + 护盾的约束 */
	void ClampVital(const FGameplayAttribute& Attribute, float& NewValue) const;

	/** 进攻组的约束 */
	void ClampOffense(const FGameplayAttribute& Attribute, float& NewValue) const;

	/** 防御组的约束 */
	void ClampDefense(const FGameplayAttribute& Attribute, float& NewValue) const;

	/** 元属性换算：Damage → Health */
	void ResolveDamage();

	/** 元属性换算：Healing → Health */
	void ResolveHealing();

	/** 元属性换算：ShieldBreak → ShieldLayers */
	void ResolveShieldBreak();

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	void OnRep_ShieldLayers(const FGameplayAttributeData& OldShieldLayers);

	UFUNCTION()
	void OnRep_MaxShieldLayers(const FGameplayAttributeData& OldMaxShieldLayers);

	UFUNCTION()
	void OnRep_Attack(const FGameplayAttributeData& OldAttack);

	UFUNCTION()
	void OnRep_CritRate(const FGameplayAttributeData& OldCritRate);

	UFUNCTION()
	void OnRep_CritDamage(const FGameplayAttributeData& OldCritDamage);

	UFUNCTION()
	void OnRep_DamageMultiplier(const FGameplayAttributeData& OldDamageMultiplier);

	UFUNCTION()
	void OnRep_BreakPower(const FGameplayAttributeData& OldBreakPower);

	UFUNCTION()
	void OnRep_DamageReduction(const FGameplayAttributeData& OldDamageReduction);

	UFUNCTION()
	void OnRep_ParryEfficiency(const FGameplayAttributeData& OldParryEfficiency);

	UFUNCTION()
	void OnRep_DodgeEfficiency(const FGameplayAttributeData& OldDodgeEfficiency);

	UFUNCTION()
	void OnRep_Speed(const FGameplayAttributeData& OldSpeed);
};
