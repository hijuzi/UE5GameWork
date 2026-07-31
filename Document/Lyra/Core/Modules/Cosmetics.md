---
module: cosmetics
complexity: Medium
loc: 1329
file_count: 11
---

# Cosmetics

## 架构

双组件代理模式：

```
ULyraControllerComponent_CharacterParts (Controller, 权威)
    └── 维护 Part 请求列表
    └── Gameplay + Cheat + DevSettings 三种来源

ULyraPawnComponent_CharacterParts (Pawn, 复制)
    └── FLyraCharacterPartList (FastArray)
        └── PreReplicatedAdd → SpawnChildActor (客户端生成)
        └── PreReplicatedRemove → DestroyChildActor

FLyraAnimLayerSelectionSet / FLyraAnimBodyStyleSelectionSet
    └── 标签驱动选择 AnimationLayer / BodyMesh / PhysicsAsset
```

## 关键类

| 类 | 职责 |
|----|------|
| `ULyraPawnComponent_CharacterParts` | FastArray 复制列表，生成/销毁 ChildActor |
| `ULyraControllerComponent_CharacterParts` | 权威端 Part 请求管理 |
| `ULyraCosmeticCheats` | 控制台 Cheat 命令 (AddCharacterPart 等) |
| `ULyraCosmeticDeveloperSettings` | PIE 编辑器预览设置 |
| `FLyraAnimLayerSelectionSet` | 标签选择 AnimationBlueprint 层 |

## 设计模式

- **Handler**: FLyraCharacterPartHandle 不透明句柄
- **FastArray 复制**: 每 Entry 增删，自动生成/销毁 Actor
- **Source-Tracking Enum**: Natural/Cheat/DevSettings，精确生命周期管理
- **Tag 驱动选择**: Animation 和 BodyMesh 通过规则表+Tag 匹配
