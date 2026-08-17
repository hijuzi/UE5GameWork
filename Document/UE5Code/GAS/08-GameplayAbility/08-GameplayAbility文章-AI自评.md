# AI 自审查 — 08 | GameplayAbility Task/输入/预测 (下)

> 审查日期：2026-08-17
> 审查版本：重写版（套用 05/06 统一格式）
> 评分标准：✅ 通过 / ⚠️ 有改进空间 / ❌ 需修正

---

## 一、源码准确性审查

本次重写修正了原文多处臆造 API 与字段，逐项核验如下。

### 1.1 UAbilityTask 基类声明 ✅

原文把基类写成 `UCLASS(Abstract, BlueprintType, meta=(ExposedAsyncProxy=AsyncTask))`，并错误声明了 `virtual void Activate() override;`。重写版已修正为：

- `UCLASS(Abstract, MinimalAPI)`（`AbilityTask.h:21`）；
- `AbilitySystemComponent` 为 `TWeakObjectPtr<UAbilitySystemComponent>`（原文误作 `TObjectPtr`）；
- **`UAbilityTask` 基类并未重新声明 `Activate()`**——它继承自 `UGameplayTask` 的 protected 虚函数，由具体子类按需 override（如 `UAbilityTask_WaitTargetData` 第 54 行）；
- `OnDestroy` 参数名为 `bInOwnerFinished`（原文误作 `bAbilityEnded`）。

### 1.2 激活机制 ✅

原文在 8.2.2 写 `ReadyForActivation()`、8.3.1 写 `Task->Activate()`，自相矛盾。重写版已统一为准确链路：

`静态工厂 → NewAbilityTask（只创建不激活）→ 绑定委托 → ReadyForActivation()（public）→ Activate()（protected，子类 override）`。

并以 `WaitDelay` 真实源码（工厂不激活、`Activate()` 里 SetTimer）佐证，明确指出"外部不能调用 `Activate()`（protected）"。

### 1.3 工厂命名 ✅

- `WaitForAttributeChange`（原文误作 `WaitAttributeChange`）已修正；
- `WaitForOverlap`、`CreatePlayMontageAndWaitProxy`、`WaitTargetDataUsingActor`、`WaitConfirmCancel`、`WaitInputPress`/`WaitInputRelease` 均与头文件静态工厂名一致。

### 1.4 输入路由 API ✅

原文臆造了 `AbilityInputCachePressed` / `AbilityInputCacheReleased` / `ProcessAbilityInput`。重写版已改为真实 API `AbilityLocalInputPressed(int32)` / `AbilityLocalInputReleased(int32)`（`AbilitySystemComponent.h:1363-1364`），并明确指出臆造函数"源码中并不存在"。

### 1.5 TargetActor 成员 ✅

- 删除了不存在的 `TargetConfirmation` 成员；
- 委托 `TargetDataReadyDelegate` / `CanceledDelegate` 更正为 `FAbilityTargetData` 类型（原文误作 `FGameplayAbilityTargetDataHandle`）；
- 持有关系更正为 `OwningAbility`（原文误作 `OwningAbilitySystemComponent`）；
- `UCLASS(Blueprintable, abstract, notplaceable, MinimalAPI)` 已按源码补齐 `MinimalAPI`。

### 1.6 EGameplayTargetingConfirmation ✅

已确认为 `UENUM(BlueprintType) namespace ... enum Type : int`（非 `enum class`），取值 `Instant / UserConfirmed / Custom / CustomMulti` 与源码一致。

### 1.7 TargetData 的 NetSerialize 归属 ✅

原文把 `NetSerialize` 写成 `FGameplayAbilityTargetData` 基类的虚方法。重写版已修正：基类**无 NetSerialize**，靠 `GetScriptStruct()` 做多态序列化，`NetSerialize` 在具体子类（如 `_SingleTargetHit`）。

### 1.8 FPredictionKey 字段 ✅

- `Current` / `Base` 更正为 `int16`（原文误作 `int32`），并说明 `typedef int16 KeyType`；
- 删除不存在的 `bIsStale` 字段，补入真实的 `bIsServerInitiated`；
- 关键方法（`IsValidKey` / `IsLocalClientKey` / `WasReceived` / `WasLocallyGenerated` 等）与 `GameplayPrediction.h` 一致。

---

## 二、逻辑清晰度审查

