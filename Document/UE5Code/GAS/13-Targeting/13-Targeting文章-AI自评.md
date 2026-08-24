# AI 自审查 — 13 | Targeting 瞄准系统

> 审查日期：2026-08-24
> 审查版本：新建版（套用 05-12 统一格式）
> 评分标准：✅ 通过 / ⚠️ 有改进空间 / ❌ 需修正

---

## 一、源码准确性审查

### 1.1 AGameplayAbilityTargetActor 类声明 ✅

`UCLASS(Blueprintable, abstract, notplaceable, MinimalAPI)`，继承 `AActor`（`GameplayAbilityTargetActor.h:26-27`）✅。头注释"spawned once per ability activation / not very efficient / not well tested by internal games"（19-25 行）逐字转述 ✅。

### 1.2 核心成员与委托 ✅

- `TargetDataReadyDelegate` / `CanceledDelegate` 类型为 `FAbilityTargetData`（`GameplayAbilityTargetActor.h:69-70`）✅
- `FAbilityTargetData` 是 `DECLARE_MULTICAST_DELEGATE_OneParam`，参数 `const FGameplayAbilityTargetDataHandle&`（`GameplayAbilityTargetTypes.h:655`）✅
- 持有 `OwningAbility`（`TObjectPtr<UGameplayAbility>`，78 行）✅

### 1.3 确认流程方法 ✅

`StartTargeting`/`IsConfirmTargetingAllowed`/`ConfirmTargetingAndContinue`/`ConfirmTargeting`/`CancelTargeting`/`BindToConfirmCancelInputs`/`ShouldProduceTargetDataOnServer` 签名与 `GameplayAbilityTargetActor.h:36-62` 一致 ✅。`ShouldProduceTargetDataOnServer` 注释"client 只需发 confirm"（36 行）✅。

### 1.4 EGameplayTargetingConfirmation ✅

`UENUM + namespace + enum Type : int`，取值 `Instant/UserConfirmed/Custom/CustomMulti`（`GameplayAbilityTargetTypes.h:25-43`）✅。四个取值的语义注释准确转述 ✅。

### 1.5 FGameplayAbilityTargetData 基类 ✅

接口 `GetActors`/`SetActors`/`HasHitResult`/`GetHitResult`/`HasOrigin`/`GetOrigin`/`HasEndPoint`/`GetEndPoint`/`GetEndPointTransform`/`GetScriptStruct` 签名与默认实现（`GameplayAbilityTargetTypes.h:96-168`）一致 ✅。`GetScriptStruct` 注释 "must always be overridden"（150 行）✅。

### 1.6 四个子类 ✅

`_LocationInfo`/`_ActorArray`/`_SingleTargetHit`/`_SourceLocation` 及其关键 override 均与源码一致。`TStructOpsTypeTraits` 的 `WithNetSerializer = true` 注释 "REQUIRED for FGameplayAbilityTargetDataHandle net serialization"（如 384、451、554、650 行）✅。

### 1.7 FGameplayAbilityTargetingLocationInfo ✅

`EGameplayAbilityTargetingLocationType` 三取值 `LiteralTransform/ActorTransform/SocketTransform`（171-186）✅。`GetTargetingTransform` 的 switch 逻辑（`GameplayAbilityTargetTypes.cpp:95-122`）✅。`NetSerialize` 按需序列化（292-315）✅。

### 1.8 多态序列化 NetSerialize ✅

`FGameplayAbilityTargetDataHandle::NetSerialize`（`GameplayAbilityTargetTypes.cpp:195-291`）：
- 先 `Ar << UniqueId` + `DataNum` ✅
- `TargetDataStructCache.NetSerialize` 序列化 ScriptStruct ✅
- 加载时 `FMemory::Malloc(GetStructureSize())` + `InitializeStruct` ✅
- 自定义 `FGameplayAbilityTargetDataDeleter`（186-193）用 `DestroyStruct` + `Free` ✅
- 无 `STRUCT_NetSerializeNative` 则 `ABILITY_LOG(Fatal)`（252 行）✅
- `TARGETDATAHANDLE_SAFE_NET_SERIALIZE` 注释 "untested/unproven still"（180-183）✅

### 1.9 可进一步核实点 ⚠️

§六.2 提到 `TargetDataStructCache`，但未展开其内部实现（`AbilitySystemGlobals` 里的 `FGameplayAbilityTargetDataStructCache`，映射 ScriptStruct↔ID）。当前表述准确，但可补一句说明它是"类型表"。

---

## 二、逻辑清晰度审查

