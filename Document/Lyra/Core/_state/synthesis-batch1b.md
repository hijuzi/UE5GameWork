# Core Synthesis — Batch 1b Update

## Modules Added

| 模块 | LOC | 文件 | 复杂度 | 核心类 |
|------|-----|------|--------|--------|
| Weapons | 2,289 | 16 | High | LyraWeaponInstance, LyraRangedWeaponInstance, LyraWeaponStateComponent, LyraWeaponSpawner |
| Equipment | 1,195 | 12 | Medium | LyraEquipmentManagerComponent, LyraEquipmentInstance, LyraQuickBarComponent, LyraEquipmentDefinition |
| Inventory | 1,049 | 16 | Medium | LyraInventoryItemInstance, LyraInventoryManagerComponent, LyraInventoryItemDefinition |
| Interaction | 1,107 | 17 | Medium | IInteractableTarget, LyraGameplayAbility_Interact, AbilityTask_GrantNearbyInteraction |
| Feedback | 1,927 | 19 | Medium | LyraContextEffectsSubsystem, LyraContextEffectsLibrary, NumberPopsSubsystem |
| Cosmetics | 1,329 | 11 | Medium | LyraPawnComponent_CharacterParts, LyraControllerComponent_CharacterParts |

## Dependency Chain

```
Inventory ← Equipment ← Weapons
                         ↓
                  AbilitySystem
                         ↑
Interaction ────────────┘

Feedback (独立, WorldSubsystem based)
Cosmetics (独立, Component based)
```

## Architecture Patterns

- **Weapons**: 双层实例 + GAS 技能：ULyraWeaponInstance（数据）→ ULyraGameplayAbility_RangedWeapon（行为），通过 SourceObject 追溯
- **Equipment**: FastArray 复制 + AbilitySet 授予：装备时自动授予技能集，卸载时移除
- **Inventory**: Fragment 模式：定义由多个可编辑 Fragment 组成，实例化时各自初始化
- **Interaction**: 接口 + 轮询任务：IInteractableTarget 接口，AbilityTask 周期扫描
- **Feedback**: WorldSubsystem 中介：ContextEffects 和 NumberPops 通过 Subsystem 协调效果生成
- **Cosmetics**: 双组件代理：Controller 组件管理请求，Pawn 组件 FastArray 复制实现生成
