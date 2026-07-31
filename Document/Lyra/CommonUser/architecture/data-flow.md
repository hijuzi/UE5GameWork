# 数据流图

> **NW: Architecture > CommonUser > Data Flow**

---

## 用户登录流程

```mermaid
sequenceDiagram
    participant BP as Blueprint/GameCode
    participant Async as AsyncAction_CommonUserInitialize
    participant Sub as UCommonUserSubsystem
    participant Login as FUserLoginRequest
    participant OSS as Online Subsystem

    BP->>Async: InitializeForLocalPlay(PlayerIndex, InputDevice)
    Async->>Async: Activate()
    Async->>Sub: TryToInitializeUser(Params)
    Sub->>Login: CreateLoginRequest(Params)
    
    rect rgb(240, 248, 255)
        Note over Login: Step 1: TransferPlatformAuth
        Login->>OSS: Transfer platform auth token
        OSS-->>Login: (optional, console only)
        
        Note over Login: Step 2: AutoLogin
        Login->>OSS: Auto-login with stored credentials
        OSS-->>Login: Success / Fail
        
        alt AutoLogin Failed
            Note over Login: Step 3: LoginUI (optional)
            Login-->>BP: Show Login UI
            BP-->>Login: User credentials
            Login->>OSS: Manual login
        end
        
        Note over Login: Step 4: QueryUserPrivilege
        Login->>OSS: Check privileges (CanPlay, CrossPlay, etc.)
        OSS-->>Login: Privilege results
        
        Note over Login: Step 5: CreateLocalPlayer
        Login->>Sub: Create new ULocalPlayer
    end
    
    Login-->>Sub: OnLoginComplete(Result)
    Sub-->>Async: OnUserInitializeComplete(UserInfo, bSuccess)
    Async-->>BP: OnInitializationComplete delegate
```

## 字段与决策点

| 步骤 | 枚举值 | 关键决策 |
|------|--------|---------|
| 1. TransferPlatformAuth | `TransferPlatformAuth` | 仅在 Console 平台执行，PC 跳过 |
| 2. AutoLogin | `AutoLogin` | 失败时进入 LoginUI（如果允许） |
| 3. LoginUI | `LoginUI` | 仅当 `bCanShowLoginUI = true` |
| 4. QueryPrivileges | `QueryPrivileges` | 检查 `RequestedPrivilege` 指定权限 |
| 5. CreateLocalPlayer | `CreateLocalPlayer` | 仅在 `bCanCreateNewLocalPlayer = true` 时 |
| 6. Complete | `Complete` | 终止状态 |

## 会话创建流程

```mermaid
sequenceDiagram
    participant BP as Game Code
    participant CS as UCommonSessionSubsystem
    participant OSS as Online Subsystem
    participant Bcn as PartyBeacon

    BP->>CS: HostSession(HostRequest)
    CS->>CS: ResolveMapID → AssetManager
    CS->>OSS: CreateSession(SessionSettings)
    OSS-->>CS: SessionCreated(SessionName, bSuccess)
    
    alt Success
        CS->>CS: UpdateCrossplaySettings
        CS->>Engine: ServerTravel(MapURL)
        CS->>Bcn: CreateBeacon(Reservation)
    end
    
    CS-->>BP: OnSessionCreated delegate
```

## 会话加入流程（Reservation）

```mermaid
sequenceDiagram
    participant BP as Game Code
    participant CS as UCommonSessionSubsystem
    participant OSS as Online Subsystem
    participant Bcn as PartyBeacon

    BP->>CS: JoinSession(SearchResult)
    CS->>Bcn: RequestReservation(PlayerCount)
    Bcn-->>CS: ReservationResponse
    
    alt Reservation Approved
        CS->>OSS: JoinSession(SearchResult)
        OSS-->>CS: JoinComplete
        CS->>CS: ClientTravel(MapURL)
    else Reservation Denied
        CS-->>BP: OnSessionJoinFailed
    end
```

## Presence 更新流程

```mermaid
sequenceDiagram
    participant Sess as UCommonSessionSubsystem
    participant Prs as UCommonUserBasicPresence
    participant OSS as Online Subsystem

    Sess->>Sess: Session State Changes (e.g. MatchmakingStarted)
    Sess-->>Prs: OnSessionInformationChangedEvent(State, GameMode, Map)
    
    opt bEnableSessionsBasedPresence == true & !DedicatedServer
        Prs->>Prs: SessionStateToBackendKey(State)
        Prs->>OSS: SetPresence / PartialUpdatePresence
    end
```

## 关键异步操作列表

| 操作 | 入口 | 完成回调 | 耗时特征 |
|------|------|---------|---------|
| 用户初始化 | `TryToInitializeUser` | `OnUserInitializeComplete` | 50ms~10s+ (取决于平台) |
| 创建会话 | `HostSession` | `OnCreateSessionCompleteEvent` | 100ms~2s |
| 搜索会话 | `FindSessions` | `OnFindSessionsCompleteEvent` | 500ms~10s |
| 加入会话 | `JoinSession` | `OnJoinSessionCompleteEvent` | 500ms~5s |
| Presence更新 | `OnNotifySessionInformationChanged` | 无（fire-and-forget） | <50ms |
