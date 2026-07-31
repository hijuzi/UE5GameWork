---
module: animation
purpose: Minimal bridge between animation blueprint and GAS via tag-to-variable mapping
complexity: Low
loc: 113
file_count: 2
---

# Animation

## 用途

极简的动画蓝图与 GAS 之间的桥接层，通过 `FGameplayTagBlueprintPropertyMap` 实现 GameplayTag 到动画蓝图变量的自动映射。

## 架构

### 核心机制

1. **GameplayTag → 动画变量**: 
   - `FGameplayTagBlueprintPropertyMap` 自动根据 ASC 上的 GameplayTag 驱动动画蓝图变量 (bool/float/int)
   - 设计师在动画蓝图图表中配置映射，免去手动查询标签

2. **GroundDistance 缓存**:
   - 每帧从 `ULyraCharacterMovementComponent::GetGroundInfo()` 读取
   - 提供给动画蓝图做落地预测和足部 IK

3. **双路径 ASC 初始化**:
   - 主动: `ULyraAbilitySystemComponent::InitAbilityActorInfo()` 主动调用 `InitializeWithAbilitySystem()`
   - 回退: `NativeInitializeAnimation()` 通过 `UAbilitySystemGlobals` 懒发现

## 关键类

| 类 | 职责 |
|----|------|
| `ULyraAnimInstance` | 唯一类，动画蓝图 C++ 基类 |

### 关键方法

- `InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)` — 绑定标签属性映射
- `NativeInitializeAnimation()` — 回退 ASC 发现
- `NativeUpdateAnimation(DeltaTime)` — 每帧更新 GroundDistance

## 设计模式

- **桥接模式**: C++ 层极薄，重逻辑在动画蓝图 (Content/)
- **自动映射**: GameplayTagBlueprintPropertyMap 免去手工标签轮询

## 依赖

AnimInstance, GameplayAbilities (GameplayTagBlueprintPropertyMap), Character (LyraCharacter, MovementComponent)
