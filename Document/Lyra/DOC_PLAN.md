# Lyra 代码库文档生成计划

> 用于后续分批让 AI 执行的指令文档。
> 每次执行时，将对应 Batch 的路径和指令粘贴给 code-to-docs 技能即可。

---

## 环境信息

| 字段 | 值 |
|------|-----|
| 源码根目录 | `e:\UE_Project\UE5GameWork\LyraStarterGame` |
| 文档输出根目录 | `e:\UE_Project\UE5GameWork\Document\Lyra` |
| 技能 | `code-to-docs` (完整模式 / full mode) |
| 语言 | C++ (Unreal Engine 5) |
| 已完成 | Batch 0: CommonUser（输出到 `CommonUser/`） |
| 总规模 | ~703 源文件，~7-9 万行 |

---

## 输出结构

```
Document/Lyra/
├── CommonUser/              ✅ 已完成
├── Infrastructure/          ← Batch 2
├── Core/                    ← Batch 1
├── Systems/                 ← Batch 3
├── Loading/                 ← Batch 4
├── GameFeatures/            ← Batch 5
└── Worlds/                  ← Batch 6
```

---

## Batch 1: Source 核心游戏代码

**优先级**: ⭐⭐⭐ (最高)

### 分析范围

```
e:\UE_Project\UE5GameWork\LyraStarterGame\Source\
  ├── LyraGame/          (457 文件 — 核心游戏逻辑)
  └── LyraEditor/        (25 文件 — 编辑器工具)
```

### 输出路径

```
e:\UE_Project\UE5GameWork\Document\Lyra\Core\
```

### 执行指令

> 使用 code-to-docs 技能，完整模式(full mode)，分析 `e:\UE_Project\UE5GameWork\LyraStarterGame\Source` 目录下所有 C++ 源码，将文档输出到 `e:\UE_Project\UE5GameWork\Document\Lyra\Core`。
>
> 注意：Source 包含 LyraGame（457 文件）和 LyraEditor（25 文件），两者耦合紧密，一起分析。LyraGame 子模块包括：AbilitySystem、Character、Camera、Equipment、Weapons、UI、Settings、GameModes、GameFeatures、Interaction、Inventory、Input、Teams、Player、System、Feedback、Cosmetics、Performance、Replays、Hotfix、Development、Animation、Audio、Messages、Physics、Tests 等。

---

## Batch 2: Plugins — 基础框架层

**优先级**: ⭐⭐⭐ (最高，被其他模块依赖)

### 分析范围

```
e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins\CommonGame\
e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins\AsyncMixin\
e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins\ModularGameplayActors\
e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins\UIExtension\
```

| 插件 | 文件数 | 作用 |
|------|--------|------|
| CommonGame | 29 | 游戏框架基础（GameInstance、GameMode、UI策略） |
| AsyncMixin | 3 | 异步操作 Mixin 工具 |
| ModularGameplayActors | 15 | 模块化 Gameplay Actor 框架 |
| UIExtension | 7 | UI 扩展点系统 |

### 输出路径

```
e:\UE_Project\UE5GameWork\Document\Lyra\Infrastructure\
```

### 执行指令

> 使用 code-to-docs 技能，完整模式(full mode)，分析 `e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins` 下的 CommonGame、AsyncMixin、ModularGameplayActors、UIExtension 四个插件，将文档输出到 `e:\UE_Project\UE5GameWork\Document\Lyra\Infrastructure`。这四个插件构成 Lyra 基础框架层，需要一起分析以展示依赖关系。

---

## Batch 3: Plugins — 系统功能层

**优先级**: ⭐⭐

### 分析范围

```
e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins\GameSettings\
e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins\GameSubtitles\
e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins\GameplayMessageRouter\
```

| 插件 | 文件数 | 作用 |
|------|--------|------|
| GameSettings | 59 | 游戏设置系统（含 UI 控件） |
| GameSubtitles | 10 | 字幕系统 |
| GameplayMessageRouter | 9 | 游戏玩法消息路由 |

### 输出路径

```
e:\UE_Project\UE5GameWork\Document\Lyra\Systems\
```

### 执行指令

