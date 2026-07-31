# coding-standards 使用指南

## 功能

按照 Epic 官方编码规范书写 UE5 C++，包括类型前缀、命名约定、大括号风格、include 顺序、const 正确性等。

## 使用方式

```
"按照 UE 规范检查这段代码"
"给这个 actor 起个符合规范的名字"
"整理这个头文件的 include 顺序"
```

## 使用示例

### 示例 1：纠正变量命名

> **你**: "这段代码哪里不符合规范？"
> ```cpp
> bool isActive;       // ❌ 不符合
> int player_index;    // ❌ 不符合
> ```

> **AI**:
> ```cpp
> bool bIsActive;      // ✅ bool 用 b 前缀
> int32 PlayerIndex;   // ✅ PascalCase
> ```

### 示例 2：清理头文件

> **你**: "按规范整理这个头文件"

> **AI**: 公共头 → 引擎头 → 项目头 → 模块头 → `generated.h`（最后），添加前向声明减少编译依赖。

### 示例 3：类型前缀检查

> **你**: "给下面的类型加上正确前缀"
> ```
> class Monster : public AActor    // ✅ A 前缀
> struct ItemData                  // → FItemData
> enum class Team                  // → ETeam
> ```

> **AI**: 规范前缀表 — UObject 派生类用 U，Actor 用 A，结构体用 F，枚举用 E，接口用 I，模板用 T，Slate 用 S。

## 核心规范速查

| 项目 | 规范 |
|------|------|
| 类型前缀 | U/A/F/E/I/T/S |
| 命名风格 | PascalCase |
| bool 前缀 | `b` (bIsDead) |
| 大括号 | Allman 风格（换行） |
| 缩进 | Tab |
| 字符串 | `TEXT("...")` |
| 容器 | TArray / TMap / TSet |
| include | `generated.h` 必须在最后 |

## 适用场景

- 新建任何 UE C++ 文件
- 代码审查时检查风格一致性
- 为类型/成员/函数命名
- 整理头文件结构
