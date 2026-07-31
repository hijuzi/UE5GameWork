# uds-cinematics-rendering 使用指南

## 功能

UDS 过场动画 —— Sequencer 关键帧、Movie Render Queue (MRQ)、Path Tracer。

## 使用方式

```
"怎么用 Sequencer 做天空从白天变夜晚？"
"过场渲染需要影视级画质"
```

## 使用示例

> **你**: "用 Sequencer 做时间流逝动画：24 小时内从日出到日落"

> **AI**: Sequencer 添加 UDS Actor → 展开 Time of Day → 第 0 帧设置 6:00，第 500 帧设置 18:00 → 添加 Cloud Coverage 关键帧。

> **你**: "渲染 4K 高质量过场"

> **AI**: Movie Render Queue → 添加 UDS 渲染配置 → 启用 `Adjust for Path Tracer` → 高质量输出设置。

## 关键工作流

| 步骤 | 说明 |
|------|------|
| 添加 UDS 到 Sequencer | 追踪 UDS Actor |
| 关键帧 Time of Day | 时间流动 |
| MRQ 配置 | 高质量渲染设置 |
| Path Tracer 适配 | 影视级光照 |

## 适用场景

- 过场动画天空变化
- 影视级渲染
