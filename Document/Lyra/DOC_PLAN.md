# Lyra 代码库文档生成计划

> 用于后续分批让 AI 执行的指令文档。
> 每次执行时，将对应 Batch 的路径和指令粘贴给 code-to-docs 技能即可。

---

## 环境信息

| 字段 | 值 |
|------|-----|
| workspace 根目录 | 当前工作空间（即 `UE5GameWork/`） |
| 源码相对路径 | `LyraStarterGame/` |
| 文档相对路径 | `Document/Lyra/` |
| 技能 | `code-to-docs` (完整模式 / full mode) |
| 语言 | C++ (Unreal Engine 5) |
| 已完成 | Batch 0: CommonUser（输出到 `Document/Lyra/CommonUser/`） |
| 总规模 | ~703 源文件，~7-9 万行 |

> 以下所有路径均为**相对路径**，基于 workspace 根目录。

---

## 输出结构

```
Document/Lyra/
├── CommonUser/              ✅ 已完成
├── Core/                    ← Batch 1a~1e (5 个子批次)
├── Infrastructure/          ← Batch 2
├── Systems/                 ← Batch 3
├── Loading/                 ← Batch 4
├── GameFeatures/            ← Batch 5
└── Worlds/                  ← Batch 6
```

---

## Batch 1a: 核心战斗系统

**优先级**: ⭐⭐⭐ (最高)
**预估文件数**: ~95

### 分析范围

```
LyraStarterGame/Source/LyraGame/
  ├── AbilitySystem/      (51 文件 — GAS 技能系统)
  ├── Character/          (16 文件 — 角色/角色组件)
  ├── Camera/             (12 文件 — 相机系统)
  ├── Animation/          (2 文件 — 动画实例)
  └── Input/              (14 文件 — 输入映射/配置)
```

### 输出路径

```
Document/Lyra/Core\
```

### 执行指令

> 使用 code-to-docs 技能，完整模式(full mode)，分析 `LyraStarterGame/Source/LyraGame` 下的 AbilitySystem、Character、Camera、Animation、Input 五个子模块，将文档输出到 `Document/Lyra/Core`。
>
> 这些模块构成 Lyra 的核心战斗与操作层：GAS 技能系统（属性、技能、效果、AbilitySet）、角色系统（LyraCharacter、PawnData、HeroComponent）、相机模式、动画蓝图绑定、增强输入配置。

---

## Batch 1b: 武器装备与交互

**优先级**: ⭐⭐⭐ (最高)
**预估文件数**: ~91

### 分析范围

```
LyraStarterGame/Source/LyraGame/
  ├── Weapons/            (16 文件 — 武器系统)
  ├── Equipment/          (12 文件 — 装备系统)
  ├── Inventory/          (16 文件 — 背包/物品)
  ├── Interaction/        (17 文件 — 交互系统)
  ├── Feedback/           (19 文件 — 反馈系统)
  └── Cosmetics/          (11 文件 — 外观/装饰)
```

### 输出路径

```
Document/Lyra/Core\
```

### 执行指令

> 使用 code-to-docs 技能，完整模式(full mode)，分析 `LyraStarterGame/Source/LyraGame` 下的 Weapons、Equipment、Inventory、Interaction、Feedback、Cosmetics 六个子模块，将文档输出到 `Document/Lyra/Core`。
>
> 这些模块构成 Lyra 的装备与交互层：武器（LyraWeapon、弹道扩散）、装备管理器（LyraEquipmentManager）、背包物品实例（LyraInventoryItem）、交互系统（IInteractableTarget、GameplayAbility 交互）、反馈系统（ContextEffects 上下文效果库）、外观系统（PawnComponent、角色外观）。

---

## Batch 1c: UI 与 HUD

**优先级**: ⭐⭐⭐ (最高)
**预估文件数**: ~83

### 分析范围

```
LyraStarterGame/Source/LyraGame/
  ├── UI/                 (79 文件 — 全量 UI/Widget)
  └── 根文件:              (6 文件 — Build.cs, Module, GameplayTags, LogChannels)
```

### 输出路径

```
Document/Lyra/Core\
```

### 执行指令

> 使用 code-to-docs 技能，完整模式(full mode)，分析 `LyraStarterGame/Source/LyraGame` 下的 UI 子模块以及根目录的 `LyraGameModule.*`、`LyraGameplayTags.*`、`LyraLogChannels.*`，将文档输出到 `Document/Lyra/Core`。
>
> UI 模块是 Lyra 最大的单模块（79 文件），包含 HUD、Layer/Layout、Indicator、Subsystem、Frontend、Common 控件、自定义 Slate UI 等全部界面逻辑。根文件包含模块声明、全局 GameplayTag 定义和日志通道。

