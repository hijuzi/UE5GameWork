---
module: infrastructure-code-review
---

# Code Review — Infrastructure

## 审查范围

4 个插件，57 个文件，5,626 行代码。自动审查（Phase 1 分析阶段），非人工逐行审查。

## AsyncMixin

### ✅ 优点
- 零内存成本的巧妙静态存储设计
- 通过 `TSharedRef` + `Loading.Remove(this)` 正确管理生命周期
- 全面的 API 覆盖（软引用、条件、事件三种异步模式）
- `AutoStart` 的自动恢复机制防止开发者忘记调用 `StartAsyncLoading()`

### ⚠️ 建议
- 文件数量和 LOC 较低，但由于涉及异步生命周期，仍评 Medium
- `FAsyncStep` 的模板逻辑较复杂，新贡献者需熟悉

## ModularGameplayActors

### ✅ 优点
- 设计中做到最少 — 不过度功能膨胀
- 统一且可预测的行为（所有类使用相同模式）
- 保留 `Base` 和非 `Base` 版本以支持最大灵活性

### ⚠️ 建议
- 显著的样板代码 — 所有类本质相同。可考虑用 C++ 模板（CRTP）或代码生成减少重复
- `ModularPawn` 和 `ModularCharacter` 都实现完全相同逻辑；`ModularCharacter` 继承自 `ACharacter`（本身继承自 `APawn`），但未利用此继承关系

## CommonGame

### ✅ 优点
- 清晰的关注点分离 — UI、玩家、消息、异步操作
- 精细且可控的 UI 系统设计（Subsystem → Policy → Layout 分层）
- 平台无关 — CommonInput 抽象处理输入差异

### ⚠️ 建议
- `UGameUIPolicy` 是一个可扩展点，但在 Lyra 中只有一个实现（`LyraUIPolicy`）。这是一个设计良好的接口，但目前未被充分测试。如果添加第二个策略实现，可能会暴露 API 缺陷。
- 异步 Action 类没有超时机制 — 如果资产加载失败/挂起，调用方永远不会收到通知
- `NotifyPlayerAdded/Removed/Destroyed` 的三个回调签名几乎相同 — 可能统一

## UIExtension

### ✅ 优点
- 优雅的 GameplayTag 路由实现
- 双向通知引擎处理扩展与扩展点到达顺序的任意组合
- GC 友好的 `AddReferencedObjects` 集成
- 对 C++ 和 Blueprint 都开放的扩展设计

### ⚠️ 建议
- 双向通知可能因同一帧内的相互通知而导致意外行为
- `PartialMatch` 遍历父标签链的时间复杂度依赖标签树深度
- 扩展和扩展点句柄的生命周期管理需要理解两个系统（Subsystem + Widget）

---

## 总体评价

代码整体遵循 Epic 风格指南，设计合理，无重大缺陷。主要建议是减少 ModularGameplayActors 中的重复样板，并为 AsyncAction 类添加超时处理。
