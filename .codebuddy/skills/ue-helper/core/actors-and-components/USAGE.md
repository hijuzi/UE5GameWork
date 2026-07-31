# actors-and-components 使用指南

## 功能

构建 Actor 和 Component 组合模式 —— AActor 生命周期、组件类型、层次附加、生成和注册。

## 使用方式

```
"帮我创建一个带生命值的敌人 Actor"
"怎么在运行时动态添加组件？"
"BeginPlay 和构造函数里做什么？"
```

## 使用示例

### 示例 1：创建组合式 Actor

> **你**: "做一个可拾取物品，有球形碰撞和动画"

> **AI**: 创建 AItem Actor，添加 USphereComponent（碰撞检测）和 UStaticMeshComponent（视觉表示），而非继承深层次类。

### 示例 2：运行时动态组件

> **你**: "敌人中弹后给它加个着火特效组件"

> **AI**:
> ```cpp
> UFireEffectComponent* Fire = NewObject<UFireEffectComponent>(this);
> Fire->RegisterComponent();
> ```

### 示例 3：生命周期调试

> **你**: "为什么我的 Tick 不运行？"

> **AI**: 检查 `PrimaryActorTick.bCanEverTick = true` 且 `bStartWithTickEnabled = true`。

## 生命周期顺序

```
构造 → PostInitializeComponents → BeginPlay → Tick → EndPlay → Destroyed
```

## 适用场景

- 创建任意 Actor 或 Component
- 设置组件层次和附加关系
- 动态生成组件
- 调试生命周期/Tick/碰撞问题
