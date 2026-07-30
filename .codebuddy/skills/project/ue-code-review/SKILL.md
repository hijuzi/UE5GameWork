---
name: ue-code-review
description: Review Unreal Engine 5 C++ code for correctness, safety, performance, and style.
  Covers coding standards compliance, UObject lifecycle and GC safety, networking/replication,
  Gameplay Ability System patterns, Enhanced Input, memory management, threading, asset
  references, common pitfalls and anti-patterns, error handling, and optimization. Use when
  reviewing any UE5 C++ code, pull requests, or performing code audits on Unreal Engine projects.
metadata:
  engine-version: "5.7"
  category: project
---

# UE5 C++ Code Review

Systematic code review for Unreal Engine 5 C++ projects. The goal is to catch bugs,
enforce best practices, and ensure the code is maintainable and performant.

Related skills: `coding-standards`, `cpp-fundamentals`, `memory-and-gc`, `networking-and-replication`,
`gameplay-ability-system`, `enhanced-input`, `subsystems`, `delegates-and-events`.

## When to use this skill

- Reviewing a pull request or commit in a UE5 C++ project.
- Auditing existing code for safety, performance, or correctness issues.
- Onboarding a new team member — using this as a checklist for what to look for.
- Before submitting code for QA or production builds.

## Review workflow

1. **Understand intent** — read the PR description or commit message. What problem is being solved?
2. **Architecture check** — does the change belong where it is? Could it be a subsystem, component, or plugin instead?
3. **Line-by-line review** — go through each file systematically, applying the checklist below.
4. **Build and test** — does it compile? Are there any new warnings? Have tests been updated?
5. **Performance impact** — any hot-path concerns? Profiling needed?

## Review checklist

### 1. Coding standards (`coding-standards`)

- [ ] Type prefixes correct (`U`, `A`, `F`, `E`, `I`, `T`, `S`, `b`)?
- [ ] PascalCase for all identifiers?
- [ ] Allman braces, tab indentation?
- [ ] `GENERATED_BODY()` as first member in every reflected class?
- [ ] `generated.h` is the **last** include?
- [ ] Module `*_API` export macro on public classes?
- [ ] `Category` set on all editor-visible `UPROPERTY`/`UFUNCTION`?
- [ ] `TEXT()` around string literals constructing `FString`/`FName`?
- [ ] `TObjectPtr<T>` for UPROPERTY UObject members (UE5+)?
- [ ] No `std::` containers in engine-facing code?

### 2. UObject lifecycle & GC (`memory-and-gc`)

- [ ] `UPROPERTY()` protects UObject pointers from GC — any raw `UObject*` without UPROPERTY?
- [ ] `TWeakObjectPtr<>` used for non-owning references where the target might be GC'd?
- [ ] `BeginDestroy()` / `EndPlay()` properly clean up resources?
- [ ] No heavy work in constructors — prefer `PostInitProperties()` or `BeginPlay()`?
- [ ] Lambda captures: `CreateWeakLambda` / `BindWeakLambda` for deferred UObject lambdas?
- [ ] No `[=]` captures that silently hold UObject pointers invisible to GC?

### 3. Networking & Replication (`networking-and-replication`)

- [ ] Server-authoritative: server validates all client RPCs?
- [ ] `Server_` / `Client_` / `NetMulticast_` RPC naming convention followed?
- [ ] `WithValidation` on Server RPCs that take client-supplied data?
- [ ] `DOREPLIFETIME` / `DOREPLIFETIME_CONDITION` macros in `GetLifetimeReplicatedProps()`?
- [ ] Replicated properties using `OnRep_` notifiers instead of polling?
- [ ] No replicated properties updated every tick when a condition would suffice?
- [ ] `IsLocallyControlled()` checks before showing UI / playing effects?

### 4. Gameplay Ability System (`gameplay-ability-system`)

- [ ] Ability activation via `TryActivateAbility` — proper tag blocking / cost checks?
- [ ] `CommitAbility()` called before applying effects/costs?
- [ ] `EndAbility()` always called (including error paths)?
- [ ] Attribute setters go through `UAbilitySystemComponent` (not directly)?
- [ ] `FGameplayEffectSpec` properly created and applied?
- [ ] `GameplayTags` used instead of enums/strings for ability/state queries?

### 5. Enhanced Input (`enhanced-input`)

- [ ] `UInputAction` data assets used (not raw key bindings)?
- [ ] `UEnhancedInputComponent::BindAction()` with `ETriggerEvent` correctly chosen?
- [ ] Input handled in `SetupPlayerInputComponent()` (not in Tick)?
- [ ] `AddMappingContext()` with appropriate priority?

### 6. Delegates & Events (`delegates-and-events`)

