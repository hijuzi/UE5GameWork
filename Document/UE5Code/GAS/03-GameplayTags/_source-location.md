# 源码定位清单 — 03-GameplayTags文章

> 引擎版本：UE 5.8  
> 引擎源码根路径：`F:\Epic\UE\UE_5.8\Engine`  
> GAS 插件路径：`Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/`

---

## 本文涉及源码文件

| # | 文件 | 引擎相对路径 | 文中引用处 | 用途 |
|---|------|-------------|-----------|------|
| 1 | `GameplayTagContainer.h` | `Engine/Source/Runtime/GameplayTags/Classes/GameplayTagContainer.h` | §二.3（L117：`EGameplayTagQueryExprType` 枚举，L690） | 8 种查询表达式类型定义 |
| 2 | `GameplayAbility.h` | `Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/Abilities/GameplayAbility.h` | §三.1（L144：`Category = Tags` 分组的 UPROPERTY 成员） | GA 的 5 类 Tag 字段声明 |
| 3 | `TargetTagsGameplayEffectComponent.h` | `Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/GameplayEffectComponents/TargetTagsGameplayEffectComponent.h` | §三.2（L197：`InheritableGrantedTagsContainer`） | GE 通道一：授予 Tag |
| 4 | `TargetTagRequirementsGameplayEffectComponent.h` | `Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/GameplayEffectComponents/TargetTagRequirementsGameplayEffectComponent.h` | §三.2（L201-203：`ApplicationTagRequirements` / `OngoingTagRequirements`） | GE 通道二：施加条件 + 持续条件 |
| 5 | `RemoveOtherGameplayEffectComponent.h` | `Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/GameplayEffectComponents/RemoveOtherGameplayEffectComponent.h` | §三.2（L207：`RemoveGameplayEffectQueries`） | GE 通道三：移除其他 GE |
| 6 | `TargetTagRequirementsGameplayEffectComponent.cpp` | `Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Private/GameplayEffectComponents/TargetTagRequirementsGameplayEffectComponent.cpp` | §三.5（L279-290：`OnAddedToActiveContainer`）；§三.6（L309-323：`OnTagChanged`） | 施加时注册 Tag 回调 + 持续条件判定 |
| 7 | `RemoveOtherGameplayEffectComponent.cpp` | `Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Private/GameplayEffectComponents/RemoveOtherGameplayEffectComponent.cpp` | §三.7（L353-372：`OnGameplayEffectApplied`） | 移除匹配 GE 的实现 |
| 8 | `AbilitySystemComponent.cpp` | `Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Private/AbilitySystemComponent.cpp` | §三.4（L224-228：`CanApply` 调用点）；§三.6（L328-344：`SetActiveGameplayEffectInhibit`） | ASC 中的施加前检查 + Tag 抑制/恢复 |
| 9 | `AbilitySystemComponent.h` | `Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/AbilitySystemComponent.h` | §三.9（L395-401：`GameplayTagCountContainer` / `BlockedAbilityTags`）；§四.3（L473：`MinimalReplicationTags`） | ASC Tag 容器 + 网络复制字段 |
| 10 | `GameplayEffect.cpp` | `Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Private/GameplayEffect.cpp` | §三.4（L234-247：`CanApply`）；§三.5（L296-300：`UpdateTagMap`）；§三.7（L377-381：`UpdateTagMap` 计数 -1） | GE 施加/移除时的 Tag 计数修改 |
| 11 | `GameplayEffect.h` | `Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/GameplayEffect.h` | §三.5（`CachedGrantedTags`）；§三.8（GE Component 化设计） | GE 基类：`CanApply`、GrantedTags 缓存 |
| 12 | `GameplayEffectTypes.h` | `Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/GameplayEffectTypes.h` | §三.4（L252-256：`FGameplayTagRequirements` 结构体）；§三.9（L406-417：`FGameplayTagCountContainer` 类定义） | Tag 条件结构体 + 引用计数容器 |
| 13 | `GameplayEffectTypes.cpp` | `Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Private/GameplayEffectTypes.cpp` | §三.4（L262-269：`RequirementsMet`） | Tag 条件判定实现 |
| 14 | `GameplayTagsManager.h` | `Engine/Source/Runtime/GameplayTags/Classes/GameplayTagsManager.h` | §二.2（L97：`UGameplayTagsManager::GameplayTagTree`） | 全局静态 Tag 树 |

### 模块级目录引用（延伸阅读用）

| 目录 | 引擎相对路径 | 文中引用处 |
|------|-------------|-----------|
| GameplayTags 模块 | `Engine/Source/Runtime/GameplayTags/` | §五（延伸阅读） |
| GE Components 实现 | `Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Private/GameplayEffectComponents/` | §五（延伸阅读） |

---

*本文基于 UE 5.8 源码分析，所有行号引用均以此版本为准。*
