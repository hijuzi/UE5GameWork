# Skills 技能使用指南

## 什么是 Skill？

Skill 是 AI 编程助手的**专业能力扩展包**。你可以把它理解为一个"领域专家顾问"——当你需要做某个特定方向的工作时（比如写 UE5 网络同步、生成代码文档），AI 会自动加载对应 Skill，以最专业的标准辅助你完成任务。

> 当前共 **65 个技能**，按用途分为三大类。

---

## 目录结构

```
.codebuddy/skills/
├── project/          # 项目级技能（代码审查、质量控制）
├── tool/             # 工具类技能（文档生成、文本润色）
└── ue-helper/        # UE5 专业技术技能
    ├── core/         # 引擎核心系统 (44个)
    ├── ultra-dynamic-sky/    # UDS 天空插件 (10个)
    └── ultra-dynamic-weather/  # UDW 天气插件 (5个)
```

---

## 一、工具类技能 (tool/)

| 技能 | 用途 | 何时使用 |
|------|------|---------|
| `code-to-docs` | 从代码生成架构文档/Obsidian 知识库 | 需要为代码库生成完整文档 |
| `code-to-docs-digest` | 读取已有文档上下文（只读） | 开始编程前快速了解代码库 |
| `code-to-docs-update` | 增量更新文档 | 代码改动后同步更新文档 |
| `code-to-docs-hooks` | 自动化文档更新钩子 | 提交 Git 后自动刷新文档 |
| `humanizer` | 消除 AI 写作痕迹，让文字更自然 | 润色文档、让 AI 生成的文字更有人味 |

## 二、项目级技能 (project/)

| 技能 | 用途 | 何时使用 |
|------|------|---------|
| `ue-code-review` | UE5 C++ 代码审查 | 检查代码是否符合 Epic 标准、是否存在性能/安全/风格问题 |

## 三、UE5 引擎核心技能 (ue-helper/core/) — 44个

### 编程基础
| 技能 | 功能简述 |
|------|---------|
| `cpp-fundamentals` | UObject 反射系统、UCLASS/USTRUCT 宏 |
| `coding-standards` | Epic C++ 编码规范（类型前缀、命名约定） |
| `core-types-and-containers` | UE 容器（TArray/TMap/TSet）替代 STL |
| `modules-and-build-system` | 模块结构和 `*.Build.cs` 构建配置 |
| `memory-and-gc` | UObject 生命周期和垃圾回收 |
| `logging-and-assertions` | UE_LOG 日志和运行时断言 |
| `debugging-techniques` | 原生调试器、Live Coding、可视化调试 |

### 游戏框架
| 技能 | 功能简述 |
|------|---------|
| `gameplay-framework` | GameInstance/GameMode/GameState 架构 |
| `actors-and-components` | Actor 和 Component 组合模式 |
| `character-and-movement` | ACharacter 和角色移动系统 |
| `subsystems` | 引擎级单例服务（UWorldSubsystem 等） |
| `blueprint-cpp-integration` | UFUNCTION/UPROPERTY 暴露蓝图 |
| `blueprint-fundamentals` | 蓝图可视化脚本概念 |

### 输入、动画、物理
| 技能 | 功能简述 |
|------|---------|
| `enhanced-input` | Enhanced Input 系统（UInputAction） |
| `animation-system` | AnimInstance/动画蓝图 |
| `control-rig-and-ik` | Control Rig 程序化动画/IK |
| `physics-and-chaos` | Chaos 物理碰撞、射线检测 |

### AI、网络、数据
| 技能 | 功能简述 |
|------|---------|
| `ai-and-navigation` | AIController、行为树、导航网格 |
| `gameplay-ability-system` | GAS 技能/属性/GameplayEffect |
| `gameplay-tags` | FGameplayTag 层级标签管理 |
| `networking-and-replication` | 服务器权威多人游戏（RPC、属性复制） |
| `data-driven-design` | DataAsset/DataTable 数据驱动设计 |
| `save-and-load` | SaveGame 存档系统 |

### 渲染、音频、UI
| 技能 | 功能简述 |
|------|---------|
| `umg-and-slate` | UMG User Widget 和 Slate 底层 UI |
| `materials-and-shaders` | UMaterial 材质与着色器 |
| `niagara-vfx` | Niagara 粒子特效 |
| `audio-and-metasounds` | SoundCue/MetaSound 音频系统 |
| `lighting-and-lumen` | 光照组件和 Lumen 全局光照 |
| `nanite-and-rendering` | Nanite 虚拟化几何体渲染 |

