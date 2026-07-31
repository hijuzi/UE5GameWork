# navigating-engine-source 使用指南

## 功能

在磁盘引擎源码中定位、阅读和引用 UE API —— 源码目录结构、关键模块位置。

## 使用方式

```
"UE 的 Actor Spawn 源码在哪里？"
"引擎的这个功能怎么实现的，帮我找源码"
"引用引擎 API 时怎么知道路径？"
```

## 使用示例

### 示例 1：查找源码位置

> **你**: "UMovementComponent 的 TickComponent 实现在哪？"

> **AI**: `Engine/Source/Runtime/Engine/Private/Components/MovementComponent.cpp` → `UMovementComponent::TickComponent()`。

### 示例 2：引用引擎 API

> **你**: "怎么在代码中引用引擎源码中的头文件？"

> **AI**: 先在 `.Build.cs` 添加依赖模块 → `#include "Engine/Classes/GameFramework/MovementComponent.h"` → 使用对应 API。

## 关键目录

| 目录 | 内容 |
|------|------|
| `Source/Runtime/` | 核心 Runtime 模块 |
| `Source/Editor/` | 编辑器工具 |
| `Source/Developer/` | 开发工具 |
| `Plugins/` | 引擎内置插件 |

## 适用场景

- 研究引擎实现
- 参考引擎代码风格
- 解决问题时查看引擎逻辑
