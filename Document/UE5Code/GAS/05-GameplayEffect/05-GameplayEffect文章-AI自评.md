# AI 自审查 — 05 | GameplayEffect 数据结构与配置体系

> 审查日期：2026-08-12
> 审查版本：当前编辑版本
> 评分标准：✅ 通过 / ⚠️ 有改进空间 / ❌ 需修正

---

## 一、源码准确性审查

### 1.1 枚举值命名不一致 ⚠️

**问题位置**：§4.1 的源码摘录

文章在 §4.1 的用户面枚举定义（第 240-252 行）使用 **新命名**：
```
AddBase = 0, MultiplyAdditive = 1, DivideAdditive = 2, MultiplyCompound = 4, AddFinal = 5
```

但后续 Aggregator 源码引用（L279、L303、L329、L389）使用了**旧命名**：
```cpp
SumMods(Mods[EGameplayModOp::Additive], ...)       // 旧名
SumMods(Mods[EGameplayModOp::Multiplicitive], ...)  // 旧名
SumMods(Mods[EGameplayModOp::Division], ...)         // 旧名
```

**分析**：UE 引擎内部 `EvaluateWithBase` 实现中确实仍使用旧枚举名（`Additive`/`Multiplicitive`/`Division`），但文章没有解释两套命名的关系。读者看到 §4.1 表格中 `AddBase = 0`，翻到 Aggregator 源码却是 `EGameplayModOp::Additive`，会产生困惑。

**建议**：在 §4.1 的聚合公式表格前增加一句说明："Aggregator 内部仍使用旧版枚举名 `Additive`/`Multiplicitive`/`Division`，分别对应新版 `AddBase`/`MultiplyAdditive`/`DivideAdditive`。"

### 1.2 字段定义文件位置描述可以更精确 ⚠️

**问题位置**：§4.2，第 473 行

```cpp
// GameplayEffectTypes.h（实际位于 GameplayEffect.h 内）
struct FGameplayEffectModifierMagnitude
```

这个注释 "实际位于 GameplayEffect.h 内" 试图纠正文件归属，但表述值得怀疑。`FGameplayEffectModifierMagnitude` 在 UE5.8 中是否确实定义在 `GameplayEffect.h` 而非 `GameplayEffectTypes.h`？需要对照实际源码确认。如果它确实在 `GameplayEffect.h`，注释本身是正确的，但 `// GameplayEffectTypes.h` 的标注就成了误导信息。

**建议**：直接标为 `// GameplayEffect.h`，移除矛盾注释。

### 1.3 综合示例数学验证 ✅

§4.1 的综合实例（第 422-463 行）：

```
AddBase: 20+10=30, MultiplyAdditive: 1+(1.5-1)+(1.3-1)=1.8
DivideAdditive: 1+(2-1)=2.0, MultiplyCompound: 1.5, AddFinal: 5

((100+30)×1.8÷2.0×1.5)+5 = 180.5 ✓
```

逐项验算通过，数值语义正确。

### 1.4 源码行号范围自洽 ✅

§4.1 的聚合公式表（第 411-418 行）引用了 `GameplayEffectAggregator.cpp` 的行号 L78-L98，这些行号均在 §4.1 开头标注的范围 76-99 之内，且逻辑顺序（Override→Additive→Multiplicitive→Division→FinalAdd→CompoundMultiply→公式组装）与源码执行流一致。

### 1.5 Duration Policy 描述准确 ✅

Instant → 直写 BaseValue → 不创建 FActiveGameplayEffect
HasDuration → AggregatorMod 注册 → Timer 到期移除
Infinite → AggregatorMod 注册 → 无 Timer → 手动移除

三种模型的区分逻辑清晰，与引擎行为一致。

### 1.6 GE Component 列表检查 ✅

§7.1 列出的 11 个内置组件与 UE5.8 的 `GameplayAbilities/Public/GameplayEffectComponents/` 目录一致。关于 "没有 GameplayCuesGameplayEffectComponent" 的特别说明准确。

### 1.7 FGameplayEffectSpec 字段列表 ✅

§8.1 的 FGameplayEffectSpec 结构（第804-841行）包含：
- `CapturedSourceTags`/`CapturedTargetTags` 类型为 `FTagContainerAggregator`（非裸容器）✅
- `SetByCallerNameMagnitudes` + `SetByCallerTagMagnitudes` 双 Map 通道 ✅
- `EffectContext`、`Level`、`DynamicAssetTags`、`StackCount` 为 private ✅

---

## 二、逻辑清晰度审查

### 2.1 整体结构 ⭐ 优秀

```
问题驱动（为什么需要 GE）
  → 核心概念速览（全景图）
  → 模块逐一拆解：Duration → Modifier → Tags → Stacking → Component → Spec → Context
  → 设计思考
  → 总结
```

这个"先全景、再逐一、最后升华"的结构与技能要求完全一致。系列导航表位于文首，前置知识清楚。

