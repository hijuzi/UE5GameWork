# animation-system 使用指南

## 功能

驱动骨骼网格动画 —— AnimInstance / Animation Blueprint，状态机，混合空间，AnimNotify，动画分层。

## 使用方式

```
"帮我做一个角色的行走/跑步混合动画"
"怎么在动画中触发攻击判定？"
"动画蓝图怎么获取 C++ 变量？"
```

## 使用示例

### 示例 1：动画蓝图获取 C++ 速度

> **你**: "动画蓝图要知道角色速度来切换走/跑"

> **AI**: 在 C++ Character 中暴露：
> ```cpp
> UFUNCTION(BlueprintPure)
> float GetSpeed() const { return GetVelocity().Size2D(); }
> ```

### 示例 2：AnimNotify 触发攻击判定

> **你**: "挥刀动画中第 15 帧开始伤害判定"

> **AI**: 在动画资源添加 UAnimNotify，Notify 事件中调用 `ACharacter::DoDamageCheck()`。

### 示例 3：分层动画（上半身射击）

> **你**: "走跑用下半身，上半身单独播放射击"

> **AI**: AnimBP 中用 Layered Blend Per Bone，上半身槽位播放射击蒙太奇。

## 核心组件

| 组件 | 用途 |
|------|------|
| AnimInstance | C++ 动画逻辑（蓝图父类） |
| Blend Space | 多轴混合（速度+方向） |
| State Machine | 状态切换（Idle/Walk/Run） |
| AnimMontage | 一次性动画播放（攻击、技能） |
| AnimNotify | 动画时间线事件 |
| IK Rig | 反向运动学 |

## 适用场景

- 动画蓝图状态机搭建
- 混合空间和过渡配置
- AnimNotify 触发逻辑
- 动画分层和 IK
