# data-driven-design 使用指南

## 功能

用外部数据驱动 Gameplay —— DataAsset、DataTable、CurveTable，替代硬编码数值。

## 使用方式

```
"帮我用 DataAsset 配置武器属性"
"DataTable 怎么读取 CSV 数据？"
"Curve 怎么做伤害衰减？"
```

## 使用示例

### 示例 1：DataAsset 配置武器

> **你**: "每个武器需要不同伤害、射速、后坐力，不要硬编码"

> **AI**: 创建 `UPrimaryDataAsset`：
> ```cpp
> UCLASS()
> class UWeaponData : public UPrimaryDataAsset {
>     UPROPERTY(EditAnywhere) float Damage;
>     UPROPERTY(EditAnywhere) float FireRate;
>     UPROPERTY(EditAnywhere) UCurveFloat* DamageFalloff;
> };
> ```

### 示例 2：DataTable 批量配置

> **你**: "有 50 个怪物，每个血量/经验值不同，怎么管理？"

> **AI**: 定义 `FMonsterRow` 结构体 → 创建 DataTable 资产 → 导入 CSV → `GetDataTableRowFromName()` 读取。

### 示例 3：Curve 衰减曲线

> **你**: "伤害随距离衰减" 

> **AI**: 创建 UCurveFloat 资产，编辑曲线，运行时用 `Curve->GetFloatValue(Distance)` 获取衰减系数。

## 工具选择

| 工具 | 适用 |
|------|------|
| DataAsset | 单个配置（每个武器一个资产） |
| DataTable | 批量数据（怪物表、掉落表） |
| CurveTable/CurveFloat | 连续变化值（衰减、成长曲线） |

## 适用场景

- 武器/技能数据配置
- 怪物/掉落批量管理
- 曲线数值（伤害衰减、经验加成）
