---
module: ability-system
purpose: GAS wrapper with Lyra-specific ASC, AbilitySet, damage/heal executions, ability tag relationships
complexity: High
loc: 5400
file_count: 51
---

# AbilitySystem

## 用途

围绕 Epic Gameplay Ability System (GAS) 构建的完整封装层，提供 Lyra 特定的 ASC、AbilitySet 数据驱动能力授予、伤害/治疗执行管道、能力标签关联和激活组管理。

## 架构

### 五层结构

```
1. ASC 层: ULyraAbilitySystemComponent
   ├── InputTagPressed/Held/Released (输入路由)
   ├── ProcessAbilityInput() (每帧处理)
   └── ActivationGroup 管理

2. 能力层: ULyraGameplayAbility
   ├── ActivationPolicy: OnInputTriggered/WhileInputActive/OnSpawn
   ├── ActivationGroup: Independent/Exclusive/ExclusiveReplaceable
   └── Cost + AdditionalCosts

3. 属性层: ULyraAttributeSet → ULyraHealthSet, ULyraCombatSet
   └── PostGameplayEffectExecute 伤害处理

4. 执行层: ULyraDamageExecution, ULyraHealExecution
   └── 距离/物理材质/队伍修正

5. 支持层: AbilityTagRelationshipMapping, GamePhaseAbility
```

### 能力激活流程

```
Input → ASC.AbilityInputTagPressed(Tag)
  → 缓存 InputPressedSpecHandles + InputHeldSpecHandles
  → ProcessAbilityInput():
    - OnInputTriggered → 排队未激活能力
    - WhileInputActive → 排队未激活能力
  → TryActivateAbility():
    - CanActivateAbility() 检查标签/TagRelationship 扩展
    - ActivationGroup 冲突检查
    - Cost 检查
  → ActivateAbility() 执行
  → EndAbility() 清理
```

## 关键类

| 类 | 职责 |
|----|------|
| `ULyraAbilitySystemComponent` | ASC 核心，输入路由，激活组管理 |
| `ULyraGameplayAbility` | 能力基类，策略/组/Cost |
| `ULyraAbilitySet` | DataAsset 驱动能力授予 |
| `ULyraHealthSet` | Health/MaxHealth/Damage/Healing 属性 |
| `ULyraCombatSet` | BaseDamage/BaseHeal 基础属性 |
| `ULyraDamageExecution` | 伤害计算执行（距离/材质/队伍修正） |
| `AbilityTagRelationshipMapping` | 标签关联扩展 CanActivateAbility |
| `GamePhaseAbility` | 游戏阶段能力基类 |

## 设计模式

- **数据驱动**: AbilitySet DataAsset 外部化配置
- **策略模式**: ActivationPolicy 定义不同输入响应策略
- **责任链**: TagRelationship 扩展 CanActivateAbility 检查链

## 依赖

GameplayAbilities, GameplayTags, Team, Character, Input, Messages
