# AI 自审查 — 10 | Prediction 预测与回滚

> 审查日期：2026-08-24
> 审查版本：新建版（套用 05-09 统一格式）
> 评分标准：✅ 通过 / ⚠️ 有改进空间 / ❌ 需修正

---

## 一、源码准确性审查

### 1.1 FPredictionKey 字段 ✅

- `Current` / `Base` 为 `int16`，`typedef int16 KeyType`（`GameplayPrediction.h:300-309`）✅
- `Base` 带 `NotReplicated`（`GameplayPrediction.h:308`）✅
- `bIsServerInitiated` 存在，**无 `bIsStale` 字段**（已明确纠正）✅
- `PredictiveConnectionObjectKey`（`FObjectKey`）用于 `WasReceived()`/`WasLocallyGenerated()`（`GameplayPrediction.h:359-368, 412-413`）✅

### 1.2 关键方法 ✅

`IsValidKey` / `IsLocalClientKey` / `IsServerInitiatedKey` / `IsValidForMorePrediction` / `WasReceived` / `WasLocallyGenerated` 的实现逐字与 `GameplayPrediction.h:336-368` 一致。✅

### 1.3 生成逻辑 ✅

- `CreateNewPredictionKey` 在 `ROLE_Authority` 上返回无效键（`GameplayPrediction.cpp:223-233`）✅
- `GenerateNewPredictionKey` 用 `static KeyType GKey = 1` 自增 + int16 溢出回绕（`GameplayPrediction.cpp:189-197`）✅
- `CreateNewServerInitiatedKey` 用独立 `GServerKey` 计数器，注释"确保与客户端不同步"（`GameplayPrediction.cpp:235-252`）✅

### 1.4 委托机制 ✅

`NewRejectedDelegate` / `NewCaughtUpDelegate` / `NewRejectOrCaughtUpDelegate` 三方法（`GameplayPrediction.h:324-331`）与 `FPredictionKeyDelegates` 的 `TMap<KeyType, FDelegates>` 结构（`GameplayPrediction.h:440-451`）✅。`Rejected`（明确拒绝）vs `CaughtUp`（状态追上，不暗示接受）的语义区分与注释一致。✅

### 1.5 依赖链 ✅

`GenerateDependentPredictionKey`：`Base` 首次设为当前键、深度检测 `Current - Base < 20`、`AddDependency`（`GameplayPrediction.cpp:199-221`）✅。`AddDependency` 中"BaseKey 被 Reject 则 ThisKey 也 Reject"的逻辑与 `GameplayPrediction.cpp:359-360` 一致。✅

### 1.6 NetSerialize ✅

"预测键只回传给发起客户端"的核心逻辑（`GameplayPrediction.cpp:122-129`）已准确转述，`PredictiveConnectionObjectKey == FObjectKey(Map)` 的比较与源码一致。✅

### 1.7 复制与 CatchUp ✅

- `FReplicatedPredictionKeyMap` 用 FastArray 逐键确认，防丢包跳跃的理由（头文件注释第 552-564 行）✅
- `FReplicatedPredictionKeyItem::OnRep`：跳过 `bIsServerInitiated`、`CatchUpTo(PredictionKey.Current)`、陈旧 Key 清理（`GameplayPrediction.cpp:594-678`）✅
- 三个 CVar（`StaleKeyBehavior` 0/1/2、`DepChainBehavior`、`MaxStaleKeysBeforeAck`）✅

### 1.8 ASC 侧实现 ✅

- `InternalTryActivateAbility` 预测分支：`FScopedPredictionWindow(this, true)` → `SetPredicting` → `CallServerTryActivateAbility`（立即）→ `NewCaughtUpDelegate`（`AbilitySystemComponent_Abilities.cpp:1927-1945`）✅
- `ServerTryActivateAbility_Implementation`（2016）✅
- `ClientActivateAbilityFailed_Implementation` → `BroadcastRejectedDelegate`（2279-2285）✅

### 1.9 GE 回滚用例 ✅

`GameplayEffect.cpp:4523-4528` 的 `NewCaughtUpDelegate`（OnCaughtUpActiveGameplayEffect）+ `NewRejectedDelegate`（OnRejectedActiveGameplayEffect）双钩子，与源码一致。✅

### 1.10 可进一步核实点 ⚠️

§八 `FScopedPredictionWindow` 的 WaitInputRelease 流程（头文件注释第 202-211 行）转述准确，但未展开 `FScopedPredictionWindow` 析构函数（`GameplayPrediction.cpp` 第 383 行起"Server version constructor"之后）的具体清理逻辑。当前处理合理，属篇幅取舍。

---

## 二、逻辑清晰度审查

### 2.1 问题驱动切题 ⭐ 优秀

以"按下技能键瞬间技能立刻反应，但网络延迟是物理矛盾"引出预测的必要性，再从"预测带来五个难题"进入正题，锚点清晰。

### 2.2 六大难题框架贯穿全文 ✅

