// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "CatAttributeSetBase.generated.h"

/**
 * Cat 系列属性集的公共基类。
 *
 * 职责：
 *  1. 统一「写入前约束」入口：把 PreAttributeChange（CurrentValue 写入前）与
 *     PreAttributeBaseChange（BaseValue 写入前）收敛到基类，二者共同调用同一个
 *     ClampAttribute —— 子类只需实现一处规则，Base 层与 Current 层永远一致，避免规则漂移。
 *  2. 提供跨属性集的公共小工具（如「上限被压低时把当前值修正到新上限」）。
 *
 * 约定：
 *  - 子类不得在运行时直接写属性字段，外部统一通过 GameplayEffect / 元属性修改。
 *  - 子类只负责「属性定义 + Clamp 规则 + 结算回调」，不承担驱动逻辑（驱动交给 GE + Timing）。
 */
UCLASS(Abstract)
class GAMEWORK_API UCatAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()

public:
	/** CurrentValue 写入前：交给子类的 ClampAttribute 做最终值约束 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/** BaseValue 写入前：与 PreAttributeChange 共用同一套 Clamp 规则 */
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

protected:
	/**
	 * 子类实现各自的数值约束（如 Health ∈ [0, MaxHealth]）。
	 * 会被 PreAttributeChange 与 PreAttributeBaseChange 共同调用，因此实现里
	 * 不要触发任何游戏事件 —— 那是 PreGameplayEffectExecute / PostAttributeChange 的职责。
	 *
	 * 注意：这里不能用纯虚函数 —— UHT 会为每个 UCLASS 生成 CDO 构造代码，
	 * 存在纯虚成员会导致「无法实例化抽象类」编译失败。默认空实现即可。
	 */
	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

	/**
	 * 上限类属性被压低时，用 Override 把当前值修正到新上限（供 Health / AP 等「当前值-上限」组合复用）。
	 * @param CurrentAttribute 需要被修正的当前值属性（如 Health / AP）
	 * @param NewMaxValue      压低后的新上限
	 */
	void ClampCurrentValueToMax(const FGameplayAttribute& CurrentAttribute, float NewMaxValue) const;
};
