// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnVolume.generated.h"

UCLASS()
class FIRSTPROJECT_API ASpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// 액터 속성의 기본값들 설정
	ASpawnVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	class UBoxComponent* SpawningBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	//TSubclassOf<class ACritter> PawnToSpawn;	//스펀할 객체 : 여기서는 ACritter로 지정되어 있는데 Critter는 액터 중에 하나일 뿐이다. 그래서 블루프린트에서 Critter 밖에 안 나타나는 것.
	TSubclassOf<AActor> ActorToSpawn1; //액터로 설정해주어야 액터 클래스 기반의 다른 클래스들도 PawnToSpawn으로 설정 가능하다.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<AActor> ActorToSpawn2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<AActor> ActorToSpawn3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<AActor> ActorToSpawn4;

	TArray<TSubclassOf<AActor>> SpawnArray; //위 객체들을 저장할 배열을 설정.


protected:
	// 게임이 시작하거나 스펀되었을 때 발동된다
	virtual void BeginPlay() override;

public:	
	// 프레임마다 호출되도록!
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category = "Spawning")
	TSubclassOf<AActor> GetSpawnActor();

	UFUNCTION(BlueprintPure, Category="Spawning")
	FVector GetSpawnPoint();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spanwing")
	void SpawnOurActor(UClass* ToSpawn, const FVector& Location);

};
