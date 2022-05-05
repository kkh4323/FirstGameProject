// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Main.h"
#include "CoreMinimal.h"
#include "Item.h"	//���� Ŭ������ item���κ��� onoverlapbegin, end ���� �Ϻ� �Լ��� ��ӹ޾� ����� ���̴�.
#include "Engine/SkeletalMeshSocket.h"
#include "Weapons.generated.h"


/**
 * 
 */



/*
�������� ���� ���Ⱑ �յ� ���ٴϴ� ���·� �÷��̾ ������ �������� �ִ� �� ���� ���� �ʴ�.
����� ������ �������� Ȱ��ȭ �ǵ��� ������ ���¸� ������ ������ ������ �Ѵ�.
�� �ڷ����� enum�� Ȱ���� ���� �������ش�.
*/

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Pickup UMETA(DisplyName = "Pickup"), //�׳� �������� ���� ����
	EWS_Equipped UMETA(DisplyName = "Equipped"), //�÷��̾ �ֿ��� ���� �������� �� ����

	EWS_MAX UMETA(DisplyName = "DefaultMax")
};

UCLASS()
class FIRSTPROJECT_API AWeapons : public AItem
{
	GENERATED_BODY()
	
public:
	AWeapons();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Item")
	EWeaponState WeaponState; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particles")
	bool bWeaponParticle;


	/*
	������ ��� static mesh�� �ƴ϶� skeletal mesh�̴�. �׷��� �ٸ� item ó�� mesh���� ������ �� ����.
	skeletal mesh���� ������ �� �ִ� ������ ������ �Ѵ�.
	USkeletalMeshComponent�� Ȱ���Ѵ�.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SkeletalMesh")
	class USkeletalMeshComponent* SkeletalMesh;	//SkeletalMesh�� ����


	//���⸦ ����� �� �Ҹ��� ������ �Ѵ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Sound")
	class USoundCue* OnEquipSound;


	//UFUNCTION() //��ӹ��� �Լ��� UFUNCTION���� ǥ�õ� �� ����.
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override; //�θ� Ŭ������ Item�� �����Լ��� �Ļ�Ŭ������ ����� �� �� ǥ��(override)�� ���ش�(�Լ� �������̵�)
	//UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	/*
	�÷��̾ ��� ������ �� �ְ� �Ѵ�. 
	Equip �Լ��� ���. �Ű������δ� Main, �� ���ΰ� ĳ���Ͱ� ���;� �Ѵ�.
	�⺻���� ���� : ������ mesh�� �������Ʈ�� ���� ������ 'ĳ������ ����'�� ������Ű�� ���̴�.
	*/
	void Equip(class AMain* Character);

	//WeaponState�� set �Ǵ� get
	FORCEINLINE void SetWeaponState(EWeaponState State) { WeaponState = State; } 
	FORCEINLINE EWeaponState GetWeaponState() { return WeaponState; }
};
