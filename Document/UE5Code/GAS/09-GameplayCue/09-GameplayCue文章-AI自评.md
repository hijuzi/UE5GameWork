# AI 自审查 — 09 | GameplayCue 表现层触发机制

> 审查日期：2026-08-24
> 审查版本：新建版（套用 05-08 统一格式）
> 评分标准：✅ 通过 / ⚠️ 有改进空间 / ❌ 需修正

---

## 一、源码准确性审查

### 1.1 EGameplayCueEvent 枚举 ✅

已确认为 `UENUM(BlueprintType) namespace EGameplayCueEvent { enum Type : int {...}; }`（`GameplayEffectTypes.h:964-982`），非 `enum class`。四个取值 `OnActive` / `WhileActive` / `Executed` / `Removed` 与注释一一对应，均准确。

### 1.2 三类 Notify ✅

- `UGameplayCueNotify_Static`：`UCLASS(Blueprintable, meta=(ShowWorldContextPin), hidecategories=(Replication), MinimalAPI)`，继承 `UObject`（非 Actor），四个事件均为 `BlueprintNativeEvent + BlueprintPure + const`（`GameplayCueNotify_Static.h:44-58`）。✅
- `AGameplayCueNotify_Actor`：继承 `AActor`，清理字段 `bAutoDestroyOnRemove` / `AutoDestroyDelay` / `WarnIfTimelineIsStillRunning` / `WarnIfLatentActionIsStillRunning`（`GameplayCueNotify_Actor.h:70-84`）。✅
- `AGameplayCueNotify_Looping`：继承 `AGameplayCueNotify_Actor`，四组特效 `ApplicationEffects` / `LoopingEffects` / `RecurringEffects` / `RemovalEffects`（`GameplayCueNotify_Looping.h:66-96`），四个 `BlueprintImplementableEvent` 钩子。✅

### 1.3 核心字段 ✅

`GameplayCueTag`（`meta=(Categories="GameplayCue")`）、`GameplayCueName`（`AssetRegistrySearchable`）、`IsOverride`、`bUniqueInstancePerInstigator`、`bUniqueInstancePerSourceObject`、`bAllowMultipleOnActiveEvents`、`bAllowMultipleWhileActiveEvents`、`bAutoAttachToOwner`、`NumPreallocatedInstances` 均与 `GameplayCueNotify_Actor.h:106-152` 一致。✅

### 1.4 IsOverride 语义 ✅

层级 Tag 从具体到通用依次调用的描述，与源码注释（`GameplayCueNotify_Actor.h:118`）"If this is Damage.Physical.Slash, we won't call Damage.Physical after we run this cue" 一致。✅

### 1.5 FGameplayCueParameters ✅

`NormalizedMagnitude` / `RawMagnitude` / `EffectContext` / `MatchedTagName`（NotReplicated）/ `OriginalTag`（NotReplicated）/ `AggregatedSourceTags` / `Location`（`FVector_NetQuantize10`）/ `Normal`（`FVector_NetQuantizeNormal`）/ `Instigator`（TWeakObjectPtr）/ `EffectCauser`（TWeakObjectPtr）/ `SourceObject` / `PhysicalMaterial` / `GameplayEffectLevel` / `AbilityLevel` / `TargetAttachComponent`，均与 `GameplayEffectTypes.h:864-933` 一致。✅

### 1.6 UGameplayCueManager ✅

- `UCLASS(MinimalAPI) class UGameplayCueManager : public UDataAsset`（`GameplayCueManager.h:129-130`）。✅
- 三步路由：`ShouldSuppressGameplayCues` → `TranslateGameplayCue` → `RouteGameplayCue`，注释编号 1/2/3（`GameplayCueManager.h:173-180`）。✅
- ObjectLibrary 双库（Runtime / Editor）与注释一致（`GameplayCueManager.h:228-237`）。✅
- 回收池 `Recycle()` / `ReuseAfterRecycle()` / `NotifyGameplayCueActorFinished`。✅

### 1.7 FGameplayEffectCue ✅

`MagnitudeAttribute` / `MinLevel` / `MaxLevel` / `GameplayCueTags` / `NormalizeLevel()`（`GameplayEffect.h:605-649`），以及 `bRequireModifierSuccessToTriggerCues` / `bSuppressStackingCues`（`GameplayEffect.h:2323-2328`）。✅

### 1.8 IGameplayCueInterface ✅

