---
module: loading-index
---

# Loading — 加载与工具层

> 生成日期：2026-07-31 | 模式：Full | 模块数：3 | 总文件数：~37 | 总代码行：~1,286

## 概述

加载与工具层（Loading）管理引擎启动和关卡加载过程中的 UI 呈现，以及编辑器辅助工具。

## 模块

| 模块 | 代码量 | 复杂度 | 文档 |
|------|--------|--------|------|
| **CommonLoadingScreen** | 1,034 行 | Medium-High | [详情](Modules/CommonLoadingScreen.md) |
| **CommonStartupLoadingScreen** | 166 行 | Low | [详情](Modules/CommonStartupLoadingScreen.md) |
| **LyraExtTool** | 86 行 | Very Low | [详情](Modules/LyraExtTool.md) |

## 加载流程

```
CommonStartupLoadingScreen → CommonLoadingScreen → 游戏就绪
     (引擎初始化)              (关卡加载/地图切换)
```

## 相关目录

- ← [Infrastructure（基础框架层）](../Infrastructure/Index.md)
- ← [Core（核心游戏代码）](../Core/Index.md)
- → [GameFeatures（游戏玩法）](../GameFeatures/Index.md)