### 2.1 问题驱动切题 ⭐ 优秀

以"近战/鼠标/AOE/MMO"四种瞄准形态的多样性切入，点出"目标形态完全不同，但要回答同一个问题 + 能跨网络传输"，自然引出三层分工。

### 2.2 三层分工贯穿 ✅

§二 表格（TargetActor/TargetData/Handle）+ §三/四/五/六 分别对应，结构清晰。尤其 §二 引用头注释"为什么不用 UObject"直击核心，为 §七 的设计思考埋下伏笔。

### 2.3 多态序列化讲透 ⭐ 优秀

§六 用"先传类型再传数据"一句点破多态序列化的本质，配合代码逐段解读（UniqueId → DataNum → GetScriptStruct → malloc/InitializeStruct → 自定义 Deleter → Fatal 检查），把最难的机制讲得清楚。

---

## 三、深度审查

### 3.1 设计思考有高度 ⭐ 优秀

§七 三个权衡层层递进：
- 为什么不用 UObject（规避对象复制重机制）；
- 多态序列化的代价（类型表 + 手动内存管理）；
- TargetActor 的"教学范本"定位，并点出"GAS 子系统成熟度不均等"这一洞察。

尤其 §7.3 把 TargetActor 的"简陋"解读为"示范原理的教学级"，并与 GE/属性（生产级）对照，是很有价值的分析角度。

### 3.2 诚实的源码注释成为素材 ✅

三处坦诚标注（TargetActor"不高效/未测试"、`TARGETDATAHANDLE_SAFE_NET_SERIALIZE`"未验证"）都被提炼成"GAS 文档如实说明不完美"的论据，延续了系列的风格。

### 3.3 可进一步深挖 ⚠️

- §三.3 未展开 `ConfirmTargetingAndContinue` 与 `ConfirmTargeting` 的差异细节（前者不销毁 TargetActor，后者触发销毁）；
- §六 未展开 `FGameplayAbilityTargetData_SingleTargetHit` 的 `bHitReplaced`（`NotReplicated`，635 行）这个预测相关字段。

---

## 四、实践价值审查

### 4.1 选型表 ✅

四个子类的"用途 + 关键 override"表，可直接指导开发时选 `_ActorArray`（AOE）还是 `_SingleTargetHit`（单体）。

### 4.2 诚实标注的实操意义 ✅

§三.1 明确告知"默认 TargetActor 不高效，需要子类化"，避免读者把引擎自带当生产方案直接用于高频技能。

### 4.3 元数据字数 ⚠️

文首标注 `~6200`，按正文中文内容估算（不含代码块与表格）实际约 5600-6000 字，基本吻合，可保留。

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
| 设问句滥用 | §一、§七 开头各 1 处，合理 |

### 5.3 结论

AI 味较低，技术密度高，行文以"源码 → 表格 → 设计解读"节奏推进。

---

## 六、图表一致性审查

本文暂未配 drawio 图。**建议补充两张图**：

1. **目标确认时序图**（§三）：技能 → WaitTargetData → TargetActor（StartTargeting → Confirm/Cancel）→ 广播 TargetDataReadyDelegate 的交互；
2. **多态序列化流程图**（§六）：`GetScriptStruct` → 类型序列化 → malloc/InitializeStruct → 数据序列化 → 自定义 Deleter 的完整链路。

---

## 七、总体评价

| 维度 | 评分 | 说明 |
|------|------|------|
| 源码准确性 | ✅ 优秀 | 全部类声明/接口签名/NetSerialize 实现均已对照 UE 5.8 源码核验 |
| 逻辑清晰度 | ✅ 优秀 | 三层分工主线 + 多态序列化讲透 |
| 深度 | ✅ 优秀 | 三个权衡 + 子系统成熟度洞察，有设计高度 |
| 实践价值 | ✅ 良好 | 选型表 + 诚实标注的实操意义 |
| AI 味 | ✅ 良好 | 整体自然 |

### 待修复项汇总

| # | 严重程度 | 位置 | 问题 | 建议 |
|---|---------|------|------|------|
| 1 | ⚠️ 中 | 全文 | 暂无 drawio 图 | 补确认时序图 + 多态序列化流程图 |
| 2 | 🔧 优化 | §三.3 | ConfirmTargetingAndContinue 差异未展开 | 补一句说明 |
| 3 | 🔧 优化 | §六 | TargetDataStructCache 内部实现未展开 | 补一句"类型表"说明 |

---

*审查完成。本文新建质量高，源码引用准确，多态序列化机制讲解透彻。建议优先补充两张图（#1），其余为优化项。*
