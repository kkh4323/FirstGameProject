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

	TArray<FVector> PickupLocations;
	/*레벨 내에서 아이템을 줍거나 특정 이벤트가 발생했을 때, 그 위치정보를 저장하는 기능이 있다면 편하다. 
	이를 돕는 것이 TArray이다. 이름처럼 배열이다. 아이템을 획득할 때마다 증가하는 동적 배열.*/


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
	void Die();

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
};
