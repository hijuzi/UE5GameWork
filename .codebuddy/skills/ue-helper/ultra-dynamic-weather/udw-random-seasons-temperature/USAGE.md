# udw-random-seasons-temperature 使用指南

## 功能

随机天气变化、季节循环、温度系统。

## 使用方式

```
"天气随机变化，不是每天都晴天"
"冬季下雪、夏季雷雨，按季节来"
```

## 使用示例

> **你**: "模拟温带气候：冬季雪，春季雨，夏季晴"

> **AI**: UDW → Enable Seasons = true → Season = Winter → 设置 `Temperature Range = -10 ~ 5°C` → 雪概率 80%。

> **你**: "天气随机，但不要连着 3 天暴雨"

> **AI**: 设置 `Random Weather` → `Max Consecutive Rain Days = 2`。

## 系统参数

| 参数 | 说明 |
|------|------|
| Season | 当前季节 |
| Temperature | 温度（影响雨/雪类型） |
| Random Interval | 天气变化间隔 |
| Weather Probability | 各天气概率权重 |

## 适用场景

- 生存游戏天气
- 模拟经营季节
