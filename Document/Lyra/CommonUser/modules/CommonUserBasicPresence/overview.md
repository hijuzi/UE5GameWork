# CommonUserBasicPresence

> **NW: Modules > CommonUser > Presence**  
> *基于会话状态的在线 Presence 自动更新*

---

## 基本信息

| 属性 | 值 |
|------|-----|
| 基类 | `UGameInstanceSubsystem` |
| 头文件 | `CommonUserBasicPresence.h` |
| 依赖 | `UCommonSessionSubsystem` |

---

## 功能概述

`UCommonUserBasicPresence` 监听 `UCommonSessionSubsystem::OnSessionInformationChangedEvent` 事件，自动将用户的在线 Presence 同步到平台后端。

### Presence 映射

| 会话状态 | Presence Key | 说明 |
|---------|-------------|------|
| `OutOfGame` | `"MainMenu"` | 不在游戏中，主菜单 |
| `Matchmaking` | `"Matchmaking"` | 正在匹配 |
| `InGame` | `"InGame"` | 游戏中 |

### 额外属性

| 属性 Key | 内容 |
|----------|------|
| `"GameMode"` | 当前游戏模式名称 |
| `"MapName"` | 当前地图名称（截取路径末尾） |

---

## 配置

```cpp
// 是否启用基于会话的 Presence
bool bEnableSessionsBasedPresence = true;

// Presence 属性键名
FString PresenceStatusMainMenu = TEXT("MainMenu");
FString PresenceStatusMatchmaking = TEXT("Matchmaking");
FString PresenceStatusInGame = TEXT("InGame");
FString PresenceKeyGameMode = TEXT("GameMode");
FString PresenceKeyMapName = TEXT("MapName");
```

---

## 工作原理

```
Session State Changes
        │
        ▼
OnSessionInformationChanged(State, GameMode, MapName)
        │
        ├── bEnableSessionsBasedPresence? ─── No ──► 跳过
        │
        ▼
   Dedicated Server? ─── Yes ──► 跳过（服务器不需要 Presence）
        │
        ▼
   SessionStateToBackendKey(State)
        │
        ▼
        ├── OSSv1: IOnlinePresence::SetPresence()
        └── OSSv2: IPresence::PartialUpdatePresence()
```

---

## 双 OSS 实现差异

| 方面 | OSSv1 (`IOnlinePresence`) | OSSv2 (`IPresence`) |
|------|--------------------------|---------------------|
| 接口 | `SetPresence(UniqueNetId, Status)` | `PartialUpdatePresence(Params)` |
| 用户标识 | `FUniqueNetId` 指针 | `FAccountId` (v2) |
| 全量/增量 | 全量设置 | 增量更新（只改变化字段） |
| 属性设置 | `Status.Properties.Emplace()` | `Mutations.UpdatedProperties.AddVariant()` |

---

## 示例

### 触发流程

1. 游戏代码调用 `UCommonSessionSubsystem::HostSession()` 或 `JoinSession()`
2. 会话创建/加入完成后，Subsystem 广播 `OnSessionInformationChangedEvent`
3. `UCommonUserBasicPresence` 收到事件，调用平台 API 更新 Presence

### 自定义 Presence 状态

```cpp
// 在子类或配置中修改 Presence Key
MyPresence->PresenceStatusInGame = TEXT("InBattle");
MyPresence->PresenceKeyGameMode = TEXT("Mode");
MyPresence->bEnableSessionsBasedPresence = true;
```

---

## 注意事项

- **DedicatedServer** 不会更新 Presence（跳过所有本地玩家）
- **LAN 模式**下可能不会触发 Presence 更新（取决于 OSS 实现）
- Presense 更新是 **fire-and-forget**，无回调确认

---

## 相关文件

- [CommonSessionSubsystem](../CommonSessionSubsystem/overview.md)
- [Presence 更新数据流](../../architecture/data-flow.md)
- [OSS 版本适配](../../cross-cutting/oss-versioning.md)