### 2.2 ModOp 的 Step-by-Step 递进式讲解 ⭐ 优秀

§4.1 的 Step 0 → Step 5 递进式讲解是文章的亮点设计：
- 每次只加一种运算，公式逐步扩增
- 每步都锚定到源码行号
- 每步都有表格说明
- 最后用同一个综合实例串联所有 ModOp

这是将复杂源码讲清楚的最佳方式。

### 2.3 Tags 一节的结构可以更清晰 ⚠️

§5 的 Tags 系统介绍涉及 7 种不同的 Tag 字段（AssetTags、GrantedTags、BlockedAbilityTags、OngoingTagRequirements、ApplicationTagRequirements、RemovalTagRequirements、RemoveGameplayEffectsWithTags），但行文以枚举字段列表开头，然后逐个解释。

问题在于：§5.1 标题是 "GE Asset 自身的 Tags"，但实际内容混合了 AssetTags（GE 自身标签）、GrantedTags（授予目标）、OngoingTagRequirements（持续条件）等不同语义类别。建议对 Tags 做小型分类：
- **GE 身份标签**：AssetTags
- **目标授予标签**：GrantedTags、BlockedAbilityTags
- **条件关卡**：Application / Ongoing / Removal TagRequirements
- **免疫与清理**：ImmunityTags、RemoveGameplayEffectsWithTags

### 2.4 图表引用完整 ✅

- `GE_ClassDiagram.png`（第 88 行）—— CDO→Spec→ActiveGE 三级流水线
- `GE_ModifierSystem.png`（第 231 行）—— Modifier 体系全貌

两张图均包含图注，drawio 源文件存在且内容与文章描述一致。

---

## 三、深度审查

### 3.1 深入引擎内部机制 ✅

文章不止于 API 使用说明，深入到了：
- **Bias 机制**：解释了 SumMods 为什么 AddBase 用 Bias=0、MultiplyAdditive 用 Bias=1.0
- **MultiplyCompound 不走 SumMods**：明确指出用 MultiplyMods 连乘，而非叠加
- **DivideAdditive 的设计意图**：减伤收益递减，永远达不到 100%
- **Multi-Channel 级联**：每个 Channel 独立执行完整公式，上一 Channel 输出为下一 Channel 的 InlineBaseValue

### 3.2 有设计层面的思考 ✅

§10.3 "Spec'冻结'的真正代价是什么？" 讨论了冻结时机不可变、AttributeBased 快照时序陷阱、SetByCaller 必须在 Spec 创建前注入。这是只有实际用过 GAS 的开发者才能写出的内容。

### 3.3 可以进一步深挖的点 ⚠️

以下主题在本文中提及但未展开，可在后续或补充说明中处理：
1. **`SnapshottedSourceAttributes` 参数**（第 934 行提到但一笔带过）—— 这是控制 AttributeBased 是否冻结的关键开关，建议至少用一句话说明其默认值和行为。
2. **`FGameplayEffectModifierMagnitude` 的 `SetByCaller` 同时支持 DataTag 和 DataName**（§4.2 已说明，但未解释优先级的冲突处理）—— 如果同时设置了 DataTag 和 DataName，Spec 查找时优先哪个？

---

## 四、实践价值审查

### 4.1 典型场景映射充足 ✅

§4.1 的 ModOp 映射表（diagram 中）为每种 ModOp 提供了：
- 聚合方式
- 典型场景
- GE 示例命名
- 注意事项

### 4.2 选型指南清晰 ✅

| 位置 | 选型内容 |
|------|---------|
| §4.1 | MultiplyAdditive vs MultiplyCompound 选型表（两行对比 + 一句话总结） |
| §6.3 | Stacking 三种 Expiration 策略的适用场景（中毒/护盾/永续被动） |
| §10.2 | 为什么是 5 种 ModOp 而非 3 种 |

### 4.3 「常见设计」提示分散但实用 ✅

- 第 465 行：Multi-Channel 级联使用场景
- 第 592 行：SetByCaller 是"动态伤害最常用的方式"
- 第 744 行：中毒/护盾/永续被动三种 Stacking 设计模式

### 4.4 元数据标注不准确 ⚠️

文首元数据（第 7 行）：
> 字数: ~4000

按正文中文内容（不含代码块和表格）估算，实际中文字符数约 **5200-5500** 字。建议修正为 `~5200` 或按实际统计更新。

---

## 五、AI 味审查

### 5.1 Em Dash（—）使用统计

全文中文破折号（—）出现约 25-30 次。在 5200 字的深度技术文章中，这个频率略高但仍在可接受范围。部分使用可替换为逗号或分号以增加节奏变化。

