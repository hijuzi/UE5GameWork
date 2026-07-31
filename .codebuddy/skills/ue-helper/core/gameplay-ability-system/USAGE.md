# gameplay-ability-system 使用指南

## 功能

使用 Gameplay Ability System (GAS) 构建技能、属性和效果 —— UAbilitySystemComponent、UGameplayAbility、UGameplayEffect、UAttributeSet。

## 使用方式

```
"帮我用 GAS 实现一个火焰技能"
"属性修改怎么触发 UI 刷新？"
"技能冷却和消耗怎么做？"
```

## 使用示例

### 示例 1：实现技能

> **你**: "做一个火球技能：消耗 20 蓝，3 秒冷却，造成 50 伤害"

> **AI**: 创建 `UGameplayAbility` 子类，`ActivateAbility` 中：
> 1. `CommitAbility`（检查消耗和冷却）
> 2. 施加 GameplayEffect（伤害）
> 3. 播放动画/特效
> 4. `EndAbility`

### 示例 2：属性集

> **你**: "定义血量、蓝量、攻击力"

> **AI**:
> ```cpp
> UCLASS()
> class UMyAttributeSet : public UAttributeSet
> {
>     UPROPERTY(Replicated)
>     FGameplayAttributeData Health;
>     // PostGameplayEffectExecute 中 Clamp 血量
> };
> ```

### 示例 3：GameplayTag 条件

> **你**: "只有未眩晕时才能释放技能"

> **AI**: 在 UGameplayAbility 中设置 `ActivationBlockedTags` 包含 `State.Stunned`。

## 核心概念

| 概念 | 说明 |
|------|------|
| AbilitySystemComponent | GAS 核心组件 |
| GameplayAbility | 单个技能逻辑 |
| GameplayEffect | 属性修改器（瞬时/持续/无限） |
| AttributeSet | 属性集合（血量、蓝量等） |
| GameplayTag | 状态/条件标签 |
| GameplayCue | 技能表现（特效/音效） |

## 适用场景

- 实现 MOBA/RPG 技能系统
- Buff/Debuff 效果
- 属性系统（血量、能量、属性）
- 技能冷却和消耗
