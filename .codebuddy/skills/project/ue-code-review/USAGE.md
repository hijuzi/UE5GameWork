# ue-code-review 使用指南

## 功能

系统化审查 UE5 C++ 代码，覆盖编码规范、GC 安全、网络复制、GAS、性能等 13 个维度。

## 使用方式

```
"帮我 review 这段 UE5 代码"
"审查我的 Pull Request"
```

## 审查 13 个维度

| # | 维度 | 重点关注 |
|---|------|---------|
| 1 | 编码规范 | 类型前缀、命名、include 顺序、TEXT() 宏 |
| 2 | UObject 生命周期 | UPROPERTY 保护、弱引用、Lambda 捕获 |
| 3 | 网络复制 | 服务器权威、RPC 验证、条件复制 |
| 4 | GAS | 技能激活、属性设置、GameplayEffect |
| 5 | Enhanced Input | InputAction、BindAction、MappingContext |
| 6 | 委托/事件 | 动态委托、解绑、内存安全 |
| 7 | 性能 | Tick 优化、缓存、容器预留 |
| 8 | 内存管理 | 禁止 raw new/delete、智能指针 |
| 9 | 错误处理 | check/ensure/UE_LOG 选择、空指针检查 |
| 10 | 资源引用 | TSoftObjectPtr、异步加载 |
| 11 | 编辑器代码 | WITH_EDITOR 隔离、PostEditChangeProperty |
| 12 | 线程安全 | IsInGameThread()、AsyncTask 回主线程 |
| 13 | 反模式 | 构造器中 GetWorld() = nullptr 等 |

## 使用示例

### 示例 1：审查单个文件

> **你**: "帮我 review `MyCharacter.cpp`"

> **AI**: 逐行分析，输出 Critical / Major / Minor 三级问题。

### 示例 2：审查 PR

> **你**: "Review 我最近的改动，主要改了技能系统和网络同步"

> **AI**: 重点检查 GAS + Networking 维度，同时全量过其他维度。

### 示例 3：代码审计

> **你**: "全面审计 `Source/MyGame/Battle/` 目录，关注性能和内存"

> **AI**: 扫描整个目录，12 维度全量审查，侧重性能和内存部分。

## 输出格式

```markdown
## Critical（必须修复）
- [Networking] Server RPC `RequestSpawnItem` 缺少 _Validate 函数
  → 客户端可伪造参数，造成作弊

## Major（应当修复）
- [Performance] `Tick()` 中每次调用 `FindComponent<UHealthBar>()`
  → 移到 BeginPlay 缓存

## Minor（建议优化）
- [Style] 变量命名 `hp` → `CurrentHealth` （不符合 PascalCase）

## Positive Highlights
- GAS 中正确使用 GameplayTags 替代枚举判断
```

## 适用场景

- PR 提交前的自我审查
- 团队 Code Review 流程
- 新成员代码质量检查
- 重构后的安全审计
