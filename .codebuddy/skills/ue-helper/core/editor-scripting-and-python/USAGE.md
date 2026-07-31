# editor-scripting-and-python 使用指南

## 功能

用 Python 自动化编辑器 —— `unreal` 模块、启动脚本、Editor Utility Widget (Blutility)、批量资产处理。

## 使用方式

```
"帮我写个 Python 脚本批量重命名资产"
"怎么做一个编辑器工具按钮，一键生成关卡？"
"Commandlet 怎么在命令行跑？"
```

## 使用示例

### 示例 1：Python 批量处理

> **你**: "把所有贴图后缀 _D 改成 _Diffuse"

> **AI**:
> ```python
> import unreal
> assets = unreal.EditorAssetLibrary.list_assets("/Game/Textures")
> for asset in assets:
>     if asset.endswith("_D"):
>         new_name = asset.replace("_D", "_Diffuse")
>         unreal.EditorAssetLibrary.rename_asset(asset, new_name)
> ```

### 示例 2：Editor Utility Widget

> **你**: "做一个编辑器面板，一键布置关卡怪物"

> **AI**: 继承 `UEditorUtilityWidget` → 蓝图中添加按钮 → `SpawnActor` 批量放置。

### 示例 3：Commandlet 批量转换

> **你**: "CI 中自动导出所有 FBX 资源信息"

> **AI**: 继承 `UCommandlet`，`Main()` 中遍历资产并输出。

## 工具选择

| 工具 | 适用 |
|------|------|
| Python (`unreal`) | 一次性脚本、CI |
| Blutility | 编辑器交互面板 |
| Commandlet | 命令行批处理 |
| UAssetActionUtility | 资产右键菜单 |

## 适用场景

- 批量资产操作
- CI 自动化
- 自定义编辑器工具
