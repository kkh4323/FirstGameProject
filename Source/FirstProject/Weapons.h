// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Main.h"
#include "CoreMinimal.h"
#include "Item.h"	//무기 클래스는 item으로부터 onoverlapbegin, end 등의 일부 함수를 상속받아 사용할 것이다.
#include "Engine/SkeletalMeshSocket.h"
#include "Weapons.generated.h"


/**
 * 
 */



/*
장착하지 않은 무기가 둥둥 떠다니는 상태로 플레이어나 적에게 데미지를 주는 건 보기 좋지 않다.
사용할 때에만 데미지가 활성화 되도록 무기의 상태를 저장할 변수를 만들어야 한다.
그 자료형은 enum을 활용해 새로 정의해준다.
*/

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Pickup UMETA(DisplyName = "Pickup"), //그냥 아이템일 때의 상태
	EWS_Equipped UMETA(DisplyName = "Equipped"), //플레이어가 주워서 장착 아이템이 된 상태

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
	무기의 경우 static mesh가 아니라 skeletal mesh이다. 그래서 다른 item 처럼 mesh값을 설정할 수 없다.
	skeletal mesh값을 설정할 수 있는 변수를 만들어야 한다.
	USkeletalMeshComponent를 활용한다.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SkeletalMesh")
	class USkeletalMeshComponent* SkeletalMesh;	//SkeletalMesh의 생성


	//무기를 장비할 때 소리가 나도록 한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Sound")
	class USoundCue* OnEquipSound;


	//UFUNCTION() //상속받은 함수는 UFUNCTION으로 표시될 수 없다.
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override; //부모 클래스인 Item의 가상함수를 파생클래스가 덮어씌울 땐 이 표시(override)를 해준다(함수 오버라이딩)
	//UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	/*
	플레이어가 장비를 장착할 수 있게 한다. 
	Equip 함수를 사용. 매개변수로는 Main, 즉 주인공 캐릭터가 들어와야 한다.
	기본적인 개념 : 무기의 mesh를 블루프린트를 통해 생성한 '캐릭터의 소켓'에 부착시키는 것이다.
	*/
	void Equip(class AMain* Character);

	//WeaponState를 set 또는 get
	FORCEINLINE void SetWeaponState(EWeaponState State) { WeaponState = State; } 
	FORCEINLINE EWeaponState GetWeaponState() { return WeaponState; }
};
