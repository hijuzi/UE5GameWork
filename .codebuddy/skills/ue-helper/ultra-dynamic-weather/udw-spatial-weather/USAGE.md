# udw-spatial-weather 使用指南

## 功能

区域天气覆盖、径向风暴、天气遮罩（Weather Mask）。

## 使用方式

```
"只在下雨区域有雨，别的区域晴天"
"风暴眼怎么做？中心晴，外围暴雨"
```

## 使用示例

> **你**: "山上暴风雪，山脚晴天"

> **AI**: UDW → Weather Mask → 在山上绘制 Weather Mask 区域 → 该区域内启用 Snow。

> **你**: "做台风风暴眼效果"

> **AI**: 使用 `Radial Storm` → 设置中心 Clear 半径 → 外围 Rain/Snow → 风暴随时间移动。

## 空间控制

| 功能 | 说明 |
|------|------|
| Weather Mask | 区域天气贴图 |
| Radial Storm | 圆形风暴 |
| Local Weather | 局部天气覆盖 |

## 适用场景

- 区域差异化天气
- 动态风暴系统
