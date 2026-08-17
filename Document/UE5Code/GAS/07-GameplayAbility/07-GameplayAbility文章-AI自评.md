# AI 自审查 — 07 | GameplayAbility 技能激活与核心框架 (上)

> 审查日期：2026-08-17
> 审查版本：重写版（套用 05/06 统一格式）
> 评分标准：✅ 通过 / ⚠️ 有改进空间 / ❌ 需修正

---

## 一、源码准确性审查

本次重写的核心目标之一就是修正原文大量不准确的源码引用，逐项核验如下。

### 1.1 策略枚举的形式与位置 ✅

原文将四个枚举写成 `enum class ... : uint8` 且位置标注为 `GameplayAbility.h`，均为错误。重写版已修正为：

- 真实形式：`UENUM(BlueprintType) namespace X { enum Type : int {...}; }`（非 `enum class`）；
- 真实位置：`GameplayAbilityTypes.h`（36-110 行）；
- 补齐了原文遗漏的第四个枚举 `EGameplayAbilityNetSecurityPolicy`（78-96 行），并说明其"控制谁有权结束/终止技能"。

已对照 `GameplayAbilityTypes.h` 源码逐值核验，四个枚举的取值与顺序均准确。

### 1.2 Spec 字段类型 ✅

原文多处字段类型错误，重写版已修正：

- `SourceObject`：`TWeakObjectPtr<UObject>`（弱引用），原文误作 `TObjectPtr`；
- `ActiveCount` / `InputPressed`：均带 `NotReplicated` 的本地状态；
- `InputPressed`：`uint8 : 1` 位域，非 `bool`；
- `ActivationInfo` / `DynamicAbilityTags`：已标注 5.5 DEPRECATED。

字段行号（195-272）已按 `GameplayAbilitySpec.h` 实际位置校准。

### 1.3 SpecHandle 的 GenerateNewHandle ✅

原文将 `GenerateNewHandle()` 写成内联实现（`static int32 GHandle = 1; ...`），系臆造。重写版已改为"仅声明，实现在 .cpp"，符合 `GameplayAbilitySpecHandle.h` 现状。

### 1.4 CommitAbility 真实顺序 ✅

原文将 Cost 放在 Cooldown 之前。重写版已修正为：

- `CommitCheck`：先 `CheckCooldown`（671-674）后 `CheckCost`（676-679）；
- `CommitExecute`：先 `ApplyCooldown`（686）后 `ApplyCost`（688）；
- 并解释了"检查顺序与执行顺序严格对应"的设计意图。

### 1.5 EndAbility 真实清理 ✅

原文将蓝图回调放在流程末尾、Task 清理简化失实。重写版已修正为：

- `K2_OnEndAbility` 在流程**最前**触发；
- Task 清理用 `TaskOwnerEnded()`（非 `EndTask()`），倒序遍历 + `ActiveTasks.Reset()`。

### 1.6 CanActivateAbility 检查链 ✅

原文的"检查关卡"顺序模糊。重写版已按 `GameplayAbility.cpp:457` 实序归纳为九道检查：AvatarActor 有效 → ASC 有效 → Spec 有效 → 输入抑制 → 冷却 → 消耗 → Tag 要求 → 输入阻塞 → 蓝图覆写，并指出"冷却先于消耗、成本从低到高"。

### 1.7 可进一步核实点 ⚠️

`§5.3` 中第 2、3 步（ASC 有效、Spec 有效）以注释形式省略了完整代码。当前表述准确，但读者若想复现完整实现需自行翻阅源码，可考虑在后续补充完整代码块。

---

## 二、逻辑清晰度审查

### 2.1 三层模型开篇 ⭐ 优秀

"Handle（门牌号）→ Spec（档案）→ Instance（执行体）"的类比表放在概念速览，为后文 Spec/Handle/实例化三个话题提供了统一坐标系，降低了理解门槛。

### 2.2 激活链路 ASCII 流程图 ✅

`TryActivateAbility → InternalTryActivateAbility → CanActivateAbility → 实例化 → CallActivateAbility → ActivateAbility` 的缩进流程图清晰还原了调用层级。

