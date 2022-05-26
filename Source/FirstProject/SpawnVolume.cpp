// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVolume.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Enemy.h"
#include "Critter.h"
#include "AIController.h"

// Sets default values
ASpawnVolume::ASpawnVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
}

// Called when the game starts or when spawned
void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();

	//게임 시작과 함께 Tarray배열 하나가 스펀할 Actor들로 채워진다.
	if (ActorToSpawn1 && ActorToSpawn2 && ActorToSpawn3 && ActorToSpawn4)
	{
		SpawnArray.Add(ActorToSpawn1);
		SpawnArray.Add(ActorToSpawn2);
		SpawnArray.Add(ActorToSpawn3);
		SpawnArray.Add(ActorToSpawn4);
	}

}

// Called every frame
void ASpawnVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector ASpawnVolume::GetSpawnPoint()
{
	FVector Extent = SpawningBox->GetScaledBoxExtent();
	FVector Origin = SpawningBox->GetComponentLocation();
	
	FVector Point = UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);

	return Point;
}


void ASpawnVolume::SpawnOurActor_Implementation(UClass* ToSpawn, const FVector& Location)
{
	if (ToSpawn)
	{
		UWorld* World = GetWorld();
		FActorSpawnParameters SpawnParams;
		if (World)
		{
			AActor* Actor = World->SpawnActor<AActor>(ToSpawn, Location, FRotator(0.f), SpawnParams);

			AEnemy* Enemy = Cast<AEnemy>(Actor);
			if (Enemy)
			{
				Enemy->SpawnDefaultController(); //AIController를 가져와 적 Pawn 클래스에 적용
				AAIController* AIC = Cast<AAIController>(Enemy->GetController());
				if (AIC)
				{
					Enemy->AIController = AIC; //AIController에 접근해 moveto와 같은 함수를 불러올 것이다.
				}

			}
		}
	}
}


TSubclassOf<AActor> ASpawnVolume::GetSpawnActor() //스펀 박스 안에 스펀할 액터를 고르는 매커니즘.
{
	if (SpawnArray.Num() > 0)	//SpawnArray 배열은 일단 비어있으면 안 된다. 비어있으면 배치할 액터가 없다는 얘기.
	{
		int32 Selection = FMath::RandRange(0, SpawnArray.Num() - 1);	//배열 안 원소 중 하나를 랜덤으로 골라냄
		return SpawnArray[Selection];	//스펀할 객체가 들어있는 배열의 랜덤한 원소를 반환, 즉 액터 중 하나를 랜덤으로 배치한다는 얘기.
	}
	else
	{
		return nullptr;
	}
}
