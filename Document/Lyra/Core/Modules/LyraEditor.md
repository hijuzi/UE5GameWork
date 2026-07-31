---
module: lyraeditor
complexity: Medium
loc: 2625
file_count: 25
---

# LyraEditor — 编辑器工具链

独立编辑器模块 (非 LyraGame)，提供完整的内容验证、CI/CD 命令、自定义引擎和控件工具。

## 四层架构

### 1. 验证器管线

| 验证器 | 职责 |
|--------|------|
| `UEditorValidator` | 基类: P4集成, 批量验证, 项目设置检查 |
| `EditorValidator_Blueprints` | 非纯数据蓝图 + 传递引用者检查 |
| `EditorValidator_Load` | 内存重载: 复制→重载→捕获重复加载警告 |
| `EditorValidator_MaterialFunctions` | 材质函数引用材质检查 |
| `EditorValidator_SourceControl` | P4 依赖项完整性 |

**Core Feature**: `GetChangedAssetsForCode` — 头文件变更 → C++ 类映射 → AssetRegistry 蓝图继承查询 → 级联受影响蓝图 (上限 200)

### 2. CI/CD Commandlet

`UContentValidationCommandlet`: P4 管道 → 仓库路径解析 → 批量验证

### 3. 自定义引擎

`ULyraEditorEngine`: PIE 配置, `ForceStandaloneNetMode`, PIE 事件回调

### 4. 编辑器工具

- **ChaosMeshCollision**: 退化三角形检查
- **CreateRedirectorPackage**: 资产重定向创建
- **DiffCollectionReferenceSupport**: 集合引用差异分析
- **ContextEffectsLibrary**: 工厂 + 资产类型操作

## 设计亮点

- **内存重载验证**: 临时副本重载检测抑制的错误
- **P4 CI 集成**: 直接 pipe p4.exe，仓库路径→资产路径映射
- **级联影响分析**: .h 变更 → 蓝图依赖追踪
