# udw-setup-and-state 使用指南

## 功能

安装配置 Ultra Dynamic Weather (UDW)，控制雨雪天气，改变天气状态。

## 使用方式

```
"帮我装 UDW 并设置下雨效果"
"运行时切换到暴风雪天气"
"不同区域不同天气怎么做？"
```

## 使用示例

> **你**: "设置小雨，雨量 0.3，无闪电"

> **AI**: UDW → `Precipitation Type = Rain` → `Precipitation Amount = 0.3` → `Lightning = false`。

> **你**: "5 秒内从晴天过渡到大雨"

> **AI**: `Transition Duration = 5.0` → 切换目标天气 State。

## 天气状态

| 类型 | 说明 |
|------|------|
| Clear | 晴天 |
| Rain | 雨（可调节强度） |
| Snow | 雪 |
| Transition | 过渡中 |

## 适用场景

- UDW 初始设置
- 天气切换
