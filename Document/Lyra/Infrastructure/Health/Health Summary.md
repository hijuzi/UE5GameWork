---
module: infrastructure-health-summary
---

# Health Summary — Infrastructure

## 总体评分: 🟢 健康

| 指标 | 值 | 评级 |
|------|-----|------|
| 总代码行 | 5,626 | 🟢 可控 |
| 平均模块复杂度 | 1 Low, 3 Medium | 🟢 可维护 |
| 循环依赖 | 0 | 🟢 无循环 |
| 文档覆盖率 | 100% (Phase 1 完整分析) | 🟢 完整 |
| 已知技术债务 | 0 项 | 🟢 清洁 |

## 各模块健康度

| 模块 | LOC | 复杂度 | 潜在问题 |
|------|-----|--------|---------|
| **AsyncMixin** | 976 | Medium | 静态映射中的生命周期竞争 — 需在析构函数中正确取消 |
| **ModularGameplayActors** | 572 | Low | 重复样板代码 — 每个类模式完全相同，可选代码生成 |
| **CommonGame** | 3,138 | Medium | 中等复杂度，模板方法模式增加调试难度 |
| **UIExtension** | 940 | Medium | 双向通知复杂性 — 新扩展/扩展点注册时的相互通知存在死锁风险 |

## 风险评估

| 风险 | 严重度 | 可能性 | 缓解措施 |
|------|--------|--------|---------|
| UIExtension 双向通知死锁 | Medium | Low | 通知前先注册条目，同一帧不做递归 |
| AsyncMixin GC 后悬垂引用 | Low | Very Low | `TSharedRef` 生命周期 + 析构函数 `Loading.Remove(this)` |
| CommonGame 模板方法链断裂 | Low | Low | Lyra 作为唯一消费者，继承链可控 |
