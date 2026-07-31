# CommonSessionSubsystem

> **NW: Modules > CommonUser > Sessions**  
> *在线会话管理 — 创建、搜索、加入在线游戏会话*

---

## 基本信息

| 属性 | 值 |
|------|-----|
| 基类 | `UGameInstanceSubsystem` |
| 头文件 | `CommonSessionSubsystem.h` |
| 生命周期 | 随 GameInstance 创建/销毁 |
| 依赖 | `UCommonUserSubsystem`, AssetManager, PartyBeacon, GameplayTags |

---

## 功能概述

`UCommonSessionSubsystem` 负责管理与在线会话相关的所有操作:

- **Host Session** — 创建在线会话并启动 ServerTravel
- **Find Sessions** — 搜索在线会话（支持跨平台过滤）
- **Join Session** — 加入已发现的会话（含 PartyBeacon Reservation）
- **Clean Up** — 清理不再需要的会话
- **Play Together** — Lyra 专用功能，快速与好友同玩
- **Crossplay 设置** — 管理平台的跨平台游玩设置

---

## 关键数据结构

### FCommonSession_HostSessionRequest

```cpp
UCLASS(BlueprintType)
class UCommonSession_HostSessionRequest : public UObject
{
    // 主地图资产 ID（用于 AssetManager 解析为路径）
    FPrimaryAssetId MapID;
    
    // 游戏模式类
    TSubclassOf<AGameModeBase> GameMode;
    
    // 额外地图参数
    FString ExtraArgs;
    
    // 最大玩家数
    int32 MaxPlayerCount = 16;
    
    // 是否允许在线模式
    bool bIsOnlineMode = true;
    
    // 是否使用 Lobby
    bool bUseLobby = false;
    
    // 是否使用 Presence（好友可见）
    bool bUsePresence = true;
    
    // 是否使用 SAN（专用游戏服务器网络）
    bool bUseSAN = false;
    
    // 是否允许通过 Presence 邀请好友
    bool bAllowInvites = true;
    
    // 跨平台设置
    bool bAllowJoinViaPresence = true;
    bool bAllowJoinViaPresenceFriendsOnly = false;
    bool bAntiCheatProtected = false;
    bool bIsDedicatedServer = false;
    
    // 会话 ID 覆盖（用于网络连接）
    FName SessionNameOverride;
    
    // 最大搜索结果数
    int32 MaxSearchResults = 50;
};
```

### FCommonSession_SearchSessionRequest

```cpp
UCLASS(BlueprintType)
class UCommonSession_SearchSessionRequest : public UObject
{
    // 是否在线搜索
    bool bOnlineSearch = true;
    
    // 搜索类型（LAN/Online）
    ESessionSearchType SearchType;
    
    // 搜索 Ping 阈值（ms）
    int32 PingBucketInfo = 0;
};
```

## 关键 API

| 方法 | 说明 |
|------|------|
| `HostSession(PlayerController, HostRequest)` | 创建并托管在线会话 |
| `QuickPlaySession(PlayerController, Request)` | 快速游玩（搜索+加入） |
| `FindSessions(PlayerController, Request)` | 搜索可用会话 |
| `JoinSession(PlayerController, SearchResult)` | 加入指定会话 |
| `CleanUpSessions(LocalPlayer)` | 清理该玩家的所有会话 |
| `SetMaxPlayerCount(int32)` | 设置最大玩家数（实时调整） |
| `UpdateCrossplaySettings(...)` | 更新跨平台设置 |

## 委托

| 委托 | 触发时机 |
|------|---------|
| `OnCreateSessionCompleteEvent` | 会话创建完成 |
| `OnFindSessionsCompleteEvent` | 搜索完成 |
| `OnJoinSessionCompleteEvent` | 加入完成 |
| `OnSessionFailureEvent` | 会话失败 |
| `OnSessionJoinFailureEvent` | 加入失败 |
| `OnSessionInformationChangedEvent` | 会话状态变化 |

## 会话状态枚举

```cpp
enum class ECommonSessionInformationState : uint8
{
    OutOfGame,     // 未在游戏中
    Matchmaking,   // 匹配中
    InGame,        // 游戏中
};
```

## 示例

### C++ — 创建会话

```cpp
auto* SessionSystem = GetGameInstance()->GetSubsystem<UCommonSessionSubsystem>();

UCommonSession_HostSessionRequest* Request = NewObject<UCommonSession_HostSessionRequest>();
Request->MapID = FPrimaryAssetId(TEXT("Map"), TEXT("L_LyraMain"));
Request->GameMode = AGameModeBase::StaticClass();
Request->MaxPlayerCount = 8;
Request->bIsOnlineMode = true;

SessionSystem->HostSession(GetOwningPlayer(), Request);
```

### C++ — 搜索并加入会话

```cpp
auto* SessionSystem = GetGameInstance()->GetSubsystem<UCommonSessionSubsystem>();

// 绑定搜索结果回调
SessionSystem->OnFindSessionsCompleteEvent.AddUObject(this, &UMyClass::OnSessionsFound);

UCommonSession_SearchSessionRequest* SearchRequest = NewObject<UCommonSession_SearchSessionRequest>();
SearchRequest->bOnlineSearch = true;
SessionSystem->FindSessions(GetOwningPlayer(), SearchRequest);

void UMyClass::OnSessionsFound(const TArray<UCommonSession_SearchResult*>& Results)
{
    if (Results.Num() > 0)
    {
        auto* SessionSystem = GetGameInstance()->GetSubsystem<UCommonSessionSubsystem>();
        SessionSystem->JoinSession(GetOwningPlayer(), Results[0]);
    }
}
```

---

## Reservation 机制

`UCommonSessionSubsystem` 使用 `UPartyBeaconClient` 实现 Reservation 机制：

1. 客户端在加入前向服务端发送 Reservation 请求
2. 服务端检查空位，批准或拒绝
3. 批准后客户端才真正连接

这避免了连接成功后发现服务器已满的问题。

### 关键参数

| 参数 | 值 |
|------|-----|
| `BeaconHostName` | 目标服务器地址 |
| `BeaconPort` | Beacon 端口 (通常 = GamePort + 1) |
| `PartySize` | 队伍人数 |
| `bRanked` | 是否排位模式 |

---

## 相关文件

- [[/modules/CommonSessionSubsystem/api|API 参考]]
- [[/architecture/data-flow|数据流图 — 会话创建/加入]]
- [[/modules/CommonUserBasicPresence/overview|Presence 自动更新]]
