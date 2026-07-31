---
module: lyragame-root
complexity: Low
loc: 233
file_count: 6
---

# LyraGame Root

## 概述

LyraGame 模块的根级源文件：模块入口、全局 GameplayTag 注册表、日志基础设施。

## 模块入口

**FLyraGameModule** (LyraGameModule.cpp, 无独立头文件):
- 主游戏模块 (`IMPLEMENT_PRIMARY_GAME_MODULE`)
- `StartupModule()` 和 `ShutdownModule()` 均为空 — 实际启动由 Experience 系统处理

## GameplayTag 注册表

全部使用原生 C++ 定义 (`UE_DEFINE_GAMEPLAY_TAG_COMMENT`)，分 10 个语义类别：

| 类别 | 标签数 | 用途 |
|------|--------|------|
| Ability_ActivateFail | 7 | 能力激活失败原因 |
| Ability | 8 | 能力操作/事件 (InputTag 等) |
| GameplayEvent | 6 | 通用游戏事件 (Death/Reset 等) |
| Movement | 3 | 移动模式 (Walking/PAD_* 等) |
| Status | 7 | 状态标签 (Death/Despawning/等) |
| Cheat | 4 | 作弊标签 (UnlimitedHealth 等) |
| ShooterGame | 17 | 射击游戏：武器/热身/回合/技能 |
| ShooterWeapon | 5 | 武器状态 (Equipping/PrimaryClipFull 等) |
| GameMode | 8 | 游戏模式/阶段标签 |
| UI | 25 | UI 层/扩展/动作 (全量 UI 命名空间) |

工具函数: `FindTagByString(TagString, bMatchPartial)`

## 日志系统

4 个日志通道 + 上下文注释工具：

- `LogLyra` (主日志)
- `LogLyraExperience`
- `LogLyraAbilitySystem`
- `LogLyraTeams`

`LogVerbose(LogName, FString)` 自动附加上下文（客户端/服务器 + LocalPlayerIndex）。

## 依赖

GameplayTags (核心), GameplayAbilities, ModularGameplay, EnhancedInput
