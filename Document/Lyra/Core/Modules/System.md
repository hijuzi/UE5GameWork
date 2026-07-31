---
module: system
complexity: Medium
loc: 2300
file_count: 27
---

# System — 系统基础服务

## 架构

基础设施骨干，引擎类型继承 + Lyra 特定扩展：

```
ULyraGameEngine → ULyraGameInstance (InitState管理, DTLS加密, 无缝旅行)
ULyraAssetManager (StartupJob管道, 加权进度, 同步/异步加载)
ULyraReplicationGraph (完整自定义图: 空间化+频率限制+每连接AlwaysRelevant)
ULyraSignificanceManager (占位桩, 未来LOD用)
```

## 关键类

| 类 | 职责 |
|----|------|
| `ULyraAssetManager` | 加权StartupJob管道, Sync/Async加载, 已加载资产跟踪 |
| `ULyraGameInstance` | InitState注册, DTLS加密, Session Travel加密令牌, UserInit |
| `ULyraReplicationGraph` | 自定义复制图: GridSpatialization2D, FastShared移动, PlayerState频率限制 (2/帧) |
| `ULyraSystemStatics` | Blueprint工具: PlayNextGame (无缝服务器旅行), PrimaryAssetId转换 |
| `FGameplayTagStack` | FastArray复制的标签堆叠系统: AddStack/RemoveStack, TagToCountMap |
| `ULyraDevelopmentStatics` | 编辑器工具: PIE世界查找, 蓝图类查找, Debug设置 |
| `ULyraGameData` | 顶级游戏数据资产 (关卡列表, 体验集) |

## 网络亮点: ReplicationGraph

完整的自定义复制图 (最大文件, 997行cpp):
- `GridSpatialization2D`: 基于距离的Actor路由
- `AlwaysRelevant_ForConnection`: 每连接重建
- `PlayerStateFrequencyLimiter`: 滚动子集, 默认2/帧
- `FastShared路径`: 角色移动, 带宽上限~10KB/s
- 自行注册为`UReplicationDriver`工厂

## 设计模式

- **StartupJob管道**: 加权进度报告
- **FastArray序列化**: GameplayTagStack最小网络开销
- **Lazy Init**: ReplicationGraph回调初始化未引用类
