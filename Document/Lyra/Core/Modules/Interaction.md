---
module: interaction
complexity: Medium
loc: 1107
file_count: 17
---

# Interaction

## 架构

GAS 驱动的交互系统：

```
接口层: IInteractableTarget (GatherInteractionOptions)
       IInteractionInstigator (ChooseBestInteractionOption)
       FInteractionOptionBuilder (构建器)

任务层: UAbilityTask_GrantNearbyInteraction (Sphere重叠, 预授予能力)
       UAbilityTask_WaitForInteractableTargets_SingleLineTrace (LineTrace轮询)
       └── UpdateInteractableOptions() (解析 → 过滤 CanActivateAbility → 变化检测 → 广播)

能力层: ULyraGameplayAbility_Interact (ActivationPolicy=OnSpawn)
       └── TriggerInteraction() → Target ASC.TriggerAbilityFromGameplayEvent

UI层:  ULyraIndicatorManagerComponent (屏幕空间提示)
```

## 双交互模式

1. **Grant-to-instigator**: 向玩家 ASC 授予能力类
2. **Trigger-on-target**: 通过 GameplayEvent 触发目标 ASC 上已有的能力

## 关键类

| 类 | 职责 |
|----|------|
| `IInteractableTarget` | 交互对象合约：收集选项 + 自定义事件数据 |
| `ULyraGameplayAbility_Interact` | 主能力：激活近距离扫描 + 更新 UI 指示器 + 触发交互 |
| `UAbilityTask_GrantNearbyInteraction` | Sphere 轮询：预授予能力给 ASC |
| `UAbilityTask_WaitForInteractableTargets` | 抽象基类：变化检测 + 广播 |
| `UInteractionStatics` | 蓝图库：从物理查询提取交互对象 |

## 设计模式

- **接口隔离**: 两个窄接口 (Interactable + Instigator)
- **Builder**: FInteractionOptionBuilder 构建选项数组
- **轮询**: Timer 驱动而非事件驱动，控制扫描频率
- **变化检测**: 排序+比较，仅变化时广播
