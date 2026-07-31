# OSS 版本适配

> **NW: Cross-Cutting > CommonUser > OSS Versioning**

---

## 概述

CommonUser 需要同时支持两套在线子系统 API。这是通过 **编译期宏 `COMMONUSER_OSSV1`** 实现的。

---

## 影响范围

整个插件中，**所有与在线平台交互的代码**都受 OSS 版本影响:

| 文件 | OSS 相关代码行数 | 分支数 |
|------|-----------------|--------|
| `CommonUserSubsystem.cpp` | ~200 | 6 |
| `CommonSessionSubsystem.cpp` | ~300 | 8 |
| `CommonUserBasicPresence.cpp` | ~40 | 2 |
| `CommonUserTypes.cpp` | ~10 | 1 |
| **合计** | **~550** | **~17** |

---

## API 映射表

### 子系统访问

| OSSv1 | OSSv2 |
|-------|-------|
| `Online::GetSubsystem(World)` | `UE::Online::GetServices(World)` |
| 返回 `IOnlineSubsystem*` | 返回 `IOnlineServicesPtr` |

### 用户身份

| OSSv1 | OSSv2 |
|-------|-------|
| `IOnlineIdentityPtr` | `IAuthPtr` |
| `FUniqueNetId` (指针) | `FAccountId` (值类型) |
| `GetUniquePlayerId(Index)` | `GetLocalOnlineUserByPlatformUserId(...)` |
| `Login(Index, Credentials)` | `Login(Params)` |
| `AutoLogin(Index)` | `AutoLogin(Params)` |

### 用户权限

| OSSv1 | OSSv2 |
|-------|-------|
| `IUserInterface` | `IPrivilegeCheckingPtr` |
| `QueryUserPrivilege(...)` | `CheckPrivilege(...)` |
| 同步或异步 | 统一异步 |

### 在线会话

| OSSv1 | OSSv2 |
|-------|-------|
| `IOnlineSessionPtr` | `ISessionsPtr` |
| `CreateSession(Settings)` | `CreateSession(Params)` |
| `FindSessions(Search)` | `FindSessions(Params)` |
| `JoinSession(Result)` | `JoinSession(Params)` |

### Presence

| OSSv1 | OSSv2 |
|-------|-------|
| `IOnlinePresencePtr` | `IPresencePtr` |
| `SetPresence(Id, Status)` — 全量更新 | `PartialUpdatePresence(Params)` — 增量更新 |
| `FOnlineUserPresenceStatus` | `FPartialUpdatePresence::Params` |

---

## 编译配置

`COMMONUSER_OSSV1` 宏的定义取决于目标平台和引擎版本:

```
UE5.0 - UE5.2: 默认 OSSv1 (OnlineServices 尚未成熟)
UE5.3+:        默认 OSSv2 (OnlineServices 成为推荐)
特定 Console:  可能始终 OSSv1 (取决于平台 SDK 支持)
```

---

## 迁移检查清单

当从 OSSv1 迁移到 OSSv2 时:

- [ ] 确认目标平台 SDK 支持 OSSv2
- [ ] 确认所有使用的 OSS 接口有 OSSv2 对应物
- [ ] 更新 `CommonUser.Build.cs` 依赖（从 OnlineSubsystem 到 OnlineServices）
- [ ] 测试: 登录 → 创建会话 → 搜索 → 加入 → Presence
- [ ] 测试: 错误路径（网络断开、平台拒绝）

---

## 相关文件

- [[/patterns/dual-oss-abstraction|双 OSS 抽象模式]]
- [[/cross-cutting/platform-abstraction|平台抽象]]
- [[/health/report|代码健康报告 — OSS 分支复杂度]]
