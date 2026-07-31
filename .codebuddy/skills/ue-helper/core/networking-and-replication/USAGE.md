# networking-and-replication 使用指南

## 功能

实现服务器权威多人游戏 —— 网络角色（ROLE_Authority/ROLE_SimulatedProxy）、RPC（Server/Client/NetMulticast）、属性复制（DOREPLIFETIME）、网络相关性。

## 使用方式

```
"帮我做一个血量的网络同步"
"Server RPC 怎么验证客户端参数？"
"多人游戏中只有附近玩家才同步怎么配置？"
```

## 使用示例

### 示例 1：属性复制

> **你**: "同步血量和弹药数"

> **AI**:
> ```cpp
> UPROPERTY(Replicated)
> float Health;
> UPROPERTY(Replicated)
> int32 Ammo;
> // .cpp:
> void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const {
>     DOREPLIFETIME(AMyCharacter, Health);
>     DOREPLIFETIME_CONDITION(AMyCharacter, Ammo, COND_OwnerOnly);
> }
> ```

### 示例 2：Server RPC 加验证

> **你**: "客户端请求开火，服务器怎么防作弊？"

> **AI**:
> ```cpp
> UFUNCTION(Server, Reliable, WithValidation)
> void ServerFire(FVector_NetQuantize AimLocation);
> // _Implementation 执行逻辑
> // _Validate 检查 AimLocation 是否合理（范围、频率）
> ```

### 示例 3：网络相关性

> **你**: "100 米外的敌人不需要同步位置"

> **AI**: 设置 `NetCullDistanceSquared` 或 `bAlwaysRelevant = false`。

## 网络角色

| 角色 | 在哪里 | 能做什么 |
|------|--------|---------|
| ROLE_Authority | 服务器 | 所有操作 |
| ROLE_AutonomousProxy | 所属客户端 | 输入+预测 |
| ROLE_SimulatedProxy | 其他客户端 | 仅接收复制 |

## 适用场景

- 多人游戏属性同步
- 客户端→服务器 RPC 调用
- 网络相关性优化
- 反作弊验证
