---
module: core-health-summary
---

# Health Summary — Core

## 总体评分: 🟡 需要关注

| 指标 | 值 | 评级 |
|------|-----|------|
| 总代码行 | 11,041 | 🟡 规模较大 |
| 平均复杂度 | 2 High, 2 Medium, 1 Low | 🟡 中高 |
| 循环依赖 | 0 | 🟢 无 |
| 已知技术债务 | AbilitySystem 高复杂度 | 🟡 注意 |

## 各模块健康度

| 模块 | LOC | 复杂度 | 风险 |
|------|-----|--------|------|
| **AbilitySystem** | 5,400 | High | 🔴 最大模块，多层抽象（ASC→Ability→Attribute→Execution），新人学习曲线陡峭 |
| **Character** | 2,945 | High | 🟡 InitState 状态机在 5+ 组件间协调，调试困难 |
| **Camera** | 1,690 | Medium | 🟢 设计清晰，Stack 模式易于扩展 |
| **Input** | 893 | Medium | 🟢 薄封装，依赖 EnhancedInput 成熟度 |
| **Animation** | 113 | Low | 🟢 极简桥接，几乎无风险 |

## 风险评估

| 风险 | 严重度 | 可能性 | 缓解 |
|------|--------|--------|------|
| AbilitySystem 过度复杂 | Medium | High | 充分文档化，使用 AbilitySet 数据资产简化配置 |
| InitState 状态机死锁 | Medium | Low | InitState 框架已成熟，统一模式降低风险 |
| CameraMode 混合边界情况 | Low | Low | CameraModeStack 有完善的 BlendStack 逻辑 |
