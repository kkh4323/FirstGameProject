// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Main.h"
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
	EMS_EnemyDead UMETA(DisplayName = "EnemyDead"), //적이 죽은 상태. 적이 죽으면 이 상태로 전환한다. EnemyDie함수 내에서 발동.


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
	FORCEINLINE EEnemyMovementStatus GetEnemyMovementStatus() { return EnemyMovementStatus; }

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
	

	//전투 반경에 닿자마자 플레이어를 공격하면 게임이 너무 어려워질 수 있음. 시간차를 두고 공격하게끔 할 것임. 
	//시간제어 관련 기능을 가진 FTimerHandle을 활용한다.
	//시간지연의 범위는 밑의 두 변수 사이이다.
	FTimerHandle AttackTimer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackMinTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackMaxTime;

	//적이 갖는 체력

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")//세 변수 모두 인스턴스마다 블루프린트로 수정 가능하도록 정한다.
	float EnemyHealth; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float EnemyMaxHealth; //헬스바를 표현할 때 이 고정치 변수가 필요하다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float Damage; //적이 플레이어에게 주는 데미지.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UAnimMontage* CombatMontage;

	//bool bShift;
	bool bHasValidTarget; // 유효한 공격 대상을 가지는지 판단하는 기준으로, 이것이 false이면 적은 플레이어를 더이상 공격대상으로 생각하지 않고 공격을 중단한다.

	/*쉬프트 키가 눌려졌을 때, 다시 떼어졌을 때 동작을 담당하는 함수.*/
	void ShiftKeyDown();

	void ShiftKeyUp();


	/*
	적이 공격을 받았을 때 파티클 시스템을 통해 출혈효과를 만들 것이다.
	enemy 헤더 파일에 이를 구현하는 이유는 적 클래스를 통해 나오는 객체마다 다른 종류의 파티클 시스템을 적용할 수 있도록 하기 위함이다.
	예를 들어 어떤 적은 초록색 피가 나오는 반면 어떤 적은 빨간 피가 나온다든지.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UParticleSystem* HitParticles;

	//적이 무기에 맞았을 때 소리를 내도록 한다.(비명이라든지 둔탁한 소리라든지 등등.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class USoundCue* HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class USoundCue* HitSound2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* ScreamingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
		USoundCue* ScreamingSound1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
		USoundCue* ScreamingSound2;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* EnemyDeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* EnemyDeathSound1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* EnemyDeathSound2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* EnemyDeathSound3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* EnemyBattleCry1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* EnemyBattleCry2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* EnemyBattleCry3;

	//적이 무기를 휘두를 때 나오는 소리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* SwingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* SwingSound2;


	//적 탐지 반경에 들어섰을 때 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* EnemySpotSound1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* EnemySpotSound2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* EnemySpotSound3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* EnemySpotSound4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* EnemySpotSound5;


	//적 객체가 가할 데미지의 종류
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

	//적이 휘두르는 무기에 박스 컴포넌트를 붙여 플레이어에 대한 타격 유효범위가 되도록 만들 것이다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class UBoxComponent* CombatCollision;

	FTimerHandle DeathTimer;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DeathDelay;//죽은 후에 월드에 남는 시간


	virtual void EnemyDecrementHealth(float Amount, AActor* DamageCauser);


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



	//적 NPC가 현재 공격을 진행중인지 아닌지 알 수 있도록 하는 불리언 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bAttacking; 

	//타겟으로 AI가 이동하도록 하는 함수. Main형 target를 매개변수로 가짐.(플레이어 캐릭터가 타겟이 된다.)
	UFUNCTION(BlueprintCallable)
	void MoveToTarget(class AMain* Target);


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bOverlappingCombatSphere; //NPC가 플레이어를 공격하는 도중 플레이어가 달아나 버려도 NPC는 진행하던 공격은 끝내고 플레이어를 추적하게끔 할 것임. 이 때 필요한 변수.

	bool bPainSoundOnGoing;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	AMain* CombatTarget;

	//Weapons.h에 있는 것을 그대로 가져온 것. 무기가 타격효과를 발하는 시점은 NPC의 무기가 플레이어 캐릭터의 몸체와 부딪혔을 때, 즉 Overlap되었을 때이다.
	UFUNCTION()
	void CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void ActivateCollision();

	UFUNCTION(BlueprintCallable)
	void DeactivateCollision();

	//적 공격시 실행할 내용을 담은 함수와 적 공격 중단 함수

	void EnemyAttack();


	UFUNCTION(BlueprintCallable)
	void EnemyAttackEnd();

	UFUNCTION(BlueprintCallable)
	void EnemySwingingSound1();

	//적 또한 플레이어로부터 공격을 받으면 데미지를 받아야 하므로 TakeDamage이용해 관련 기능 구현.
	virtual float TakeDamage(float DamageAmount,struct FDamageEvent const& DamageEvent,class AController* EventInstigator,AActor* DamageCauser) override; 

	void EnemyDie(AActor* Causer);

	//적은 죽으면 죽어 누워있는 동작상태를 계속 유지해야 한다. 이를 위해 애니메이션 몽타주에서 적이 죽어 누워있는 지점에 노티파이를 생성하고 C++를 통해 그 지점에 계속 적의 애니메이션이 멈춰있도록 해주어야 한다.
	//UFUNCTION(BlueprintCallable)
	//void DeathEnd();

	//적이 살아있는지 확인. 살아있을 때에만 탐지범위 내 플레이어를 인식하고 공격하는 등 활동을 하게 하기 위함. 모든 Sphere안에 활성 조건으로 들어갈 것.
	bool IsAlive();


	void Disappear();//적이 죽으면 사라지도록 하는 함수.
};
