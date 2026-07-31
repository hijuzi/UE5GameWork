# packaging-and-deployment 使用指南

## 功能

Cook、打包和发布 UE 项目 —— Cook 流程、打包配置、平台设置、发布优化。

## 使用方式

```
"帮我打 Windows 安装包"
"移动端打包失败怎么解决？"
"打包太大怎么瘦身？"
```

## 使用示例

### 示例 1：Windows 打包

> **你**: "打包一个 Windows 可玩版本"

> **AI**: Project Settings → Packaging → Windows → Shipping 配置 → File → Package Project → Windows (64-bit)。

### 示例 2：移动端配置

> **你**: "Android 打包闪退"

> **AI**: 检查 Android SDK/NDK 版本 → 调整 `Project Settings → Android` → 设置正确的 `Minimum SDK` 和 `Target SDK`。

### 示例 3：包体瘦身

> **你**: "打包完 15GB 太大了"

> **AI**: 启用压缩 → 排除未使用资产 → 调整贴图质量设置 → 禁用不必要的插件 → Shipping 模式。

## 打包配置

| 配置 | 用途 |
|------|------|
| Development | 开发调试 |
| Shipping | 最终发布（优化） |
| Test | 测试配置 |
| DebugGame | 调试用 |

## 适用场景

- 项目打包发布
- 包体优化
- 平台特定配置
