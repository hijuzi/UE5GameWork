---
module: core-dependency-map
---

# Dependency Map — Core

## 模块间依赖

```mermaid
graph TD
    subgraph Core
        AS[AbilitySystem<br/>5,400 LOC] --> CH[Character<br/>2,945 LOC]
        CH --> CA[Camera<br/>1,690 LOC]
        CH --> IN[Input<br/>893 LOC]
        IN --> AS
        AN[Animation<br/>113 LOC] --> AS
        AN --> CH
    end

    subgraph External
        AS --> GA[GameplayAbilities Plugin]
        CH --> MGA[ModularGameplayActors]
        IN --> EI[EnhancedInput Plugin]
    end
```

## 依赖矩阵

|  | AbilitySystem | Character | Camera | Input | Animation |
|--|:---:|:---:|:---:|:---:|:---:|
| **AbilitySystem** | - | ❌ | ❌ | ❌ | ❌ |
| **Character** | ✅ | - | ❌ | ❌ | ❌ |
| **Camera** | ❌ | ✅ | - | ❌ | ❌ |
| **Input** | ✅ | ✅ | ❌ | - | ❌ |
| **Animation** | ✅ | ✅ | ❌ | ❌ | - |

## 耦合度分析

| 模块 | 内部依赖 | 外部依赖 | 评级 |
|------|---------|---------|------|
| AbilitySystem | 0 | GameplayAbilities, GameplayTags | 🟢 内聚独立 |
| Character | 1 (AbilitySystem) | ModularGA, GameFeatures | 🟡 中度 |
| Camera | 1 (Character) | Engine/Camera | 🟢 轻量 |
| Input | 2 | EnhancedInput, Settings | 🟡 中度 |
| Animation | 2 | Character, AbilitySystem | 🟢 轻量薄层 |
