# nanite-and-rendering 使用指南

## 功能

配置 Nanite 虚拟化几何体 —— FMeshNaniteSettings、Nanite 静态/骨骼网格、渲染管线配置。

## 使用方式

```
"怎么开启 Nanite？"
"Nanite 对性能有什么影响？"
"骨骼网格能用 Nanite 吗？"
```

## 使用示例

### 示例 1：开启 Nanite

> **你**: "这个高面数建筑模型用 Nanite 优化"

> **AI**: 选中 StaticMesh 资产 → Details → Enable Nanite Support → Apply。

### 示例 2：C++ 中开启 Nanite

> **你**: "代码中怎么给网格启用 Nanite？"

> **AI**:
> ```cpp
> FMeshNaniteSettings Settings;
> Settings.bEnabled = true;
> StaticMesh->GetNaniteSettings(); // 或通过 FMeshBuildSettings
> ```

### 示例 3：性能对比

> **你**: "Nanite 场景为什么反而卡？"

> **AI**: 检查是否有 WPO（World Position Offset）材质、大量透明材质、过度复杂的着色器等不支持 Nanite 的情况。

## Nanite 限制

| 支持 | 不支持 |
|------|--------|
| 静态网格（不透明） | 透明材质/遮罩 |
| 骨骼网格（5.5+） | WPO 材质 |
| LOD 自动 | 双面渲染 |

## 适用场景

- 高面数建筑/雕塑
- 开放世界场景
- Nanite 性能优化
