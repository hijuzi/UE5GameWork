# AI 自审查报告 — BTDecorator_Cooldown：有状态的"计时门卫"

> 审查方式：全文逐段重读；对照 `_source-location-Cooldown.md` 中 6 个源码文件逐一验证代码引用、调用链、类继承关系；对照 `diagrams/` 两张新图验证与源码一致；并与上一篇 CheckGameplayTagsOnActor 文章做交叉核对。

## 评分总览

| 维度 | 分数 (1-10) | 评价 |
|------|------------|------|
| 结构完整性 | 9/10 | 五段式 + 🟢🔵🔴 分层递进完整，阅读导航块齐全，对照表格清晰 |
| 分析深度 | 9/10 | 有状态/无状态对照、按需 Tick、边沿触发、Abort 语义裁剪、FValueOrBBKey 演进，每个"为什么"都有源码证据 |
| 可读性 | 9/10 | 状态机图 + 时序图 + 对照表，拟人化"门卫"贯穿，代码块均 ≤20 行 |
| AI痕迹清除 | 9/10 | 无作者技法标注，无 AI 高频英文词，中文语感自然 |
| 技术准确性 | 9/10 | 源码逐条比对通过，修正 2 处表述（详见下方） |
| **总分** | **45/50** | 通过线（42）以上，达到交付标准 |

## 具体问题与改进建议

| # | 位置 | 维度 | 问题 | 修改建议 |
|---|------|------|------|---------|
| 1 | §四.6 | 准确性 | 初稿写 `ToString()` 引用 `ValueOrBBKey.h` L94/L111-113，L94 是基类空实现、L111 是 Bool 的引用，定位不准 | 已改为 L102-113 的 `ToStringInternal`，语义描述更准确 |
| 2 | §五.6 局限表 | 准确性 | 初稿写"冷却时长包含暂停时间"，与 `GetTimeSeconds()` 实际语义矛盾（暂停时冻结，不包含暂停） | 已改为"暂停时冷却计时冻结，想用真实时间换 `GetRealTimeSeconds()`" |
| 3 | §五.2 | 可读性 | "教科书级性能技巧"略有夸大 | 保留（作为修辞可接受），未改动 |

## 源码验证结果（§9.6）

| # | 文章位置 | 引用代码 | 验证结果 | 修正方式 |
|---|---------|---------|---------|---------|
| 1 | §四.2 | `CalculateRawConditionValue`：`RecalcTime = Now - CD; return RecalcTime >= LastUseTimestamp` | 与 cpp L24-29 一致 | 无需修改 |
| 2 | §四.3 | `OnNodeDeactivation`：写 `LastUseTimestamp = Now` + 重置 `bRequestedRestart` | 与 cpp L31-36 一致 | 无需修改 |
| 3 | §四.4 | `TickNode`：`bRequestedRestart` 边沿触发 + `RequestExecution(this)` | 与 cpp L38-50 一致 | 无需修改 |
| 4 | §三.4 | `PostLoad`：`bNotifyTick = (FlowAbortMode != None)` | 与 cpp L18-22 一致 | 无需修改 |
| 5 | §四.5 | 内存三件套：`GetInstanceMemorySize`/`InitializeMemory`/`CleanupMemory` + `TNumericLimits::Lowest()` 初值 | 与 cpp L74-93 一致 | 无需修改 |
| 6 | §四.1 | `INIT_DECORATOR_NODE_NOTIFY_FLAGS()` 宏用 `std::is_same_v` 检测 override | 与 `BTDecorator.h` L129-152 一致 | 无需修改 |
| 7 | §四.3 | `FValueOrBBKey_Float` 的 `GetValue` 语义（键名非空读黑板，否则返回 DefaultValue） | 与 `ValueOrBBKey.h` L50-71、L281-308 一致 | 无需修改 |
| 8 | §四.6 | `GetStaticDescription` 拼 `lock for Xs...` + `DescribeRuntimeValues` 的 unlock/restart 分支 | 与 cpp L52-72 一致 | 无需修改 |
| 9 | §二.2 | `FValueOrBBKey_Float CoolDownTime` 默认 5.0f | 与 `BTDecorator_Cooldown.h` L27-28 及 cpp L12 一致 | 无需修改 |
| 10 | §三.2 | `FBTCooldownDecoratorMemory` 结构（`double LastUseTimestamp` + `uint8 bRequestedRestart`） | 与 `BTDecorator_Cooldown.h` L11-15 一致 | 无需修改 |
| 11 | §五.1 | "节点对象模板共享、运行时数据放 NodeMemory"（`UBTDecorator.h` L30-33 注释） | 与头文件注释一致 | 无需修改 |
| 12 | §五.3 | 基类默认 `bAllowAbortLowerPri = true` | 与 `BTDecorator.cpp` L14 一致 | 无需修改 |

> 核心原则：所有代码引用均能在 `_source-location-Cooldown.md` 定位到的源码中找到对应，无凭空发明。

## 画图验证结果（§9.7）

| # | 图文件 | 涉及代码元素 | 验证结果 | 修正方式 |
|---|--------|------------|---------|---------|
| 1 | `diagrams/state-cooldown.drawio` | 三状态 `Ready→Running→Cooling→Ready` 流转：条件通过进分支、`OnNodeDeactivation` 记时间戳、冷却结束回 Ready | 与 cpp 执行流一致 | 无需修改 |
| 2 | `diagrams/sequence-cooldown-lifecycle.drawio` | 7 消息调用链：`CalculateRawConditionValue`→读时间戳→`OnNodeDeactivation` 写时间戳→`[Abort] TickNode`→`RequestExecution` | 与 cpp 各函数调用顺序一致 | 无需修改 |

## 对比审查

| 维度 | 本篇（Cooldown） | 上一篇（CheckGameplayTagsOnActor） | 差距分析 |
|------|-----------------|------------------------------------|---------|
| 结构 | 五段式 + 分层，一致 | 同 | 一致，系列风格统一 |
| 深度 | 有状态/按需 Tick/边沿触发/Abort 裁剪 | 接口解耦/描述缓存/静默 fail | 各有侧重，互为补充 |
| 可读性 | 状态机图 + 时序图 | 类图 + 时序图 | 一致 |
| 准确性 | 逐条验证，2 处修正 | 逐条验证，3 处修正 | 一致 |

两篇形成"无状态纯查询 vs 有状态计时器"的对照叙事，系列线索清晰。