- [ ] `DECLARE_DYNAMIC_MULTICAST_DELEGATE` for Blueprint-exposed events?
- [ ] `DECLARE_DELEGATE` / `DECLARE_MULTICAST_DELEGATE` for C++-only?
- [ ] `AddDynamic()` / `RemoveDynamic()` for UFUNCTION-bound delegates?
- [ ] Delegates unbound in `EndPlay()` or destructor to prevent dangling references?
- [ ] `RemoveAll()` on owned delegates before destruction?

### 7. Performance & Optimization (`profiling-and-optimization`)

- [ ] Any `Tick()` that could be replaced with a timer or event-driven approach?
- [ ] Expensive operations in constructors or `BeginPlay()`?
- [ ] `SCOPE_CYCLE_COUNTER` on performance-critical functions?
- [ ] `GetWorld()` / `GetOwner()` / `FindComponent` cached instead of called repeatedly?
- [ ] `Cast<>` results cached when used multiple times in a frame?
- [ ] Heavy loops avoiding `Cast<>`, `FindObject<>`, or `LoadObject<>` inside?
- [ ] Parallel-for (`ParallelFor`) with thread-safe operations?
- [ ] Container pre-allocation (`Reserve()`) where size is known?

### 8. Memory Management (`memory-and-gc`)

- [ ] No raw `new` / `delete` for UObjects (use `NewObject<>` / GC)?
- [ ] No `std::shared_ptr` / `std::unique_ptr` for engine types?
- [ ] `TUniquePtr<>` / `TSharedPtr<>` used for non-UObject smart pointers?
- [ ] Large arrays using `TArray::Shrink()` after heavy removal?
- [ ] String concatenation in loops using `FString::Reserve()` or `TStringBuilder`?

### 9. Error Handling & Logging (`logging-and-assertions`)

- [ ] `check()` for invariants (not for user errors)?
- [ ] `ensure()` / `ensureMsgf()` for recoverable "shouldn't happen" scenarios?
- [ ] `UE_LOG` with appropriate category and verbosity level?
- [ ] No `check()` with side effects?
- [ ] Null pointer checks on parameters from external sources?
- [ ] `IsValid()` on UObject pointers before use?

### 10. Asset References (`asset-management`)

- [ ] `TSoftObjectPtr<>` for cross-package / optional references?
- [ ] `TSoftClassPtr<>` for Blueprint class references?
- [ ] `FStreamableManager` for async asset loading?
- [ ] No `LoadObject<>()` in `BeginPlay()` — prefer async?
- [ ] `FObjectFinder` in constructors (editor-time) vs runtime loading?

### 11. Editor & Tooling

- [ ] `#if WITH_EDITOR` / `#if WITH_EDITORONLY_DATA` for editor-only code?
- [ ] `CallInEditor` UFUNCTIONs have guards against invalid state?
- [ ] `PostEditChangeProperty()` handles component hierarchy changes?

### 12. Threading & Async (`timers-and-async`)

- [ ] Game thread access enforced — `IsInGameThread()` checks where needed?
- [ ] `AsyncTask(ENamedThreads::GameThread, ...)` to marshal back to game thread?
- [ ] `FGraphEventRef` / `TaskGraph` for parallel work?
- [ ] `FRunnable` properly cleaned up in `Stop()` / `Exit()`?
- [ ] No UObject access from non-game threads?

### 13. Common Anti-Patterns & Gotchas

- [ ] No `Cast<ACharacter>(GetPawn())` in AI controller constructors (pawn may not exist yet)?
- [ ] No `GetWorld()` in constructors (returns nullptr)?
- [ ] `CreateDefaultSubobject<>` only in constructors, not `BeginPlay()`?
- [ ] `BeginPlay()` calls `Super::BeginPlay()`?
- [ ] Actor component `OnComponentCreated()` vs `BeginPlay()` distinction?
- [ ] `FTimerHandle` properly cleared with `GetWorldTimerManager().ClearTimer()`?
- [ ] No circular `#include` dependencies?
- [ ] Forward declarations in headers, full includes in `.cpp`?

## Review output format

When providing a code review, structure the feedback as:

### Critical (must fix)

Issues that will cause crashes, GC leaks, network desyncs, or build failures.

### Major (should fix)

Bugs, performance regressions, or violations of engine best practices.

### Minor (consider)

Style improvements, naming suggestions, or optional optimizations.

### Positive Highlights

Well-implemented patterns worth calling out.

## References and source material

- [References/code-review-examples.md](references/code-review-examples.md) — Common review scenarios with before/after examples.
- [References/checklist-detailed.md](references/checklist-detailed.md) — Expanded checklist with engine evidence and code snippets.
