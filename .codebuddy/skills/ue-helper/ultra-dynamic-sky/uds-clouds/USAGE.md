# uds-clouds 使用指南

## 功能

配置 UDS 云层 —— 体积云/静态云/2D 动态云/Voxel 云、Cloud Painter、Light Rays。

## 使用方式

```
"云看起来太低了，怎么升高？"
"加第二层高空云"
"Cloud Painter 怎么画特定区域云？"
```

## 使用示例

> **你**: "云层高度提到 5km，加一些卷云"

> **AI**: `Cloud Altitude = 5000` → Enable Second Layer → 设置高空薄云参数。

> **你**: "用 Cloud Painter 在山顶画浓云"

> **AI**: Cloud Painter Tool → 选择 Cloud Coverage 画笔 → 在山顶区域涂抹高覆盖值。

## 云类型选择

| 类型 | 特点 |
|------|------|
| Volumetric | 3D 体积渲染，写实 |
| Static | 静态贴图，性能高 |
| 2D Dynamic | 2D 可移动，移动端可用 |
| Voxel | 体素风格 |

## 适用场景

- 天气云层变化
- 高空云层分层
- 区域云密度绘制
