# gameplay-framework 使用指南

## 功能

实现 UE5 游戏框架：GameInstance、GameMode、GameState、PlayerController、Pawn/Character、PlayerState、HUD，理解服务器登录/重生流程。

## 使用方式

```
"帮我搭建游戏框架，包含登录和重生"
"这段逻辑应该放在哪个类里？"
"怎么设置默认 Pawn 和 Controller？"
```

## 使用示例

### 示例 1：搭建基础游戏框架

> **你**: "新建一个多人射击游戏的基础框架"

> **AI**: 创建 CustomGameInstance → CustomGameMode（设置默认 Pawn 和 PlayerController）→ CustomGameState（复制分数/时间）→ CustomPlayerState（复制玩家数据）。

### 示例 2：逻辑归属判断

> **你**: "玩家得分逻辑放哪？"

> **AI**: GameMode（服务器权威，记分规则）→ 更新 PlayerState（复制给客户端显示）。

### 示例 3：处理登录和重生

> **你**: "玩家死亡后 3 秒重生，怎么做？"

> **AI**: 在 GameMode 中 `RestartPlayer()` 处理，用 Timer 延迟。

## 类职责速查

| 类 | 存活范围 | 放什么逻辑 |
|----|---------|-----------|
| GameInstance | 整个进程 | 持久数据、跨关卡系统 |
| GameMode | 当前关卡（服务器） | 游戏规则、胜利条件、重生 |
| GameState | 当前关卡（全端） | 比分、倒计时（复制） |
| PlayerController | 单个玩家 | 输入、UI 管理 |
| PlayerState | 单个玩家（全端） | 玩家名、得分（复制） |
| Pawn/Character | 单个玩家 | 移动、物理、动画 |

## 适用场景

- 设置游戏规则和默认类
- 处理玩家登录/生成/占有流程
- 复制游戏或玩家状态
- 获取重生逻辑
- 单一"这段逻辑该放哪"决策
