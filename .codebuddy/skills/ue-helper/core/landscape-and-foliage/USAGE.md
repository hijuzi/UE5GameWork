# landscape-and-foliage 使用指南

## 功能

地形和植被 —— Landscape、Foliage (instanced)、HISM、Grass Tools、PCG (Procedural Content Generation)。

## 使用方式

```
"帮我做个地形，生成草地和树木"
"大世界草太卡怎么优化？"
"PCG 怎么自动生成岩石分布？"
```

## 使用示例

### 示例 1：创建地形

> **你**: "从高度图生成地形"

> **AI**: Landscape Mode → Import Heightmap → 设置 Section Size 和每个组件的分辨率 → 应用材质 → 用 Sculpt 工具雕刻细节。

### 示例 2：HISM 高性能植被

> **你**: "10 万棵树怎么不卡？"

> **AI**: 用 `AHierarchicalInstancedStaticMeshActor` 替代单独放置的 StaticMesh，GPU 批量绘制。

### 示例 3：PCG 程序化生成

> **你**: "在地形坡地上自动放岩石"

> **AI**: PCG Graph → Surface Sampler → Transform → Point Filter (Normal.Z 小于阈值 = 坡地) → StaticMesh Spawner 生成岩石。

## 核心组件

| 组件 | 用途 |
|------|------|
| Landscape | 地形 |
| Foliage Tool | 笔刷放置植被 |
| HISM / ISM | 实例化网格（高性能） |
| Grass Type | 地形草系统 |
| PCG | 程序化内容生成 |

## 适用场景

- 开放世界地形
- 大规模植被
- 程序化生成内容
