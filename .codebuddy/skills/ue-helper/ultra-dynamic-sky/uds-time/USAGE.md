# uds-time 使用指南

## 功能

控制 UDS 时间 —— 时间推进、时间段设置、时间动画化与 Sequencer。

## 使用方式

```
"怎么让一天 24 小时循环？"
"时间流速可以改成现实 2 倍吗？"
```

## 使用示例

> **你**: "设置 UDS 时间流速为 10 分钟 = 24 小时"

> **AI**: UDS 蓝图中设置 `Time Multiplier = 144.0`（144 个 10 分钟 = 24 小时）。

> **你**: "清晨 6 点开始，时间自动推进"

> **AI**: 设置 `Start Time = 6.0`，确保 `Enable Time Progression = true`。

## 关键参数

| 参数 | 说明 |
|------|------|
| Time of Day | 当前时间（0-24） |
| Time Multiplier | 时间流速倍数 |
| Enable Time Progression | 是否自动推进 |

## 适用场景

- 昼夜循环
- 时间流速控制
