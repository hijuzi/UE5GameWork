---
module: core-code-review
---

# Code Review — Core

## AbilitySystem (High Complexity, 5,400 LOC)

### Pros
- 完善的 GAS 封装：ActivationPolicy、ActivationGroup、AbilitySet 数据驱动
- TagRelationshipMapping 支持标签关联的 CanActivateAbility 检查扩展
- 失败消息系统（GameplayMessageSubsystem 广播）提供用户友好的能力激活反馈

### Cons
- 51 文件分散在 15+ 子目录，导航复杂度高
- Execution 类依赖 `#if WITH_SERVER_CODE` 条件编译，双端逻辑分散
- 伤害计算的多个修正因子（距离、物理材质、队伍）分布在多处

## Character (High Complexity, 2,945 LOC)

### Pros
- PawnData DataAsset 使角色配置外部化，设计优雅
- InitState 状态机确保组件初始化顺序正确
- 清晰的组件职责分离（Extension/Health/Hero/Camera）

### Cons
- InitState 流程在多个组件间分散，不便追踪完整初始化路径
- `ALyraCharacterWithAbilities` 作为 `ALyraCharacter` 子类添加 ASC 所有权，引入继承树复杂性

## Camera (Medium, 1,690 LOC)

### Pros
- Stack 模式使多 CameraMode 混合直观
- Penetration Avoidance 实现完整
- 通过委托选择 CameraMode，松耦合

### Cons
- CameraModeStack 的混合计算较复杂，需理解 POV 插值算法

## Input (Medium, 893 LOC)

### Pros
- GameplayTag 桥接设计让输入→GAS 路由简洁
- InputConfig DataAsset 支持不同 Pawn 的独立输入配置
- 自定义 InputModifier 读取用户设置

### Cons
- 依赖 EnhancedInput 系统，调试输入问题需理解 UE 输入管道全链路

## Animation (Low, 113 LOC)

### Pros
- 设计做到极简 — 仅做必要的桥接
- GameplayTagBlueprintPropertyMap 自动化标签到动画变量映射

### Cons
- 功能极度依赖动画蓝图（Content/ 非 C++），C++ 层几乎无自主功能
