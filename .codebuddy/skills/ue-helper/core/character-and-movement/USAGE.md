# character-and-movement 使用指南

## 功能

实现玩家/AI 角色 —— ACharacter + UCharacterMovementComponent，覆盖移动模式、跳跃、蹲伏、Root Motion、客户端预测网络移动。

## 使用方式

```
"帮我创建一个玩家角色，能跑能跳"
"怎么自定义移动模式（飞行/游泳）？"
"网络移动延迟大怎么优化？"
```

## 使用示例

### 示例 1：基础角色设置

> **你**: "创建一个第三人称角色，走路 600，跳跃 420"

> **AI**:
> ```cpp
> AMyCharacter::AMyCharacter() {
>     GetCharacterMovement()->MaxWalkSpeed = 600.0f;
>     GetCharacterMovement()->JumpZVelocity = 420.0f;
> }
> ```

### 示例 2：自定义移动模式

> **你**: "加一个滑翔移动模式"

> **AI**: 重写 `PhysCustom()` 处理自定义重力/加速度逻辑。

### 示例 3：Root Motion 攻击动画

> **你**: "攻击动画需要 Root Motion 驱动位移"

> **AI**: 在动画资源启用 Root Motion，代码中 `bAllowPhysicsRotationDuringAnimRootMotion` 防止物理干扰。

## 核心配置

| 属性 | 说明 |
|------|------|
| MaxWalkSpeed | 最大行走速度 |
| JumpZVelocity | 跳跃初速度 |
| GravityScale | 重力缩放 |
| AirControl | 空中可控度 |
| MovementMode | 步行/下落/飞行/游泳/自定义 |

## 适用场景

- 创建或配置 Character 类
- 设置移动速度/重力/空中控制
- 实现自定义移动模式
- 添加 Root Motion
- 调试网络移动平滑和预测
