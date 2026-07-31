# OnlineContext 解析

> **NW: Patterns > CommonUser > Context Resolution**

---

## 模式概述

`ECommonUserOnlineContext` 是一个三级枚举，用于控制用户操作优先使用本地还是在线上下文。这是一种 **Priority Chain** 模式。

---

## 枚举定义

```cpp
enum class ECommonUserOnlineContext : uint8
{
    Default,   // 默认：优先本地，必要时升级到在线
    Online,    // 强制在线上下文
    Local,     // 强制本地上下文
};
```

---

## 解析策略

```
选择 OnlineContext
        │
        ▼
    Default? ──► 尝试本地 → 需要在线能力? ──► 升级到 Online
        │                    │
        │                    └── 不需要 ──► 保持 Local
        │
    Online? ──► 强制在线上下文（需要登录 + 在线权限）
        │
    Local? ──► 强制本地上下文（仅离线权限）
```

---

## 使用场景

| 上下文 | 典型场景 |
|--------|---------|
| `Default` | 主菜单 — 先用本地数据，联网功能需要时在线 |
| `Online` | 多人对战 — 必须在线，需要 `CanPlayOnline` 权限 |
| `Local` | 离线模式 — 不需要网络连接 |

### 示例：AsyncAction 的两种模式

```cpp
// InitializeForLocalPlay — 使用 Default 上下文
Action->Params.RequestedPrivilege = ECommonUserPrivilege::CanPlay;        // 离线权限
Action->Params.OnlineContext = ECommonUserOnlineContext::Default;         // 默认（本地优先）

// LoginForOnlinePlay — 使用 Online 上下文
Action->Params.RequestedPrivilege = ECommonUserPrivilege::CanPlayOnline;  // 在线权限
Action->Params.OnlineContext = ECommonUserOnlineContext::Online;          // 强制在线
```

---

## 运行时切换

`UCommonUserSubsystem` 提供运行时切换接口:

```cpp
// 切换用户的权限检查上下文
bool TryToChangePrivilegeCheckContext(
    const UCommonUserInfo* UserInfo, 
    ECommonUserOnlineContext OnlineContext
);
```

调用时机:
- 用户从主菜单进入在线模式
- 网络连接丢失时降级到本地模式
- 用户手动切换离线模式

---

## 与 PlatformTraits 联动

`OnlineContext` 与 `PlatformTraits` GameplayTag 系统协同工作:

| Platform Trait | 默认上下文行为 |
|---------------|---------------|
| `Platform.Trait.RequiresStrictControllerMapping` | Console 平台，强制在线 |
| `Platform.Trait.Input.SupportMouseAndKeyboard` | PC 平台，本地优先 |

---

## 相关文件

- [[/modules/CommonUserSubsystem/overview|CommonUserSubsystem]]
- [[/modules/AsyncAction_CommonUserInitialize/overview|AsyncAction 初始化]]
- [[/architecture/data-flow|登录数据流]]
