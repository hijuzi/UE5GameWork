---
module: camera
purpose: Stack-based camera mode blending with smooth transitions and penetration avoidance
complexity: Medium
loc: 1690
file_count: 12
---

# Camera

## 用途

可堆叠的相机模式混合系统，提供平滑过渡、穿透避免和基于 GameplayTag 的模式选择机制。

## 架构

### 数据流

```
ALyraPlayerCameraManager::UpdateViewTarget()
  → ULyraCameraComponent::GetCameraView()
    → DetermineCameraModeDelegate (HeroComponent 查询)
    → CameraModeStack::PushCameraMode()
    → EvaluateStack() + BlendStack()
    → FLyraCameraModeView (Location/Rotation/FOV)
```

### CameraMode Stack

```
Stack Bottom → Mode A (BlendWeight=0.2)
            → Mode B (BlendWeight=0.8)  ← 最新
Stack Top    → 最终混合 POV
```

- 每个 CameraMode 独立计算自己的 View
- Stack 从底向上按 BlendWeight 混合
- 新 Mode Push 时自动 BlendIn，旧 Mode BlendOut 后移除

## 关键类

| 类 | 职责 |
|----|------|
| `ALyraPlayerCameraManager` | 全局相机管理，UI 相机优先级 |
| `ULyraCameraComponent` | 持有 CameraModeStack，暴露模式选择委托 |
| `ULyraCameraMode` | CameraMode 基类（混合参数、FOV、过渡） |
| `ULyraCameraMode_ThirdPerson` | 第三人称穿透避免实现 |
| `ULyraCameraModeStack` | 混合栈管理 |

### 穿透避免

`ULyraCameraMode_ThirdPerson` 实现：
- 多个 Feeler 射线检测
- 碰撞通道过滤
- `bPreventPenetration` 平滑返回延迟
- `bDoPredictiveAvoidance` 蹲伏偏移预测

## 设计模式

- **策略/栈混合**: 多 CameraMode 按权重混合
- **委托查询**: `DetermineCameraModeDelegate` 松耦合模式选择

## 依赖

Engine/Camera, GameplayAbilities, GameplayTags
