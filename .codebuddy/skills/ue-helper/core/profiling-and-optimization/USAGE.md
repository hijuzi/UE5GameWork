# profiling-and-optimization 使用指南

## 功能

性能分析和优化 —— Unreal Insights、stat 命令、GPU Visualizer、ProfileGPU、蓝图优化。

## 使用方式

```
"帧率太低，帮我分析瓶颈在哪"
"这个 Tick 函数太慢了，怎么优化？"
"蓝图怎么转 C++ 提升性能？"
```

## 使用示例

### 示例 1：Stat 命令分析

> **你**: "运行时帧率只有 30，怎么查？"

> **AI**: 控制台输入 `stat fps` → `stat unit`（看 Game/Draw/GPU 占比）→ `stat game`（看 Tick 开销）→ `stat gpu`（渲染开销）。

### 示例 2：Unreal Insights 追踪

> **你**: "启动加载太慢，用 Insights 分析"

> **AI**: `-trace=cpu,loadtime` 启动 → 打开 UnrealInsights 查看加载时间线。

### 示例 3：Tick 优化

> **你**: "NPC 每帧检测附近玩家，很卡"

> **AI**: `SetActorTickInterval(0.1f)` 每 0.1 秒检测一次，或用 `FTimerManager` 替代 Tick。

## 性能工具

| 工具 | 用途 |
|------|------|
| stat fps / stat unit | 快速查看帧率瓶颈 |
| stat game | Game 线程耗时细节 |
| Unreal Insights | 深度时间线分析 |
| ProfileGPU | GPU 渲染分析 |
| stat memory | 内存使用 |

## 适用场景

- 帧率优化
- 内存分析
- 蓝图转 C++
- 加载时间优化
