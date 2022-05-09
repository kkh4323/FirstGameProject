// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h" //무기에 박스 컴포넌트를 입혀주므로 필요한 헤더 파일.
#include "Enemy.h"
#include "Engine/SkeletalMeshSocket.h"

AWeapons::AWeapons()
{
	//스켈레탈 메시컴포넌트 생성
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision")); //전투 시 무기가 적의 몸체에 닿았을 때를 알리기 위한 컴포넌트.
	CombatCollision->SetupAttachment(GetRootComponent());//콜리젼을 위한 박스컴포넌트 생성. 이제 블루프린트에서 볼 수 있다.

	

	bWeaponParticle = false;

	WeaponState = EWeaponState::EWS_Pickup; 

	Damage = 25.f; //무기가 디폴트로 갖는 데미지.
}

//뭐든 무기와 부딪히는 것이 있다면 OverlapBegin과 OverlapEnd함수가 호출될 것이다.
void AWeapons::BeginPlay()
{
	Super::BeginPlay();

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeapons::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AWeapons::CombatOnOverlapEnd);
	

	//무기에 콜리젼 오버랩이 될 때마다 파티션이 재생되는 것을 방지한다.
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); //ActivateCollision이 호출되기 전까지는 어떤 충돌도 무시한다.
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); //기본적으로는 어떠한 것과 오버랩 되어도 무시하도록 한다.
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); //Pawn과 오버랩 될 때에만 반응하도록 한다.


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

//무기의 몸체부분에 생성한 컴포넌트 박스와 적 NPC의 몸체가 겹쳤을 대 발생시킬 이벤트를 여기서 정의한다.	
void AWeapons::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//무기와 overlap될 다른 액터를 check할 것이다.
	if (OtherActor) //만약 무기와 Overlap된 다른 액터가 유효한 것이라면(여기서는 당연히 적이다. 그러므로 enemy헤더파일을 인클루드 해야 한다.)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor); //확인한 OtherActor를 Enemy타입으로 Enemy라는 변수에 넣어준다.
		if (Enemy)
		{
			if (Enemy->HitParticles) // Particle 이 설정되지 않은 상태에서 이를 수정하려 하면 에디터에서 크래쉬가 발생한다.
			{
				const USkeletalMeshSocket* WeaponSocket = SkeletalMesh->GetSocketByName("WeaponSocket"); //블루프린트 에디터에서 무기의 날 부분에 만든 소켓을 적 NPC를 공격할 때 유효한 타격점으로 만들 것이다.(이렇게 안 하면 무기 날 부분이 아니라 일반적으로 root로 지정되어 있는 무기의 손잡이 부분이 타격점이 되는 현상이 발생한다.)
				if (WeaponSocket)
				{
					FVector SocketLocation = WeaponSocket->GetSocketLocation(SkeletalMesh);
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->HitParticles, SocketLocation, FRotator(0.f), false);
					//HitParticle이 유효한지 확인된다면 Emitter가 이 위치(적이 칼을 맞는 자리)에서 특정 파티클 시스템이 나타나도록 할 것이다.
					//3번째 매개변수인 SocketLocation은 파티클시스템 Emitter를 무기의 위치에 발생시킬 것이다. 
					//마지막 매개변수는 파티클 시스템이 재생된 이후이 자동으로 없어지는지 설장하는 것이다. 한번만 재생할 것이므로 false를 할 것이다.
				}
			}
			if (Enemy->HitSound) //적을 타격하고 파티클까지 실행되는 것까지 확인한 후 HitSound 재생
			{
				UGameplayStatics::PlaySound2D(this, Enemy->HitSound); //HitSound는 enemy헤더파일에 있으나 include 했으므로 여기서도 사용 가능.
			}

			if (Enemy->ScreamingSound) //마찬가지로 ScreamingSound 도 재생
			{
				UGameplayStatics::PlaySound2D(this, Enemy->ScreamingSound); 
			}
		}
	}
}

void AWeapons::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}





void AWeapons::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // ActivateCollision이 호출될 때에만 QueryOnly로 충돌이 발생한다.
}

void AWeapons::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); //디폴트로 어떤 충돌도 무시한다.
}