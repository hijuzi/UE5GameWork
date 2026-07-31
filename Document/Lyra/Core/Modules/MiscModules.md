---
module: misc-audio-hotfix-dev-replays-tests-physics
complexity: Low-Medium
loc: 2501
file_count: 30
---

# 杂项模块

## Audio (561 LOC, 4 文件) — Medium

`ULyraAudioMixEffectsSubsystem` (WorldSubsystem): 将用户音量设置应用到 Control Bus Mix。HDR/LDR Submix 效果链切换。监听 LoadingScreen 可见性以激活加载混音。

## Hotfix (427 LOC, 6 文件) — Medium

- **ULyraHotfixManager**: 扩展 OnlineHotfixManager, 自定义 INI/DeviceProfile 修补, DDoS 配置重载
- **ULyraRuntimeOptions**: `-ro.*` 命令行开关
- **ULyraTextHotfixConfig**: FText 热修复 (PolyglotTextData 注入)

## Development (557 LOC, 6 文件) — Medium

- **ULyraDeveloperSettings**: PIE Experience 覆盖, Bot 数量, 跳过装饰性背景
- **ULyraPlatformEmulationSettings**: 编辑器平台模拟 (Traits 注入, 设备档案)
- **ULyraBotCheats**: Cheat 命令 AddBot/RemoveBot

## Replays (426 LOC, 4 文件) — Medium

- **ULyraReplaySubsystem** (GameInstanceSubsystem): 平台支持检测, 播放, 录制, 自动清理旧录像
- **UAsyncAction_QueryReplays**: 异步查询可用录像列表

## Tests (464 LOC, 7 文件) — Medium

- **ULyraGameplayRpcRegistrationComponent**: HTTP RPC 端点 (cheat 命令执行, 玩家状态查询)
- **Gauntlet 启动测试**: 菜单自动化 (AutomationDriver) + Experience 加载验证

## Physics (66 LOC, 3 文件) — Low

- 4 个自定义碰撞通道: Interaction, Weapon, Weapon_Capsule, Weapon_Multi
- **UPhysicalMaterialWithTags**: 带 GameplayTags 的物理材质子类
