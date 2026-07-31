# control-rig-and-ik 使用指南

## 功能

程序化动画和反向运动学 —— Control Rig 骨骼控制器、IK Rig、Full Body IK、Foot IK。

## 使用方式

```
"帮我实现角色脚部贴合地面的 IK"
"怎么用 Control Rig 做程序化脊柱弯曲？"
"IK 瞄准怎么做？"
```

## 使用示例

### 示例 1：Foot IK

> **你**: "角色站在斜坡上脚陷进去了，怎么让脚贴合地面？"

> **AI**: 在 AnimBP 中使用 `FABRIK` 或 `Two Bone IK` 节点，射线检测地面高度，调整 IK 目标位置。

### 示例 2：瞄准 IK

> **你**: "角色需要看向目标物体"

> **AI**: Control Rig 中用 Aim 约束，将目标位置传入动画蓝图。

### 示例 3：呼吸动画

> **你**: "用 Control Rig 做角色站立时的轻微呼吸起伏"

> **AI**: Control Rig 中用正弦函数驱动脊柱骨骼旋转，叠加到基础姿态。

## 核心概念

| 工具 | 用途 |
|------|------|
| IK Rig | 定义 IK 骨骼链和解算器 |
| IK Retargeter | 不同骨骼间动画重定向 |
| Control Rig | 蓝图式骨骼操作 |
| Full Body IK | 全身多约束 IK |

## 适用场景

- 脚部地形贴合
- 手部抓取 IK
- 程序化脊椎/脖子动画
- 不同骨骼动画重定向
