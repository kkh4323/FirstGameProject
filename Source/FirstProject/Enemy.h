// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"



/*
적의 이동 로직은 크게 3가지이다.
1. 아이들 상태(플레이어 발견못함)
2. 추적 상태(플레이어 발견 후 추적)
3. 공격 상태(플레이어에게 일정거리 이하로 다가와 공격)
*/
UENUM(BlueprintType)
enum class EEnemyMovementStatus :uint8
{
	EMS_Idle UMETA(DisplayName = "Idle"),	//아무것도 안 하는 상태
	EMS_MoveToTarget UMETA(DisplayName = "MoveToTarget"), //플레이어를 추적하는 상태
	EMS_Attacking UMETA(DisplayName = "Attacking"), //플레이어를 공격하는 상태

	EMS_MAX UMETA(DisplayName = "DefaultMax")
};


UCLASS()
class FIRSTPROJECT_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	EEnemyMovementStatus EnemyMovementStatus;

	FORCEINLINE void SetEnemyMovementStatus(EEnemyMovementStatus Status) { EnemyMovementStatus = Status; }

	/*
	플레이어가 일정 반경 안에 들어오면 NPC가 이를 인지하고 플레이어를 쫒아가도록 만들 것이다.
	이를 위한 구를 생성.
	일정수준 이내로 가까워지면 NPC로 하여금 플레이어를 공격하도록 함.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class USphereComponent* AgroSphere; //어그로를 끄는 반경

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	USphereComponent* CombatSphere; //어그로를 끄는 반경

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class AAIController* AIController; //AI와 관련된 기능을 담은 클래스.


	//적이 갖는 체력

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")//세 변수 모두 인스턴스마다 블루프린트로 수정 가능하도록 정한다.
	float Health; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MaxHealth; //헬스바를 표현할 때 이 고정치 변수가 필요하다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float Damage; //적이 플레이어에게 주는 데미지.



	/*
	적이 공격을 받았을 때 파티클 시스템을 통해 출혈효과를 만들 것이다.
	enemy 헤더 파일에 이를 구현하는 이유는 적 클래스를 통해 나오는 객체마다 다른 종류의 파티클 시스템을 적용할 수 있도록 하기 위함이다.
	예를 들어 어떤 적은 초록색 피가 나오는 반면 어떤 적은 빨간 피가 나온다든지.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UParticleSystem* HitParticles;

	//적이 무기에 맞았을 때 소리를 내도록 한다.(비명이라든지 둔탁한 소리라든지 등등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class USoundCue* HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* ScreamingSound;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;



	//어그로 반경과 공격 시작 반경에 플레이어가 '접'했음을 알림.
	UFUNCTION()
	virtual void AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	UFUNCTION()
	virtual void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	//타겟으로 AI가 이동하도록 하는 함수. Main형 target를 매개변수로 가짐.(플레이어 캐릭터가 타겟이 된다.)
	UFUNCTION(BlueprintCallable)
	void MoveToTarget(class AMain* Target);


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bOverlappingCombatSphere; //NPC가 플레이어를 공격하는 도중 플레이어가 달아나 버려도 NPC는 진행하던 공격은 끝내고 플레이어를 추적하게끔 할 것임. 이 때 필요한 변수.

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	AMain* CombatTarget;
};