`meta=(CannotImplementInterfaceInBlueprint)`（`GameplayCueInterface.h:24`）、`HandleGameplayCue` 的 `UObject*` 新版 vs `AActor*` DEPRECATED 版（`GameplayCueInterface.h:49-58`）、`ForwardGameplayCueToParent`、`BlueprintCustomHandler`（`BlueprintImplementableEvent + BlueprintCosmetic`）、`GameplayCueDefaultHandler` 均准确。✅

### 1.9 可进一步核实点 ⚠️

`FActiveGameplayCueContainer` / `FMinimalGameplayCueReplicationProxy` 在正文 §8.2 仅提及，未展开 `NetDeltaSerialize` 细节。当前处理合理（属于第 12 篇"网络序列化"范畴），已用一句话留了钩子。

---

## 二、逻辑清晰度审查

### 2.1 问题驱动切题 ✅

以"属性改了，但屏幕上什么都没发生"引出表现层职责，与前四篇（GE/GA 逻辑层）形成清晰的层次对照，读者能自然理解 GC 在 GAS 中的定位。

### 2.2 概念速览表 ✅

"节目单 / 道具 / 演员 / 剧院经理"的类比表 + 完整生命周期 ASCII 图，为后文七个章节提供了统一坐标系。

### 2.3 四事件模型 ⭐ 优秀

`OnActive` vs `WhileActive` 的"亲眼见证激活"vs"首次看到激活态"的区分，是 GC 最易混淆的点，本文用中毒例子 + 对比表讲得很清楚，尤其点出"中途加入玩家看不到持续状态"的实际意义。

### 2.4 章节编排 ✅

从"事件模型 → Notify 载体 → 字段 → 参数 → Manager → 触发路径 → 接口"的递进，符合"是什么 → 怎么配 → 谁调度 → 谁触发 → 谁接收"的认知顺序，与 05-08 篇风格一致。

---

## 三、深度审查

### 3.1 设计思考有高度 ⭐ 优秀

§十 提出"三层解耦"框架（Tag 解耦触发/表现、事件解耦发生了什么/怎么表现、Manager 解耦加载/路由），并落到"表现层可整体替换"这一 GAS 罕见的属性上，同时诚实指出解耦的代价（调试成本）与 `GAMEPLAYCUE_DEBUG` 的存在意义。

### 3.2 可进一步深挖 ⚠️

- §6 提到 `NormalizedMagnitude` 驱动强度缩放，但未展开"如何用蓝图里的 Normalize 节点做具体缩放"的实践细节；
- §7.5 回收池未展开 `PreallocationInfo` 的容量溢出处理（源码 `DumpPreallocationStats` 已存在，可提一句）。

---

## 四、实践价值审查

### 4.1 选型表 ✅

三类 Notify 的"需求 → 选择"决策表 + 四事件"触发时机/中毒例子/典型用途"表，可直接指导实际开发时的选型。

### 4.2 元数据字数 ⚠️

文首标注 `~6500`，按正文中文内容估算（不含代码块与表格）实际约 6000-6500 字，基本吻合，可保留或微调。

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
| 设问句滥用 | §一 开头 1 处，合理 |

### 5.3 结论

AI 味较低，技术密度高，行文以"源码 → 表格 → 设计解读"节奏推进。

---

## 六、图表一致性审查

本文暂未配 drawio 图。后续可考虑补充「GC 路由三步流程」与「四事件生命周期」两张图，与 07/08 的图文风格对齐。

---

## 七、总体评价

| 维度 | 评分 | 说明 |
|------|------|------|
| 源码准确性 | ✅ 优秀 | 全部关键 API/字段/行号均已对照 UE 5.8 源码核验，无臆造 |
| 逻辑清晰度 | ✅ 优秀 | 概念表 + 四事件模型 + 三层解耦，结构清晰 |
| 深度 | ✅ 良好 | 三层解耦框架有高度，实践细节可再补 |
| 实践价值 | ✅ 良好 | 选型表充足，可直接指导开发 |
| AI 味 | ✅ 良好 | 整体自然 |

### 待修复项汇总

| # | 严重程度 | 位置 | 问题 | 建议 |
|---|---------|------|------|------|
| 1 | 🔧 优化 | §6 | NormalizedMagnitude 缩放实践细节未展开 | 补一句蓝图 Normalize 用法 |
| 2 | 🔧 优化 | §7.5 | 回收池容量溢出未提 | 可提 `DumpPreallocationStats` |
| 3 | 🔧 优化 | 全文 | 暂无 drawio 图 | 补充路由流程/四事件图 |

---

*审查完成。本文新建质量高，源码引用准确，无臆造 API。三个待修复项均为优化建议，不阻塞发布。*
