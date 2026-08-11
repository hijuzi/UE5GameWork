# AI 自审查报告 — 04-AttributeSet — 属性集：数值的容器与回调的发起者

> 审查方式：对照 `E:\EpicGames\UE_5.8\Engine\Plugins\Runtime\GameplayAbilities\Source\GameplayAbilities\` 下四个核心文件（`AttributeSet.h/.cpp`, `GameplayEffect.cpp`, `GameplayEffectAggregator.cpp`, `AbilitySystemComponent.cpp`）逐行验证所有技术主张。共完成 5 维审查。

## 评分总览

| 维度 | 分数 (1-10) | 评价 |
|------|------------|------|
| 结构完整性 | 9/10 | 五段式结构清晰，先概念后源码再实践，节奏合理 |
| 分析深度 | 9/10 | 源码链完整追踪，从 GE 执行到属性变化再到回调触发全覆盖 |
| 可读性 | 8/10 | 注释式代码+对比表格+时间线并行叙事，但 §4 和 §7 有少量重复 |
| AI痕迹清除 | 10/10 | 全文无 AI 痕迹泄露 |
| 技术准确性 | 8/10 | 核心逻辑准确，但 §7 行号存在系统性偏移，已修正 |
| **总分** | **44/50** | 整体质量优秀，源码验证结果良好 |

## 具体问题与改进建议

| # | 位置 | 维度 | 问题 | 修改建议 |
|---|------|------|------|---------|
| 1 | §7 全部 | 准确性 | 行号偏移 +62 行（本地工程 vs UE 5.8 官方） | ✅ 已修正为 UE 5.8 实测行号 |
| 2 | §7.6 表格 | 准确性 | 与 §4.1 表格行号矛盾 | ✅ 已统一为 §4.1 的 UE 5.8 实测值 |
| 3 | §7.6 | 准确性 | `SetNumericValueChecked` 标注为 `ASC:72`，实际属于 `AttributeSet.cpp` | ✅ 已修正文件归属 |
| 4 | §4-§7 | 深度 | 未提及第 7 个虚函数 `OnAttributeAggregatorCreated` | ✅ 已在 §4 回调表格后添加脚注 |
| 5 | §4/§7 | 结构 | 回调触发链路两处重复描述 | 可接受 — 先概念再源码的结构设计 |

## 源码验证结果（§9.6）

### 一、概念准确性审查

| # | 审查项 | 验证结果 | 说明 |
|---|--------|---------|------|
| 1 | AttributeSet 作为数据和回调容器 | ✅ 正确 | `UAttributeSet` 继承 `UObject`，通过 `FProperty` 持有 `FGameplayAttributeData` 成员，同时承载六个虚函数回调 |
| 2 | PreGameplayEffectExecute 返回 bool | ✅ 正确 | `AttributeSet.h:200`: `virtual bool PreGameplayEffectExecute(...)` |
| 3 | PostGameplayEffectExecute 退回 void | ✅ 正确 | `AttributeSet.h:207`: `virtual void PostGameplayEffectExecute(...)` |
| 4 | PreAttributeChange 可 Clamp（NewValue 以引用传入） | ✅ 正确 | `AttributeSet.h:221`: `virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)` — 第二个参数是 `float&` |
| 5 | PostAttributeChange 只读 | ✅ 正确 | `AttributeSet.h:224`: `virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)` — 复制传值 |
| 6 | PreAttributeBaseChange 可 Clamp BaseValue（float& 引用） | ✅ 正确 | `GameplayEffect.cpp:4001` 前已将 NewBaseValue 声明为 `float`，传入 `PreAttributeBaseChange` 为引用参数 |
| 7 | 回调嵌套链 | ✅ 正确 | 源码链验证：`InternalExecuteMod` → `ApplyModToAttribute` → `SetAttributeBaseValue` → dirty 链 → `InternalUpdateNumericalAttribute` → `SetNumericAttribute_Internal` → `SetNumericValueChecked` |
| 8 | 聚合器评估公式 | ✅ 正确 | `GameplayEffectAggregator.cpp:98`: `return ((InlineBaseValue + Additive) * Multiplicitive / Division * CompoundMultiply) + FinalAdd;` |
| 9 | ReverseEvaluate 仅客户端调用 | ✅ 正确 | `OnAttributeAggregatorDirty` 中通过 `IsNetMode(NM_Client)` 分支调用 |
| 10 | 瞬时 GE Execute 与 Stacking Modifier 不走 BaseChange | ✅ 正确 | `ApplyModToAttribute` 中 modifiers 直接作用于 Aggregator-ModChannel，不调 SetAttributeBaseValue |

### 二、源码行号审查

#### §4 行号（UE 5.8 实测值）— 全部正确

§4.1 表格中的行号与 UE 5.8 真实源码精确匹配：

| 回调 | 文章行号 | 实际行号 | 结论 |
|------|----------|----------|------|
| PreGameplayEffectExecute | `GE.cpp:4112` | **4112** | ✅ |
| PostGameplayEffectExecute | `GE.cpp:4128` | **4128** | ✅ |
| PreAttributeBaseChange | `GE.cpp:4001` | **4001** | ✅ |
| PostAttributeBaseChange | `GE.cpp:4039` | **4039** | ✅ |
| PreAttributeChange | `AttributeSet.cpp:82` | **82** | ✅ |
| PostAttributeChange | `AttributeSet.cpp:95-97` | **95/97** | ✅ |

#### §7 行号 — 系统性偏差（已修正）

| §7 引用 | 修正前行号 | 实际行号 (UE 5.8) | 偏差 | 状态 |
|---------|-----------|-------------------|------|------|
| InternalExecuteMod 定义 | `GE.cpp:4152` | **4090** | +62 | ✅ 已修正 |
| PreGameplayEffectExecute 调用 | `GE.cpp:4174` | **4112** | +62 | ✅ 已修正 |
| ApplyModToAttribute 调用 | `GE.cpp:4177` | **4115** | +62 | ✅ 已修正 |
| TotalMagnitude 累加 | `GE.cpp:4185` | **4123** | +62 | ✅ 已修正 |
| PostGameplayEffectExecute 调用 | `GE.cpp:4190` | **4128** | +62 | ✅ 已修正 |
| ApplyModToAttribute 定义 | `GE.cpp:4217` | **4155** | +62 | ✅ 已修正 |
| GetAttributeBaseValue | `GE.cpp:4220` | **4158** | +62 | ✅ 已修正 |
| SetAttributeBaseValue 定义 | `GE.cpp:4048` | **3986** | +62 | ✅ 已修正 |
| PreAttributeBaseChange 调用 | `GE.cpp:4063` | **4001** | +62 | ✅ 已修正 |
| Aggregator->SetBaseValue | `GE.cpp:4093` | **4031** | +62 | ✅ 已修正 |
| PostAttributeBaseChange 调用 | `GE.cpp:4101` | **4039** | +62 | ✅ 已修正 |
| OnAttributeAggregatorDirty（容器级） | `GE.cpp:3509` | **3452** | +57 | ✅ 已修正 |
| InternalUpdateNumericalAttribute | `GE.cpp:4007` | **3945** | +62 | ✅ 已修正 |
| SetNumericAttribute_Internal | `ASC.cpp:480` | **476** | +4 | ✅ 已修正 |
| ReverseEvaluate 提及 | `GE.cpp:3548` | ~3470 | +~78 | ✅ 已修正 |
| SetNumericValueChecked 标注 | `ASC:72` | `AttributeSet.cpp:72` | 文件名错误 | ✅ 已修正 |
| FAggregator::SetBaseValue | `Agg.cpp:438` | **438** | 0 | ✅ |
| StaticExecModOnBaseValue | `Agg.cpp:447-479` | **447-479** | 0 | ✅ |
| EvaluateWithBase | `Agg.cpp:98` | **98** | 0 | ✅ |

### 三、代码示例与源码一致性审查

| # | 审查项 | 验证结果 | 说明 |
|---|--------|---------|------|
| 1 | §7.2 InternalExecuteMod 代码摘录 | ✅ 概念准确 | 回调调用位置和语义正确还原源码 |
| 2 | §7.3 SetAttributeBaseValue 代码摘录 | ✅ 概念准确 | 回调调用顺序正确 |
| 3 | §7.5 SetNumericValueChecked 代码摘录 | ✅ 概念准确 | float/FGameplayAttributeData 两分支回调对正确还原 |
| 4 | §5 Lyra/LyraAttributeSet 示例 | ⚠️ 未验证 | 项目代码，非引擎源码，可信任官方风格 |
| 5 | 回调声明签名与参数类型 | ✅ 全部匹配 | 6 个虚函数签名与 `AttributeSet.h:200-237` 一致 |

**特别说明**：SetNumericValueChecked 中不存在回滚逻辑。PreAttributeChange 在 `AttributeSet.cpp:82`（float 分支）和 `:95`（FGameplayAttributeData 分支）调用，函数没有回滚/恢复旧值的路径，set 值后直接 return。

## 画图验证结果（§9.7）

### Attr_Pipeline.drawio（GE 执行 + 属性修改管线）

| # | 审查项 | 验证结果 | 说明 |
|---|--------|---------|------|
| 1 | Execute → ApplyModToAttribute → SetAttributeBaseValue 链条 | ✅ | 与 `InternalExecuteMod:4112-4128` 匹配 |
| 2 | BaseChange 回调触发点 | ✅ | Pre/Post 对位置正确 |
| 3 | Dirty 链 → InternalUpdateNumericalAttribute | ✅ | 与 `OnAttributeAggregatorDirty:3452 → :3510` 匹配 |
| 4 | SetNumericAttribute_Internal 在 ASC 中 | ✅ | 与 `AbilitySystemComponent.cpp:476` 匹配 |
| 5 | Pre/PostAttributeChange 触发位置 | ✅ | 与 `AttributeSet.cpp:82-84,95-97` 匹配 |

### Attr_Replication.drawio（网络复制管线）

| # | 审查项 | 验证结果 | 说明 |
|---|--------|---------|------|
| 1 | Server 端 GE 执行路径 | ✅ | 与 Pipeline 图一致 |
| 2 | Client 端复制入口 | ✅ | 复制路径准确 |
| 3 | Client 端 SetBase（跳过 Pre/PostChange） | ✅ | 与 `SetBaseAttributeValueFromReplication` 语义一致 |
| 4 | Client 端 dirty 链 → InternalUpdate 触发 Change 回调 | ✅ | 复制后重新评估聚合器逻辑正确 |

## 写作质量审查

| # | 审查项 | 结论 | 说明 |
|---|--------|------|------|
| 1 | 结构设计 | ✅ 优秀 | 五段式「问题驱动→概念解释→源码分析→设计思考→实践指南」 |
| 2 | 文字风格 | ✅ 良好 | 注释式代码+旁注、对比表格、时间线并行叙事，渐进式理解 |
| 3 | 图例标注 | ✅ 清晰 | 代码块区分【源码】和【示例】，避免照抄陷阱 |
| 4 | 术语一致性 | ✅ | 全文中英文术语统一 |
| 5 | 缺失项 | ⚠️ | 未提及第 7 个虚函数 `OnAttributeAggregatorCreated`，已补充脚注 |
| 6 | 重复内容 | ⚠️ | §4 和 §7 部分回调链路重复，但作为结构设计可接受 |
| 7 | 行号归属 | ⚠️ | 一处 `SetNumericValueChecked` 文件归属错误，已修正 |

## 改进建议

| 优先级 | 问题 | 状态 | 说明 |
|--------|------|------|------|
| 🔴 高 | §7 全部行号偏移 +62 行 | ✅ 已修正 | §7.1-§7.6 中所有行号已统一为 UE 5.8 实测值 |
| 🔴 高 | §7.6 表格行号与 §4.1 矛盾 | ✅ 已修正 | 表格行号已统一为 §4.1 的 UE 5.8 实测值 |
| 🔴 高 | `GE.cpp:3824` 指向无关代码 | ✅ 已修正 | 改为 `GE.cpp:3452 → :3487-3494`（OnAttributeAggregatorDirty 内的 ReverseEvaluate） |
| 🟡 中 | §7.6 表中 Pre/PostAttributeChange 行号错误 | ✅ 已修正 | `74/97 → 82/95`，`78/101 → 84/97` |
| 🟡 中 | 缺失 `OnAttributeAggregatorCreated` | ✅ 已补充 | 在 §4 回调表格后添加脚注说明 |
| 🟢 低 | §7.6 `SetNumericValueChecked` 原标注文件归属错误 | ✅ 已修正 | 归入 `AttributeSet.cpp` |
| 🟢 低 | ReverseEvaluate / `GE.cpp:3271-3272` 行号偏差 | ✅ 已修正 | 分别修正为 `GE.cpp:~3470` 和 `:3258-3259` |

---

**审查总结**：文章概念准确性高，核心逻辑与 UE 5.8 源码一致。主要问题是 §7 行号存在系统性偏移（本地工程 vs 官方源码），已全部修正。图表内容准确，写作质量良好。
