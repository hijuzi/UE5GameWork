# AsyncAction_CommonUserInitialize

> **NW: Modules > CommonUser > Async Action**  
> *蓝图可调用的用户初始化异步节点*

---

## 基本信息

| 属性 | 值 |
|------|-----|
| 基类 | `UBlueprintAsyncActionBase` |
| 头文件 | `AsyncAction_CommonUserInitialize.h` |
| 依赖 | `UCommonUserSubsystem` |

---

## 功能概述

`UAsyncAction_CommonUserInitialize` 将 `UCommonUserSubsystem::TryToInitializeUser()` 包装为蓝图可用的异步动作节点。它提供两个静态工厂方法，对应两种使用场景:

| 工厂方法 | 用途 | 创建 LocalPlayer? | 默认权限 |
|---------|------|-------------------|---------|
| `InitializeForLocalPlay` | 本地/离线游玩 | 是 | `CanPlay` |
| `LoginForOnlinePlay` | 在线游玩 | 否 | `CanPlayOnline` |

---

## 蓝图节点

在蓝图中，右键搜索 "CommonUser" 可找到:

- **Initialize For Local Play** — 输入: `Target`, `Local Player Index`, `Primary Input Device`, `Can Use Guest Login`
- **Login For Online Play** — 输入: `Target`, `Local Player Index`

两个节点都输出:
- **On Initialization Complete** — 初始化完成（含成功/失败信息）

---

## 工作流程

```
Static Factory Method (Blueprint entry point)
        │
        ├── 创建 UAsyncAction_CommonUserInitialize 对象
        ├── RegisterWithGameInstance(Target)
        ├── 填充 FCommonUserInitializeParams
        │
        ▼
Activate() 被引擎调用
        │
        ├── 绑定 HandleInitializationComplete 回调
        ├── Subsystem->TryToInitializeUser(Params)
        │
        ├── 成功 → 等待 Subsystem 回调
        ├── 失败 → SetTimerForNextTick(HandleFailure)
        │
        ▼
HandleInitializationComplete(UserInfo, bSuccess, Error, Privilege, Context)
        │
        ├── OnInitializationComplete.Broadcast(...)
        └── SetReadyToDestroy()
```

---

## 回调 Pin 信息

| Pin | 类型 | 说明 |
|-----|------|------|
| `User Info` | `const UCommonUserInfo*` | 初始化的用户信息对象 |
| `Success` | `bool` | 是否成功 |
| `Error` | `FText` | 错误描述文本 |
| `Requested Privilege` | `ECommonUserPrivilege` | 请求的权限级别 |
| `Online Context` | `ECommonUserOnlineContext` | 在线上下文 |

---

## 示例

### Blueprint — 本地游玩初始化

1. 拖入 `Initialize For Local Play` 节点
2. `Target` → `Get Game Instance` → `Get Subsystem (CommonUserSubsystem)`
3. `Local Player Index` → `0`
4. `Primary Input Device` → `Get Default Input Device`
5. `Can Use Guest Login` → `true`
6. 从 `OnInitializationComplete` 拉出 → 检查 `Success`

### C++ 等效代码

```cpp
// 本地游玩初始化
UAsyncAction_CommonUserInitialize::InitializeForLocalPlay(
    UserSubsystem,
    0, // PlayerIndex
    IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(), // Default device
    true // Allow guest login
);

// 在线游玩登录
UAsyncAction_CommonUserInitialize::LoginForOnlinePlay(
    UserSubsystem,
    0 // PlayerIndex
);
```

---

## 注意事项

- 该对象注册到 GameInstance，生命周期由引擎管理
- 完成后调用 `SetReadyToDestroy()` 自动销毁
- 失败回调延迟到下一帧执行（避免重入问题）
- 如果注册前 `Target` 无效，对象会立即销毁

---

## 相关文件

- [[/modules/CommonUserSubsystem/overview|CommonUserSubsystem]]
- [[/architecture/data-flow|登录数据流]]
- [[/patterns/login-state-machine|登录状态机]]
