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
	// ���� �Ӽ��� �⺻���� ����
	ASpawnVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	class UBoxComponent* SpawningBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<class ACritter> PawnToSpawn;

protected:
	// ������ �����ϰų� ���ݵǾ��� �� �ߵ��ȴ�
	virtual void BeginPlay() override;

public:	
	// �����Ӹ��� ȣ��ǵ���!
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category="Spawning")
	FVector GetSpawnPoint();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spanwing")
	void SpawnOurPawn(UClass* ToSpawn, const FVector& Location);

};
