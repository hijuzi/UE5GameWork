# physics-and-chaos 使用指南

## 功能

实现碰撞、物理模拟和世界查询 —— Chaos 物理、碰撞通道、射线检测、重叠检测、物理约束。

## 使用方式

```
"帮我做一个射线检测，判断前方是否有敌人"
"两个物体碰撞怎么接收事件？"
"怎么做球形范围检测？"
```

## 使用示例

### 示例 1：射线检测

> **你**: "用射线检测前方 1000 单位内第一个可攻击的目标"

> **AI**:
> ```cpp
> FHitResult Hit;
> bool bHit = GetWorld()->LineTraceSingleByChannel(
>     Hit, Start, End, ECC_Visibility);
> if (bHit && Hit.GetActor()) { /* 命中 */ }
> ```

### 示例 2：重叠检测

> **你**: "检测角色 500 范围内的所有敌人"

> **AI**:
> ```cpp
> TArray<FOverlapResult> Overlaps;
> FCollisionQueryParams Params;
> Params.AddIgnoredActor(this);
> GetWorld()->OverlapMultiByObjectType(
>     Overlaps, Location, FQuat::Identity,
>     FCollisionObjectQueryParams::AllDynamicObjects,
>     FCollisionShape::MakeSphere(500.0f), Params);
> ```

### 示例 3：碰撞事件

> **你**: "子弹击中敌人触发伤害"

> **AI**: `OnComponentHit` 委托绑定，在回调中 `Cast<AEnemy>(Hit.GetActor())->TakeDamage()`。

## 碰撞通道设置

| 通道 | 用途 |
|------|------|
| WorldStatic | 静态世界几何体 |
| WorldDynamic | 可移动物体 |
| Pawn | 角色 |
| Visibility | 可见性检测 |
| Camera | 相机遮挡 |
| Custom | 自定义通道 |

## 适用场景

- 射线/球形/盒形检测
- 物理碰撞事件处理
- 物理约束（门、绳索）
- 碰撞通道配置
