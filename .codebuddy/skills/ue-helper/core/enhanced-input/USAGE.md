# enhanced-input 使用指南

## 功能

实现玩家输入 —— Enhanced Input 系统，UInputAction 数据资产，InputMappingContext，BindAction 绑定回调，优先级管理。

## 使用方式

```
"帮我设置 WASD 移动和鼠标转向"
"怎么在游戏中切换输入模式？"
"输入优先级冲突怎么处理？"
```

## 使用示例

### 示例 1：绑定移动输入

> **你**: "给 Character 绑定 WASD 移动"

> **AI**: 创建 IA_Move InputAction → 创建 IMC_Default MappingContext → SetupPlayerInputComponent 中绑定：
> ```cpp
> if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent)) {
>     EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyCharacter::Move);
> }
> ```

### 示例 2：切换输入上下文

> **你**: "开车时切换到车辆控制，下车恢复人物控制"

> **AI**: 用 `Subsystem->AddMappingContext(VehicleIMC, Priority)` 添加高优先级车辆输入，下车时 `RemoveMappingContext`。

### 示例 3：组合键

> **你**: "Shift + E 放下物品"

> **AI**: 在 InputAction 设置 Triggers 为 Combo 触发，确保 Shift 已在同一 MappingContext 中绑定。

## 核心概念

| 概念 | 说明 |
|------|------|
| InputAction | 抽象输入动作（不绑定具体按键） |
| MappingContext | 按键→动作的映射表 |
| Priority | 高优先级屏蔽低优先级 |
| TriggerEvent | Started/Triggered/Completed/... |

## 适用场景

- 设置 WASD/鼠标/手柄输入
- 多输入模式切换（人物/载具/UI）
- 组合键和长按
- 输入优先级管理
