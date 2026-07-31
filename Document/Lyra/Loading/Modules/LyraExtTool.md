---
module: lyra-ext-tool
complexity: Very Low
loc: 86
file_count: 4
---

# LyraExtTool

> 插件路径：`LyraStarterGame/Plugins/LyraExtTool/Source/`

## 概述

仅含一个 Blueprint 函数的骨架插件，提供编辑器级 StaticMesh 材质批量替换能力。

## 关键类

| 类 | 职责 |
|----|------|
| `UBPFunctionLibrary` | BlueprintFunctionLibrary，暴露 `ChangeMeshMaterials()` 函数 |

## API

```cpp
// 批量为 StaticMesh 替换材质（含 Editor Undo 支持）
UFUNCTION(BlueprintCallable, Category = "Lyra|ExtTool")
static void ChangeMeshMaterials(UStaticMesh* InMesh);
```

## 依赖

- Engine（StaticMesh API）
- EditorSubsystem（Undo 支持）

## 相关文档

- [Core/LyraEditor](../Core/Modules/LyraEditor.md) — Lyra 编辑器工具链