### 2.1 问题驱动切题 ✅

以"长时异步难题"（延迟 / 播动画 / 等松键 / 等确认）引出 Task，与上一篇"激活"形成自然的上下篇衔接。

### 2.2 输入/瞄准/预测三个子系统的串联 ✅

§四、§五、§六 都以"真实 API 是什么 → 如何被 Task 封装 → 设计意图"三层推进，把输入路由、瞄准、预测串成一条主线。

### 2.3 章节编号与格式 ✅

中文数字章节、`# 08 | ...` 标题、头部元信息块、系列导航表、上一篇/下一篇链接、落款，与 05/06/07 完全统一。

### 2.4 可优化点 ⚠️

§五 的 `WaitTargetData` 蓝图三连用伪代码表达，无真实蓝图节点图。读者对"WaitTargetData + WaitConfirmCancel 组合"的节点连线可能仍不够直观，建议后续补图。

---

## 三、深度审查

### 3.1 设计思考有高度 ✅

§七 提出"Task 是对象而非回调"的核心论点，并解释 `EndAbility` 里 `ActiveTasks.Reset()` 能"一把清空挂起异步"的前提正是对象化；还点出 InputID/TargetData 两层解耦是 GAS 可复用性的根源。

### 3.2 预测本质点到即止 ⚠️

§6.3 将预测本质作为"预告"处理，明确留给第 10 篇展开回滚、依赖预测。这是合理的篇幅控制，但需保证第 10 篇确实承接此处埋的钩子（`NewRejectedDelegate` / `NewCaughtUpDelegate`）。

---

## 四、实践价值审查

### 4.1 关键纠正的实操意义 ✅

"`AbilityLocalInputPressed/Released` 才是真实 API"、"`Activate()` 是 protected 不能外部调用"、"`WaitForAttributeChange` 而非 `WaitAttributeChange`"等纠正，对读者抄代码时的踩坑有直接价值。

### 4.2 元数据字数 ⚠️

文首标注 `~6000`，按正文中文内容估算实际约 5200-5600 字，建议校准为 `~5500`。

---

## 五、AI 味审查

### 5.1 Em Dash（—）使用统计

全文中文破折号（—）约 20 次，集中在标题与元信息，正文密度适中。

### 5.2 结构性句式检查

| 检查项 | 结果 |
|--------|------|
| "首先...其次..." 机械排比 | 未发现 |
| "不仅...而且..." | 未发现 |
| "值得注意的是 / 需要强调的是" | 未发现 |
| "综上所述" | 未发现 |
| 设问句滥用 | §一 开头 1 处，合理 |

### 5.3 结论

AI 味较低，技术密度高，无空洞套话。

---

## 六、图表一致性审查

本文暂未配 drawio 图（08 目录下无 `diagrams/`）。后续可补充「Task 激活链路」与「WaitTargetData 蓝图三连」两张图。

---

## 七、总体评价

| 维度 | 评分 | 说明 |
|------|------|------|
| 源码准确性 | ✅ 优秀 | 臆造 API（AbilityInputCache*、bIsStale、TargetConfirmation）全部清除，字段类型与路径均已核验 |
| 逻辑清晰度 | ✅ 优秀 | 三子系统主线清晰，格式与 05/06/07 统一 |
| 深度 | ✅ 良好 | "对象而非回调"设计思考有高度，预测留钩子待第 10 篇 |
| 实践价值 | ✅ 良好 | 纠错项对抄代码有直接价值，字数需校准 |
| AI 味 | ✅ 良好 | 整体自然 |

### 待修复项汇总

| # | 严重程度 | 位置 | 问题 | 建议 |
|---|---------|------|------|------|
| 1 | ⚠️ 低 | §1 L7 | 字数标注 ~6000，实际约 5200-5600 | 校准为 ~5500 |
| 2 | 🔧 优化 | §五 | WaitTargetData 蓝图三连为伪代码 | 补充真实蓝图节点图 |
| 3 | 🔧 优化 | §6.3 | 预测本质为预告 | 确保第 10 篇承接 NewRejected/NewCaughtUpDelegate 钩子 |
| 4 | 🔧 优化 | 全文 | 暂无 drawio 图 | 补充 Task 激活链路图 |

---

*审查完成。本文重写质量高，原文臆造 API 已全部清除。建议优先校准字数（#1），其余为优化项。*
