# uds-modifiers-configs-state 使用指南

## 功能

UDS 天空预设（State）保存/加载、后处理设置。

## 使用方式

```
"保存当前天空配置为预设"
"运行时切换到暴风雨预设"
"不同关卡用不同天空预设"
```

## 使用示例

> **你**: "保存一个日落预设，在游戏加载时切换"

> **AI**: UDS → States → `Save Current as State` → 命名为 "Sunset" → 运行时 `Load State("Sunset")`。

> **你**: "切换预设需要平滑过渡"

> **AI**: 设置 `State Transition Time = 3.0s` 平滑过渡。

## 预设功能

| 功能 | 说明 |
|------|------|
| Save State | 保存当前全部天空参数 |
| Load State | 加载预设 |
| Transition Time | 过渡时间 |

## 适用场景

- 天空预设切换
- 关卡间天空统一
