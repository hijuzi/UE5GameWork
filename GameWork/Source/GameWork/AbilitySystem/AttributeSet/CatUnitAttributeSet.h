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
 * 单位战斗数值集（L3：战斗实时属性，玩家 + 敌人共用）。
 *
 * 一个单位在战斗中的全部次级数值。官方并列的五个（Health、Attack Power、Speed、Defense、
 * Critical Rate）在这里被完整覆盖，伤害结算的输出端和承伤端也在同一个集里。
 *
 * 单文件属性多，所以内部按五个分组组织，回调只做分发，实际规则进各自的私有函数：
 *   生命    Health / MaxHealth                 → ClampVital() / ResolveDamage() / ResolveHealing()
 *   护盾    ShieldLayers / MaxShieldLayers      → ClampVital() / ResolveShieldBreak()
 *   进攻    Attack / CritRate / CritDamage / DamageMultiplier / BreakPower → ClampOffense()
 *   防御    DamageReduction / ParryEfficiency / DodgeEfficiency            → ClampDefense()
 *   行动    Speed
 *
 * 官方对照：
 *   - 暴击在基础游戏里固定 +50% 伤害，不随属性成长，`CritDamage` 默认 1.5。
 *   - 护盾按层计（血条上的盾牌图标），有盾完全免伤，且只有瞄准射击（Aim Shot）能削掉它，
 *     与 Break 是两套独立系统（见 `UCatPostureAttributeSet`）。
 *   - `DamageMultiplier` / `BreakPower` / `ParryEfficiency` / `DodgeEfficiency` 在官方 UI 里
 *     没有对应项，是本项目的实现层系数。
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
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Vital")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, Health);

	/** 最大生命（只保下限，不设上限） */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Vital")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, MaxHealth);

	/** 元属性：受伤量。仅 GE execute 瞬间存在，结算后清零 */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Vital")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, Damage);

	/** 元属性：治疗量。仅 GE execute 瞬间存在，结算后清零 */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Vital")
	FGameplayAttributeData Healing;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, Healing);

	/** 是否处于倒地状态（Health <= 0）。由 PostAttributeChange 派生 */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Cat|Unit|Vital")
	bool bOutOfHealth = false;

	// ==================================================================
	// 护盾组
	// ==================================================================

	/** 当前护盾层数（1 层 = 血条上 1 个盾牌图标） */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Vital")
	FGameplayAttributeData ShieldLayers;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, ShieldLayers);

	/** 护盾层数上限（0 = 无护盾能力） */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Vital")
	FGameplayAttributeData MaxShieldLayers;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, MaxShieldLayers);

	/** 元属性：破盾量（层）。只有瞄准射击（Aim Shot）命中时写入，结算后清零 */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Vital")
	FGameplayAttributeData ShieldBreak;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, ShieldBreak);

	// ==================================================================
	// 进攻组
	// ==================================================================

	/** 基础攻击力（官方次级数值 Attack Power，由力量 / 武器派生） */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Offense")
	FGameplayAttributeData Attack;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, Attack);

	/** 暴击率 [0, 1]（官方次级数值 Critical Rate） */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Offense")
	FGameplayAttributeData CritRate;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, CritRate);

	/** 暴击伤害倍率（>= 1，官方基础游戏固定 +50% → 默认 1.5） */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Offense")
	FGameplayAttributeData CritDamage;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, CritDamage);

	/** 全局伤害倍率（实现层系数，承接 Mark / Berserk 等状态，默认 1.0） */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Offense")
	FGameplayAttributeData DamageMultiplier;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, DamageMultiplier);

	/** Break 积累效率（实现层系数，默认 1.0） */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Offense")
	FGameplayAttributeData BreakPower;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, BreakPower);

	// ==================================================================
	// 防御组
	// ==================================================================

	/** 减伤率 [0, 1]（官方次级数值 Defense 的结算端表达） */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Defense")
	FGameplayAttributeData DamageReduction;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, DamageReduction);

	/** 格挡收益（实现层系数，影响反击强度，默认 1.0） */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Defense")
	FGameplayAttributeData ParryEfficiency;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, ParryEfficiency);

	/** 闪避收益（实现层系数，默认 1.0） */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Defense")
	FGameplayAttributeData DodgeEfficiency;
	ATTRIBUTE_ACCESSORS_BASIC(UCatUnitAttributeSet, DodgeEfficiency);

	// ==================================================================
	// 行动组
	// ==================================================================

	/**
	 * 速度（官方次级数值 Speed，由 Agility 主 + Luck 次派生）。
	 *
	 * 它影响每回合的出手次数，这点没有争议。是否影响行动顺序，资料里有两种说法：
	 * 一份官方指南称顺序由队伍排列决定，速度只决定出手次数；玩家实测的说法是排列只决定
	 * 第一轮，速度会影响后续轮次的行动频率，甚至能连续行动。
	 *
	 * 项目暂时按「速度参与行动顺序计算」处理，因为 `USVCombatTurnCoordinator` 排 Timeline
	 * 时需要读它。若实机验证后确认无关，把读取点去掉即可，属性本身不受影响。
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Cat|Unit|Speed")
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

protected:
	// ~begin UCatAttributeSetBase
	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const override;
	// ~end UCatAttributeSetBase

	/** 生命 + 护盾的约束 */
	void ClampVital(const FGameplayAttribute& Attribute, float& NewValue) const;

	/** 进攻组的约束 */
	void ClampOffense(const FGameplayAttribute& Attribute, float& NewValue) const;

	/** 防御组与行动组的约束 */
	void ClampDefense(const FGameplayAttribute& Attribute, float& NewValue) const;

	/** 元属性换算：Damage → Health */
	void ResolveDamage();

	/** 元属性换算：Healing → Health */
	void ResolveHealing();

	/** 元属性换算：ShieldBreak → ShieldLayers */
	void ResolveShieldBreak();
};