---

## Batch 1d: 系统设置与消息

**优先级**: ⭐⭐
**预估文件数**: ~93

### 分析范围

```
LyraStarterGame/Source/LyraGame/
  ├── Settings/           (41 文件 — 游戏设置/控制台)
  ├── System/             (27 文件 — 系统基础服务)
  ├── GameFeatures/       (16 文件 — GameFeature 桥梁)
  └── Messages/           (9 文件 — 消息定义)
```

### 输出路径

```
Document/Lyra/Core\
```

### 执行指令

> 使用 code-to-docs 技能，完整模式(full mode)，分析 `LyraStarterGame/Source/LyraGame` 下的 Settings、System、GameFeatures、Messages 四个子模块，将文档输出到 `Document/Lyra/Core`。
>
> 这些模块构成 Lyra 的基础服务层：Settings（游戏设置、控制台命令、系统音频/性能设置）、System（子系统、Experience 管理、AssetManager、SignificanceManager）、GameFeatures（GameFeatureAction 集成桥接）、Messages（GameplayMessage 结构定义）。

---

## Batch 1e: 游戏模式与多人 + Editor

**优先级**: ⭐⭐
**预估文件数**: ~120

### 分析范围

```
LyraStarterGame/Source/LyraGame/
  ├── GameModes/          (20 文件 — 游戏模式/状态机)
  ├── Player/             (16 文件 — 玩家控制器/PlayerState)
  ├── Teams/              (22 文件 — 队伍系统)
  ├── Audio/              (4 文件 — 音频子系统设置)
  ├── Hotfix/             (6 文件 — 热更新管理器)
  ├── Development/        (6 文件 — 开发者工具/模拟)
  ├── Performance/        (6 文件 — 性能统计/内存诊断)
  ├── Physics/            (3 文件 — 碰撞通道/物理材质)
  ├── Replays/            (4 文件 — 回放子系统)
  ├── Tests/              (7 文件 — 自动化测试)
  └── LyraEditor/         (25 文件 — 编辑器工具/验证)
```

### 输出路径

```
Document/Lyra/Core\
```

### 执行指令

> 使用 code-to-docs 技能，完整模式(full mode)，分析 `LyraStarterGame/Source/LyraGame` 下的 GameModes、Player、Teams、Audio、Hotfix、Development、Performance、Physics、Replays、Tests 子模块，以及 `LyraStarterGame/Source/LyraEditor` 全量，将文档输出到 `Document/Lyra/Core`。
>
> 这些模块构成 Lyra 的游戏模式与辅助系统：GameModes（GameMode、GameState、Experience 加载、Bot 控制）、Player（LyraPlayerController、LyraPlayerState、CheatManager）、Teams（LyraTeamSubsystem、TeamState）、杂项系统（音频、热更新、开发者工具、性能统计、物理通道、回放、测试）以及编辑器工具链（内容验证 Commandlet、蓝图工具、碰撞检查、资产工厂）。

---

## Batch 2: Plugins — 基础框架层

**优先级**: ⭐⭐⭐ (最高，被其他模块依赖)

### 分析范围

```
LyraStarterGame/Plugins\CommonGame\
LyraStarterGame/Plugins\AsyncMixin\
LyraStarterGame/Plugins\ModularGameplayActors\
LyraStarterGame/Plugins\UIExtension\
```

| 插件 | 文件数 | 作用 |
|------|--------|------|
| CommonGame | 29 | 游戏框架基础（GameInstance、GameMode、UI策略） |
| AsyncMixin | 3 | 异步操作 Mixin 工具 |
| ModularGameplayActors | 15 | 模块化 Gameplay Actor 框架 |
| UIExtension | 7 | UI 扩展点系统 |

### 输出路径

```
Document/Lyra/Infrastructure\
```

### 执行指令

> 使用 code-to-docs 技能，完整模式(full mode)，分析 `LyraStarterGame/Plugins` 下的 CommonGame、AsyncMixin、ModularGameplayActors、UIExtension 四个插件，将文档输出到 `Document/Lyra/Infrastructure`。这四个插件构成 Lyra 基础框架层，需要一起分析以展示依赖关系。

---

## Batch 3: Plugins — 系统功能层

**优先级**: ⭐⭐

### 分析范围

```
LyraStarterGame/Plugins\GameSettings\
LyraStarterGame/Plugins\GameSubtitles\
LyraStarterGame/Plugins\GameplayMessageRouter\
```

