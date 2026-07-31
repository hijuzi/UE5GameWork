---
module: messages
complexity: Medium
loc: 475
file_count: 9
---

# Messages — 消息系统

## 架构

基于 GameplayTag 的解耦消息总线：

```
FLyraVerbMessage (Instigator + Verb(Tag) + Target)
    ↓ 广播
UGameplayMessageSubsystem (引擎内置)
    ↓ 订阅
UGameplayMessageProcessor (ActorComponent基类, 自动管理监听)

网络复制:
FLyraVerbMessageReplication (FastArray)
    → 服务器 AddMessage
    → 客户端 PostReplicatedAdd → RebroadcastMessage()
```

## 关键类

| 类 | 职责 |
|----|------|
| `FLyraVerbMessage` | 核心载荷：(Instigator, Verb(Tag), Target) + 三组Tag容器 + Magnitude |
| `UGameplayMessageProcessor` | ActorComponent基类，BeginPlay自动StartListening，EndPlay反注册 |
| `FLyraNotificationMessage` | UI通知载荷：TargetChannel + PayloadMessage(FText) + PayloadObject |
| `FLyraVerbMessageReplication` | FastArray复制容器：服务器Add→客户端Rebroadcast |
| `ULyraVerbMessageHelpers` | 蓝图书：VerbMessage↔GameplayCueParameters双向转换，玩家查找 |

## 使用场景

5+ 模块导入 Messages:
- `LyraHealthComponent` — 生命值变化事件
- `LyraGameState` — 游戏状态事件
- `LyraPlayerState` — 玩家状态事件
- `LyraHealthSet` — 属性集事件
- `LyraDamageLogDebuggerComponent` — 伤害日志

## 设计模式

- **发布-订阅**: UGameplayMessageSubsystem + Tag路由
- **语义三元组**: Instigator-Verb-Target 模式清晰表达事件
- **FastArray复制**: 服务器消息自动传播到客户端
