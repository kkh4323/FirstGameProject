// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main.generated.h"

//ĳ���Ͱ� �ٴ� �������� �ƴ��� ���¸� �ľ�
UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	EMS_Normal UMETA(DisplayName = "Normal"),
	EMS_Sprinting UMETA(DisplayName = "Sprinting"),
	EMS_Dead UMETA(DisplayName = "Dead"),
	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};

//���¹̳� ���¸� �ľ�
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
	/*���� ������ �������� �ݰų� Ư�� �̺�Ʈ�� �߻����� ��, �� ��ġ������ �����ϴ� ����� �ִٸ� ���ϴ�. 
	�̸� ���� ���� TArray�̴�. �̸�ó�� �迭�̴�. �������� ȹ���� ������ �����ϴ� ���� �迭.*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class AMainPlayerController* MainPlayerController;

	//�� ���¹ٴ� �� ������(�Ӹ� �������)�� ��Ÿ���� �ȴ�. �׷��⿡ �̸� ǥ���ϱ� ���ؼ� ���� ��ġ�� �����صξ�� �Ѵ�.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Combat")
	FVector CombatTargetLocation; //���� ��ġ�� �� ƽ���� �ٲ�Ƿ� main�� tick�Լ����� �������ֵ��� �Ѵ�.

	UFUNCTION(BlueprintCallable) //�������Ʈ���� ����� ������ ȣ���� �� �ֵ��� �Ѵ�.	
	/*�������� ȹ���� ������ ����ױ�(sphere)�� �� ��ġ�� ��Ÿ�� ��ġ�� �˷��ֵ��� �Ѵ�.*/
	void ShowPickupLocations();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Enumes")
	EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enumes")
	EStaminaStatus StaminaStatus;

	//Enum���� ���� EStaminaStatus �ڷ����� ������ �޾� ���¹̳� ���¸� ���ϴ� �Լ�
	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaDrainRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enumes") 
	float MinSprintStamina;




	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* BloodParticles;

	//�÷��̾ ���⿡ �¾��� �� �Ҹ��� ������ �Ѵ�.(����̶���� ��Ź�� �Ҹ������ ���)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class USoundCue* SmashingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	USoundCue* PainSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	USoundCue* PainSound2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	USoundCue* PainSound3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	USoundCue* GreaterPainSound;

	//�÷��̾� ��� �� �Ҹ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	USoundCue* DeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	USoundCue* BattleCry1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	USoundCue* BattleCry2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	USoundCue* BattleCry3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	USoundCue* BattleCry4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	USoundCue* BattleCry5;


	//������ �Ҹ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	USoundCue* JumpSound;

	/*
	�����Ÿ� �ȿ� ���� �÷��̾ ���� ���� �ڵ������� �������� ���ߵ��� ���ְ� �ʹ�.
	*/
	float InterpSpeed;	//�÷��̾ ������ �������� ���ߴ� �ӵ�
	bool bInterpToEnemy; //Ư������(�Ÿ� ��)�� �����ϸ� ������ ���ߴ� �۾��� �����ϵ��� �� ���̴�. �� �� �� �� ������ true�� �Ǿ� �־�� �Ѵ�.
	void SetInterpToEnemy(bool Interp); //�Ű������� ������ true �Ǵ� false�� ���� ������ ������ ���ߵ��� �ϴ� �Լ�. �� �Լ��� ���ݰ� ���ÿ� ����Ǿ�� �� ���̹Ƿ� Attack�Լ��� ���� �Ѵ�.


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class AEnemy* CombatTarget;	//������ ���� ����� �־�� �Ѵ�. �翬�� ���̹Ƿ� EnemyŬ������ �ν��Ͻ��� �����.
	FORCEINLINE void SetCombatTarget(AEnemy* Target) { CombatTarget = Target; } //������ ���� ����� �����ϴ� �Լ�. ������ �����Ÿ� �ݰ�, �� Combat Sphere�� ������ �Ǿ��� �� �����Ѵ�.

	FRotator GetLookAtRotationYaw(FVector Target); // �÷��̾ ������ ������ ���߷��� �׿� ��� �������� ������ ���� ȸ���� �� ������ �˷��־�� �Ѵ�. �¿�θ� ȸ���ϹǷ� ȸ�� ������ Yaw�̴�.






	//�̵����¿�(�ȴ��� �ٴ���) �޸��� �ӵ��� ���Ѵ�.
	void SetMovementStatus(EMovementStatus Status);

	//�Ϲ� �޸���� �������� �ӵ� ����
	//SetMovementStatus()�� �Ű������� ����.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float RunningSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float SprintingSpeed;


	//�𸮾� �����Ϳ��� sprintŰ�� ���ε��� shift�� ������ �ִ��� Ȯ���ϴ� bool�����̴�.
	bool bShiftKeyDown;

	/*����Ʈ Ű�� �������� ��, �ٽ� �������� �� ������ ����ϴ� �Լ�.*/
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
	/* �÷��̾��� ����
	/* 
	*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats") //MaxHealth�� MaxStamina�� ������ ���� ���̶� EditsDefaultsOnly�� �Ѵ�.
	float MaxHealth;	/*�� ü������ �� �ｺ���� �ۼ�Ƽ��*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats") //�ݸ� Health�� Stamina�� ��� �ٰų� �� �� �����Ƿ� EditAnywhere�� �Ѵ�.
	float Health;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats") //����� VisibleAnywhere�� EditDefaultOnly �Ǵ� EditAnywhere�� �Բ� UPROPERTY�� �Ű������� �� �� ����.
	float MaxStamina;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats") //������ ��쵵 ���������� �ٰų� �� �� �����Ƿ� EditAnywhere�� �Ѵ�.
	int32 Coins;		//�𸮾� �������� �ַ� ���Ǵ� ������ �ڷ����� int32 �ڷ����̴�. 

	void DecrementHealth(float Amount);	//ü���� �پ��� ���
	
	//�÷��̾�� �������� �����ؾ� �Ѵ�. ����� �������� ��ȯ�ϴ� �Լ��� ������ ���̴�. 
	//�𸮾� ���� ��ü �����ϴ� �Լ��� �ִ� : TakeDamage
	virtual float TakeDamage(float DamageAmount,struct FDamageEvent const& DamageEvent,class AController* EventInstigator,AActor* DamageCauser) override; 
	//ù��° �Ű������� ���������̴�. 
	//������ �Ű������ �������� �ִ� �ٸ� ������ �����̴�.
	//�𸮾��� ���Ĺ����� TakeDamage ����ٶ�

	UFUNCTION(BlueprintCallable)
	void Die();

	virtual void Jump() override;

	UFUNCTION(BlueprintCallable)
	void IncrementCoins(int32 Amount);
	UFUNCTION(BlueprintCallable)
	void IncrementHealth(float Amount);


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

	//�����¿�� �����̰� �ִ� ������ Ȯ��
	bool bMovingForward;
	bool bMovingRight;

	//1,2�� ���� ���޾� �ϱ� ���� ���� ��.
	int32 AttackStandard;

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


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Items") //���ΰ� ĳ���ʹ� �ϳ����̱� ������ ���� ����������¸� ���������ϰ� ���� �ʿ䰡 ����.
	class AWeapons* WeaponEquipped;


	/*
	�÷��̾�� ���⿡ ������ �ٰ����� �� ���⸦ ����� �� ������, ������� �� ���� ������ �� �ְ� �Ѵٸ� �� ����.
	�̸� ���� ������ ������ش�. ��� ����Ӹ� �ƴ϶� �屸���� �ش��Ѵ�.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items") 
	class AItem* ActiveOverlappingItem;

	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) { ActiveOverlappingItem = Item; }

	void SetWeaponEquipped(AWeapons* WeaponToSet);

	FORCEINLINE AWeapons* GetWeaponEquipped() { return WeaponEquipped; } //������ ��� ����

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anims")
	bool bAttacking; //�Ϲݰ��� ���� ����

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bStrongAttacking; //������ ���� ����

	void Attack();//���� ����
	void StrongAttack();//������ ����

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* CombatMontage; //C++���� �ִԸ�Ÿ�ְ� ȣ��Ǵ� ��� : UAnimMontage

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* CombatMontage_StrongAtck; //C++���� �ִԸ�Ÿ�ְ� ȣ��Ǵ� ��� : UAnimMontage
	
	UFUNCTION(BlueprintCallable)
	void AttackEnd(); //���� ��

	UFUNCTION(BlueprintCallable)
	void StrongAttackEnd(); //������ ��


	UFUNCTION(BlueprintCallable)
	void SwingingSound(); //���� �ֵθ��� �Ҹ�

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	void RefreshTarget(); //Ÿ���� ���� �����ϰ� ��

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<AEnemy> TargetingActor; //�÷��̾� ĳ���Ϳ� ��ġ�� ���� �� �� Ŭ������ ��󳻱� ���� "GetOverlappingActors"�� �Ű������� �ѱ�� ����.

};
