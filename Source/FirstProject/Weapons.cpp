// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "particles/ParticleSystemComponent.h"

AWeapons::AWeapons()
{
	//스켈레탈 메시컴포넌트 생성
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());


	bWeaponParticle = false;

	WeaponState = EWeaponState::EWS_Pickup; 
}

void AWeapons::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	/*무기를 드는 것은 캐릭터가 무기 아이템에 접근해 무기Mesh와 캐릭터 Mesh가 겹쳤을 때 발생해야 한다.*/
	if ((WeaponState == EWeaponState::EWS_Pickup) && OtherActor) //아직 플레이어에 의해 장착되지 않은 무기인 경우. OtherActor은 무기와 닿은 액터가 캐릭터인지 아닌지 확인하는 것.
	{
		/*
		무기아이템과 닿은 것이 캐릭터(Main)인지 확인하고 맞다면 Equip 함수를 호출. 
		Equip은 Main을 매개변수로 받아 아이템을 장착시킨다.
		*/
		AMain* Main = Cast<AMain>(OtherActor);	
		if (Main) //캐릭터가 맞는지 확인
		{
			Main->SetActiveOverlappingItem(this); //무기 등 아이템에 접촉했을 때 overlapping 한 상태임을 알린다. 무기를 바꿀 것인지, 획득할 것인지 등을 묻는 데 사용할 것임.
		}
	}
}
void AWeapons::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	if (OtherActor)
	{
		/*
		무기와 캐릭터가 접촉하지 않은 상태라면 overlapping 상태를 해제할 수 있다. 
		*/
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main) //캐릭터가 맞는지 확인
		{
			Main->SetActiveOverlappingItem(nullptr); //아무것도 접촉하지 않은 상태이기에 nullptr을 매개변수로 준다.
		}
	}
}

void AWeapons::Equip(AMain* Character)
{
	if (Character) //매개변수로 들어온 캐릭터가 NULL이 아니어야 진행
	{
		//카메라가 무기를 캐릭터 메시로 착각해 무기에 줌인이 되지 않도록 방지해야 한다.(카메라가 무기를 무시하도록 만들어야 한다.)
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		//플레이어를 제외한 다른 Pawn들에게도 마찬가지
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		//무기를 장착할 때 물리효과가 적용되면 플레이어의 손과 무기가 계속 충돌하는 문제가 발생할 수 있음. 이를 제거.
		SkeletalMesh->SetSimulatePhysics(false);

		/*캐릭터 스켈레톤에 소켓을 추가한다.*/
		const USkeletalMeshSocket* Weapon_R = Character->GetMesh()->GetSocketByName("Weapon_R");
		/*
		GetSocketByName 은 이름으로 장착시킬 아이템의 소켓을 찾는다.
		블루프린트에서 Weapon_R이라는 이름으로 소켓을 만들고 저장했으므로 이를 전달한다.
		결과적으로 이 코드는 블루프린트에서 만든 Weapon_R 소켓을 참조하는 것이다.
		*/



		if (Weapon_R)
		{
			//소켓에 부착시킨 액터와 캐릭터를 붙인다.
			Weapon_R->AttachActor(this, Character->GetMesh());
			/*
			Item 클래스를 상속했기 때문에 rotate 을 활성화하면 캐릭터의 스켈레톤에 부착되었을 때에도 회전한다.
			캐릭터에게 부착되고 난 뒤에는 회전하지 않도록 회전을 비활성화 시킨다.
			*/
			bRotate = false;

			//Character->GetWeaponEquipped()->Destroy(); //장착했던 기존 장비는 완전삭제한다. 장비를 장착하지 않고 있더라도 동일하게 동작 가능. 
			Character->SetWeaponEquipped(this); //this 는 WeaponToSet으로, 플레이어가 들 무기를 매개변수로 전달하는 것이다.
			Character->SetActiveOverlappingItem(nullptr); // 이미 장비를 부착하고 아무것도 접촉하지 않는 상태. 
		
			WeaponState = EWeaponState::EWS_Equipped; //공격 애니메이션 1회 시행 후 무기가 손에서 사라지는 것을 방지.

		}

		//무기를 장착하면 소리가 나도록 한다.
		if (OnEquipSound)
		{
			UGameplayStatics::PlaySound2D(this, OnEquipSound);
		}


		if (!bWeaponParticle)
		{
			IdleParticlesComponent->Deactivate();
		}
	}
}
//결과적으로 Equip 함수를 호출하면 아이템이 부착된 소켓을 플레이어 스켈레톤에 다시 부착시키는 것이다.