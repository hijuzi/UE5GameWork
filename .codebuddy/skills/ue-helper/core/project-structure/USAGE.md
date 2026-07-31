# project-structure 使用指南

## 功能

导航和配置 UE 项目 —— .uproject 描述符、项目目录布局、Content vs Source、Config 文件。

## 使用方式

```
"UE 项目的标准目录结构是什么？"
"怎么添加新的自定义 Config 文件？"
"Source 目录下的模块应该怎么组织？"
```

## 使用示例

### 示例 1：标准项目结构

> **你**: "我的项目结构对吗？看看目录"

> **AI**: 标准结构：
> ```
> MyProject/
> ├── MyProject.uproject       # 项目描述符
> ├── Config/                   # DefaultEngine.ini 等
> ├── Content/                  # 资产（蓝图/贴图/模型）
> ├── Source/
> │   ├── MyProject.Target.cs   # Game Target
> │   ├── MyProjectEditor.Target.cs  # Editor Target
> │   └── MyProject/            # 主模块
> │       ├── MyProject.Build.cs
> │       ├── Public/
> │       └── Private/
> └── Plugins/                  # 项目插件
> ```

### 示例 2：多模块项目

> **你**: "战斗系统和 UI 系统拆成两个模块"

> **AI**: Source 下创建 BattleSystem/ 和 UISystem/ 目录，各自包含 .Build.cs + Public/ + Private/。

### 示例 3：Config 文件

> **你**: "给自定义模块加配置"

> **AI**: 在 `Config/DefaultGame.ini` 添加 `[MyModule]` 段，C++ 中用 `GConfig->GetString()` 读取。

## 适用场景

- 新项目结构搭建
- 多模块拆分
- 配置文件管理
