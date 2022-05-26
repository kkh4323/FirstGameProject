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

	//���� ���۰� �Բ� Tarray�迭 �ϳ��� ������ Actor��� ä������.
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
				Enemy->SpawnDefaultController(); //AIController�� ������ �� Pawn Ŭ������ ����
				AAIController* AIC = Cast<AAIController>(Enemy->GetController());
				if (AIC)
				{
					Enemy->AIController = AIC; //AIController�� ������ moveto�� ���� �Լ��� �ҷ��� ���̴�.
				}

			}
		}
	}
}


TSubclassOf<AActor> ASpawnVolume::GetSpawnActor() //���� �ڽ� �ȿ� ������ ���͸� ���� ��Ŀ����.
{
	if (SpawnArray.Num() > 0)	//SpawnArray �迭�� �ϴ� ��������� �� �ȴ�. ��������� ��ġ�� ���Ͱ� ���ٴ� ���.
	{
		int32 Selection = FMath::RandRange(0, SpawnArray.Num() - 1);	//�迭 �� ���� �� �ϳ��� �������� ���
		return SpawnArray[Selection];	//������ ��ü�� ����ִ� �迭�� ������ ���Ҹ� ��ȯ, �� ���� �� �ϳ��� �������� ��ġ�Ѵٴ� ���.
	}
	else
	{
		return nullptr;
	}
}
