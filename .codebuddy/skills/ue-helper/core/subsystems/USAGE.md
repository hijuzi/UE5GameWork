# subsystems 使用指南

## 功能

实现引擎管理的单例服务 —— UEngineSubsystem、UGameInstanceSubsystem、UWorldSubsystem、UTickableWorldSubsystem、ULocalPlayerSubsystem。

## 使用方式

```
"帮我创建一个存档管理器 Subsystem"
"这个服务该放 GameInstance 还是 World 级别？"
"Subsystem 的初始化顺序怎么控制？"
```

## 使用示例

### 示例 1：创建存档 Subsystem

> **你**: "做一个跨关卡的存档管理器"

> **AI**: 创建 `UGameInstanceSubsystem`，在 `Initialize()` 加载存档，`Deinitialize()` 保存。

### 示例 2：Tickable World 服务

> **你**: "需要一个每帧检测敌人视野的服务"

> **AI**: 继承 `UTickableWorldSubsystem`，在 `Tick()` 中执行检测逻辑。

### 示例 3：初始化依赖

> **你**: "我的 Subsystem 需要确保另一个先初始化"

> **AI**: 重写 `InitializeDependency()` 返回依赖的 Subsystem 类列表。

## 级别选择

| 类型 | 生命周期 | 典型用途 |
|------|---------|---------|
| UEngineSubsystem | 引擎启动→关闭 | 全局配置、插件管理 |
| UGameInstanceSubsystem | 游戏开始→结束 | 存档、成就、数据层 |
| UWorldSubsystem | 关卡加载→卸载 | AI 管理器、寻路服务 |
| ULocalPlayerSubsystem | 本地玩家会话 | 输入处理、本地设置 |

## 适用场景

- 创建服务/管理器（存档、技能注册、匹配）
- 决定生命周期范围（进程/游戏/世界/本地玩家）
- Subsystem vs Manager Actor vs GameInstance 抉择
