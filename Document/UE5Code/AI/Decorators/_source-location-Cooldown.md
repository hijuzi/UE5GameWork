# 源码定位清单 — BTDecorator_Cooldown深度分析

> 引擎版本：UE 5.8  
> 引擎源码根路径：`E:\EpicGames\UE_5.8\Engine`

---

## 本文涉及源码文件

| # | 文件 | 引擎相对路径 | 文中引用处 | 用途 |
|---|------|-------------|-----------|------|
| 1 | `BTDecorator_Cooldown.cpp` | `Engine/Source/Runtime/AIModule/Private/BehaviorTree/Decorators/BTDecorator_Cooldown.cpp` | §四（逐行解读主文件） | 本文主角：冷却装饰器实现 |
| 2 | `BTDecorator_Cooldown.h` | `Engine/Source/Runtime/AIModule/Classes/BehaviorTree/Decorators/BTDecorator_Cooldown.h` | §三.2（L11-15：`FBTCooldownDecoratorMemory`）；§四.1（L27-28：`CoolDownTime`） | 节点类声明 + 实例内存结构体 |
| 3 | `BTDecorator.h` | `Engine/Source/Runtime/AIModule/Classes/BehaviorTree/BTDecorator.h` | §四.1（L143-152：`INIT_DECORATOR_NODE_NOTIFY_FLAGS` 宏）；§四.2（L129-140：`InitNotifyFlags`） | 基类 notify 标志 + 模板元编程宏 |
| 4 | `ValueOrBBKey.h` | `Engine/Source/Runtime/AIModule/Classes/BehaviorTree/ValueOrBBKey.h` | §四.3（L281-308：`FValueOrBBKey_Float`）；§四.3（L50-71：`FBlackboard::GetValue`） | "值或黑板键"数据绑定机制 |
| 5 | `BTDecorator_Blackboard.cpp` | `Engine/Source/Runtime/AIModule/Private/BehaviorTree/Decorators/BTDecorator_Blackboard.cpp` | §五.2（L60-73：`OnBlackboardKeyValueChange`） | 对比：黑板观察者式事件源 |
| 6 | `BTDecorator_CheckGameplayTagsOnActor.cpp` | `Engine/Source/Runtime/AIModule/Private/BehaviorTree/Decorators/BTDecorator_CheckGameplayTagsOnActor.cpp` | §五（对照：无状态纯查询装饰器） | 上一篇文章主角，本文对照组 |

### 模块级目录引用（延伸阅读用）

| 目录 | 引擎相对路径 | 文中引用处 |
|------|-------------|-----------|
| BehaviorTree Decorators 实现 | `Engine/Source/Runtime/AIModule/Private/BehaviorTree/Decorators/` | §六（延伸阅读） |
| 值或黑板键系列类型 | `Engine/Source/Runtime/AIModule/Classes/BehaviorTree/ValueOrBBKey.h` | §四.3（延伸阅读） |

---

*本文基于 UE 5.8 源码分析，所有行号引用均以此版本为准。*
