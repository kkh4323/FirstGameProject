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
	class UBoxComponent* TriggerBox;			//이 박스에 들어갈 때마다 Floor가 바뀌도록 할 것임.

	/*스위치 : 캐릭터가 밟으면 작동하게끔 하는 장치*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Floor Switch")
		class UStaticMeshComponent* FloorSwitch;

	//스위치를 밟아 작동하면 그에 따라 움직이는 문
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Floor Switch")
	UStaticMeshComponent* Door;

	
	
	/*문의 초기 위치*/
	UPROPERTY(BlueprintReadWrite, Category="Floor Switch")
	FVector InitialDoorLocation;

	/*floor switch의 초기 위치*/
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

	//BlueprintImplementableEvent : 굳이 C++에서 실행할 필요 없이 블루프린트에서도 기능을 실행할 수 있게 해줌
	UFUNCTION(BlueprintImplementableEvent, Category="Floor Switch")
	void RaiseDoor();//내용이 없지만 이걸 블루프린트에서 정하도록 만들었기에 컴파일에 문제 없음


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
