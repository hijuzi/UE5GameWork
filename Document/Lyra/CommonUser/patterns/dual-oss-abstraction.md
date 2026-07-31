# 双 OSS 抽象

> **NW: Patterns > CommonUser > Dual OSS Abstraction**

---

## 模式概述

CommonUser 插件在编译期通过 `COMMONUSER_OSSV1` 预处理器宏在两套在线子系统 API 间做出选择，这是 **Compile-Time Strategy** 模式的典型应用。

---

## 动机

Epic 在 UE5 中引入了新一代在线服务框架 `OnlineServices`（OSSv2），但需要同时保持对 `OnlineSubsystem`（OSSv1）的兼容性。两个 API 的接口、数据结构和错误处理完全不同:

| 方面 | OSSv1 | OSSv2 |
|------|-------|-------|
| 子系统访问 | `Online::GetSubsystem(World)` | `UE::Online::GetServices(World)` |
| 身份接口 | `IOnlineIdentityPtr` | `IAuthPtr` |
| 用户 ID | `FUniqueNetId` (指针) | `FAccountId` (值类型) |
| 会话接口 | `IOnlineSessionPtr` | `ISessionsPtr` |
| 错误处理 | `FOnlineError` (bool+string) | `FOnlineError` (枚举+结构化) |

---

## 实现方式

### 编译期选择

```cpp
// 在 Build.cs 或预编译头中定义
#define COMMONUSER_OSSV1  // 或 OSSv2
```

### 代码分支

```cpp
// CommonUserSubsystem 中的典型模式
#if COMMONUSER_OSSV1
    IOnlineSubsystem* OSS = Online::GetSubsystem(GetWorld());
    IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
    // ... OSSv1 code path
#else
    UE::Online::IOnlineServicesPtr Services = UE::Online::GetServices(GetWorld());
    UE::Online::IAuthPtr Auth = Services->GetAuthInterface();
    // ... OSSv2 code path
#endif
```

### 涉及文件

| 文件 | 分支数量 | 关键差异点 |
|------|---------|-----------|
| `CommonUserSubsystem.cpp` | ~6 处 | 用户身份、登录、权限 |
| `CommonSessionSubsystem.cpp` | ~8 处 | 会话创建、搜索、加入 |
| `CommonUserBasicPresence.cpp` | ~2 处 | Presence 设置 |
| `CommonUserTypes.cpp` | ~1 处 | 错误信息提取 |

---

## 优缺点

### 优点
- **零运行时开销** — 编译期决定，无虚函数调用或分支预测失败
- **二进制大小优化** — 未使用的 OSS 代码被完全剔除
- **API 明确** — 每个分支针对特定 API 优化

### 缺点
- **代码重复** — 大量 if/else 块，不利于维护
- **测试负担** — 每种配置需要独立构建和测试
- **添加新 OSS 版本困难** — 需要修改所有分支点

---

## 错误处理差异

```cpp
// CommonUserTypes.cpp: FOnlineResultInformation::FromOnlineError

#if COMMONUSER_OSSV1
    bWasSuccessful = InOnlineError.WasSuccessful();
    ErrorId = InOnlineError.GetErrorCode();       // string
    ErrorText = InOnlineError.GetErrorMessage();   // string
#else
    bWasSuccessful = InOnlineError != UE::Online::Errors::Success();
    ErrorId = InOnlineError.GetErrorId();          // enum-based
    ErrorText = InOnlineError.GetText();           // FText
#endif
```

---

## 演进方向

当 OSSv2 稳定后，OSSv1 分支可能被移除:
1. 保留 `COMMONUSER_OSSV1` 宏定义，方便回退
2. 逐步将功能迁移到 OSSv2
3. 最终版删除所有 `#if` 分支

---

## 相关文件

- [[/cross-cutting/oss-versioning|OSS 版本适配]]
- [[/architecture/overview|架构概览]]
- [[/health/report|代码健康报告 — OSS 分支复杂度]]
