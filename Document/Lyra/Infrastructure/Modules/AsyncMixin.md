---
module: async-mixin
purpose: Zero-memory-footprint mix-in class for ordered sequential async asset loading with automatic lifetime safety
complexity: Medium
loc: 976
file_count: 3
---

# AsyncMixin

## 用途

提供一个**零内存占用**的混合类（mixin），用于在 `UObject` 派生类中管理**有序的、顺序执行的异步资产加载**，同时自动保证生命周期安全。

## 架构

```
FAsyncMixin (零数据成员)
    └── 静态 TMap<FAsyncMixin*, TSharedRef<FLoadingState>>
         └── FLoadingState
              ├── TArray<TUniquePtr<FAsyncStep>>  — 步骤队列
              ├── CurrentAsyncStep 索引
              └── FTSTicker 延迟执行

FAsyncScope : FAsyncMixin  — public 暴露所有 protected 方法
```

### 加载流程

```
1. 多次调用 AsyncLoad() 排队步骤
2. 调用 StartAsyncLoading() (或自动一帧后)
3. 每个步骤依次执行: StreamableHandle → 完成 → 下一个
4. 全部完成后回调 OnFinishedLoading()
5. 析构时自动取消: Loading.Remove(this)
```

## API

### 排队方法（protected）

```cpp
// 资产加载 (7 个重载)
void AsyncLoad(TSoftClassPtr<T>, FOnAsyncStepFinished Callback);
void AsyncLoad(TSoftObjectPtr<T>, FOnAsyncStepFinished Callback);
void AsyncLoad(FSoftObjectPath, FOnAsyncStepFinished Callback);

// 条件等待
void AsyncCondition(TUniqueFunction<bool()> Condition, ...);

// 纯事件
void AsyncEvent(FOnAsyncStepFinished Callback);
```

### 控制方法

| 方法 | 说明 |
|------|------|
| `StartAsyncLoading()` | 开始顺序执行 |
| `CancelAsyncLoading()` | 取消所有挂起步骤 |
| `IsAsyncLoadingInProgress()` | 是否有步骤进行中 |

### 生命周期钩子

```cpp
virtual void OnStartedLoading();   // 开始时
virtual void OnFinishedLoading();  // 全部完成
```

## 使用模式

```cpp
class UMyWidget : public UUserWidget, public FAsyncMixin
{
    void LoadDependencies()
    {
        AsyncLoad(SoftIcon, [this](){ OnIconLoaded(); });
        AsyncLoad(SoftData, [this](){ OnDataLoaded(); });
        StartAsyncLoading();
    }

    // 析构时自动安全取消
};
```

## 依赖

- UE: Core, CoreUObject, Engine (FStreamableManager, FTSTicker)
- 无其他 Lyra 插件依赖
