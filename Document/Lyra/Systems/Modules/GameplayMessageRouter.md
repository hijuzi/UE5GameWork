---
module: gameplaymessagerouter
complexity: Medium
loc: 972
file_count: 9
---

# GameplayMessageRouter

## 用途

轻量级发布-订阅消息代理，Lyra 内部解耦通信的基础设施。5+ 个 Core 模块依赖它。

## 架构

```
UGameplayMessageSubsystem (GameInstanceSubsystem)
    ├── TMap<FGameplayTag, FChannelListenerList>
    │     └── TArray<FGameplayMessageListenerData> + HandleID 计数器
    │
    ├── BroadcastMessage<T>(Channel, Message)
    │     → 向上遍历 Tag 层级 (PartialMatch)
    │     → 运行时类型检查 (IsChildOf)
    │
    └── RegisterListener<T>(Channel, Callback) → FGameplayMessageListenerHandle
          └── Unregister() 自移除

UAsyncAction_ListenForGameplayMessage (Blueprint)
    └── UK2Node_AsyncAction_ListenForGameplayMessages (编辑器节点)
```

## 关键机制

| 机制 | 说明 |
|------|------|
| **类型擦除桥接** | C++ 模板编译时捕获 `UScriptStruct*`，运行时 `void*` + `IsChildOf()` 验证 |
| **分层 Tag 匹配** | `PartialMatch` 向上遍历父标签，`ExactMatch` 仅精确匹配 |
| **弱指针安全** | `TWeakObjectPtr` 防止悬空引用，广播前复制监听者数组 |
| **蓝图通配符** | `CustomThunk` + K2Node 编译器生成中间函数调用 |

## 设计模式

- **发布-订阅**: GameplayTag 作为通道标识符
- **类型安全的消息载荷**: 编译时模板 + 运行时验证
- **句柄模式**: 监听者持有 Handle，自动注销

## Lyra 使用场景

GamePhase 转换、AbilitySystem 事件、UI 状态变化、TeamSystem 通知、Input 触发——任何需要解耦的跨域通信。

## 依赖

仅 UE Core + Engine + GameplayTags，无其他插件依赖。
