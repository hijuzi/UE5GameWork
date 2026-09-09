# 源码定位清单 — BTDecorator_CheckGameplayTagsOnActor深度分析

> 引擎版本：UE 5.8  
> 引擎源码根路径：`E:\EpicGames\UE_5.8\Engine`

---

## 本文涉及源码文件

| # | 文件 | 引擎相对路径 | 文中引用处 | 用途 |
|---|------|-------------|-----------|------|
| 1 | `BTDecorator_CheckGameplayTagsOnActor.cpp` | `Engine/Source/Runtime/AIModule/Private/BehaviorTree/Decorators/BTDecorator_CheckGameplayTagsOnActor.cpp` | §三（逐行解读主文件） | 本文主角：装饰器节点实现 |
| 2 | `BTDecorator_CheckGameplayTagsOnActor.h` | `Engine/Source/Runtime/AIModule/Classes/BehaviorTree/Decorators/BTDecorator_CheckGameplayTagsOnActor.h` | §三.1（类声明与三个 UPROPERTY） | 节点类声明与配置字段 |
| 3 | `BTDecorator.h` | `Engine/Source/Runtime/AIModule/Classes/BehaviorTree/BTDecorator.h` | §三.2（`CalculateRawConditionValue` / `IsInversed` / Abort 标志位） | 装饰器基类：反转与中止机制 |
| 4 | `BTDecorator.cpp` | `Engine/Source/Runtime/AIModule/Private/BehaviorTree/BTDecorator.cpp` | §三.2（`WrappedCanExecute` L46-50）；§四.4（`GetStaticDescription` L122-145） | 基类实现：条件反转包装 + 描述拼接 |
| 5 | `GameplayTagAssetInterface.h` | `Engine/Source/Runtime/GameplayTags/Classes/GameplayTagAssetInterface.h` | §三.3（`HasAllMatchingGameplayTags` / `HasAnyMatchingGameplayTags`） | 被检查对象需要实现的接口契约 |
| 6 | `GameplayTagAssetInterface.cpp` | `Engine/Source/Runtime/GameplayTags/Private/GameplayTagAssetInterface.cpp` | §四.1（`HasAllMatchingGameplayTags` L21-27） | 接口默认实现：委托给 FGameplayTagContainer |
| 7 | `GameplayTagContainer.h` | `Engine/Source/Runtime/GameplayTags/Classes/GameplayTagContainer.h` | §二（`EGameplayContainerMatchType` L24-31）；§三.5（`ToMatchingText` L557） | 匹配模式枚举 + 描述文本生成 |
| 8 | `AbilitySystemComponent.h` | `Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/AbilitySystemComponent.h` | §四.1（L109：`UAbilitySystemComponent` 实现 `IGameplayTagAssetInterface`） | 说明接口的实际实现者（GAS 插件） |
| 9 | `AIModule.Build.cs` | `Engine/Source/Runtime/AIModule/AIModule.Build.cs` | §四.1（L16：依赖 `GameplayTags`，不依赖 `GameplayAbilities`） | 模块依赖边界证据 |

### 模块级目录引用（延伸阅读用）

| 目录 | 引擎相对路径 | 文中引用处 |
|------|-------------|-----------|
| BehaviorTree Decorators 实现 | `Engine/Source/Runtime/AIModule/Private/BehaviorTree/Decorators/` | §五（延伸阅读：同目录其他装饰器） |
| GameplayTags 模块 | `Engine/Source/Runtime/GameplayTags/` | §五（延伸阅读） |

---

*本文基于 UE 5.8 源码分析，所有行号引用均以此版本为准。*
