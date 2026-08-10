# UE5 Enhanced Input 系统入门

## 引言

UE5 推出了 Enhanced Input 系统，替换了旧版的 Input 系统。很多开发者在使用过程中遇到了一些问题。本文会介绍 Enhanced Input 的基本用法和一些常见配置。

## 核心概念

- **UInputAction**：输入动作
- **UInputMappingContext**：输入映射上下文
- **UEnhancedInputComponent**：增强输入组件

## 源码分析

```cpp
void UEnhancedPlayerInput::ProcessInputStack(
    const TArray<UInputComponent*>& InputComponentStack,
    const float DeltaTime,
    const bool bGamePaused)
{
    for (UInputComponent* InputComponent : InputComponentStack)
    {
        if (InputComponent)
        {
            // 处理输入
            ProcessInput(InputComponent, DeltaTime, bGamePaused);
        }
    }
}
```

上面这段代码展示了输入栈的处理流程。它会遍历所有的 InputComponent 并处理输入。

## 设计思考

Enhanced Input 的设计相对于旧版系统有很多优势。首先是它的模块化设计，其次是对多种输入设备的支持。UE 团队显然在设计时考虑了跨平台的需求。

## 总结

Enhanced Input 是 UE5 的一个重要更新，建议开发者尽快从旧版系统迁移过来。本文介绍了基本概念和使用方法。
