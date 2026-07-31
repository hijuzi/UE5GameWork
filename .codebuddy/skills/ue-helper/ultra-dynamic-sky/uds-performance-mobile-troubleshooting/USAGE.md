# uds-performance-mobile-troubleshooting 使用指南

## 功能

UDS 性能优化、移动端适配、常见问题排查。

## 使用方式

```
"UDS 帧率太低怎么优化？"
"移动端装 UDS 后闪退"
```

## 使用示例

> **你**: "体积云导致帧数从 60 掉到 30"

> **AI**: → 降低体积云分辨率 `Volumetric Cloud Resolution Scale = 0.5` → 关闭 `Cloud Shadows` → 使用 Static 云替代。

> **你**: "移动端屏幕全黑"

> **AI**: → 检查移动端是否支持体积云 → 切换到 `2D Cloud Mode` → 检查材质是否支持移动 Renderer。

## 常见问题

| 问题 | 解决方案 |
|------|---------|
| 帧率低 | 降低云/雾质量、关闭次要效果 |
| 移动端黑屏 | 切换 2D 模式 |
| 闪烁 | 检查曝光设置、阴影 Bias |

## 适用场景

- 项目优化
- 移动端移植
- Bug 排查