**重点关注**：
- 第 35 行："本文按「数据 → 执行 → 设计」的顺序展开：本篇讲 GE 的数据结构与配置体系，下一篇深入执行流程与计算链路。" —— 这个冒号不需要前面再加破折号表述，当前写法已经自然。
- §3.2 的 `>` 块引用（第 161 行）使用"——"连接长句，可考虑拆分。

### 5.2 结构性句式检查

| 检查项 | 结果 |
|--------|------|
| "首先...其次...再次...最后..." 机械排比 | 未发现 |
| "不仅...而且..." 过度使用 | 仅 1 处（§10.2），合理 |
| "值得注意的是/需要强调的是" | 仅 1 处（§3.4），可接受 |
| "综上所述/总而言之" | 未发现（总结简洁直接） |
| 设问句滥用 | §10.1-§10.3 各有 1 个设问标题，节奏合理 |

### 5.3 「思考」块的使用

文章使用了 2 处 "**思考**："（第 92 行、第 846 行）—— 这是本系列文章的一致风格（参考 06 文章也有类似用法），不构成 AI 味问题。

### 5.4 结论

文章的 AI 味程度较低。代码→分析→表格→总结的节奏自然，没有空洞的"赋能""加持"等明显 AI 词汇。唯一建议是对 em dash 做适量修剪。

---

## 六、图表一致性审查

### 6.1 GE_ClassDiagram.drawio

| 检查点 | 结果 |
|--------|------|
| 三级流水线（CDO → Spec → ActiveGE）正确 | ✅ |
| CDO 层字段（DurationPolicy, Modifiers, StackingType, GEComponents, GameplayCues）与文章描述一致 | ✅ |
| Spec 层包含 SetByCaller 双通道（Tag + FName） | ✅ |
| Context 层区分 Instigator / EffectCauser / AbilityCDO / SourceObject | ✅ |
| 扩展方式提示（继承 FGameplayEffectContext 重写 NetSerialize） | ✅ |
| Tags 带状条列出 AssetTags / GrantedTags / OngoingTagRequirements / ApplicationTagRequirements / RemoveGameplayEffectsWithTags / bRequireModifierTagsToMatch | ✅ |

### 6.2 GE_ModifierSystem.drawio

| 检查点 | 结果 |
|--------|------|
| 4 种 Magnitude 来源（ScalableFloat / AttributeBased / CustomCalculationClass / SetByCaller）全部展示 | ✅ |
| 5 种 ModOp 运算（AddBase → MultiplyAdditive → DivideAdditive → MultiplyCompound → AddFinal）按公式顺序排列 | ✅ |
| Override(3) 作为已废弃单独标注 | ✅ |
| 聚合公式 (InlineBaseValue + ΣAddBase) × ΣMultiplyAdditive ÷ ΣDivideAdditive × ΠMultiplyCompound + ΣAddFinal 完整呈现 | ✅ |
| SumMods vs MultiplyMods 区分标注（Σ vs Π） | ✅ |
| 实践关键点包含 Multi-Channel 级联和 Period 触发重新计算 | ✅ |
| 典型场景映射表 5 种 ModOp × 5 列信息完整 | ✅ |

---

## 七、总体评价

| 维度 | 评分 | 说明 |
|------|------|------|
| 源码准确性 | ⚠️ 良好 | 枚举命名不一致需修复，其他检查通过 |
| 逻辑清晰度 | ✅ 优秀 | Step-by-Step 递进+综合实例是教科书级写法 |
| 深度 | ✅ 优秀 | Bias 机制、Multi-Channel 级联、冻结代价均有涉及 |
| 实践价值 | ✅ 良好 | 选型指南充足，元数据字数需修正 |
| AI 味 | ✅ 良好 | 整体自然，em dash 可适度修剪 |

### 待修复项汇总

| # | 严重程度 | 位置 | 问题 | 建议 |
|---|---------|------|------|------|
| 1 | ⚠️ 中 | §4.1 L279/L303/L329/L389 | Aggregator 源码使用旧枚举名（Additive/Multiplicitive/Division），与用户面新名称不一致 | 增加一句过渡说明 |
| 2 | ⚠️ 低 | §4.2 L473 | `FGameplayEffectModifierMagnitude` 的文件归属注释矛盾 | 统一为 `GameplayEffect.h` |
| 3 | ⚠️ 低 | §1 L7 | 字数标注 ~4000，实际约 5200+ | 更新为 ~5200 |
| 4 | 🔧 优化 | §5.1 | Tags 节混合了不同语义的 Tag 类别 | 建议按"身份/授予/条件/清理"做子分类 |
| 5 | 🔧 优化 | 全文 | em dash 使用约 25-30 次 | 适量替换为逗号或句号 |
| 6 | 🔧 优化 | §10.3 L934 | `SnapshottedSourceAttributes` 仅提及未解释 | 增加一句说明默认值和行为 |

---

*审查完成。本文整体质量高，ModOp 的递进式讲解是系列亮点。建议优先修复 #1（枚举命名一致性），其他为优化项。*
