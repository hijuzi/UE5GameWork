# delegates-and-events 使用指南

## 功能

用委托连接回调和事件 —— 单播委托、多播委托、动态委托（蓝图）、事件广播、Lambda 绑定。

## 使用方式

```
"帮我定义一个'血量变化'事件，UI 和音效同时响应"
"委托和事件有什么区别？"
"怎么在蓝图中绑定 C++ 委托？"
```

## 使用示例

### 示例 1：血量变化多播

> **你**: "血量变化时，UI + 音效 + 特效都要响应"

> **AI**:
> ```cpp
> DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
>     FOnHealthChanged, float, NewHealth, float, MaxHealth);
> UPROPERTY(BlueprintAssignable)
> FOnHealthChanged OnHealthChanged;
> // 广播: OnHealthChanged.Broadcast(NewHealth, MaxHealth);
> ```

### 示例 2：蓝图绑定

> **你**: "让蓝图能绑定血量变化事件"

> **AI**: 用 `DECLARE_DYNAMIC_MULTICAST_DELEGATE` + `UPROPERTY(BlueprintAssignable)`，蓝图 Event Graph 中直接 Bind 并处理。

### 示例 3：Lambda 一次性绑定

> **你**: "网络请求完成后执行一次回调"

> **AI**: `Delegate.AddLambda([](){ /* 一次逻辑 */ });`

## 委托类型

| 类型 | 蓝图可用 | 多播 |
|------|---------|------|
| DECLARE_DELEGATE | ❌ | 单播 |
| DECLARE_MULTICAST_DELEGATE | ❌ | 多播 |
| DECLARE_DYNAMIC_DELEGATE | ✅ | 单播 |
| DECLARE_DYNAMIC_MULTICAST_DELEGATE | ✅ | 多播 |

## 适用场景

- 血量/分数变化通知
- 按钮点击/输入事件
- 异步操作完成回调
- 模块间解耦通信
