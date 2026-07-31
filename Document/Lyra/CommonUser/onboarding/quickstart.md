# 快速入门

> **NW: Onboarding > CommonUser > Quick Start**

---

## 5 分钟了解 CommonUser

### 这是什么？

CommonUser 是 Lyra 框架的**用户管理层**插件。它处理从"玩家按下手柄上的按钮"到"玩家已登录并能联机"之间的一切。

---

## 核心概念（3 个）

| 概念 | 一句话 |
|------|--------|
| **UCommonUserSubsystem** | 管理用户登录、权限、本地玩家创建 |
| **UCommonSessionSubsystem** | 管理在线游戏房间（创建/搜索/加入） |
| **UCommonUserBasicPresence** | 自动同步"在线"、"游戏中"状态给好友 |

---

## 最简使用（Blueprint）

### 场景1: 本地游玩（离线/主菜单）

```
Event BeginPlay
  → Get Game Instance
    → Get Subsystem (CommonUserSubsystem)
      → Initialize For Local Play  // Async Action
        - Local Player Index: 0
        - Can Use Guest Login: true
        → On Initialization Complete
          → Branch (Success?)
            → True: 继续加载主菜单
            → False: 显示错误
```

### 场景2: 在线游玩（多人联机）

```
用户点击 "Play Online"
  → Get Game Instance
    → Get Subsystem (CommonUserSubsystem)
      → Login For Online Play  // Async Action
        → On Initialization Complete (Success)
          → Get Subsystem (CommonSessionSubsystem)
            → Host Session / Quick Play Session
```

---

## 文件位置速查

```
Plugins/CommonUser/
├── CommonUser.uplugin              # 插件描述
└── Source/CommonUser/
    ├── CommonUser.Build.cs         # 构建配置
    ├── Public/
    │   ├── CommonUserSubsystem.h   # 核心用户管理
    │   ├── CommonSessionSubsystem.h # 会话管理
    │   ├── CommonUserBasicPresence.h # 在线状态
    │   ├── AsyncAction_CommonUserInitialize.h # BP 异步节点
    │   ├── CommonUserTypes.h        # 公共类型/枚举
    │   └── CommonUserModule.h       # 模块入口
    └── Private/                     # 实现文件（同上命名）
```

---

## 常见任务

### "我想添加新的权限检查类型"

1. 在 `ECommonUserPrivilege` 枚举中添加新值
2. 在 `UCommonUserSubsystem::CanCheckPrivilege()` 中实现检查逻辑
3. 确保对应的 OSS 平台支持此权限

### "我想在会话创建时自定义参数"

修改 `UCommonSession_HostSessionRequest`:
- 调整 `MaxPlayerCount`
- 设置 `bUseLobby` / `bUsePresence`
- 修改 `MapID` 指向自定义地图

### "我想集成第三方登录"

1. 在 `FUserLoginRequest` 状态机中添加新步骤
2. 实现对应的 OSS 接口适配（OSSv1 和 OSSv2）
3. 通过 `OnUserInitializeComplete` 回调返回结果

---

## 调试技巧

### 启用详细日志

```cpp
// 在 DefaultEngine.ini 中添加
[Core.Log]
LogCommonUser=Verbose
LogCommonSession=Verbose
LogUserBasicPresence=Verbose
```

### 常见错误码

| 错误 | 可能原因 |
|------|---------|
| "Unable to start login" | Subsystem 无效或输入参数错误 |
| "Platform Auth Failed" | 平台认证失败（检查网络/账号状态） |
| "Privilege denied: CanPlayOnline" | 家长控制或平台限制 |
| Session full | 目标会话已满 |

---

## 下一步

- [[/architecture/overview|架构概览]] — 深入了解系统设计
- [[/modules/CommonUserSubsystem/api|API 参考]] — 完整接口文档
- [[/patterns/login-state-machine|登录状态机]] — 理解登录流程细节
