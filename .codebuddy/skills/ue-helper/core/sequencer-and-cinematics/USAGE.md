# sequencer-and-cinematics 使用指南

## 功能

从 C++ 创建和驱动 UE 过场动画 —— ULevelSequence、UMovieSceneSequencePlayer、绑定 Actor、动态控制。

## 使用方式

```
"帮我用 C++ 播放一段过场动画"
"怎么在过场中动态替换角色？"
"Sequencer 怎么从 C++ 控制播放/暂停？"
```

## 使用示例

### 示例 1：播放过场动画

> **你**: "进入 Boss 房间时播放一段 Boss 登场动画"

> **AI**:
> ```cpp
> ALevelSequenceActor* SeqActor;
> ULevelSequence* Sequence = LoadObject<ULevelSequence>(...);
> ULevelSequencePlayer* Player = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), Sequence, FMovieSceneSequencePlaybackSettings(), SeqActor);
> Player->Play();
> ```

### 示例 2：动态绑定 Actor

> **你**: "过场中需要绑定当前玩家角色"

> **AI**: `SeqActor->GetSequencePlayer()->SetBinding(MovieSceneBindingID, {PlayerCharacter});`

### 示例 3：控制播放

> **你**: "过场可以被跳过"

> **AI**: `Player->Pause()` / `Player->Stop()` / `Player->JumpToPosition(FrameTime)`。

## 核心组件

| 组件 | 用途 |
|------|------|
| ULevelSequence | 过场动画资产 |
| ALevelSequenceActor | 关卡中的序列 Actor |
| ULevelSequencePlayer | 播放控制 |

## 适用场景

- 剧情过场动画
- 技能动画序列
- 动态绑定和 Control Rig 控制
