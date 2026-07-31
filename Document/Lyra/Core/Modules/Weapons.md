---
module: weapons
complexity: High
loc: 2289
file_count: 16
---

# Weapons

## 架构

Lyra 武器系统基于双层实例 + GAS 技能分离架构：

```
ULyraWeaponInstance (数据层)  ←──  ULyraGameplayAbility_RangedWeapon (行为层)
    │                                        │
    ├─ 热量/散布曲线 (RuntimeFloatCurve)       ├─ 6 种瞄准源 (Camera/PWeapon/Pawn)
    ├─ AnimationLayer 选择                    ├─ Ray/Sweep Trace 弹道
    ├─ 输入设备属性 (触觉/扳机)                 ├─ 材质/距离 伤害修正
    └─ 拾取定义 (WeaponSpawner)               └─ Server-authoritative HitMarker
```

## 关键类

| 类 | 职责 |
|----|------|
| `ULyraWeaponInstance` | 武器实例基类，管理生命周期/动画层/输入设备 |
| `ULyraRangedWeaponInstance` | 远程武器：3 条 RuntimeFloatCurve 驱动散布/热量系统 |
| `ULyraGameplayAbility_RangedWeapon` | 发射技能：6 种瞄准模式，Ray/Sweep 弹道，去重 |
| `ULyraWeaponStateComponent` | ControllerComponent：HitMarker 复制/预测 |
| `ALyraWeaponSpawner` | 世界拾取点：重叠 → 授予库存物品，冷却重生 |

## 设计模式

- **分离数据和行为**: Instance（数据配置）+ Ability（执行逻辑）
- **illegalSourceInterface**: 距离/物理材质伤害修正
- **FastArray 复制**: HitMarker 通过 `ClientConfirmTargetData` RPC 客户端预测+服务器确认
