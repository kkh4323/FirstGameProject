// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Main.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"



/*
���� �̵� ������ ũ�� 3�����̴�.
1. ���̵� ����(�÷��̾� �߰߸���)
2. ���� ����(�÷��̾� �߰� �� ����)
3. ���� ����(�÷��̾�� �����Ÿ� ���Ϸ� �ٰ��� ����)
*/
UENUM(BlueprintType)
enum class EEnemyMovementStatus :uint8
{
	EMS_Idle UMETA(DisplayName = "Idle"),	//�ƹ��͵� �� �ϴ� ����
	EMS_MoveToTarget UMETA(DisplayName = "MoveToTarget"), //�÷��̾ �����ϴ� ����
	EMS_Attacking UMETA(DisplayName = "Attacking"), //�÷��̾ �����ϴ� ����
	EMS_EnemyDead UMETA(DisplayName = "EnemyDead"), //���� ���� ����. ���� ������ �� ���·� ��ȯ�Ѵ�. EnemyDie�Լ� ������ �ߵ�.


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
	�÷��̾ ���� �ݰ� �ȿ� ������ NPC�� �̸� �����ϰ� �÷��̾ �i�ư����� ���� ���̴�.
	�̸� ���� ���� ����.
	�������� �̳��� ��������� NPC�� �Ͽ��� �÷��̾ �����ϵ��� ��.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class USphereComponent* AgroSphere; //��׷θ� ���� �ݰ�

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	USphereComponent* CombatSphere; //��׷θ� ���� �ݰ�

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class AAIController* AIController; //AI�� ���õ� ����� ���� Ŭ����.
	

	//���� �ݰ濡 ���ڸ��� �÷��̾ �����ϸ� ������ �ʹ� ������� �� ����. �ð����� �ΰ� �����ϰԲ� �� ����. 
	//�ð����� ���� ����� ���� FTimerHandle�� Ȱ���Ѵ�.
	//�ð������� ������ ���� �� ���� �����̴�.
	FTimerHandle AttackTimer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackMinTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackMaxTime;

	//���� ���� ü��

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")//�� ���� ��� �ν��Ͻ����� �������Ʈ�� ���� �����ϵ��� ���Ѵ�.
	float EnemyHealth; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float EnemyMaxHealth; //�ｺ�ٸ� ǥ���� �� �� ����ġ ������ �ʿ��ϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float Damage; //���� �÷��̾�� �ִ� ������.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UAnimMontage* CombatMontage;

	//bool bShift;
	bool bHasValidTarget; // ��ȿ�� ���� ����� �������� �Ǵ��ϴ� ��������, �̰��� false�̸� ���� �÷��̾ ���̻� ���ݴ������ �������� �ʰ� ������ �ߴ��Ѵ�.

	/*����Ʈ Ű�� �������� ��, �ٽ� �������� �� ������ ����ϴ� �Լ�.*/
	void ShiftKeyDown();

	void ShiftKeyUp();


	/*
	���� ������ �޾��� �� ��ƼŬ �ý����� ���� ����ȿ���� ���� ���̴�.
	enemy ��� ���Ͽ� �̸� �����ϴ� ������ �� Ŭ������ ���� ������ ��ü���� �ٸ� ������ ��ƼŬ �ý����� ������ �� �ֵ��� �ϱ� �����̴�.
	���� ��� � ���� �ʷϻ� �ǰ� ������ �ݸ� � ���� ���� �ǰ� ���´ٵ���.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UParticleSystem* HitParticles;

	//���� ���⿡ �¾��� �� �Ҹ��� ������ �Ѵ�.(����̶���� ��Ź�� �Ҹ������ ���)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class USoundCue* HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* ScreamingSound;

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

	//���� ���⸦ �ֵθ� �� ������ �Ҹ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* SwingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	USoundCue* SwingSound2;

	//�� ��ü�� ���� �������� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

	//���� �ֵθ��� ���⿡ �ڽ� ������Ʈ�� �ٿ� �÷��̾ ���� Ÿ�� ��ȿ������ �ǵ��� ���� ���̴�.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class UBoxComponent* CombatCollision;

	FTimerHandle DeathTimer;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DeathDelay;//���� �Ŀ� ���忡 ���� �ð�


	void EnemyDecrementHealth(float Amount, AActor* DamageCauser);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;



	//��׷� �ݰ�� ���� ���� �ݰ濡 �÷��̾ '��'������ �˸�.
	UFUNCTION()
	virtual void AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	UFUNCTION()
	virtual void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);



	//�� NPC�� ���� ������ ���������� �ƴ��� �� �� �ֵ��� �ϴ� �Ҹ��� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bAttacking; 

	//Ÿ������ AI�� �̵��ϵ��� �ϴ� �Լ�. Main�� target�� �Ű������� ����.(�÷��̾� ĳ���Ͱ� Ÿ���� �ȴ�.)
	UFUNCTION(BlueprintCallable)
	void MoveToTarget(class AMain* Target);


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bOverlappingCombatSphere; //NPC�� �÷��̾ �����ϴ� ���� �÷��̾ �޾Ƴ� ������ NPC�� �����ϴ� ������ ������ �÷��̾ �����ϰԲ� �� ����. �� �� �ʿ��� ����.

	bool bPainSoundOnGoing;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	AMain* CombatTarget;

	//Weapons.h�� �ִ� ���� �״�� ������ ��. ���Ⱑ Ÿ��ȿ���� ���ϴ� ������ NPC�� ���Ⱑ �÷��̾� ĳ������ ��ü�� �ε����� ��, �� Overlap�Ǿ��� ���̴�.
	UFUNCTION()
	void CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void ActivateCollision();

	UFUNCTION(BlueprintCallable)
	void DeactivateCollision();

	//�� ���ݽ� ������ ������ ���� �Լ��� �� ���� �ߴ� �Լ�

	void EnemyAttack();


	UFUNCTION(BlueprintCallable)
	void EnemyAttackEnd();

	UFUNCTION(BlueprintCallable)
	void EnemySwingingSound1();

	//�� ���� �÷��̾�κ��� ������ ������ �������� �޾ƾ� �ϹǷ� TakeDamage�̿��� ���� ��� ����.
	virtual float TakeDamage(float DamageAmount,struct FDamageEvent const& DamageEvent,class AController* EventInstigator,AActor* DamageCauser) override; 

	void EnemyDie(AActor* Causer);

	//���� ������ �׾� �����ִ� ���ۻ��¸� ��� �����ؾ� �Ѵ�. �̸� ���� �ִϸ��̼� ��Ÿ�ֿ��� ���� �׾� �����ִ� ������ ��Ƽ���̸� �����ϰ� C++�� ���� �� ������ ��� ���� �ִϸ��̼��� �����ֵ��� ���־�� �Ѵ�.
	//UFUNCTION(BlueprintCallable)
	//void DeathEnd();

	//���� ����ִ��� Ȯ��. ������� ������ Ž������ �� �÷��̾ �ν��ϰ� �����ϴ� �� Ȱ���� �ϰ� �ϱ� ����. ��� Sphere�ȿ� Ȱ�� �������� �� ��.
	bool IsAlive();


	void Disappear();//���� ������ ��������� �ϴ� �Լ�.
};
