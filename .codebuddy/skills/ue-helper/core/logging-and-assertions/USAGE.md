# logging-and-assertions 使用指南

## 功能

在 UE5 C++ 中添加结构化日志和运行时断言，覆盖 UE_LOG、UE_LOGFMT、check/ensure/verify 选择。

## 使用方式

```
"给这个模块添加日志"
"这里要用 check 还是 ensure？"
"怎么过滤日志输出？"
```

## 使用示例

### 示例 1：定义日志分类

> **你**: "给我的 BattleSystem 模块加日志"

> **AI**: 在模块主头文件声明：
> ```cpp
> DECLARE_LOG_CATEGORY_EXTERN(LogBattle, Log, All);
> ```
> 在 .cpp 定义：
> ```cpp
> DEFINE_LOG_CATEGORY(LogBattle);
> ```

### 示例 2：选择断言类型

> **你**: "玩家血量必须 >0，用哪个断言？"
> ```cpp
> void TakeDamage(float Amount) {
>     // Health 不能 <= 0
> }
> ```

> **AI**:
> ```cpp
> void TakeDamage(float Amount) {
>     check(CurrentHealth > 0);  // 开发者 bug，一定要 crash
>     // ...
>     ensure(CurrentHealth >= 0);  // 可能发生但不该，记录+继续
> }
> ```

### 示例 3：结构化日志

> **你**: "记录技能释放，包含技能名和伤害值"

> **AI**:
> ```cpp
> UE_LOGFMT(LogBattle, Log, "Skill {SkillName} dealt {Damage} damage",
>     TEXT("Fireball"), 150.0f);
> ```

## 断言选择指南

| 断言 | 行为 | 适用场景 |
|------|------|---------|
| `check` | crash（Shipping 移除） | 开发者 bug，继续下去没意义 |
| `ensure` | 报告一次，不 crash | 不该发生但可恢复的错误 |
| `verify` | `check` 但表达式一定执行 | 表达式有副作用时必须执行 |

## 适用场景

- 为新模块定义日志分类
- 添加 UE_LOG/UE_LOGFMT 诊断
- check vs ensure 决策
- 筛选/静默特定日志分类