### 2.3 章节编号与格式 ✅

已统一为中文数字章节（一、二、三…），标题采用 `# 07 | ...` 形式，补齐头部元信息块（本篇/系列/难度/字数/前置/源码路径）与系列导航表，结尾含上一篇/下一篇链接与落款，与 05/06 完全一致。

### 2.4 可优化点 ⚠️

§八 Block & Cancel 与 §5.3 的 `DoesAbilitySatisfyTagRequirements` 在"Tag 关卡"上略有交叉，但前者讲配置字段、后者讲检查时机，侧重点不同，可接受。

---

## 三、深度审查

### 3.1 设计意图层面的解释 ✅

- 解释了 CommitAbility "检查顺序决定报错优先级、执行顺序与检查严格对应"的深层原因；
- 指出"冷却/消耗本质都建模成 GE"，呼应上一篇 GE 的"一切都走 GE"设计；
- 在 §九 点出"检查（无副作用）/ 执行 / 提交 / 结束"四段划分对网络预测可预测性的支撑。

### 3.2 可进一步深挖 ⚠️

`§5.3` 提到 `ShouldIgnoreCooldowns` / `ShouldIgnoreCosts` 全局开关，但未说明它们何时为 true（如 GM 调试、GodMode 等特殊模式）。可加一句说明。

---

## 四、实践价值审查

### 4.1 选型表 ✅

四维策略枚举各配了取值表 + 典型场景（`InstancedPerActor` 有状态复用 / `InstancedPerExecution` 无状态并行 / `NonInstanced` 已废弃），具备实操指导性。

### 4.2 元数据字数 ⚠️

文首标注 `~6500`。按正文中文内容估算（不含代码块与表格）实际约 5500-6000 字，建议按实际统计校准为 `~5800` 左右。

---

## 五、AI 味审查

### 5.1 Em Dash（—）使用统计

全文中文破折号（—）出现约 20-25 次，分布在标题与 `> 本篇` 元信息中，正文密度适中，可接受。

### 5.2 结构性句式检查

| 检查项 | 结果 |
|--------|------|
| "首先...其次...再次..." 机械排比 | 未发现 |
| "不仅...而且..." | 未发现 |
| "值得注意的是 / 需要强调的是" | 未发现 |
| "综上所述 / 总而言之" | 未发现（总结用表格直接收束） |
| 设问句滥用 | 仅 §一 开头 1 处，合理 |

### 5.3 结论

AI 味较低。行文以"源码 → 表格 → 一句设计解读"的节奏推进，无空洞词汇。

---

## 六、图表一致性审查

本文暂未配 drawio 图（07 目录下无 `diagrams/`）。后续可考虑补充「三层模型」与「激活链路」两张图，与 05/06 的图文风格对齐。

---

## 七、总体评价

| 维度 | 评分 | 说明 |
|------|------|------|
| 源码准确性 | ✅ 优秀 | 原错误全部修正，四枚举、Spec 字段、Commit/End 顺序均已对照源码核验 |
| 逻辑清晰度 | ✅ 优秀 | 三层模型 + 流程图为骨架，中文数字章节结构统一 |
| 深度 | ✅ 良好 | 有设计意图解读，全局开关细节可再补一句 |
| 实践价值 | ✅ 良好 | 选型表充足，字数标注需校准 |
| AI 味 | ✅ 良好 | 整体自然 |

### 待修复项汇总

| # | 严重程度 | 位置 | 问题 | 建议 |
|---|---------|------|------|------|
| 1 | ⚠️ 低 | §1 L7 | 字数标注 ~6500，实际约 5500-6000 | 校准为 ~5800 |
| 2 | 🔧 优化 | §5.3 | ASC/Spec 有效两步以注释省略 | 可补完整代码块 |
| 3 | 🔧 优化 | §5.3 | `ShouldIgnoreCooldowns/Costs` 未解释为 true 的场景 | 加一句说明 |
| 4 | 🔧 优化 | 全文 | 暂无 drawio 图 | 补充三层模型/激活链路图 |

---

*审查完成。本文重写质量高，原源码错误已全部修正。建议优先校准字数（#1），其余为优化项。*
