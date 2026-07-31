# importing-content 使用指南

## 功能

用 Interchange 框架导入外部资产到 UE —— UInterchangeManager、自定义 Translator、Pipeline 链。

## 使用方式

```
"帮我写一个 .gltf 导入 translator"
"批量导入 FBX 怎么自动化 Pipeline？"
"导入时自动设置碰撞怎么配置？"
```

## 使用示例

### 示例 1：Python 批量导入

> **你**: "Python 脚本导入文件夹所有 FBX"

> **AI**:
> ```python
> import unreal
> task = unreal.AssetImportTask()
> task.filename = "D:/Assets/model.fbx"
> task.destination_path = "/Game/Models"
> task.automated = True
> unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
> ```

### 示例 2：自定义 Interchange Pipeline

> **你**: "导入贴图时自动设为压缩格式 B8G8R8A8"

> **AI**: 创建 `UInterchangePipelineBase` → `ExecutePipeline` 中设置贴图压缩参数。

### 示例 3：导入时自动生成碰撞

> **你**: "导入静态网格时自动生成简单碰撞"

> **AI**: 在 Import Settings 或 Pipeline 中设置 `AutoGenerateCollision = true`。

## 核心组件

| 组件 | 用途 |
|------|------|
| InterchangeManager | 导入任务管理器 |
| Translator | 解析外来格式（FBX/glTF/USD） |
| Pipeline | 导入后处理链 |
| Factory | 创建 UAsset |

## 适用场景

- 批量资源导入
- 自定义导入格式支持
- 导入自动化 Pipeline
