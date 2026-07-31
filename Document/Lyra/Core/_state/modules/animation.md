---
module: animation
purpose: Minimal bridge between animation blueprint and GAS via FGameplayTagBlueprintPropertyMap for automatic tag-to-variable mapping.
roots:
  - "LyraStarterGame/Source/LyraGame/Animation/"
complexity: Low
loc: 113
file_count: 2
deps: [Engine/AnimInstance, GameplayAbilities, LyraGame/Character]
escalate: false
---

## Architecture <!-- c2d:s1 -->

Animation 模块仅包含一个类 `ULyraAnimInstance`，是动画蓝图和 GAS 之间的极简桥接。

### 核心机制

1. **GameplayTag → 动画变量映射** (`FGameplayTagBlueprintPropertyMap`): 自动根据 ASC 上的 GameplayTag 驱动动画蓝图变量 (bool/float/int)。设计师在动画蓝图图表中配置映射，免去手动查询标签。

2. **GroundDistance 缓存**: 每帧从 `ULyraCharacterMovementComponent::GetGroundInfo()` 读取，提供给动画蓝图做落地预测。

3. **双路径 ASC 初始化**:
   - 主动: `ULyraAbilitySystemComponent::InitAbilityActorInfo()` 调用 `InitializeWithAbilitySystem()`
   - 回退: `NativeInitializeAnimation()` 通过 `UAbilitySystemGlobals` 懒发现

## Key Files

| 文件 | 重要性 |
|------|--------|
| `LyraAnimInstance.h` | 声明 GameplayTagPropertyMap 和 GroundDistance |
| `LyraAnimInstance.cpp` | 初始化绑定和每帧数据更新 |
