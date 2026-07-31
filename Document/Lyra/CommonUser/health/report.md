# 代码健康报告

> **NW: Health > CommonUser > Report**

---

## 总体评分

| 维度 | 评分 | 说明 |
|------|------|------|
| 代码清晰度 | ⭐⭐⭐⭐ | Epic 命名规范，注释覆盖中等 |
| 模块化设计 | ⭐⭐⭐⭐⭐ | 子系统分层清晰，职责分明 |
| 可测试性 | ⭐⭐ | 无单元测试，依赖 OSS 难以隔离 |
| 跨平台质量 | ⭐⭐⭐⭐ | 双 OSS 支持，PlatformUserId 抽象合理 |
| 性能 | ⭐⭐⭐⭐ | 编译期分支，无虚函数开销 |
| 文档完整度 | ⭐⭐⭐ | 代码内注释良好，但缺少外部文档 |

**综合评分**: **3.7 / 5** — 良好

---

## 已知问题

### 1. 缺少单元测试 (中)

**影响**: 模块整体无自动化测试覆盖。登录状态机和错误恢复路径依赖手动测试。

**建议**: 为 `FUserLoginRequest` 状态机添加单元测试，Mock OSS 接口。

**文件**: 全模块 `Private/` 目录

---

### 2. OSS 双重实现使维护成本高 (低)

**影响**: 每次添加功能需要在 `#if COMMONUSER_OSSV1` 和 `#else` 两个代码路径中实现。

**建议**: 创建轻量级抽象层，将 OSS 特定代码封装到策略类中。

**文件**: `CommonUserSubsystem.cpp`, `CommonSessionSubsystem.cpp`

---

### 3. 硬编码字符串 (低)

**影响**: Presence 状态键（"MainMenu", "Matchmaking", "InGame"）硬编码，不利于本地化。

**建议**: 使用 `FName` 常量或 `GameplayTags` 替代。

**文件**: `CommonUserBasicPresence.h`

---

## 代码度量

```
总行数:         ~2100
.h 文件数:      6
.cpp 文件数:    6
类数:           8 (含内部类)
最复杂函数:      CommonSessionSubsystem.cpp::HostSession ~150行
最深层嵌套:      CommonSessionSubsystem.cpp (3-4层)
最长方法:       CommonSessionSubsystem.cpp::HostSession() ~150行
```

---

## 安全检查清单

| 检查项 | 状态 |
|--------|------|
| 空指针检查 | ✅ 所有子系统访问前均检查 `IsValid()` / `ensure()` |
| 异步安全 | ✅ 使用 `TWeakObjectPtr` 保护 `this` 引用 |
| 线程安全 | ⚠️ 未明确标注 GameThread 限定，但子系统隐含单线程 |
| 内存管理 | ✅ UObject 派生，GC 自动管理 |
| 输入验证 | ⚠️ `HostSession` / `JoinSession` 部分参数未充分验证 |

---

## 改进建议

### 短期（1-2周）
1. 添加 `HostSession` / `JoinSession` 的核心路径单元测试
2. 提取 Presence 键名到配置或 GameplayTag
3. 添加 `checkf()` 验证关键路径输入参数

### 中期（1个月）
1. 创建 OSS 抽象接口，减少 `#if` 分支
2. 为 CommonSessionSubsystem 添加状态机模型（避免散落的 `bIsMatchmaking` 标志）
3. 增加日志分类和日志详细度控制

### 长期
1. 将登录 UI 逻辑移到独立模块（当前耦合在 Subsystem 中）
2. 添加完整的自动化集成测试（需要 OSS 模拟服务）

---

## 相关文件

- [OSS 版本适配问题](../cross-cutting/oss-versioning.md)
- [架构概览](../architecture/overview.md)
