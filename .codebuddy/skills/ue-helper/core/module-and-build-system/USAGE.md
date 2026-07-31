# module-and-build-system 使用指南

## 功能

创建和管理 UE5 模块，配置 `*.Build.cs` 和 `*.Target.cs`，解决链接和包含路径错误。

## 使用方式

```
"创建一个新模块"
"为什么有 unresolved external symbol 错误？"
"添加对 Engine 模块的依赖"
```

## 使用示例

### 示例 1：创建新模块

> **你**: "在项目里新建一个 BattleSystem 模块"

> **AI**: 创建 `Source/BattleSystem/` 目录，包含 `BattleSystem.Build.cs`，添加 `IMPLEMENT_MODULE`，更新 `.uproject` 或 `.Target.cs`。

### 示例 2：解决链接错误

> **你**: "编译报错: 'unresolved external symbol MYGAME_API'"

> **AI**: 该类使用了模块导出宏但可能在其他模块中引用。检查该类的模块是否作为 Public 依赖被添加：`PublicDependencyModuleNames.Add("YourModule")`

### 示例 3：添加模块依赖

> **你**: "我的模块需要用到 UMG 和 Slate"

> **AI**: 在 `.Build.cs` 中添加：
> ```csharp
> PublicDependencyModuleNames.AddRange(new string[] {
>     "UMG", "Slate", "SlateCore"
> });
> ```

## 关键概念

| 文件 | 作用 |
|------|------|
| `*.Build.cs` | 定义模块编译规则（依赖、宏、路径） |
| `*.Target.cs` | 定义构建目标（Game/Editor/Client/Server） |
| `IMPLEMENT_MODULE` | 注册普通模块 |
| `IMPLEMENT_PRIMARY_GAME_MODULE` | 注册游戏主模块 |

## 适用场景

- 新建或拆分模块
- 链接/包含路径错误排查
- 配置 public vs private 依赖
- 设置模块加载阶段
