---
module: gamesubtitles
complexity: Low
loc: 719
file_count: 10
---

# GameSubtitles

## 用途

轻量级媒体字幕插件：UMG Widget + media-aware tickable player + SubtitleDisplaySubsystem。

## 架构

```
USubtitleDisplayOptions (DataAsset): Font/Size/Color/Border/Opacity

USubtitleDisplaySubsystem (GameInstanceSubsystem): 
  FSubtitleFormat (选中枚举) → FDisplayFormatChangedEvent 广播

UMediaSubtitlesPlayer (UObject + FTickableGameObject):
  Tick() → UOverlays 查询 → FSubtitleManager::SetMovieSubtitle()

SSubtitleDisplay (SCompoundWidget) + USubtitleDisplay (UWidget):
  订阅 FSubtitleManager::OnSetSubtitleText + 格式变化事件
```

## 关键类

| 类 | 职责 |
|----|------|
| `USubtitleDisplayOptions` | DataAsset 外观配置 |
| `USubtitleDisplaySubsystem` | 管理和广播字幕显示格式 |
| `UMediaSubtitlesPlayer` | Tick 驱动, 从 UOverlays 提取字幕文本 |
| `USubtitleDisplay` | UMG Widget 封装, 运行时样式重建 |

## 依赖

Core, Overlay, UMG, MediaAssets, MediaUtils, GameplayTags
