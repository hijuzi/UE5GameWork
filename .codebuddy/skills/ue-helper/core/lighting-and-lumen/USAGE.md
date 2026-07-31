# lighting-and-lumen 使用指南

## 功能

配置 UE5 光照 —— 光照组件分类、Lumen 全局光照、阴影设置、曝光和后处理。

## 使用方式

```
"场景太暗了，怎么调亮？"
"Lumen 全局光照怎么开启？"
"怎么做昼夜循环？"
```

## 使用示例

### 示例 1：动态光照

> **你**: "创建一盏跟随玩家的手电筒光源"

> **AI**: 在 Character 上添加 `USpotLightComponent`，设置强度、颜色、衰减半径。

### 示例 2：Lumen 配置

> **你**: "室内光照不足，Lumen 怎么调？"

> **AI**: Project Settings → Rendering → Dynamic Global Illumination Method: Lumen → 调整 `LumenSceneLightingQuality`。

### 示例 3：昼夜循环

> **你**: "用 DirectionalLight 做日夜循环"

> **AI**: 旋转 DirectionalLight + 天空光 + 调整 SkyAtmosphere/SkyLight 配合 `UDirectionalLightComponent`。

## 光照组件

| 组件 | 用途 |
|------|------|
| DirectionalLight | 太阳/月光 |
| PointLight | 灯泡 |
| SpotLight | 手电筒/聚光灯 |
| SkyLight | 环境光补光 |
| RectLight | 面光源（窗户光） |

## 适用场景

- 场景光照布局
- Lumen GI 配置
- 昼夜循环
- 动态光源控制
