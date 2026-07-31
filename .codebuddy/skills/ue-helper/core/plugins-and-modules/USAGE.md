# plugins-and-modules 使用指南

## 功能

创建和管理 UE 插件 —— .uplugin 描述符、插件模块结构、插件类型、内容资产。

## 使用方式

```
"帮我创建一个编辑器插件"
"怎么在插件里添加新模块？"
"插件之间的依赖怎么配置？"
```

## 使用示例

### 示例 1：创建插件

> **你**: "创建一个自定义工具栏插件"

> **AI**: Edit → Plugins → New Plugin → Editor Toolbar Button → 自动生成 .uplugin + 模块文件。

### 示例 2：插件多模块

> **你**: "插件里需要 Runtime 和 Editor 两个模块"

> **AI**: .uplugin 中配置两个模块，类型分别为 Runtime 和 Editor，Editor 模块添加 `PublicDependencyModuleNames` 依赖 Runtime 模块。

### 示例 3：插件依赖

> **你**: "我的插件依赖另一个插件"

> **AI**: .uplugin 文件中 `Plugins: [{ "Name": "OtherPlugin" }]`。

## 插件类型

| 类型 | 打包后 |
|------|--------|
| Runtime | 打包到游戏 |
| Editor | 仅编辑器 |
| Developer | 开发期 |
| Unclassed | 运行时可选 |

## 适用场景

- 自定义编辑器工具
- 可重用 Runtime 模块
- 第三方库集成
