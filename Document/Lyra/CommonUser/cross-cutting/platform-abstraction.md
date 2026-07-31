# 平台抽象

> **NW: Cross-Cutting > CommonUser > Platform Abstraction**

---

## 概述

CommonUser 使用 UE5 的输入/用户抽象层来统一 PC 和 Console 平台的用户管理模型。通过 `FPlatformUserId` 和 `FInputDeviceId` 抽象，相同的 C++ 代码可以在所有平台上运行。

---

## 核心抽象类型

### FPlatformUserId

```cpp
// 平台的"用户"概念
// PC: 通常映射到操作系统用户（概念性）
// Console: 映射到手柄已登录的账号
// 本地多人: 每个分屏玩家有独立 PlatformUserId
```

### FInputDeviceId

```cpp
// 平台的"输入设备"概念
// PC: 键盘+鼠标 = 一个 ID，手柄 = 另一个 ID
// Console: 每个手柄有独立 ID
// 可通过 IPlatformInputDeviceMapper 查询设备和用户的映射关系
```

---

## 输入到用户映射

```
┌──────────────────────────────────────────┐
│         IPlatformInputDeviceMapper        │
│                                           │
│  Keyboard →  InputDeviceId_0              │
│  Gamepad1 →  InputDeviceId_1              │
│  Gamepad2 →  InputDeviceId_2              │
│                                           │
│  InputDeviceId_0 ──► FPlatformUserId_A    │
│  InputDeviceId_1 ──► FPlatformUserId_B    │
│  InputDeviceId_2 ──► FPlatformUserId_C    │
└──────────────────────────────────────────┘
```

### 常见 API

```cpp
// 获取默认输入设备
FInputDeviceId DefaultDevice = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();

// 获取平台用户对应的输入设备
FInputDeviceId Device = IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(PlatformUser);

// 获取所有已连接输入设备
TArray<FInputDeviceId> AllDevices = IPlatformInputDeviceMapper::Get().GetAllConnectedInputDevices();
```

---

## 在 CommonUser 中的使用

### 初始化时

```cpp
// AsyncAction_CommonUserInitialize.cpp
if (!PrimaryInputDevice.IsValid())
{
    // 未指定设备时使用默认设备
    PrimaryInputDevice = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
}
```

### 用户管理时

```cpp
// CommonUserSubsystem 可以使用两种方式查找用户

// 方式1: 按 LocalPlayer 索引
auto* UserInfo = Subsystem->GetUserInfoForLocalPlayerIndex(0);

// 方式2: 按 PlatformUser
auto* UserInfo = Subsystem->GetUserInfoForPlatformUser(PlatformUser);

// 方式3: 按 ControllerId（兼容旧版）
auto* UserInfo = Subsystem->GetUserInfoForControllerId(0);
```

---

## 平台差异

### PC (Win64/Mac/Linux)

```
特性:
- 单个 PlatformUser（通常）
- 键盘+鼠标 为主输入设备
- 支持多手柄分屏（少见）
- 访客登录通常允许

CommonUser 行为:
- TransferPlatformAuth 步骤跳过
- 支持 Guest Login
- bCanShowLoginUI = true（可显示登录界面）
```

### Console (PS5/Xbox/Switch)

```
特性:
- 每个手柄 = 一个 PlatformUser
- 无法使用键盘+鼠标（或受限）
- 每个用户需要一个账号登录
- 强制在线上下文（特定功能）

CommonUser 行为:
- TransferPlatformAuth 步骤执行（传递平台 Token）
- Guest Login 可能受限
- 严格的 InputDevice <-> User 绑定
```

---

## PlatformTraits GameplayTag 系统

常用标签影响 CommonUser 行为:

| Tag | 影响 |
|-----|------|
| `Platform.Trait.RequiresStrictControllerMapping` | 强制一对一手柄-用户映射 |
| `Platform.Trait.Input.SupportMouseAndKeyboard` | 允许键鼠输入，不用手柄也能操作 |
| `Platform.Trait.Online.Presence` | 平台支持在线 Presence（基础功能） |
| `Platform.Trait.Online.PresenceJoin` | 支持通过 Presence 加入好友游戏 |

---

## 添加新平台支持

1. 在目标平台上实现 `IPlatformInputDeviceMapper`
2. 确保 OSS（v1 或 v2）支持目标平台
3. 测试 CommonUser 的完整登录流程
4. 添加对应的 `PlatformTraits` GameplayTag

---

## 相关文件

- [OSS 版本适配](oss-versioning.md)
- [CommonUserSubsystem](../modules/CommonUserSubsystem/overview.md)
- [架构概览](../architecture/overview.md)
