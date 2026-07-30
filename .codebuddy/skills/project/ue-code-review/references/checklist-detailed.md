# Detailed Checklist with Engine Evidence

Expanded review checklist with specific UE5 engine patterns and code snippets.

## 1. Coding Standards Deep Dive

### Type Prefix Check
UHT enforces prefixes. Common mistakes:
- `class MyData : public UObject` → should be `class UMyData`
- `struct CharacterData` → should be `struct FCharacterData` (plain struct)
- `enum ItemType` → should be `enum class EItemType`

### Include Order
```cpp
// CORRECT header include order:
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"       // engine headers
#include "MyProject/SomeHelper.h"      // project headers
#include "MyClass.generated.h"         // ALWAYS LAST
```

### API Export Macro
Every public class in a module needs `MODULENAME_API`:
```cpp
class MYGAME_API UMyGameplayManager : public UObject
```
Missing this causes linker errors when referenced from other modules.

## 2. UObject Lifecycle Deep Dive

### Garbage Collection Rules
- `UPROPERTY()` is the ONLY thing that keeps UObject pointers alive.
- Raw `UObject*` members without `UPROPERTY()` WILL be garbage collected.
- `TArray<UObject*>` needs `UPROPERTY()` on the TArray itself.
- `TMap<int32, TObjectPtr<UObject>>` needs `UPROPERTY()` on the TMap.

### Object Creation
```cpp
// CORRECT:
UMyObject* Obj = NewObject<UMyObject>(this);

// WRONG (never use new/delete for UObjects):
UMyObject* Obj = new UMyObject();  // Will crash
```

### Weak Pointers
```cpp
// Use when you want to observe without preventing GC:
TWeakObjectPtr<AActor> WeakActor = SomeActor;

// Check validity before use:
if (WeakActor.IsValid())
{
    WeakActor->DoSomething();
}
```

## 3. Networking Deep Dive

### Replication Setup
```cpp
void AMyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMyActor, Health);
    DOREPLIFETIME_CONDITION(AMyActor, TeamId, COND_OwnerOnly);
}
```

### Common Networking Mistakes
- Checking `HasAuthority()` for local-only effects instead of `IsLocallyControlled()`
- Replicating large arrays every tick
- RPC called without checking role first: `if (!HasAuthority()) { ServerRPC(); return; }`

## 4. Performance Deep Dive

### Container Best Practices
```cpp
// Pre-allocate when size is known:
TArray<int32> Numbers;
Numbers.Reserve(1000);  // Avoid re-allocations

// Use Emplace to avoid copies:
MyActors.Emplace(GetWorld(), TEXT("MyActor"));

// RemoveSwap vs RemoveAt:
MyArray.RemoveAt(Idx);        // Preserves order, slower
MyArray.RemoveAtSwap(Idx);    // Faster, order doesn't matter
```

### Tick Optimization
```cpp
// Set reasonable tick interval:
PrimaryActorTick.TickInterval = 0.1f;  // Tick every 100ms

// Or disable tick entirely when not needed:
PrimaryActorTick.bCanEverTick = false;

// Use Timer instead of Tick for periodic work:
GetWorldTimerManager().SetTimer(UpdateTimer, this, &AMyActor::PeriodicUpdate, 1.0f, true);
```

## 5. Smart Pointer Usage

| Type | Use Case |
|------|----------|
| `TUniquePtr<>` | Exclusive ownership of non-UObject |
| `TSharedPtr<>` | Shared ownership of non-UObject |
| `TSharedRef<>` | Non-null shared ownership |
| `TWeakPtr<>` | Non-owning reference to shared ptr |
| `TWeakObjectPtr<>` | Non-owning reference to UObject |

`TSharedPtr` on UObjects causes double-deletion. Never mix.