> 使用 code-to-docs 技能，完整模式(full mode)，分析 `e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins` 下的 GameSettings、GameSubtitles、GameplayMessageRouter 三个插件，将文档输出到 `e:\UE_Project\UE5GameWork\Document\Lyra\Systems`。

---

## Batch 4: Plugins — 加载与工具层

**优先级**: ⭐

### 分析范围

```
e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins\CommonLoadingScreen\
e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins\CommonStartupLoadingScreen\
e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins\LyraExtTool\
```

| 插件 | 文件数 | 作用 |
|------|--------|------|
| CommonLoadingScreen | 8 | 通用 Loading 界面 |
| CommonStartupLoadingScreen | 5 | 启动 Loading 界面 |
| LyraExtTool | 4 | Lyra 扩展工具 |

### 输出路径

```
e:\UE_Project\UE5GameWork\Document\Lyra\Loading\
```

### 执行指令

> 使用 code-to-docs 技能，完整模式(full mode)，分析 `e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins` 下的 CommonLoadingScreen、CommonStartupLoadingScreen、LyraExtTool 三个插件，将文档输出到 `e:\UE_Project\UE5GameWork\Document\Lyra\Loading`。

---

## Batch 5: Plugins — GameFeatures 游戏玩法

**优先级**: ⭐⭐

### 分析范围

```
e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins\GameFeatures\ShooterCore\
e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins\GameFeatures\ShooterTests\
e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins\GameFeatures\TopDownArena\
```

| 插件 | 文件数 | 作用 |
|------|--------|------|
| ShooterCore | 25 | 射击核心玩法（武器、技能、重生等） |
| ShooterTests | 14 | 射击玩法测试 |
| TopDownArena | 10 | 俯视角竞技场玩法 |

### 输出路径

```
e:\UE_Project\UE5GameWork\Document\Lyra\GameFeatures\
```

### 执行指令

> 使用 code-to-docs 技能，完整模式(full mode)，分析 `e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins\GameFeatures` 下的 ShooterCore、ShooterTests、TopDownArena 三个 GameFeature 子插件，将文档输出到 `e:\UE_Project\UE5GameWork\Document\Lyra\GameFeatures`。

---

## Batch 6: Plugins — 世界/场景层

**优先级**: ⭐

### 分析范围

```
e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins\PocketWorlds\
e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins\LyraExampleContent\
```

| 插件 | 文件数 | 作用 |
|------|--------|------|
| PocketWorlds | 11 | 独立小世界/关卡管理 |
| LyraExampleContent | - | Lyra 示例内容（可能无源码） |

### 输出路径

```
e:\UE_Project\UE5GameWork\Document\Lyra\Worlds\
```

### 执行指令

> 使用 code-to-docs 技能，完整模式(full mode)，分析 `e:\UE_Project\UE5GameWork\LyraStarterGame\Plugins` 下的 PocketWorlds、LyraExampleContent 两个插件，将文档输出到 `e:\UE_Project\UE5GameWork\Document\Lyra\Worlds`。如果 LyraExampleContent 无 C++ 源码则跳过。

---

## 跨 Vault 索引（全部完成后创建）

**输出路径**: `e:\UE_Project\UE5GameWork\Document\Lyra\README.md`

### 执行指令

> 扫描 `e:\UE_Project\UE5GameWork\Document\Lyra\` 下所有子目录中的 `index.md` 文件，整合生成为一个总 README.md，包含：
> 1. 所有 Batch 的导航链接
> 2. 核心架构关系图（哪个模块依赖哪个）
> 3. 推荐阅读顺序（新人入门路径）
>
> 输出到 `e:\UE_Project\UE5GameWork\Document\Lyra\README.md`。

---

## 执行注意事项

1. **每次执行一个 Batch**，避免文档质量下降和 token 超限。
2. 粘贴执行指令时，**同时附上对应源码目录路径**（用 IDE 中附件功能）。
3. 如果某个 Batch 太大（如 Batch 1 的 482 文件），AI 可能会请求进一步拆分 — 建议接受。
4. `GreenRoom` 和 `RedRoom` 插件在扫描中未发现源码，可能在后期或用 Blueprint 实现，暂不纳入。
