// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main.generated.h"

//캐릭터가 뛰는 상태인지 아닌지 상태를 파악
UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	EMS_Normal UMETA(DisplayName = "Normal"),
	EMS_Sprinting UMETA(DisplayName = "Sprinting"),
	EMS_Dead UMETA(DisplayName = "Dead"),
	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};

//스태미너 상태를 파악
UENUM(BlueprintType)
enum class EStaminaStatus : uint8
{
	ESS_Normal UMETA(DisplayName = "Normal"),
	ESS_BelowMinimum UMETA(DisplayName = "BelowMinimum"),
	ESS_Exhausted UMETA(DisplayName = "Exhausted"),
	ESS_ExhaustedRecovering UMETA(DisplayName = "ExhaustedRecovering"),

	ESS_MAX UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class FIRSTPROJECT_API AMain : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMain();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasCombatTarget;
	FORCEINLINE void SetHasCombatTarget(bool HasTarget) { bHasCombatTarget = HasTarget; }

	TArray<FVector> PickupLocations;
	/*레벨 내에서 아이템을 줍거나 특정 이벤트가 발생했을 때, 그 위치정보를 저장하는 기능이 있다면 편하다. 
	이를 돕는 것이 TArray이다. 이름처럼 배열이다. 아이템을 획득할 때마다 증가하는 동적 배열.*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class AMainPlayerController* MainPlayerController;

	//적 상태바는 적 가까이(머리 위라든지)에 나타나야 안다. 그렇기에 이를 표시하기 위해선 적의 위치를 저장해두어야 한다.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Combat")
	FVector CombatTargetLocation; //적의 위치는 매 틱마다 바뀌므로 main의 tick함수에서 설정해주도록 한다.

	UFUNCTION(BlueprintCallable) //블루프린트에서 디버깅 도구를 호출할 수 있도록 한다.	
	/*아이템을 획득할 때마다 디버그구(sphere)가 그 위치에 나타나 위치를 알려주도록 한다.*/
	void ShowPickupLocations();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Enumes")
	EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enumes")
	EStaminaStatus StaminaStatus;

	//Enum으로 만든 EStaminaStatus 자료형의 변수를 받아 스태미너 상태를 정하는 함수
	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaDrainRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enumes") 
	float MinSprintStamina;




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* BloodParticles;

	//플레이어가 무기에 맞았을 때 소리를 내도록 한다.(비명이라든지 둔탁한 소리라든지 등등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class USoundCue* SmashingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	USoundCue* PainSound;

	/*
	일정거리 안에 들어서면 플레이어가 적을 향해 자동적으로 조준점을 맞추도록 해주고 싶다.
	*/
	float InterpSpeed;	//플레이어가 적에게 조준점을 맞추는 속도
	bool bInterpToEnemy; //특정조건(거리 등)을 만족하면 조준점 맞추는 작업을 진행하도록 할 것이다. 그 때 이 불 변수는 true가 되어 있어야 한다.
	void SetInterpToEnemy(bool Interp); //매개변수로 들어오는 true 또는 false에 따라 적에게 초점을 맞추도록 하는 함수. 이 함수는 공격과 동시에 실행되어야 할 것이므로 Attack함수에 들어가야 한다.


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class AEnemy* CombatTarget;	//초점을 맞출 대상이 있어야 한다. 당연히 적이므로 Enemy클래스의 인스턴스를 만든다.
	FORCEINLINE void SetCombatTarget(AEnemy* Target) { CombatTarget = Target; } //초점을 맞출 대상을 설정하는 함수. 적과의 전투거리 반경, 즉 Combat Sphere와 오버랩 되었을 때 실행한다.

	FRotator GetLookAtRotationYaw(FVector Target); // 플레이어가 적에게 초점을 맞추려면 그에 어느 방향으로 무엇을 향해 회전을 할 것인지 알려주어야 한다. 좌우로만 회전하므로 회전 방향은 Yaw이다.






	//이동상태와(걷는지 뛰는지) 달리는 속도를 정한다.
	void SetMovementStatus(EMovementStatus Status);

	//일반 달리기와 전력질주 속도 변수
	//SetMovementStatus()의 매개변수로 들어간다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float RunningSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float SprintingSpeed;


	//언리얼 에디터에서 sprint키로 바인딩한 shift가 눌려져 있는지 확인하는 bool변수이다.
	bool bShiftKeyDown;

