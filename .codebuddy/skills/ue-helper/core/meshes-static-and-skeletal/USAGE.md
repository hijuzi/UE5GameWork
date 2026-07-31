# meshes-static-and-skeletal 使用指南

## 功能

操作静态和骨骼网格 —— UStaticMesh + UStaticMeshComponent、USkeletalMesh + USkeletalMeshComponent、LOD、Socket。

## 使用方式

```
"怎么运行时换武器的 StaticMesh？"
"骨骼网格上怎么挂载武器（Socket）？"
"LOD 怎么配置？"
```

## 使用示例

### 示例 1：运行时换模型

> **你**: "角色装备不同的帽子，换模型"

> **AI**:
> ```cpp
> HatComponent->SetStaticMesh(HatMesh);
> ```

### 示例 2：Socket 挂载

> **你**: "武器挂载到角色右手 Socket"

> **AI**: 在骨骼网格中定义 `hand_r` Socket → `Weapon->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "hand_r")`。

### 示例 3：LOD 配置

> **你**: "50 米外降低模型面数"

> **AI**: StaticMesh Editor → LOD Settings → 配置 LOD0（最近）/ LOD1（中距）/ LOD2（远距）面数和屏幕尺寸。

## 组件类型

| 组件 | 用于 |
|------|------|
| UStaticMeshComponent | 静态网格（石头、建筑） |
| USkeletalMeshComponent | 骨骼网格（角色、动物） |
| UInstancedStaticMeshComponent | 大量相同物体（草、树） |

## 适用场景

- 运行时换模型
- Socket 挂载
- LOD 性能优化
