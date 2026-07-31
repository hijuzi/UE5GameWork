# CommonUserSubsystem API

> **NW: Modules > CommonUser > API Reference**

---

## 完整公共 API

### 初始化方法

```cpp
// 开始本地游玩初始化
bool TryToInitializeUser(const FCommonUserInitializeParams& Params);

// 开始在线对局登录（简化版）
bool TryToLoginForOnlinePlay(FCommonUserInitializeParams& Params);

// 检查是否需要等待启动
bool ShouldWaitForStartup() const;
```

### 用户信息查询

```cpp
// 按 LocalPlayer 索引查找
const UCommonUserInfo* GetUserInfoForLocalPlayerIndex(int32 LocalPlayerIndex) const;

// 按平台用户 ID 查找
const UCommonUserInfo* GetUserInfoForPlatformUser(FPlatformUserId PlatformUser) const;

// 获取第一个已登录用户索引
int32 GetFirstPlayerIndexForOnlinePlay() const;

// 获取用于在线游玩的 LocalPlayer 索引
int32 GetLocalPlayerIndexForOnlinePlay() const;

// 通过 ControllerId 查找用户
const UCommonUserInfo* GetUserInfoForControllerId(int32 ControllerId) const;

// 获取指定 LocalPlayer 的 UniqueNetId
FUniqueNetIdRepl GetUniqueNetIdForLocalPlayerIndex(int32 LocalPlayerIndex) const;

// 获取用户总数
int32 GetNumLocalPlayers() const;

// 重置用户状态（等用于部分登出）
void ResetUserState(const UCommonUserInfo* User);
```

### 权限相关

```cpp
// 检查是否可验证指定权限
bool CanCheckPrivilege(const UCommonUserInfo* UserInfo, ECommonUserPrivilege Privilege) const;

// 切换权限检查上下文
bool TryToChangePrivilegeCheckContext(const UCommonUserInfo* UserInfo, ECommonUserOnlineContext OnlineContext);

// 快速检查是否可以游玩（已登录 + CanPlay 权限）
bool CanPlayOnline(int32 LocalPlayerIndex) const;
```

### 委托

```cpp
// 用户权限变更广播（多播委托）
FCommonUserOnUserPrivilegeChanged OnUserPrivilegeChangedDelegate;
```

### 系统消息

```cpp
// 发送系统消息（通过 GameplayTags）
void SendMessageToClient(const UE::GameplayTags::FGameplayTag& Tag, const FText& MessageText);
```

---

## FCommonUserInitializeParams 完整字段

```cpp
struct FCommonUserInitializeParams
{
    // === 输入配置 ===
    int32 LocalPlayerIndex = 0;                         // 目标 LocalPlayer 索引
    FInputDeviceId PrimaryInputDevice;                   // 主输入设备
    bool bCanUseGuestLogin = false;                      // 是否允许访客登录
    bool bCanCreateNewLocalPlayer = true;               // 是否创建新 LocalPlayer
    bool bCanShowLoginUI = true;                         // 是否显示登录UI
    
    // === 权限配置 ===
    ECommonUserPrivilege RequestedPrivilege = ECommonUserPrivilege::CanPlay;
    ECommonUserOnlineContext OnlineContext = ECommonUserOnlineContext::Default;
    
    // === 回调 ===
    FOnUserInitializeComplete OnUserInitializeComplete;  // 完成回调
};
```

---

## ECommonUserPrivilege 枚举

```cpp
enum class ECommonUserPrivilege : uint8
{
    CanPlay,                    // 可单机游玩
    CanPlayOnline,              // 可在线游玩
    CanCommunicateViaTextOnline, // 可文字交流
    CanCommunicateViaVoiceOnline, // 可语音交流
    CanUseUserGeneratedContent,  // 可使用 UGC
    CanCrossPlay,                // 可跨平台游玩
};
```

---

## ECommonUserOnlineContext 枚举

```cpp
enum class ECommonUserOnlineContext : uint8
{
    Default,    // 默认（本地优先）
    Online,     // 强制在线
    Local,      // 强制本地
};
```

---

## 内部类型（供参考）

```cpp
// 登录步骤（内部状态机）
enum class ECommonUserLoginFlowStep : uint8
{
    TransferPlatformAuth,  // 0
    AutoLogin,             // 1
    LoginUI,               // 2
    QueryPrivileges,       // 3
    CreateLocalPlayer,     // 4
    Complete,              // 5
};

// 用户信息（内部使用）
class UCommonUserInfo : public UObject
{
    // 平台用户 ID
    // 本地玩家指针
    // 登录状态
    // OSS 特有数据
};
```

---

## 回调签名

```cpp
// 用户初始化完成
DECLARE_DELEGATE_FiveParams(
    FOnUserInitializeComplete,
    const UCommonUserInfo* /*UserInfo*/,
    bool /*bSuccess*/,
    FText /*Error*/,
    ECommonUserPrivilege /*RequestedPrivilege*/,
    ECommonUserOnlineContext /*OnlineContext*/
);

// 用户权限变更（多播）
DECLARE_MULTICAST_DELEGATE_FourParams(
    FCommonUserOnUserPrivilegeChanged,
    const UCommonUserInfo* /*UserInfo*/,
    ECommonUserPrivilege /*Privilege*/,
    ECommonUserPrivilegeAvailability /*OldAvailability*/,
    ECommonUserPrivilegeAvailability /*NewAvailability*/
);
```
