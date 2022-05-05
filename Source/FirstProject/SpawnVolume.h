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
	TSubclassOf<class ACritter> PawnToSpawn;

protected:
	// 게임이 시작하거나 스펀되었을 때 발동된다
	virtual void BeginPlay() override;

public:	
	// 프레임마다 호출되도록!
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category="Spawning")
	FVector GetSpawnPoint();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spanwing")
	void SpawnOurPawn(UClass* ToSpawn, const FVector& Location);

};
