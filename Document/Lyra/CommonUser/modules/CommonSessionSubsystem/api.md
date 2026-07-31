# CommonSessionSubsystem API

> **NW: Modules > CommonUser > Sessions > API Reference**

---

## 完整公共 API

### 会话生命周期

```cpp
// 创建并托管在线会话
void HostSession(APlayerController* Player, UCommonSession_HostSessionRequest* Request);

// 快速游玩：自动搜索+加入
void QuickPlaySession(APlayerController* Player, UCommonSession_HostSessionRequest* Request);

// 搜索可用会话
void FindSessions(APlayerController* Player, UCommonSession_SearchSessionRequest* Request);

// 加入已找到的会话
void JoinSession(APlayerController* Player, UCommonSession_SearchResult* SearchResult);

// 清理指定 LocalPlayer 的所有会话
void CleanUpSessions(ULocalPlayer* LocalPlayer);
```

### 会话属性设置

```cpp
// 实时调整最大玩家数
void SetMaxPlayerCount(int32 NewMaxPlayerCount);

// 更新跨平台游玩设置
// 调用时机：用户在设置中修改跨平台选项时
void UpdateCrossplaySettings(const FCrossplaySettings& NewSettings);
```

### 状态查询

```cpp
// 获取指定用户的当前会话信息状态
ECommonSessionInformationState GetSessionState(ULocalPlayer* LocalPlayer) const;

// 获取指定用户的当前游戏模式名称
FString GetGameMode(ULocalPlayer* LocalPlayer) const;

// 获取指定用户的当前地图名称
FString GetMapName(ULocalPlayer* LocalPlayer) const;
```

---

## 委托签名

```cpp
// 会话创建完成
DECLARE_MULTICAST_DELEGATE_TwoParams(
    FOnCreateSessionComplete,
    FName /*SessionName*/,
    bool /*bWasSuccessful*/
);

// 搜索完成
DECLARE_MULTICAST_DELEGATE_TwoParams(
    FOnFindSessionsComplete,
    bool /*bWasSuccessful*/,
    const TArray<UCommonSession_SearchResult*>& /*Results*/
);

// 加入完成
DECLARE_MULTICAST_DELEGATE_OneParam(
    FOnJoinSessionComplete,
    EOnJoinSessionCompleteResult::Type /*Result*/
);

// 会话失败
DECLARE_MULTICAST_DELEGATE_OneParam(
    FOnSessionFailure,
    ECommonSessionFailureReason /*Reason*/
);

// 加入失败
DECLARE_MULTICAST_DELEGATE_OneParam(
    FOnSessionJoinFailure,
    ECommonSessionJoinFailureReason /*Reason*/
);

// 会话信息状态变化（被 Presence 系统监听）
DECLARE_MULTICAST_DELEGATE_ThreeParams(
    FOnSessionInformationChanged,
    ECommonSessionInformationState /*NewState*/,
    FString /*GameMode*/,
    FString /*MapName*/
);
```

---

## UCommonSession_HostSessionRequest 完整字段

```cpp
UCLASS(BlueprintType)
class UCommonSession_HostSessionRequest : public UObject
{
    // === 地图与模式 ===
    FPrimaryAssetId MapID;                    // 主地图 AssetID
    TSubclassOf<AGameModeBase> GameMode;       // 游戏模式类
    FString ExtraArgs;                         // 额外 URL 参数

    // === 容量 ===
    int32 MaxPlayerCount = 16;

    // === 在线选项 ===
    bool bIsOnlineMode = true;                  // LAN 还是 Online
    bool bUseLobby = false;                     // 是否使用 Lobby
    bool bUsePresence = true;                   // 是否好友可见
    bool bUseSAN = false;                       // 专用服务器网络
    bool bAllowInvites = true;                  // 允许邀请
    bool bAllowJoinViaPresence = true;          // 允许通过 Presence 加入
    bool bAllowJoinViaPresenceFriendsOnly = false;
    bool bAntiCheatProtected = false;           // 反作弊保护
    bool bIsDedicatedServer = false;            // 是否专用服务器

    // === 搜索参数 ===
    int32 MaxSearchResults = 50;               // 搜索结果最大数量

    // === 网络标识 ===
    FName SessionNameOverride;                  // 覆盖会话名称
};
```

---

## UCommonSession_SearchSessionRequest 完整字段

```cpp
UCLASS(BlueprintType)
class UCommonSession_SearchSessionRequest : public UObject
{
    bool bOnlineSearch = true;              // 在线搜索（false=LAN）
    ESessionSearchType SearchType;          // 搜索类型
    int32 PingBucketInfo = 0;               // Ping 阈值（ms）
};
```

---

## 搜索相关枚举

```cpp
// 会话搜索结果
UCLASS(BlueprintType)
class UCommonSession_SearchResult : public UObject
{
    // 封装 OSS FOnlineSessionSearchResult
    // 提供 Blueprint 友好的属性访问
    // 内部持有 FOnlineSessionSearchResult 副本
};

// 搜索类型
enum class ESessionSearchType : uint8
{
    Search,             // 标准搜索
    FindFriend,         // 查找好友
};

// 加入结果类型
enum class EOnJoinSessionCompleteResult : uint8
{
    Success,            // 成功
    SessionIsFull,      // 会话已满
    SessionDoesNotExist, // 会话不存在
    CouldNotRetrieveAddress, // 无法获取地址
    AlreadyInSession,   // 已在会话中
    UnknownError,       // 未知错误
};

// 会话失败原因
enum class ECommonSessionFailureReason : uint8
{
    Unknown,
    ServiceNotAvailable,
    SessionFull,
    Timeout,
};

// 加入失败原因
enum class ECommonSessionJoinFailureReason : uint8
{
    Unknown,
    ReservationDenied,
    ReservationTimeout,
    ConnectionFailed,
};
```

---

## Reservation 请求参数

```cpp
// PartyBeacon Reservation 内部参数
struct FPartyReservation
{
    int32 TeamNum;                          // 队伍编号
    FUniqueNetIdRepl PartyLeader;           // 队长 ID
    TArray<FPlayerReservation> PartyMembers; // 队员列表
    bool bRanked;                           // 排位模式
};

// 单个玩家 Reservation
struct FPlayerReservation
{
    FUniqueNetIdRepl UniqueId;              // 玩家 ID
    FString ValidationString;               // 验证字符串
    FString Platform;                       // 平台标识
    bool bAllowCrossPlay;                   // 是否允许跨平台
    float ElapsedTime;                      // 已耗时
};
```
