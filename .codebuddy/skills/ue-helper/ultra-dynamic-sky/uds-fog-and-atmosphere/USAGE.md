# uds-fog-and-atmosphere 使用指南

## 功能

配置雾、体积雾、大气散射 —— Fog Density、Volumetric Fog、Sky Atmosphere、全局体积材质。

## 使用方式

```
"雾太浓了，看不清远处"
"地面雾怎么加？"
"天空色调偏蓝怎么调？"
```

## 使用示例

> **你**: "清晨加一层地面薄雾，远处群山有雾感"

> **AI**: UDS → Fog → `Fog Density = 0.3` → `Ground Fog = true` → 调整高度衰减。

> **你**: "黄昏时天空偏橙红色"

> **AI**: UDS → Sky Atmosphere → 调整 Rayleigh Scattering 颜色 → 增加红色分量。

## 雾参数

| 参数 | 效果 |
|------|------|
| Fog Density | 整体雾浓度 |
| Height Falloff | 高度衰减（地面雾） |
| Start Distance | 起始雾距离 |
| Volumetric Fog | 体积雾光散射 |
| Dust | 沙尘效果 |

## 适用场景

- 清晨/黄昏雾气
- 沙尘暴
- 体光效果
