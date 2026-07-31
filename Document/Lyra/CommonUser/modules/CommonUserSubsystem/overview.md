# CommonUserSubsystem

> **NW: Modules > CommonUser > Core**  
> *用户管理的核心子系统 — 登录、权限、本地玩家创建*

---

## 基本信息

| 属性 | 值 |
|------|-----|
| 基类 | `UGameInstanceSubsystem` |
| 头文件 | `CommonUserSubsystem.h` |
| 生存期 | 随 GameInstance 创建/销毁 |

---

## 功能概述

`UCommonUserSubsystem` 是 CommonUser 插件的核心，负责:

- 管理所有本地用户的身份和登录状态
- 通过多步状态机执行用户登录流程
- 检查用户在线权限（CanPlay, CanPlayOnline, CanCommunicate 等）
- 创建和管理 `ULocalPlayer` 实例
- 通过 `IPlatformInputDeviceMapper` 映射输入设备到本地用户
- 发送系统消息/通知（使用 `UE::GameplayTags`）

---

## 核心内部类

### FUserLoginRequest

登录状态机，按顺序执行以下步骤:

```
TransferPlatformAuth → AutoLogin → LoginUI → QueryPrivileges → CreateLocalPlayer → Complete
```

| 步骤 | 行为 | 可跳过? |
|------|------|---------|
| `TransferPlatformAuth` | 传递平台认证 Token（Console） | 是（PC） |
| `AutoLogin` | 使用缓存凭据自动登录 | 否 |
| `LoginUI` | 显示登录 UI 让用户输入 | 是（如已登录） |
| `QueryPrivileges` | 检查请求的权限 | 否 |
| `CreateLocalPlayer` | 创建 ULocalPlayer | 是（如需已存在） |
| `Complete` | 清理、广播结果 | 否 |

### UCommonUserInfo

每用户数据容器（仅 C++ 内部可见）:

- `FPlatformUserId` — 平台用户 ID
- `ULocalPlayer*` — 关联的本地玩家
- `ECommonUserLoginState` — 当前登录状态
- Online 子系统特有数据

### FPendingLocalUserOperation

用于处理用户的本地操作，等待异步完成。

---

## 关键 API

| 方法 | 说明 |
|------|------|
| `TryToInitializeUser(Params)` | 启动用户初始化流程 |
| `TryToLoginForOnlinePlay(Params)` | 启动在线对局登录 |
| `GetUserInfoForLocalPlayerIndex(idx)` | 获取指定索引的用户信息 |
| `GetUserInfoForPlatformUser(id)` | 按平台用户 ID 查找 |
| `ResetUserState(user)` | 重置用户状态（登出） |
| `TryToChangePrivilegeCheckContext(user, ctx)` | 切换权限检查上下文 |
| `CanCheckPrivilege(user, privilege)` | 检查是否可验证某权限 |
| `ShouldWaitForStartup()` | 是否等待启动完成 |
| `CanPlayOnline(LocalPlayerIndex)` | 快速检查是否能在线游玩 |

## FCommonUserInitializeParams

| 字段 | 类型 | 说明 |
|------|------|------|
| `LocalPlayerIndex` | `int32` | 目标 LocalPlayer 索引 |
| `PrimaryInputDevice` | `FInputDeviceId` | 主输入设备 ID |
| `RequestedPrivilege` | `ECommonUserPrivilege` | 请求的权限级别 |
| `OnlineContext` | `ECommonUserOnlineContext` | 在线上下文类型 |
| `bCanCreateNewLocalPlayer` | `bool` | 是否允许创建新的 LocalPlayer |
| `bCanUseGuestLogin` | `bool` | 是否允许访客登录 |
| `bCanShowLoginUI` | `bool` | 是否允许显示登录 UI |
| `OnUserInitializeComplete` | `FOnCompleteDelegate` | 完成回调 |

## 委托

| 委托 | 说明 |
|------|------|
| `OnUserPrivilegeChangedDelegate` | 用户权限变更 |
| `OnUserInitializeComplete` (per-request) | 单次初始化完成 |

## 示例

### Blueprint（通过 AsyncAction）

在蓝图中使用 `AsyncAction_CommonUserInitialize` 节点。

### C++

```cpp
// 获取子系统
UGameInstance* GI = GetGameInstance();
UCommonUserSubsystem* UserSystem = GI->GetSubsystem<UCommonUserSubsystem>();

// 构建初始化参数
FCommonUserInitializeParams Params;
Params.LocalPlayerIndex = 0;
Params.RequestedPrivilege = ECommonUserPrivilege::CanPlay;
Params.PrimaryInputDevice = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
Params.bCanUseGuestLogin = true;
Params.bCanCreateNewLocalPlayer = true;
Params.OnUserInitializeComplete.BindLambda([](const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege Priv, ECommonUserOnlineContext Ctx) {
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("User initialized!"));
    }
});

// 启动初始化
UserSystem->TryToInitializeUser(Params);
```

---

## 跨平台抽象

`UCommonUserSubsystem` 通过 `#if COMMONUSER_OSSV1` 编译期宏在两套 API 间切换:

| 功能 | OSSv1 实现 | OSSv2 实现 |
|------|-----------|-----------|
| 获取身份接口 | `OnlineSub->GetIdentityInterface()` | `OnlineServices->GetAuthInterface()` |
| 获取用户权限 | `OnlineSub->GetUserInterface()` | `OnlineServices->GetPresenceInterface()` |
| 平台用户解析 | `Identity->GetPlatformUserIdFromUniqueNetId()` | `Auth->GetLocalOnlineUserByPlatformUserId()` |

---

## 相关文件

- [API 参考](api.md)
- [数据流图 — 登录流程](../../architecture/data-flow.md)
- [登录状态机模式](../../patterns/login-state-machine.md)
