# automation-and-testing 使用指南

## 功能

编写和运行 UE 自动化测试 —— 简单/复杂自动化测试、Latent 测试、Functional Test、Gauntlet。

## 使用方式

```
"帮我给这个健康系统写单元测试"
"怎么在 CI 中跑自动化测试？"
"关卡流程怎么自动化测试？"
```

## 使用示例

### 示例 1：单元测试

> **你**: "测试血量不会超过最大值的逻辑"

> **AI**:
> ```cpp
> IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHealthClampTest,
>     "Game.Health.Clamp", EAutomationTestFlags::EditorContext)
> bool FHealthClampTest::RunTest(const FString&) {
>     // SetMaxHealth(100)
>     // Heal(200)
>     // TestEqual(CurrentHealth, 100.0f)
>     return true;
> }
> ```

### 示例 2：Latent 测试

> **你**: "测试生成敌人后 3 秒内是否开始巡逻"

> **AI**: 用 `ADD_LATENT_AUTOMATION_COMMAND` 延迟断言。

### 示例 3：Functional Test

> **你**: "测试 AI 是否能走到目标点"

> **AI**: 放置 `AFunctionalTest` Actor，设定导航路径点，自动验证。

## 测试类型

| 类型 | 用途 |
|------|------|
| Simple Automation | 快速单元测试 |
| Complex Automation | 需要参数化的测试 |
| Latent Automation | 需要等待帧/时间的测试 |
| Functional Test | 关卡内行为测试 |
| Gauntlet | CI 大规模回归测试 |

## 适用场景

- 核心逻辑单元测试
- AI 寻路测试
- CI 自动化回归
