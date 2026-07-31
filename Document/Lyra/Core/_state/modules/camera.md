---
module: camera
purpose: Stack-based camera mode blending system with smooth transitions, penetration avoidance, and GameplayTag-driven mode selection.
roots:
  - "LyraStarterGame/Source/LyraGame/Camera/"
complexity: Medium
loc: 1690
file_count: 12
deps: [Engine/Camera, GameplayAbilities, GameplayTags]
escalate: false
---

## Architecture <!-- c2d:s1 -->

Lyra 相机系统基于**可堆叠 CameraMode 混合架构**。

### 数据流

```
ALyraPlayerCameraManager::UpdateViewTarget()
  → ULyraCameraComponent::GetCameraView()
    → DetermineCameraMode (从 LyraHeroComponent 查询)
    → CameraModeStack::PushCameraMode()
    → EvaluateStack() → BlendStack()
    → FLyraCameraModeView (Location, Rotation, FOV)
```

### 类关系

| 类 | 继承 | 职责 |
|----|------|------|
| `ALyraPlayerCameraManager` | APlayerCameraManager | 全局相机管理，UI 相机优先级 |
| `ULyraCameraComponent` | UCameraComponent | 持有 CameraModeStack，暴露委托 |
| `ULyraCameraMode` | UObject | CameraMode 基类，混合参数 |
| `ULyraCameraMode_ThirdPerson` | ULyraCameraMode | 第三人称穿透避免 |
| `ULyraCameraModeStack` | UObject | 混合栈管理 |

### 穿透避免

`ULyraCameraMode_ThirdPerson` 实现完整的射线穿透检测，支持：
- 多个 Feeler 射线
- 碰撞通道过滤
- 平滑返回延迟 (`bPreventPenetration`)
- 蹲伏偏移自动调整 (`bDoPredictiveAvoidance`)

## Key Files

| 文件 | 重要性 |
|------|--------|
| `LyraCameraComponent.h/.cpp` | 相机组件核心 |
| `LyraCameraMode.h/.cpp` | CameraMode 基类 |
| `LyraCameraMode_ThirdPerson.h/.cpp` | 第三人称实现 |
| `LyraPlayerCameraManager.h/.cpp` | 全局相机管理 |