§一 的表格（Can I do this / Undo / Redo / Completeness / Dependencies / Override）成为全文的"骨架"，§四、§五、§七 分别对应 Undo/Redo、Completeness/Dependencies、Override，前后呼应。

### 2.3 FPredictionKey 生命周期主线清晰 ✅

§二（结构）→ §三（技能激活全流程）→ §四（回滚）→ §五（依赖链）→ §六（服务器追上）→ §七（网络复制）→ §八（额外窗口）的递进符合"是什么 → 怎么跑 → 怎么纠错 → 怎么扩展"的认知顺序。

### 2.4 可优化点 ⚠️

§三 的 6 步流程用 ASCII 图表达，但未配 drawio 时序图。预测流程是典型的多角色交互时序，配一张时序图会显著提升可读性（见 §六 图表一致性）。

---

## 三、深度审查

### 3.1 设计思考有高度 ⭐ 优秀

§九 提出"三个诚实"框架（诚实面对未解决部分 / 预测本质是 delta / 预测窗口的短暂性），直接引用了源码注释里坦诚的局限性（Triggered events 回滚不完整、Meta 属性不可预测、% GE 偏差的 `500→550→605` 例子），体现了"从第一性原理出发、不假装完美"的深度。

### 3.2 关键纠正有实操价值 ✅

"`FPredictionKey` 无 `bIsStale` 字段"这一纠正，配合 §六 里"陈旧状态通过扫描+超时阈值动态判定"的解释，形成了一个完整、可验证的论证闭环，不是单纯纠错。

### 3.3 可进一步深挖 ⚠️

- §七 NetSerialize 的"为什么只回传本人"解释充分，但未展开 `UE_WITH_REMOTE_OBJECT_HANDLE` 分支（`GameplayPrediction.cpp:131-146` 的代理连接特例）；
- §五 依赖链的 `CVarDependentChainBehaviorValue`（bitmask 0x1/0x2）行为差异未展开。

---

## 四、实践价值审查

### 4.1 六大难题对照表 ✅

§一 表格直接可作为"理解预测系统"的速查卡，六个难题 → 对应机制的一一映射清晰。

### 4.2 官方绕法建议 ✅

§五.3 和 §九.1 都给出了"用 GameplayTag 做激活条件"这一官方推荐的绕法，对实际开发有直接指导意义。

### 4.3 元数据字数 ⚠️

文首标注 `~6800`，按正文中文内容估算（不含代码块与表格）实际约 6200-6700 字，基本吻合，可保留。

---

## 五、AI 味审查

### 5.1 Em Dash（—）使用统计

全文中文破折号（—）约 18-20 次，集中在标题与元信息，正文密度适中。

### 5.2 结构性句式检查

| 检查项 | 结果 |
|--------|------|
| "首先...其次...再次..." 机械排比 | 未发现 |
| "不仅...而且..." | 未发现 |
| "值得注意的是 / 需要强调的是" | 未发现 |
| "综上所述 / 总而言之" | 未发现 |
| 设问句滥用 | §一、§八开头各 1 处，合理 |

### 5.3 结论

AI 味较低，技术密度高，行文以"源码 → 表格 → 设计解读"节奏推进。

---

## 六、图表一致性审查

本文暂未配 drawio 图。**建议补充两张图**（优先级高）：

1. **预测生命周期时序图**（§三）：客户端 ASC / 服务器 ASC / 属性复制 三者的 `TryActivate → ServerTryActivate → Succeed/Failed → ReplicatedPredictionKey 追上` 时序；
2. **依赖链类图/流程图**（§五）：`X → Y → Z` 的 Base Key 链 + `AddDependency` 回滚传播。

---

## 七、总体评价

| 维度 | 评分 | 说明 |
|------|------|------|
| 源码准确性 | ✅ 优秀 | 全部关键字段/方法/行号均已对照 UE 5.8 源码核验，无臆造 API |
| 逻辑清晰度 | ✅ 优秀 | 六大难题框架贯穿，FPredictionKey 生命周期主线清晰 |
| 深度 | ✅ 优秀 | 三个诚实 + 源码注释局限性的引用，有设计高度 |
| 实践价值 | ✅ 良好 | 难题对照表 + 官方绕法建议，可直接指导开发 |
| AI 味 | ✅ 良好 | 整体自然 |

### 待修复项汇总

| # | 严重程度 | 位置 | 问题 | 建议 |
|---|---------|------|------|------|
| 1 | ⚠️ 中 | 全文 | 暂无 drawio 图 | 补时序图（预测生命周期）+ 依赖链图 |
| 2 | 🔧 优化 | §七 | 未展开 `UE_WITH_REMOTE_OBJECT_HANDLE` 代理特例 | 可补一句说明 |
| 3 | 🔧 优化 | §五 | 未展开 `CVarDependentChainBehaviorValue` 行为差异 | 可补一句说明 |

---

*审查完成。本文新建质量高，源码引用准确，无臆造 API。建议优先补充两张图（#1），其余为优化项。*
