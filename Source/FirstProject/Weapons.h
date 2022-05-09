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

	/*
	무기가 적과 부딪히면 데미지를 주고 피와 같은 파티클 시스템이 작동해야 한다. 이것이 구현되어 있지 않으면 무기는 그냥 적의 몸을 통과할 뿐이다.
	그래서 무기에 박스 컴포넌트를 붙여준다.(무기의 모양과 가장 유사한직사각형이 제일 적합할 것)
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item | Combat")
	class UBoxComponent* CombatCollision;


	//무기를 장비할 때 소리가 나도록 한다. sound 큐를 생성하고 블루프린트에서 이를 수정할 수 있게 한다. 어딘가에 소리를 넣고 싶을 때 이렇게 헤더파일에 먼저 생성하고 cpp 파일의 해당 위치에 넣는다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Sound")
	class USoundCue* OnEquipSound;	//이 헤더 파일에서 처음으로 soundcue를 만드는 것임. 이럴 땐 class를 기입해주어야 한다. 이 다음부터는 x.
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Combat")
	float Damage; //무기가 주는 데미지. 각 인스턴스마다 이 데미지를 블루프린트를 통해 수정 가능하도록 한다.


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Sound")
	USoundCue* SwingSound;


protected:
	virtual void BeginPlay() override;




public:


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

	

	//무기가 타격효과를 발하는 시점은 무기가 적 NPC의 몸체와 부딪혔을 때, 즉 Overlap되었을 때이다.
	UFUNCTION()
	void CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void ActivateCollision();

	UFUNCTION(BlueprintCallable)
	void DeactivateCollision();

};
