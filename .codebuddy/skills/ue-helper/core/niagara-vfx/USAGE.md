# niagara-vfx 使用指南

## 功能

创建和控制 Niagara 粒子特效 —— UNiagaraSystem、UNiagaraComponent、参数控制、事件系统。

## 使用方式

```
"帮我做一个爆炸粒子特效"
"怎么在 C++ 中控制粒子参数？"
"Niagara 事件怎么触发？"
```

## 使用示例

### 示例 1：C++ 生成粒子

> **你**: "命中敌人时在命中点播放火花粒子"

> **AI**:
> ```cpp
> UNiagaraFunctionLibrary::SpawnSystemAtLocation(
>     GetWorld(), SparkEffect, Hit.Location, FRotator::ZeroRotator);
> ```

### 示例 2：控制粒子参数

> **你**: "武器充能时粒子亮度随充能进度变化"

> **AI**: 获取 UNiagaraComponent → `SetVariableFloat("ChargeAmount", Progress)`。

### 示例 3：粒子事件触发伤害

> **你**: "火球爆炸粒子扩散到敌人时造成伤害"

> **AI**: Niagara 中用 Generate Location Event 模块 → C++ 中 `OnSystemFinished` 或自定义绑定处理碰撞。

## Niagara vs Cascade

| | Niagara | Cascade |
|---|---------|---------|
| 架构 | 模块化、数据驱动 | 传统节点式 |
| 性能 | 更优（GPU 模拟） | CPU 为主 |
| 复杂度 | 中等 | 入门简单 |
| 推荐 | ✅ UE5 主推 | 遗留系统 |

## 适用场景

- 技能/攻击特效
- 环境效果（雨、雪、火）
- GPU 大规模粒子
