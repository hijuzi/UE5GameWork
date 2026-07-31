# CommonUser Plugin

> **NW: Plugin > UE5 > Online Subsystem**  
> *Lyra Starter Game 用户管理与在线会话子系统插件*

---

## 概述

CommonUser 是 Lyra 框架中的核心插件，负责管理本地用户身份、在线登录流程、多平台会话创建/搜索/加入、以及基础在线状态（Presence）。它抽象了 OnlineSubsystem v1 和 OnlineServices v2 两套 OSS API，提供统一的跨平台用户管理接口。

| 属性 | 值 |
|------|-----|
| 模块名 | `CommonUser` |
| 类型 | Runtime Plugin |
| 加载阶段 | `Default` |
| 依赖 | OnlineSubsystem, OnlineServices, GameplayTags, AssetManager |
| 平台 | Win64 + Consoles |

---

## 快速导航

### 架构
- [架构概览](architecture/overview.md) — 系统整体设计、模块划分与依赖关系
- [数据流图](architecture/data-flow.md) — 用户登录与会话创建的完整数据流

### 模块
- [CommonUserSubsystem](modules/CommonUserSubsystem/overview.md) — 核心用户管理（登录、权限、本地玩家）
- [CommonUserSubsystem API](modules/CommonUserSubsystem/api.md) — 完整接口参考
- [CommonSessionSubsystem](modules/CommonSessionSubsystem/overview.md) — 在线会话创建/搜索/加入
- [CommonSessionSubsystem API](modules/CommonSessionSubsystem/api.md) — 完整接口参考
- [CommonUserBasicPresence](modules/CommonUserBasicPresence/overview.md) — 在线状态自动更新
- [AsyncAction_CommonUserInitialize](modules/AsyncAction_CommonUserInitialize/overview.md) — Blueprint 异步初始化节点

### 设计模式
- [登录状态机](patterns/login-state-machine.md) — `FUserLoginRequest` 多步登录流程
- [双 OSS 抽象](patterns/dual-oss-abstraction.md) — OSSv1/OSSv2 共存的抽象策略
- [OnlineContext 解析](patterns/context-resolution.md) — 本地/在线上下文的优先级解析

### 横切关注点
- [OSS 版本适配](cross-cutting/oss-versioning.md) — 编译期与运行时的版本适配
- [平台抽象](cross-cutting/platform-abstraction.md) — PlatformUserId / InputDeviceId 抽象层

### 健康与问题
- [代码健康报告](health/report.md) — 质量评估、已知问题与改进建议

### 入门
- [快速入门](onboarding/quickstart.md) — 5 分钟了解如何使用 CommonUser

---

## 创建时间
2026-07-31
