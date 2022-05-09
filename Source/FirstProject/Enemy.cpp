// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Main.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Editor/EditorEngine.h"



/*
적의 이동 로직은 크게 3가지이다.
1. 아이들 상태(플레이어 발견못함)
2. 추적 상태(플레이어 발견 후 추적)
3. 공격 상태(플레이어에게 일정거리 이하로 다가와 공격)
*/


// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/*
	모든 것에는 root 컴포넌트가 붙어있다. 일례로 캐릭터형의 액터에는 캡슐형의 컴포넌트가 붙어있다.
	NPC의 주의를 끄는 AgroSphere와 CombatSphere에 루트 컴포넌트를 붙여주어야 한다.
	이 두 기능은 모두 OverlapBegin에 실행된다.
	*/
	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(600.f); //적의 플레이어 인식 반경 초기화
	
	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(50.f); //적의 플레이어 공격 반경 초기화

	bOverlappingCombatSphere = false;

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision")); //적의 무기가 플레이어에 대해 갖는 유효 타격 박스
	//적의 특정 위치에 이 박스를 만들고 싶다면, 그 위치에 소켓을 붙이고 거기에 CombatCollision을 부착시키는 것이 좋다. AttachToComponent는 한 컴포넌트를 다른 컴포넌트에 부착시키는 함수이다. 이 경우에 사용.
	CombatCollision->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("weapon_rSocket"));

	Health = 75.f; //적 기본 체력
	MaxHealth = 200.f; //적 최고 체력
	Damage = 20.f; //적 데미지
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AAIController>(GetController()); //AAIController를 위한 헤더파일은 'AIController.h'이다.

	//AgroSphere, CombatSphere와 BeginOverlap 바인딩
	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	//Weapons.cpp에 있는 파라미터를 그대로 가져온 것. NPC가 플레이어를 공격할 때에도 그 반대와 같은 방식과 조건으로 동작해야 하기 때문.
	//무기에 콜리젼 오버랩이 될 때마다 파티션이 재생되는 것을 방지한다.
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); //ActivateCollision이 호출되기 전까지는 어떤 충돌도 무시한다.
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); //기본적으로는 어떠한 것과 오버랩 되어도 무시하도록 한다.
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); //Pawn과 오버랩 될 때에만 반응하도록 한다.

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapEnd);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}



//어그로 반경과 공격 시작 반경에 플레이어가 '접'했음을 알림.
void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)	//무언가 인식 반경에 닿음
	{
		AMain* Main = Cast<AMain>(OtherActor); //Main 은 주인공 캐릭터(형변환) 
		if (Main)	//닿은 물체가 주인공 캐릭터이면
		{
			MoveToTarget(Main); //Main으로 이동
		}
	}
}

void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor) //만약 플레이어가 도망친다든지 해서 AgroSphere 밖으로 나간다면
	{
		AMain* Main = Cast<AMain>(OtherActor);
		{
			if (Main)
			{
				SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle); //NPC로 하여금 플레이어를 더 이상 추적하지 않고 Idle 상태로 돌아가게 함
				if (AIController)
				{
					AIController->StopMovement(); //NPC가 뭘 하든 멈추게 하는 함수.
				}
			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor) //무언가가 CombatSphere와 겹쳤을 때
	{
		AMain* MainCharacter = Cast<AMain>(OtherActor);
		{
			if (MainCharacter) //그 겹친 무엇인가가 주인공 캐릭터라면
			{
				CombatTarget = MainCharacter;	//공격 대상은 메인캐릭터
				bOverlappingCombatSphere = true; //탐지 범위에 들어서면
				//SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking); //NPC의 동작 상태를 공격상태로 전환 *Attack함수를 구현했으므로 사용 필요 없음
				EnemyAttack(); ///NPC의 동작 상태를 공격상태로 전환
			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor) //만약 CombatSphere로부터 플레이어가 떨어진다면(도망친다든지)
	{
		AMain* MainCharacter = Cast<AMain>(OtherActor);
		{
			if (MainCharacter)
			{
				bOverlappingCombatSphere = false;
				//if (EnemyMovementStatus != EEnemyMovementStatus::EMS_Attacking) //공격을 하고 있는 중이라면 공격을 계속 끝까지 진행하도록 하겠지만 그것이 아니라면 플레이어에게 이동
				if (EnemyMovementStatus == EEnemyMovementStatus::EMS_Attacking)
				{
					MoveToTarget(MainCharacter);
					CombatTarget = nullptr;
				}
			}
		}
	}
}




void AEnemy::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//NPC의 무기와 overlap될 다른 액터를 check할 것이다. 여기서는 당연히 플레이어 캐릭터인 main이 될 것이다. enemy가 되는 weapons.cpp에서와는 다르다.
	if (OtherActor) //만약 무기와 Overlap된 다른 액터가 유효한 것이라면(여기서는 당연히 플레이어 캐릭터이다. 그러므로 Main헤더파일을 인클루드 해야 한다.)
	{
		AMain* MainCharacter = Cast<AMain>(OtherActor); //확인한 OtherActor를 Main타입으로 MainCharacter라는 변수에 넣어준다.
		if (MainCharacter) //만약 닿은 것이 주인공 캐릭터라면 아래 실행.
		{
			if (MainCharacter->BloodParticles) // Particle 이 설정되지 않은 상태에서 이를 수정하려 하면 에디터에서 크래쉬가 발생한다.
			{
				const USkeletalMeshSocket* HitPoint = GetMesh()->GetSocketByName("HitPoint"); //블루프린트 에디터에서 무기의 날 부분에 만든 소켓을 적 NPC를 공격할 때 유효한 타격점으로 만들 것이다.(이렇게 안 하면 무기 날 부분이 아니라 일반적으로 root로 지정되어 있는 무기의 손잡이 부분이 타격점이 되는 현상이 발생한다.) HitPoint라는 별명은 블루프린트에서 생성한 것이다.
				if (HitPoint)
				{
					FVector SocketLocation = HitPoint->GetSocketLocation(GetMesh());
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MainCharacter->BloodParticles, SocketLocation, FRotator(0.f), false);
					//HitParticle이 유효한지 확인된다면 Emitter가 이 위치(적이 칼을 맞는 자리)에서 특정 파티클 시스템이 나타나도록 할 것이다.
					//3번째 매개변수인 SocketLocation은 파티클시스템 Emitter를 무기의 위치에 발생시킬 것이다. 
					//마지막 매개변수는 파티클 시스템이 재생된 이후이 자동으로 없어지는지 설장하는 것이다. 한번만 재생할 것이므로 false를 할 것이다.
				}
			}
			if (MainCharacter->SmashingSound) //적을 타격하고 파티클까지 실행되는 것까지 확인한 후 HitSound 재생
			{
				UGameplayStatics::PlaySound2D(this, MainCharacter->SmashingSound); //HitSound는 enemy헤더파일에 있으나 include 했으므로 여기서도 사용 가능.(단 soundcue 헤더파일은 따로 추가해주어야 한다. 유념!)
			}

			if (MainCharacter->PainSound) //마찬가지로 ScreamingSound 도 재생
			{
				UGameplayStatics::PlaySound2D(this, MainCharacter->PainSound);
			}
		}
	}
}

