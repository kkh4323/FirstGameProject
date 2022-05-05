// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UCLASS()
class FIRSTPROJECT_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

	/*기본 형태의 Collision*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item | Collision")
	class USphereComponent* CollisionVolume;

	/*베이스 메시 컴포넌트*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item | Mesh")
	class UStaticMeshComponent* Mesh;

	/*폭발이나 아이템 가까이 가서 작동 박스를 건드리거나 획득을 할 경우 particle system이 발생할 수 있음. 연기나 잔해같은 것.
	Idle 상태에 있거나 캐릭터가 그 위로 가서 작동&획득을 할 경우 발생하게끔 한다.Item 클래스에 만듦으로써 폭발, PickUp에 상속시킨다.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particles")
	class UParticleSystemComponent* IdleParticlesComponent; 
	/*컴포넌트는 createdefaultsubobject를 사용해 생성되지만 UParticleSystem의 경우 그럴 필요 없다.블루프린트가 알아서 해줌
	Mesh의 경우도 마찬가지. => 'component'가 붙은 것은 위 함수를 사용해 만들지만 그것이 없을 경우에는 아니라는 것*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particles")
	class UParticleSystem* OverlapParticles;

	/*캐릭터가 액터에 가까이 다가가면 폭발이나 섬광과 함께 사라지게 하고 소리도 같이도 나게 하려면? USoundCue를 활용한다.
	이후에 cpp 파일에서 이 액터가 캐릭터에 의해 Overlap(겹침)되어 사라지기 전에 이를 활용해 소리가 먼저 나게 한다*/
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Sounds")
	class USoundCue* OverlapSound;


	/*만약 동전이나 아이템 등 액터가 회전하는 효과를 넣으면 더 볼만할 것이다. 그런데 Item을 상속받는 모든 클래스가 회전을 하면 어색할 수 있으므로
	이를 제어할 수 있는 boolean 값을 준다.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | ItemProperties")
	bool bRotate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | ItemProperties")
	float RotationRate;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
