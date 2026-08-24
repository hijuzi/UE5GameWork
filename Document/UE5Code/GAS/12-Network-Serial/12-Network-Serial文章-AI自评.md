# AI 自审查 — 12 | Network & Serial 网络序列化

> 审查日期：2026-08-24
> 审查版本：新建版（套用 05-11 统一格式）
> 评分标准：✅ 通过 / ⚠️ 有改进空间 / ❌ 需修正

---

## 一、源码准确性审查

### 1.1 FastArraySerializer 文件位置 ✅

关键纠正：`FFastArraySerializer` 已从 `Engine/Classes/Engine/NetSerialization.h` 迁移到 `Net/Core/Classes/Net/Serialization/FastArraySerializer.h`。`NetSerialization.h:133-135` 明确写着 "Everything related to Fast TArray Replication has been moved to Net/Serialization/FastArraySerializer.h"。本文源码路径标注正确。✅

### 1.2 FFastArraySerializerItem 字段 ✅

三个 `NotReplicated` 字段 `ReplicationID`/`ReplicationKey`/`MostRecentArrayReplicationKey`（`FastArraySerializer.h:325-332`）✅。构造函数初始化 `ReplicationID(INDEX_NONE), ReplicationKey(INDEX_NONE), MostRecentArrayReplicationKey(INDEX_NONE)` ✅。

### 1.3 三个回调 ✅

`PreReplicatedRemove`/`PostReplicatedAdd`/`PostReplicatedChange` 均标注 "intentionally not virtual; invoked via templated code"（`FastArraySerializer.h:341-356`）✅。本文"编译期多态（CRTP 风格）而非虚函数"的解读准确。

### 1.4 MarkItemDirty / MarkArrayDirty ✅

- `MarkItemDirty`：`if (ReplicationID == INDEX_NONE) ReplicationID = ++IDCounter;` 然后 `ReplicationKey++; MarkArrayDirty();`（441-454）✅
- `MarkArrayDirty`：`ItemMap.Reset();` 注释 "This allows to clients to add predictive elements to arrays without affecting replication."（459）✅

### 1.5 ID 同步、索引不同步 ✅

"Note that the ReplicationID is replicated and in sync between client and server. The indices are not."（`FastArraySerializer.h:213`）逐字转述 ✅。

### 1.6 FActiveGameplayEffectsContainer ✅

`FActiveGameplayEffectsContainer : public FFastArraySerializer`（`GameplayEffect.h:1652`），`NetDeltaSerialize`（1804），`FActiveGameplayEffect` 三回调（`GameplayEffect.h:1396-1398`）✅。

### 1.7 量化向量 ✅

`FVector_NetQuantize`（20 bits/±1,048,576）、`NetQuantize10`（24 bits/±1,677,721.6）、`NetQuantize100`（30 bits/±10,737,418.24）、`NetQuantizeNormal`（16 bits/-1..+1）与 `NetSerialization.h:398-574` 逐项一致 ✅。Packed vs Fixed 两算法（137-170 注释）✅。

### 1.8 属性预测序列化 ✅

"预测 GE 当作无限时长"（`GameplayPrediction.h:114-129`）、`REPNOTIFY_Always` 必要性、base/delta 重新聚合，与第 10 篇及 `GameplayPrediction.h` 一致 ✅。

### 1.9 可进一步核实点 ⚠️

§四.2 "GE 用 FastArray 而非普通数组"的带宽收益论述是合理推断，未给出量化数据（如"带宽与变化元素数成正比"）。当前表述准确但非源码逐字引用，可接受。

---

## 二、逻辑清晰度审查

### 2.1 问题驱动切题 ⭐ 优秀

以"TArray 里改中间第 37 个元素怎么传"的具体场景切入，点出"全量传输 vs 按索引传输"两个朴素方案都不行，自然引出"索引 vs 身份"的核心矛盾。

### 2.2 "索引是位置、身份才稳定"贯穿全文 ✅

§三 的"为什么按索引不行"→ ReplicationID 解法，与 §七.1 的"ID 同步、索引不同步是核心权衡"前后呼应，形成了一个完整的论证闭环。

### 2.3 与系列前文的呼应 ✅

§三.3 `ItemMap.Reset()` 呼应第 10 篇预测、§六 属性预测呼应第 10 篇、§六.3 `FActiveGameplayCueContainer` 呼应第 09 篇，跨篇串联到位。

---

## 三、深度审查

### 3.1 设计思考有高度 ⭐ 优秀

§七 提出"三个诚实"框架：诚实一（放弃索引一致换预测能力）、诚实二（编译期多态换性能）、诚实三（两套序列化共存是历史包袱）。尤其"诚实三"点出 Generic vs Custom 两套机制并存是"演进的历史现实，不是干净架构"，符合系列一贯的"如实说明不完美"风格。

### 3.2 可进一步深挖 ⚠️

- §三.4 未展开 `FastArrayDeltaSerialize_DeltaSerializeStructs`（735 行）这个"内部 struct 增量序列化"的进阶变体，以及 `net.SupportFastArrayDelta` CVar（221 行）；
- §五 量化向量未给具体字节数对比（如 `FVector` 24 字节 vs `NetQuantizeNormal` 6 字节），"压到极限"缺少量化锚点。

---

## 四、实践价值审查

### 4.1 量化等级选型表 ✅

§五.2 的四等级表（精度/位数/范围）+ 选型原则，可直接指导开发时选 `NetQuantize10` 还是 `NetQuantizeNormal`。

### 4.2 元数据字数 ⚠️

文首标注 `~6500`，按正文中文内容估算（不含代码块与表格）实际约 5800-6200 字，基本吻合，可保留或微调。

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
| 设问句滥用 | §一、§三、§五 开头各 1 处，合理 |

### 5.3 结论

AI 味较低，技术密度高，行文以"源码 → 表格 → 设计解读"节奏推进。

---

## 六、图表一致性审查

本文暂未配 drawio 图。**建议补充两张图**（优先级高）：

1. **FastArray 增量复制时序图**（§三/§四）：服务器 `MarkItemDirty` → `FastArrayDeltaSerialize` → 客户端 `PostReplicatedAdd/Change`/`PreReplicatedRemove` 的完整流程；
2. **ID↔Key map 示意图**（§三）：展示"数组元素增删时，ReplicationID 如何保持稳定、索引如何变化"的对照。

---

## 七、总体评价

| 维度 | 评分 | 说明 |
|------|------|------|
| 源码准确性 | ✅ 优秀 | 全部字段/回调/行号/量化参数均已对照 UE 5.8 源码核验，关键的文件迁移位置也纠正了 |
| 逻辑清晰度 | ✅ 优秀 | "索引 vs 身份"主线清晰，跨篇呼应到位 |
| 深度 | ✅ 优秀 | 三个诚实框架有设计高度 |
| 实践价值 | ✅ 良好 | 量化选型表可直接指导开发 |
| AI 味 | ✅ 良好 | 整体自然 |

### 待修复项汇总

| # | 严重程度 | 位置 | 问题 | 建议 |
|---|---------|------|------|------|
| 1 | ⚠️ 中 | 全文 | 暂无 drawio 图 | 补 FastArray 复制时序图 + ID↔Key map 示意图 |
| 2 | 🔧 优化 | §三.4 | 未展开 DeltaSerializeStructs 变体 | 可补一句说明 |
| 3 | 🔧 优化 | §五 | 量化字节数未量化 | 补具体字节数对比 |

---

*审查完成。本文新建质量高，源码引用准确（含 FastArraySerializer 文件迁移位置的关键纠正）。建议优先补充两张图（#1），其余为优化项。*
