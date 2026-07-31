# timers-and-async 使用指南

## 功能

调度和延迟执行工作 —— FTimerManager、FTimerHandle、AsyncTask、UE::Tasks、FRunnable。

## 使用方式

```
"3 秒后自动爆炸怎么做？"
"异步加载不阻塞主线程？"
"怎么做个每秒滴答的定时器？"
```

## 使用示例

### 示例 1：延迟执行

> **你**: "手雷抛出后 3 秒爆炸"

> **AI**:
> ```cpp
> GetWorldTimerManager().SetTimer(ExplodeTimer, this,
>     &AMyGrenade::Explode, 3.0f, false);
> ```

### 示例 2：循环定时器

> **你**: "每秒回复 1 点血量"

> **AI**:
> ```cpp
> GetWorldTimerManager().SetTimer(RegenTimer, this,
>     &AMyCharacter::RegenHealth, 1.0f, true);
> ```

### 示例 3：异步任务

> **你**: "处理大量寻路计算不卡主线程"

> **AI**:
> ```cpp
> AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]() {
>     // 后台计算
>     AsyncTask(ENamedThreads::GameThread, [this]() {
>         // 回主线程更新结果
>     });
> });
> ```

## 工具选择

| 工具 | 用途 |
|------|------|
| FTimerManager | 简单的延迟/循环 |
| AsyncTask | 后台任务 |
| UE::Tasks | 现代任务系统 |
| FRunnable | 长驻线程 |

## 适用场景

- 延迟/冷却逻辑
- 周期性更新
- 后台计算
