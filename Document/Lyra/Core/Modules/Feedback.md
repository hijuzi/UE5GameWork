---
module: feedback
complexity: Medium
loc: 1927
file_count: 19
---

# Feedback

## 架构

两个独立的反馈子系统：

### ContextEffects (环境效果, 10 文件)

```
UAnimNotify_LyraContextEffects → 动画触发
    ↓ 执行 Trace (表面检测)
ILyraContextEffectsInterface → 收集 Context (Tag 容器)
    ↓
ULyraContextEffectsSubsystem (WorldSubsystem) → 中央代理
    ↓ 匹配
ULyraContextEffectsLibrary (DataAsset) → EffectTag+Context → Audio/Niagara
```

- DeveloperSettings: `SurfaceTypeToContextMap` (物理表面→GameplayTag)
- 支持 ExactMatch 和 BestMatch

### NumberPops (浮动数值, 9 文件)

```
NumberPopsSubsystem (WorldSubsystem) → SpawnNumberPop()
    ↓
MaterialParameterCollection → 写入位置/颜色
    ↓
StaticMeshComponent (MeshText) 或 NiagaraComponent
    ↓
对象池 ObjectPooling + Timer 回收
```

- Tag 模式匹配选择样式 (SourceTag/TargetTag → Style)
- 暴击视觉区分

## 设计模式

- **WorldSubsystem 中介**: 解耦效果请求者和生产者
- **DataAsset 配置**: ContextEffectsLibrary 外部化效果映射
- **对象池**: NumberPops 通过对象池管理 UI 组件生命周期