| 插件 | 文件数 | 作用 |
|------|--------|------|
| GameSettings | 59 | 游戏设置系统（含 UI 控件） |
| GameSubtitles | 10 | 字幕系统 |
| GameplayMessageRouter | 9 | 游戏玩法消息路由 |

### 输出路径

```
Document/Lyra/Systems\
```

### 执行指令

> 使用 code-to-docs 技能，完整模式(full mode)，分析 `LyraStarterGame/Plugins` 下的 GameSettings、GameSubtitles、GameplayMessageRouter 三个插件，将文档输出到 `Document/Lyra/Systems`。

---

## Batch 4: Plugins — 加载与工具层

**优先级**: ⭐

### 分析范围

```
LyraStarterGame/Plugins\CommonLoadingScreen\
LyraStarterGame/Plugins\CommonStartupLoadingScreen\
LyraStarterGame/Plugins\LyraExtTool\
```

| 插件 | 文件数 | 作用 |
|------|--------|------|
| CommonLoadingScreen | 8 | 通用 Loading 界面 |
| CommonStartupLoadingScreen | 5 | 启动 Loading 界面 |
| LyraExtTool | 4 | Lyra 扩展工具 |

### 输出路径

```
Document/Lyra/Loading\
```

### 执行指令

> 使用 code-to-docs 技能，完整模式(full mode)，分析 `LyraStarterGame/Plugins` 下的 CommonLoadingScreen、CommonStartupLoadingScreen、LyraExtTool 三个插件，将文档输出到 `Document/Lyra/Loading`。

---

## Batch 5: Plugins — GameFeatures 游戏玩法

**优先级**: ⭐⭐

### 分析范围

```
LyraStarterGame/Plugins\GameFeatures\ShooterCore\
LyraStarterGame/Plugins\GameFeatures\ShooterTests\
LyraStarterGame/Plugins\GameFeatures\TopDownArena\
```

| 插件 | 文件数 | 作用 |
|------|--------|------|
| ShooterCore | 25 | 射击核心玩法（武器、技能、重生等） |
| ShooterTests | 14 | 射击玩法测试 |
| TopDownArena | 10 | 俯视角竞技场玩法 |

### 输出路径

```
Document/Lyra/GameFeatures\
```

### 执行指令

> 使用 code-to-docs 技能，完整模式(full mode)，分析 `LyraStarterGame/Plugins\GameFeatures` 下的 ShooterCore、ShooterTests、TopDownArena 三个 GameFeature 子插件，将文档输出到 `Document/Lyra/GameFeatures`。

---

## Batch 6: Plugins — 世界/场景层

**优先级**: ⭐

### 分析范围

```
LyraStarterGame/Plugins\PocketWorlds\
LyraStarterGame/Plugins\LyraExampleContent\
```

| 插件 | 文件数 | 作用 |
|------|--------|------|
| PocketWorlds | 11 | 独立小世界/关卡管理 |
| LyraExampleContent | - | Lyra 示例内容（可能无源码） |

### 输出路径

```
Document/Lyra/Worlds\
```

### 执行指令

> 使用 code-to-docs 技能，完整模式(full mode)，分析 `LyraStarterGame/Plugins` 下的 PocketWorlds、LyraExampleContent 两个插件，将文档输出到 `Document/Lyra/Worlds`。如果 LyraExampleContent 无 C++ 源码则跳过。

---

## 跨 Vault 索引（全部完成后创建）

**输出路径**: `Document/Lyra/README.md`

### 执行指令

> 扫描 `Document/Lyra/` 下所有子目录中的 `index.md` 文件，整合生成为一个总 README.md，包含：
> 1. 所有 Batch 的导航链接
> 2. 核心架构关系图（哪个模块依赖哪个）
> 3. 推荐阅读顺序（新人入门路径）
>
> 输出到 `Document/Lyra/README.md`。

---

## 执行注意事项

1. **每次执行一个子批次（Batch 1a~1e 各约 80-120 文件，Batch 2~6 各约 20-100 文件）**，避免文档质量下降和 token 超限。
2. 粘贴执行指令时，**同时附上对应源码目录路径**（用 IDE 中附件功能）。
3. **Batch 1a~1e 都输出到同一目录 `Document/Lyra/Core/`**，由 `code-to-docs` 每次执行时增量合并已有文档。
4. **推荐执行顺序**: 0(已完成) → 2(Infrastructure, 基础依赖) → 1a~1e(Core) → 3~6，最后创建跨 Vault 索引 README.md。
5. `GreenRoom` 和 `RedRoom` 插件在扫描中未发现源码，可能在后期或用 Blueprint 实现，暂不纳入。
