# materials-and-shaders 使用指南

## 功能

创建和驱动 UE5 材质 —— UMaterial 节点图、材质实例、材质参数集、动态材质实例。

## 使用方式

```
"帮我做一个溶解特效材质"
"C++ 怎么在运行时修改材质颜色？"
"材质蓝图里怎么做 UV 滚动？"
```

## 使用示例

### 示例 1：运行时改材质颜色

> **你**: "受伤时角色变红，0.5 秒后恢复"

> **AI**:
> ```cpp
> UMIDynamic = Mesh->CreateDynamicMaterialInstance(0);
> UMIDynamic->SetVectorParameterValue("BaseColor", FLinearColor::Red);
> // 0.5s 后恢复:
> UMIDynamic->SetVectorParameterValue("BaseColor", FLinearColor::White);
> ```

### 示例 2：材质参数集

> **你**: "全局控制所有草地材质的风力强度"

> **AI**: 创建 `UMaterialParameterCollection`，添加 WindStrength 参数 → 所有草地材质引用此参数集 → C++ 中 `UKismetMaterialLibrary::SetScalarParameterValue()` 修改。

### 示例 3：溶解效果

> **你**: "敌人死亡时溶解消失"

> **AI**: 材质中使用 `Noise` + `Step` 节点，暴露 DissolveAmount 参数（0~1），C++ 中随时间递增。

## 核心概念

| 概念 | 说明 |
|------|------|
| Material | 节点图资产（不可运行时改） |
| Material Instance | Material 实例（参数可调） |
| Dynamic Material Instance | 运行时创建和修改 |
| Material Parameter Collection | 全局参数集 |

## 适用场景

- 运行时材质效果（变色、溶解、描边）
- 全局参数控制（时间、天气）
- UV 动画（水流、滚动）
