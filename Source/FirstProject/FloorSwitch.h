// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorSwitch.generated.h"

UCLASS()
class FIRSTPROJECT_API AFloorSwitch : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFloorSwitch();


	/*Overlap Volume for Functionalluty to be triggered */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Floor Switch")
	class UBoxComponent* TriggerBox;			//�� �ڽ��� �� ������ Floor�� �ٲ�� �� ����.

	/*����ġ : ĳ���Ͱ� ������ �۵��ϰԲ� �ϴ� ��ġ*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Floor Switch")
		class UStaticMeshComponent* FloorSwitch;

	//����ġ�� ��� �۵��ϸ� �׿� ���� �����̴� ��
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Floor Switch")
	UStaticMeshComponent* Door;

	
	
	/*���� �ʱ� ��ġ*/
	UPROPERTY(BlueprintReadWrite, Category="Floor Switch")
	FVector InitialDoorLocation;

	/*floor switch�� �ʱ� ��ġ*/
	UPROPERTY(BlueprintReadWrite, Category = "Floor Switch")
	FVector InitialSwitchLocation;

	FTimerHandle SwitchHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Switch")
	float SwitchTime;

	bool bCharacterOnSwitch;

	void CloseDoor();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//BlueprintImplementableEvent : ���� C++���� ������ �ʿ� ���� �������Ʈ������ ����� ������ �� �ְ� ����
	UFUNCTION(BlueprintImplementableEvent, Category="Floor Switch")
	void RaiseDoor();//������ ������ �̰� �������Ʈ���� ���ϵ��� ������⿡ �����Ͽ� ���� ����


	UFUNCTION(BlueprintImplementableEvent, Category = "Floor Switch")
	void LowerDoor();


	UFUNCTION(BlueprintImplementableEvent, Category = "Floor Switch")
	void RaiseFloorSwitch();


	UFUNCTION(BlueprintImplementableEvent, Category = "Floor Switch")
	void LowerFloorSwitch();

	UFUNCTION(BlueprintCallable, Category="Floor Switch")
	void UpdateDoorLocation(float Z);


	UFUNCTION(BlueprintCallable, Category = "Floor Switch")
	void UpdateFloorSwitchLocation(float Z);
};
