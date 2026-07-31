---
module: core-system-overview
---

# System Overview — Core

## 架构概述

核心游戏代码层由 5 个模块组成，形成以 **Character** 为物理中心、**AbilitySystem** 为逻辑中心的星型架构：

```
                   ┌──────────────┐
                   │   Camera     │
                   │   (视角)      │
                   └──────┬───────┘
                          │ GetCameraView()
┌──────────┐    ┌─────────▼──────────┐    ┌──────────────┐
│  Input   │───▶│     Character       │───▶│ Animation    │
│ (玩家操作)│    │  (PawnData 驱动)     │    │ (标签映射)    │
└──────────┘    └─────────┬──────────┘    └──────────────┘
                          │ ASC
                   ┌──────▼───────┐
                   │ AbilitySystem│
                   │ (GAS 封装)    │
                   └──────────────┘
```

## 核心设计决策

1. **ASC 在 PlayerState 上**: 不在 Character 上直接持有 ASC，使 ASC 在 Pawn 死亡/重生过程中持久化。
2. **PawnData 外部化配置**: 所有角色配置（PawnClass、AbilitySet、InputConfig、CameraMode）定义在 DataAsset 中，支持不同职业/NPC 无代码配置。
3. **InitState 状态机**: 使用 GameFrameworkComponentManager 的四阶段初始化链（Spawned → DataAvailable → DataInitialized → GameplayReady），确保所有组件按正确顺序初始化。
4. **GameplayTag 作为通用标识符**: 输入路由、能力关联、动画变量映射、失败消息路由全部使用 GameplayTag。
5. **Determiner 委托模式**: CameraMode 选择通过可替换的委托 `DetermineCameraModeDelegate`，而非硬编码查询。
