---
module: performance
complexity: High
loc: 804
file_count: 6
---

# Performance — 性能统计与设置

## 架构

全面性能监控 + 平台配置系统：

```
FSampledStatCache (环形缓冲区, 125 样本)
    └── RecordSample/GetAverage/GetMin/GetMax/ForEachCurrentSample

ULyraPerformanceStatSubsystem (GameInstanceSubsystem)
    └── FLyraPerformanceStatCache (IPerformanceDataConsumer)
        ├── FFrameData: FPS, 线程帧时间
        ├── GameState: 服务器 FPS
        ├── PlayerState: Ping
        ├── UNetConnection: 丢包率, 包速率, 包大小
        └── ILatencyMarkerModule: 延迟标记 (Total/Game/Render)

ULyraPerformanceSettings (DeveloperSettings)
    └── Desktop/Console/Mobile 帧率限制
```

## 18 个可追踪统计

FPS(Client), FPS(Server), FrameTime, IdleTime, RenderThreadTime, GameThreadTime, GPUTime, RHITime, Ping, PacketLossIncoming, PacketLossOutgoing, PacketRateIncoming, PacketRateOutgoing, PacketSizeIncoming, PacketSizeOutgoing, TotalLatency, GameLatency, RenderLatency

## 关键类

| 类 | 职责 |
|----|------|
| `ULyraPerformanceStatSubsystem` | 注册/注销数据消费者 |
| `FSampledStatCache` | 环形缓冲区聚合 |
| `ULyraPerformanceSettings` | 帧率限制 + 设备档案变体 |
| `LyraMemoryDebugCommands` | 资产快照导出 + CDO 差异分析 |

## 内存调试

- `Lyra.ObjListToCollection`: 全量加载资产 → .collection 文件
- `AnalyzeObjectListForDifferences`: 实例 vs CDO 属性差异 (发现冗余序列化)
