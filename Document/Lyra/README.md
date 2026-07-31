---
vault: lyra-docs
generated: 2026-07-31
mode: full
---

# Lyra Starter Game — 代码文档

> 基于 Epic Games LyraStarterGame 的完整 C++ 代码文档，由 code-to-docs 自动生成。

## 文档结构

```
Document/Lyra/
├── README.md               ← 你在这里
├── CommonUser/             ✅ 16 个文档 (Batch 0)
├── Infrastructure/         ✅ 4 模块, 5,626 LOC (Batch 2)
├── Core/                   ✅ 23 模块, ~41,000 LOC (Batch 1a~1e)
├── Systems/                ✅ 3 模块, ~6,964 LOC (Batch 3)
├── Loading/                ✅ 3 模块, ~1,286 LOC (Batch 4)
├── GameFeatures/           ✅ 3 模块, ~3,444 LOC (Batch 5)
└── Worlds/                 ✅ 2 模块, ~699 LOC (Batch 6)
```

**总计**: 43 模块, ~58,000 LOC, 100% 覆盖率

---

## 快速导航

### 按层次浏览

| 层 | 目录 | 包含 |
|----|------|------|
| **用户体验** | [CommonUser](CommonUser/) | 登录/认证/平台授权 |
| **基础框架** | [Infrastructure](Infrastructure/) | CommonGame, AsyncMixin, ModularGA, UIExtension |
| **核心游戏** | [Core](Core/) | 战斗/角色/UI/武器/系统/多人 |
| **系统功能** | [Systems](Systems/) | 设置/字幕/消息路由 |
| **加载流程** | [Loading](Loading/) | 加载屏/启动屏/工具 |
| **游戏玩法** | [GameFeatures](GameFeatures/) | 射击/测试/俯视角 |
| **世界场景** | [Worlds](Worlds/) | 口袋世界/示例内容 |

### 核心文档入口

| 文档 | 路径 |
|------|------|
| 基础设施总览 | [Infrastructure/Index.md](Infrastructure/Index.md) |
| Core 系统总览 | [Core/Index.md](Core/Index.md) |
| Core 架构图 | [Core/Architecture/System%20Overview.md](Core/Architecture/System%20Overview.md) |
| Core 依赖图 | [Core/Architecture/Dependency%20Map.md](Core/Architecture/Dependency%20Map.md) |
| Core 健康度 | [Core/Health/Health%20Summary.md](Core/Health/Health%20Summary.md) |
| 系统功能层 | [Systems/Index.md](Systems/Index.md) |
| 加载工具层 | [Loading/Index.md](Loading/Index.md) |
| 游戏玩法 | [GameFeatures/Index.md](GameFeatures/Index.md) |
| 世界场景 | [Worlds/Index.md](Worlds/Index.md) |
| 验证报告 | [VERIFICATION_REPORT.md](VERIFICATION_REPORT.md) |

### 按角色推荐阅读

| 角色 | 推荐路径 |
|------|---------|
| **新人入门** | Infrastructure → Core/Character → Core/Input → Core/UI |
| **战斗程序员** | Core/AbilitySystem → Core/Weapons → Core/Equipment → GameFeatures/ShooterCore |
| **UI 设计师** | Infrastructure/UIExtension → Infrastructure/CommonGame → Core/UI → Systems/GameSettings |
| **网络工程师** | Core/System (ReplicationGraph) → Core/Player → Core/GameModes → Core/Teams |
| **工具程序员** | Core/LyraEditor → Core/Performance → Loading/LyraExtTool |

---

## 生成信息

- **工具**: code-to-docs (Full Mode)
- **源码**: LyraStarterGame (UE5)
- **日期**: 2026-07-31
- **批次**: 7 个 Batch (0 + 1a~1e + 2~6)
