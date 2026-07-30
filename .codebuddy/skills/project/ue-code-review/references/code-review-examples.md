# Code Review Examples

Common UE5 C++ review scenarios with before/after examples.

## Example 1: GC-Safe Lambda Capture

**❌ Before:**
```cpp
// UObject pointer captured by value — invisible to GC!
FTimerHandle Timer;
GetWorldTimerManager().SetTimer(Timer, [=]()
{
    SomeWidget->UpdateText();  // SomeWidget might have been GC'd!
}, 5.0f, false);
```

**✅ After:**
```cpp
FTimerHandle Timer;
TWeakObjectPtr<UMyWidget> WeakWidget(SomeWidget);
GetWorldTimerManager().SetTimer(Timer, [WeakWidget]()
{
    if (WeakWidget.IsValid())
    {
        WeakWidget->UpdateText();
    }
}, 5.0f, false);
```
Or use `BindWeakLambda` for `FTimerDelegate`:
```cpp
GetWorldTimerManager().SetTimer(Timer, FTimerDelegate::CreateWeakLambda(this, [this]()
{
    SomeWidget->UpdateText();  // Safe — bound weakly to 'this'
}), 5.0f, false);
```

---

## Example 2: Expensive Tick to Event-Driven

**❌ Before:**
```cpp
void AMyActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        float Dist = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
        if (Dist < 500.0f)
        {
            OnPlayerNearby();
        }
    }
}
```

**✅ After:**
```cpp
void AMyActor::BeginPlay()
{
    Super::BeginPlay();
    // Use a sphere overlap to detect nearby players
    if (USphereComponent* Sphere = FindComponentByClass<USphereComponent>())
    {
        Sphere->SetSphereRadius(500.0f);
        Sphere->OnComponentBeginOverlap.AddDynamic(this, &AMyActor::OnOverlapBegin);
    }
}

void AMyActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (OtherActor->IsA<APlayerCharacter>())
    {
        OnPlayerNearby();
    }
}
```

---

## Example 3: Missing Replication Validation

**❌ Before:**
```cpp
UFUNCTION(Server, Reliable)
void ServerDealDamage(AActor* Victim, float Damage);
```

**✅ After:**
```cpp
UFUNCTION(Server, Reliable, WithValidation)
void ServerDealDamage(AActor* Victim, float Damage);

// Validation prevents cheating: reject impossible damage values
bool AMyChar::ServerDealDamage_Validate(AActor* Victim, float Damage)
{
    return Victim != nullptr && Damage > 0.f && Damage < 99999.f;
}
```

---

## Example 4: GetWorld() in Constructor

**❌ Before:**
```cpp
AMyActor::AMyActor()
{
    UWorld* World = GetWorld();  // Always nullptr!
    if (World)
    {
        // This code never runs
    }
}
```

**✅ After:**
```cpp
AMyActor::AMyActor()
{
    // Only create components here
    MyComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MyComponent"));
}

void AMyActor::BeginPlay()
{
    Super::BeginPlay();
    UWorld* World = GetWorld();  // Valid here
    // World-dependent initialization
}
```

---

## Example 5: String Concatenation in Loop

**❌ Before:**
```cpp
FString Result;
for (const FString& Part : Parts)
{
    Result += Part;  // Re-allocates every iteration
    Result += TEXT(", ");
}
```

**✅ After:**
```cpp
// Option A: Use Join
FString Result = FString::Join(Parts, TEXT(", "));

// Option B: Pre-allocate
FString Result;
Result.Reserve(Parts.Num() * 20);  // Estimate per-part size
for (const FString& Part : Parts)
{
    Result += Part;
    Result += TEXT(", ");
}

// Option C: TStringBuilder
TStringBuilder<1024> Builder;
for (const FString& Part : Parts)
{
    Builder << Part << TEXT(", ");
}
FString Result = Builder.ToString();
```

---

## Example 6: Cache Cast Results

**❌ Before:**
```cpp
void AMyActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    for (AActor* Actor : SomeActors)
    {
        if (AMyPawn* Pawn = Cast<AMyPawn>(Actor))
        {
            Pawn->DoSomething();
        }
        if (AMyPawn* Pawn = Cast<AMyPawn>(Actor))  // Same cast repeated!
        {
            Pawn->DoAnotherThing();
        }
    }
}
```

**✅ After:**
```cpp
void AMyActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    for (AActor* Actor : SomeActors)
    {
        if (AMyPawn* Pawn = Cast<AMyPawn>(Actor))
        {
            Pawn->DoSomething();
            Pawn->DoAnotherThing();  // Reuse the cached pointer
        }
    }
}
```

---

## Example 7: Missing Super Call

**❌ Before:**
```cpp
void AMyCharacter::BeginPlay()
{
    // Missing Super::BeginPlay()!
    HealthComponent = FindComponentByClass<UHealthComponent>();
}
```

**✅ After:**
```cpp
void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();  // Always call Super first unless you have a specific reason
    HealthComponent = FindComponentByClass<UHealthComponent>();
}
```

---

## Example 8: Unsafe Server RPC Parameter

**❌ Before:**
```cpp
UFUNCTION(Server, Reliable, WithValidation)
void ServerUseItem(int32 ItemIndex);

bool AMyChar::ServerUseItem_Validate(int32 ItemIndex)
{
    return true;  // No validation — client can send any index!
}
```

**✅ After:**
```cpp
UFUNCTION(Server, Reliable, WithValidation)
void ServerUseItem(int32 ItemIndex);

bool AMyChar::ServerUseItem_Validate(int32 ItemIndex)
{
    // Validate the item exists and the player actually has it
    return InventoryComponent
        && InventoryComponent->IsValidItemIndex(ItemIndex)
        && InventoryComponent->HasItemAtIndex(ItemIndex);
}
```
