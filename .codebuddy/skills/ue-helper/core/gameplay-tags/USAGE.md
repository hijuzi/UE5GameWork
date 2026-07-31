# gameplay-tags 使用指南

## 功能

使用 Gameplay Tags —— 层级 FName 标签（FGameplayTag），在 C++ 和蓝图中替代枚举判断。

## 使用方式

```
"帮我定义 Damage.Fire 和 Damage.Ice 标签"
"怎么在 C++ 中判断技能标签？"
"标签层级关系怎么用？"
```

## 使用示例

### 示例 1：定义标签

> **你**: "定义武器伤害类型标签"

> **AI**: 在 Project Settings → GameplayTags 或 DataTable 创建：
> ```
> Damage.Fire
> Damage.Ice
> Damage.Physical.Slash
> Damage.Physical.Blunt
> ```

### 示例 2：C++ 中匹配标签

> **你**: "判断伤害是否属于物理类型"

> **AI**:
> ```cpp
> FGameplayTag DamageTag = ...;
> if (DamageTag.MatchesTag(FGameplayTag::RequestGameplayTag("Damage.Physical"))) {
>     // Damage.Physical.Slash / Damage.Physical.Blunt 都能匹配到
> }
> ```

### 示例 3：GAS 中使用

> **你**: "为火焰伤害添加燃烧 Debuff"

> **AI**: GameplayEffect 的条件标签设置 `SourceTags.RequireTag == Damage.Fire`。

## 层级匹配

```
Damage.Physical.Slash
├─ 精确匹配: "Damage.Physical.Slash"
├─ 父级匹配: "Damage.Physical" ✅
└─ 不匹配: "Damage.Magic" ❌
```

## 适用场景

- 伤害类型、状态效果分类
- GAS 技能条件和阻挡
- 替代枚举做灵活的类型系统
- 蓝图和 C++ 间共享标签
