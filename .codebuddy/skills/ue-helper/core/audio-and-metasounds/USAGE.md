# audio-and-metasounds 使用指南

## 功能

播放和控制音频 —— SoundWave、SoundCue、MetaSound、Audio Component、音频衰减。

## 使用方式

```
"帮我播放一个枪声"
"3D 音效随着距离衰减怎么配置？"
"MetaSound 和传统 SoundCue 有什么区别？"
```

## 使用示例

### 示例 1：2D 音效播放

> **你**: "UI 按钮点击播放音效"

> **AI**:
> ```cpp
> UGameplayStatics::PlaySound2D(GetWorld(), ClickSound);
> ```

### 示例 2：3D 位置音效

> **你**: "脚步声跟随玩家位置，有空间感"

> **AI**:
> ```cpp
> UGameplayStatics::PlaySoundAtLocation(GetWorld(), FootstepSound, GetActorLocation());
> ```

### 示例 3：Audio Component 精细控制

> **你**: "引擎声需要根据转速实时调整音高"

> **AI**: 用 UAudioComponent → `SetFloatParameter("RPM", CurrentRPM)`。

## 核心 API

| API | 用途 |
|-----|------|
| PlaySound2D | 无空间感音效（UI、对话） |
| PlaySoundAtLocation | 3D 音效（脚步声、枪声） |
| UAudioComponent | 精细控制（循环、参数） |
| SpawnSoundAttached | 跟随 Actor 播放 |

## 适用场景

- 武器/技能音效
- 环境氛围音
- 载具引擎/UI 声音
