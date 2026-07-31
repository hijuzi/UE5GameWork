# 登录状态机

> **NW: Patterns > CommonUser > Login State Machine**

---

## 模式概述

`FUserLoginRequest` 实现了一个**顺序状态机（Sequential State Machine）**，将用户登录过程分解为 6 个有序步骤，每一步完成后推进到下一步。这是一种 **Pipeline** 模式的变体，用于处理具有可选中间步骤的线性流程。

---

## 动机

用户登录涉及多个异步子操作（平台认证、账号登录、权限检查等），且不同平台、不同上下文下某些步骤可跳过。使用状态机而非嵌套回调，可获得:
- 清晰的流程可读性
- 统一的重试和错误处理
- 平台差异化时的条件跳过

---

## 状态定义

```cpp
enum class ECommonUserLoginFlowStep : uint8
{
    TransferPlatformAuth,  // [0] 传递平台认证
    AutoLogin,             // [1] 自动登录
    LoginUI,               // [2] 显示登录UI（可选）
    QueryPrivileges,       // [3] 查询权限
    CreateLocalPlayer,     // [4] 创建本地玩家（可选）
    Complete,              // [5] 完成清理
};
```

---

## 执行策略

```
┌─────────────────────────────────────────────────────┐
│               StartLogin(Params)                     │
│                      │                               │
│                      ▼                               │
│         ┌──────────────────────┐                     │
│         │ TransferPlatformAuth │ ← Console Only      │
│         └────────┬─────────────┘                     │
│                  │                                    │
│                  ▼                                    │
│         ┌──────────────────────┐                     │
│         │     AutoLogin         │                     │
│         └────────┬─────────────┘                     │
│                  │                                    │
│          ┌───────┴────────┐                           │
│          │  Success?      │                           │
│          ▼                ▼                           │
│    ┌──────────┐    ┌──────────┐                       │
│    │  Next    │    │ LoginUI  │ ← 如果 bCanShowLoginUI│
│    └────┬─────┘    └────┬─────┘                       │
│         │               │                             │
│         ▼               ▼                             │
│         ┌──────────────────────┐                     │
│         │  QueryPrivileges     │                     │
│         └────────┬─────────────┘                     │
│                  │                                    │
│                  ▼                                    │
│         ┌──────────────────────┐                     │
│         │ CreateLocalPlayer    │ ← 可选               │
│         └────────┬─────────────┘                     │
│                  │                                    │
│                  ▼                                    │
│         ┌──────────────────────┐                     │
│         │      Complete        │                     │
│         └──────────────────────┘                     │
└─────────────────────────────────────────────────────┘
```

---

## 关键实现细节

### 步骤推进

每一步完成后，通过 `StepComplete()` 推进到下一步:

```cpp
void FUserLoginRequest::StepComplete(bool bSuccess)
{
    CurrentStep = static_cast<ECommonUserLoginFlowStep>(
        static_cast<uint8>(CurrentStep) + 1
    );
    
    if (CurrentStep < ECommonUserLoginFlowStep::Complete)
    {
        ExecuteStep(CurrentStep);
    }
    else
    {
        CompleteLogin(bSuccess);
    }
}
```

### 跳过策略

| 步骤 | 跳过条件 |
|------|---------|
| `TransferPlatformAuth` | 非 Console 平台 |
| `LoginUI` | `bCanShowLoginUI == false` 或 AutoLogin 成功 |
| `CreateLocalPlayer` | `bCanCreateNewLocalPlayer == false` |

### 错误处理

任何步骤失败时:
1. 记录错误信息
2. 如果开启了 `bCanShowLoginUI` 且当前步骤为 `AutoLogin`，尝试 `LoginUI` 作为回退
3. 否则直接跳到 `Complete`，广播失败结果

---

## 同时进行中的请求

系统允许多个并发初始化请求，通过 `FPendingLocalUserOperation` 管理:
- 如果目标 LocalPlayer 正在被另一个请求处理，新请求排队等待
- 使用 `FPendingUserOperationHandle` 追踪待处理操作

---

## 变体与相关模式

- **Command Queue 模式** — `FPendingLocalUserOperation` 队列
- **Strategy 模式** — OSSv1/v2 分支选择
- **Facade 模式** — `TryToInitializeUser` 屏蔽内部复杂性

---

## 相关文件

- [[/architecture/data-flow|登录流程数据流]]
- [[/modules/CommonUserSubsystem/overview|CommonUserSubsystem]]
- [[/patterns/dual-oss-abstraction|双 OSS 抽象]]
