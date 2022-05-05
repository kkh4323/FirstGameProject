// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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


	//Ÿ������ AI�� �̵��ϵ��� �ϴ� �Լ�. Main�� target�� �Ű������� ����.(�÷��̾� ĳ���Ͱ� Ÿ���� �ȴ�.)
	UFUNCTION(BlueprintCallable)
	void MoveToTarget(class AMain* Target);


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	bool bOverlappingCombatSphere; //NPC�� �÷��̾ �����ϴ� ���� �÷��̾ �޾Ƴ� ������ NPC�� �����ϴ� ������ ������ �÷��̾ �����ϰԲ� �� ����. �� �� �ʿ��� ����.

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	AMain* CombatTarget;
};
