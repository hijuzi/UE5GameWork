---
module: topdownarena
complexity: Low-Medium
loc: 150
file_count: 10
---

# TopDownArena

> 插件路径：`LyraStarterGame/Plugins/GameFeatures/TopDownArena/Source/`

## 概述

俯视竞技场（Bomberman-like）玩法 GameFeature 插件。展示如何在 Lyra 框架上构建完全不同于射击模式的替代玩法。

## 核心系统

### 固定俯视相机

- BoundsSize → 距离映射曲线
- 无旋转跟踪，纯俯视视角

### GAS AttributeSet

| 属性 | 说明 |
|------|------|
| Bombs | 炸弹数量 |
| Capacity | 容量上限 |
| Range | 爆炸范围 |
| MovementSpeed | 移动速度 |

全部属性支持网络复制和钳制（Clamp）。

### 移动系统

- `ULyraCharacterMovementComponent` 绑定 `MovementSpeed` 属性
- 属性变化实时反映到移动速度

### 拾取物

- `UIData` 驱动：Niagara VFX + SFX
- 属性拾取时即时生效

## 关键类

| 类 | 职责 |
|----|------|
| `UTopDownArenaAttributeSet` | GAS 属性集 |
| `UTopDownArenaMovementComponent` | 移动组件 |
| `UTopDownArenaPickupUIData` | 拾取物 UI 数据 |

## 设计模式

- **GameFeatureAction 激活/停用**：完全可插拔的玩法切换
- **DataAsset 驱动配置**：属性集、输入配置通过数据资产定义
- **网络原生支持**：属性网络复制 + OnRep 回调

## 依赖

- Core: AbilitySystem, Character, Camera, Input
- EnhancedInput

## 相关文档

- [ShooterCore](ShooterCore.md) — 射击模式（对比参考）
- [Core/AbilitySystem](../Core/Modules/AbilitySystem.md) — GAS 框架