void AEnemy::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}


//블루프린트의 CombaMontage에서 AnimNotifier로 언제 Collision을 활성화 또는 비활성화 시킬지 시점을 구했다. 
//Weapons.cpp에서 구현한 것처럼 적의 무기에도 플레이어가 충돌하고 반응을 해야 한다. 콜리젼을 활성화,비활성화 시키는 것은 이 작업에 필요하다.
void AEnemy::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // ActivateCollision이 호출될 때에만 QueryOnly로 충돌이 발생한다.
}

void AEnemy::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); //디폴트로 어떤 충돌도 무시한다.
}



//타겟으로 이동하도록 하는 함수 : 플레이어 캐릭터가 NPC의 인식 반경에 접하는 순간 호출된다 = AgroSphereOnOverlapBegin.
void AEnemy::MoveToTarget(class AMain* Target)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

	if (AIController)
	{
		/*UE_LOG(LogTemp, Warning, TEXT("MoveToTarget()"));*/ //겹쳤을 때 풀력로그에 출력할 것
		FAIMoveRequest MoveRequest; 
		MoveRequest.SetGoalActor(Target); //GoalActor은 MoveRequest 구조체의 멤버함수로, 타겟 액터, 즉 여기서는 메인캐릭터가 된다.
		MoveRequest.SetAcceptanceRadius(1.0f); // 적 NPC의 루트 컴포넌트와 플레이어 캐릭터 사이의 루트 컴포넌트가 닿지 않도록 서로 띄워놓는 거리.

		FNavPathSharedPtr NavPath;
		AIController->MoveTo(MoveRequest, &NavPath); //MoveTo는 AI의 제어와 관련된 함수로 매개변수로 들어가는 것의 종류가 많다. 이는 언리얼 엔진 공식문서를 참고하도록.
		//AI와 캐릭터간 OverlapBegin이 되면서 MoveToTarget이 호출되고 MoveTo가 실행되고 Target을 Main, 즉 주인공캐릭터로 잡으면서 따라오게 되는 것이다.
		
		//TArray<FNavPathPoint> PathPoints = NavPath->GetPathPoints();
		
		//AgroSphere와 겹치면 그자리를 표시해줌.
		/*auto PathPoints = NavPath->GetPathPoints();
		for (auto Point : PathPoints)
		{
			FVector Location = Point.Location;

			UKismetSystemLibrary::DrawDebugSphere(this, Location, 30.f, 7, FLinearColor::Green, 10.f, 1.25f);
		}*/
	}
}

void AEnemy::EnemyAttack()//적 공격시 실행할 내용
{
	if (AIController)
	{
		AIController->StopMovement();
		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking); //AIController이 유효하다면(항상 유효함) 공격을 진행
	}
	if (!bAttacking)	//만약 공격모션을 진행하고 있는 것이 아니라면 공격모션을 시작할 수 있음
	{
		int32 AttackNumber = FMath::RandRange(0, 2);//무작위 공격 형태를 위한 난수 생성
		bAttacking = true;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); //Mesh에 있는 Animation 수행
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(CombatMontage, 1.2f);	//CombatMontage의 애니메이션 1.2배속으로 수행
			
			//AttackNumber에 따라 적 NPC는 다른 모션의 공격모션을 취할 것이다.
			if (AttackNumber==0) AnimInstance->Montage_JumpToSection(FName("Attack1"), CombatMontage); //블루프린트의 CombatMontage 애니메이션 몽타주에서 설정한 "Attack1"섹션을 FName의 파라미터로 넘겨야 함을 유의
			else if (AttackNumber==1) AnimInstance->Montage_JumpToSection(FName("Attack2"), CombatMontage);
			else AnimInstance->Montage_JumpToSection(FName("Attack3"), CombatMontage);
		}
		if (SwingSound) //적이 무기를 휘두르는 소리 재생
		{
			UGameplayStatics::PlaySound2D(this, SwingSound);
		}
	}
}

void AEnemy::EnemyAttackEnd()//적 공격이 끝났을 때.
{
	bAttacking = false;
	if (bOverlappingCombatSphere) // 공격이 끝났는데 NPC의 CombatSphere에 플레이어가 겹친다면 공격을 지속해야 함
	{
		EnemyAttack();
	}
	//그렇지 않다면 공격을 중단.
}