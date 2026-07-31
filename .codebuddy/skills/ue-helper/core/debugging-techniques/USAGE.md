# debugging-techniques 使用指南

## 功能

调试 UE5 C++ 和 Gameplay 代码 —— 原生调试器、DrawDebug、Visual Logger、Gameplay Debugger、on-screen 消息。

## 使用方式

```
"这个变量值不对，怎么可视化输出？"
"AI 行为不稳定，怎么回放查看？"
"给这个系统加 in-game 调试面板"
```

## 使用示例

### 示例 1：可视化调试

> **你**: "我想在场景里画一个射线，看看枪口指向哪里"

> **AI**:
> ```cpp
> DrawDebugLine(GetWorld(), Start, End, FColor::Red,
>     false, 2.0f, 0, 2.0f);
> ```

### 示例 2：屏幕调试消息

> **你**: "在屏幕上显示当前血量方便调试"

> **AI**:
> ```cpp
> GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green,
>     FString::Printf(TEXT("Health: %.0f"), CurrentHealth));
> ```

### 示例 3：Visual Logger 回放 AI 行为

> **你**: "AI 有时不追击玩家，帮我加 Visual Logger 诊断"

> **AI**: 使用 `UE_VLOG` 记录 AI 决策关键点，可在编辑器中时间轴回放查看。

## 工具选择

| 问题类型 | 工具 |
|---------|------|
| 某行是否执行 / 变量值 | `UE_LOG` / `ensure` |
| 世界空间中的位置/线/球 | `DrawDebug*` 辅助函数 |
| 过去 N 秒发生了什么 | Visual Logger |
| AI/Gameplay 系统当前在想什么 | Gameplay Debugger |
| 崩溃 / 单步跟踪 | 原生调试器 (VS/Rider) |

## 适用场景

- 诊断错误行为
- 可视化轨迹/范围/AI 状态
- 回放间歇性 bug
- 给自定义系统添加 in-game 调试面板
