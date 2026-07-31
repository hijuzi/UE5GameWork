# asset-management 使用指南

## 功能

引用和加载 UE 资源 —— 硬引用 vs 软引用（TObjectPtr vs TSoftObjectPtr）、异步加载、FSoftObjectPath、StreamableManager。

## 使用方式

```
"这个大贴图怎么异步加载不卡顿？"
"蓝图里选的武器应该用硬引用还是软引用？"
```

## 使用示例

### 示例 1：软引用按需加载

> **你**: "100 种武器贴图不能全加载进内存"

> **AI**:
> ```cpp
> UPROPERTY(EditAnywhere)
> TSoftObjectPtr<UTexture2D> WeaponIcon;
> // 需要时才加载:
> UTexture2D* Icon = WeaponIcon.LoadSynchronous(); // 或异步
> ```

### 示例 2：异步批量加载

> **你**: "加载关卡时异步预加载所有怪物资源"

> **AI**: 用 `FStreamableManager::RequestAsyncLoad` 批量异步加载。

## 引用类型

| 类型 | 加载时机 | 何时用 |
|------|---------|--------|
| 硬引用 `TObjectPtr` | 对象加载时立即 | 必需、总在用的资源 |
| 软引用 `TSoftObjectPtr` | 手动加载 | 不总需要的资源 |
| `TSoftClassPtr` | 手动加载 | 蓝图选择的 UClass |

## 适用场景

- 大量资源的内存管理
- 异步加载大型资产
- 蓝图资产选择
