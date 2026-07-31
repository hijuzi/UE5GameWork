---
module: shootertests
complexity: Medium
loc: 410
file_count: 17
---

# ShooterTests

> 插件路径：`LyraStarterGame/Plugins/GameFeatures/ShooterTests/Source/`

## 概述

ShooterCore 的测试容器插件，包含功能测试、消息系统验证和设备属性测试。

## 测试内容

### 异步消息系统测试

`FAsyncGameplayMessageSystem`：非游戏线程 → 游戏线程彩色消息广播，验证跨线程消息路由的正确性和性能。

### 设备属性测试

`UInputDeviceProperty` 相关测试，验证输入设备属性（如震动、灯效）的正确应用。

### 射击功能覆盖

- 构建数据验证
- 射击功能覆盖检查

## 依赖

- ShooterCore（被测对象）
- GameplayMessageRouter
- EnhancedInput

## 相关文档

- [ShooterCore](ShooterCore.md) — 被测的射击核心插件