	/*쉬프트 키가 눌려졌을 때, 다시 떼어졌을 때 동작을 담당하는 함수.*/
	void ShiftKeyDown();

	void ShiftKeyUp(); 



	/**Camera boom : positioning the camera behind the player*/
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category=Camera, meta =(AllowPrivateAccess="true"))
	class USpringArmComponent* CameraBoom;

	/**Follow camera*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))	
	class UCameraComponent* FollowCamera;

	/**Base turn rates to scale turning functions for the camera*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;


	/**
	/*
	/* 플레이어의 상태
	/* 
	*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats") //MaxHealth나 MaxStamina는 변하지 않을 값이라 EditsDefaultsOnly로 한다.
	float MaxHealth;	/*꽉 체워졌을 때 헬스바의 퍼센티지*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats") //반면 Health나 Stamina의 경우 줄거나 늘 수 있으므로 EditAnywhere로 한다.
	float Health;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats") //참고로 VisibleAnywhere는 EditDefaultOnly 또는 EditAnywhere와 함께 UPROPERTY의 매개변수로 들어갈 수 없다.
	float MaxStamina;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats") //코인의 경우도 마찬가지로 줄거나 늘 수 있으므로 EditAnywhere로 한다.
	int32 Coins;		//언리얼 엔진에서 주로 사용되는 정수형 자료형은 int32 자료형이다. 

	void DecrementHealth(float Amount);	//체력이 줄어드는 경우
	
	//플레이어에게 데미지를 적용해야 한다. 적용된 데미지를 반환하는 함수를 생성할 것이다. 
	//언리얼 엔진 자체 제공하는 함수가 있다 : TakeDamage
	virtual float TakeDamage(float DamageAmount,struct FDamageEvent const& DamageEvent,class AController* EventInstigator,AActor* DamageCauser) override; 
	//첫번째 매개변수는 데미지량이다. 
	//마지막 매개면수는 데미지를 주는 다른 액터의 종류이다.
	//언리얼엔진 공식문서의 TakeDamage 참고바람

	void Die();

	virtual void Jump() override;

	void IncrementCoins(int32 Amount);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/*Called for forwards/backwards input*/
	void MoveForward(float Value);

	/*Called for side to side input*/
	void MoveRight(float Value);


	/*Called via input to turn at a given rate
	@param Rate This is a normalized rate, i.e.1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/*Called via input to look up/down a given rate
	@param Rate This is a normalized rate, i.e.1.0 means 100% of desired look up/down rate
	*/
	void LookUpAtRate(float Rate);

	bool bLMBDown;
	bool bRMBDown;
	bool bPlayerMoving;

	void LMBDown();
	void LMBUp();
	void RMBDown();
	void RMBUp();



	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items") //주인공 캐릭터는 하나뿐이기 때문에 굳이 장비장착상태를 수정가능하게 만들 필요가 없다.
	class AWeapons* WeaponEquipped;


	/*
	플레이어는 무기에 가까이 다가가면 그 무기를 장비할 수 있지만, 장비할지 안 할지 선택할 수 있게 한다면 더 좋다.
	이를 위한 변수를 만들어준다. 비단 무기뿐만 아니라 장구류도 해당한다.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items") 
	class AItem* ActiveOverlappingItem;

	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) { ActiveOverlappingItem = Item; }

	void SetWeaponEquipped(AWeapons* WeaponToSet);

	FORCEINLINE AWeapons* GetWeaponEquipped() { return WeaponEquipped; } //장착한 장비를 리턴

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anims")
	bool bAttacking; //일반공격 세팅 변수

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bStrongAttacking; //강공격 세팅 변수

	void Attack();//공격 진행
	void StrongAttack();//강공격 진행

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* CombatMontage; //C++에서 애님몽타주가 호출되는 방식 : UAnimMontage

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* CombatMontage_StrongAtck; //C++에서 애님몽타주가 호출되는 방식 : UAnimMontage
	
	UFUNCTION(BlueprintCallable)
	void AttackEnd(); //공격 끝

	UFUNCTION(BlueprintCallable)
	void StrongAttackEnd(); //강공격 끝


	UFUNCTION(BlueprintCallable)
	void PlaySwingSound(); //무기 휘두르는 소리

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

};