### 开发流程
| 技能 | 功能简述 |
|------|---------|
| `asset-management` | 硬引用/软引用和资源加载 |
| `editor-scripting-and-python` | Python 编辑器自动化、Blutility |
| `automation-and-testing` | UE 自动化测试框架 |
| `profiling-and-optimization` | Unreal Insights 性能分析 |
| `packaging-and-deployment` | Cook 打包和发布 |
| `plugins-and-modules` | 创建和管理 UE 插件 |
| `navigating-engine-source` | 定位和引用引擎源码 API |
| `importing-content` | Interchange 框架导入外部资源 |
| `levels-and-world-partition` | 关卡结构和 World Partition 流送 |
| `meshes-static-and-skeletal` | 静态/骨骼网格资产操作 |
| `timers-and-async` | FTimerManager、AsyncTask、UE Tasks |
| `delegates-and-events` | 委托和事件回调系统 |
| `project-structure` | .uproject 描述符和项目布局 |
| `sequencer-and-cinematics` | Sequencer 过场动画控制 |
| `landscape-and-foliage` | 地形和植被系统 |

## 四、UDS 天空插件技能 (ue-helper/ultra-dynamic-sky/) — 10个

| 技能 | 功能简述 |
|------|---------|
| `uds-setup-and-modes` | 安装配置 UDS、选择天空模式 |
| `uds-time` | 昼夜循环、时间动画化 |
| `uds-simulation` | 真实天文模拟（经纬度、时区） |
| `uds-sun-moon-stars` | 太阳/月亮/星星/极光控制 |
| `uds-clouds` | 体积云/静态云/2D 云配置 |
| `uds-fog-and-atmosphere` | 雾、体积雾、大气散射 |
| `uds-lighting-and-shadows` | 天空光、阴影、曝光控制 |
| `uds-modifiers-configs-state` | 天空预设保存/加载、后处理 |
| `uds-performance-mobile-troubleshooting` | 性能优化、移动端适配、故障排查 |
| `uds-cinematics-rendering` | Sequencer 过场动画、Movie Render Queue |

## 五、UDW 天气插件技能 (ue-helper/ultra-dynamic-weather/) — 5个

| 技能 | 功能简述 |
|------|---------|
| `udw-setup-and-state` | 设置 UDW、控制雨雪、改变天气 |
| `udw-particles-lightning-wind-sounds` | 雨雪粒子、闪电、风效、天气音效 |
| `udw-material-and-screen-effects` | 材质响应天气、屏幕水滴/霜、彩虹 |
| `udw-random-seasons-temperature` | 随机天气变化、季节循环、温度系统 |
| `udw-spatial-weather` | 区域天气覆盖、径向风暴、天气遮罩 |

---

## 如何使用

### 方法一：自动匹配（推荐）

直接在对话中描述你的需求，AI 会自动加载匹配的 Skill。例如：

```
"帮我写一个 UE5 的 Character 移动组件"     → 自动加载 character-and-movement
"这个 C++ 代码帮我 review 一下"            → 自动加载 ue-code-review
"给这个项目生成技术文档"                  → 自动加载 code-to-docs
```

### 方法二：手动指定 Skill

也可以明确告诉 AI 使用哪个 Skill：

```
"用 networking-and-replication 技能，帮我实现血量同步"
"用 humanizer 润色这段技术文档"
```

### 工具类技能调用语法

`tool/` 下的文档类技能需要特殊调用：

- **code-to-docs**: 让 AI 分析代码库并生成 Obsidian 文档
- **code-to-docs-digest**: 编程前让 AI 先读取已有文档了解背景
- **code-to-docs-update**: Git 提交后让 AI 增量更新文档
- **humanizer**: 任何文本润色时自动生效

---

## 快速上手建议

1. **写代码前** → 先用 `code-to-docs-digest` 了解项目结构
2. **提交代码前** → 用 `ue-code-review` 做代码审查
3. **功能完成后** → 用 `code-to-docs-update` 同步文档
4. **发布文档前** → 用 `humanizer` 让文字更自然

---

> 更多 UE5 技能详细信息请查看各 `SKILL.md` 文件。所有技能覆盖 UE5.7 引擎版本。
