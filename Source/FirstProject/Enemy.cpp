// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Main.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "MainPlayerController.h"
#include "Editor/EditorEngine.h"



/*
적의 이동 로직은 크게 4가지이다.
1. 아이들 상태(플레이어 발견못함)
2. 추적 상태(플레이어 발견 후 추적)
3. 공격 상태(플레이어에게 일정거리 이하로 다가와 공격)
4. 죽음 상태(아무것도 안 하며 곧 월드에서 사라짐)
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

	AttackMinTime = 1.5f;
	AttackMaxTime = 3.f;

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision")); //적의 무기가 플레이어에 대해 갖는 유효 타격 박스
	//적의 특정 위치에 이 박스를 만들고 싶다면, 그 위치에 소켓을 붙이고 거기에 CombatCollision을 부착시키는 것이 좋다. AttachToComponent는 한 컴포넌트를 다른 컴포넌트에 부착시키는 함수이다. 이 경우에 사용.
	CombatCollision->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("weapon_rSocket"));

	EnemyHealth = 75.f; //적 기본 체력
	EnemyMaxHealth = 200.f; //적 최고 체력
	Damage = 20.f; //적 데미지

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle; //적 초기상태
	DeathDelay = 10.f;
	bShiftKeyDown = false;	//쉬프트 키는 디폴트로 눌려져 있지 않은 상태이다.
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
	if (OtherActor && IsAlive())	//무언가 인식 반경에 닿음
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
				if (Main->CombatTarget == this)
				{
					//적과 메인캐릭터가 전투반경에서 떨어졌다면 플레이어 캐릭터는 초점 대상을 더이상 enemy객체로 지정하지 않음(nullptr)
					//여러명의 적이 있을 경우 특정 하나의 적 객체와의 오버랩이 끝날 때 이 작업을 수행하기 위해 if문 안에 넣어준다.
					Main->SetCombatTarget(nullptr);
				}
				Main->SetHasCombatTarget(false); 
				if (Main->MainPlayerController)
				{
					Main->MainPlayerController->RemoveEnemyHealthBar(); // 플레이어가 적의 탐지반경 바깥으로 나온다면 적 체력상태바 표시를 없앤다.
				}
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
	if (OtherActor && IsAlive()) //무언가가 CombatSphere와 겹쳤을 때
	{
		AMain* MainCharacter = Cast<AMain>(OtherActor);
		{
			if (MainCharacter) //그 겹친 무엇인가가 주인공 캐릭터라면
			{
				MainCharacter->SetCombatTarget(this);	//적과 메인캐릭터가 CombatSphere에서 오버랩되었다면 플레이어 캐릭터는 이 적(this 포인터로, 적클래스 객체)에게 초점을 맞출 것이다.
				MainCharacter->SetHasCombatTarget(true);
				if (MainCharacter->MainPlayerController) //또한 적 체력상태바를 띄우도록 한다.(이렇게 띄워진 상태바는 어그로스피어를 나오면 없어지도록 한다.)
				{
					MainCharacter->MainPlayerController->DisplayEnemyHealthBar();
				}
				
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
				//적 전투 반경을 벗어나면 공격 지연을 위한 타이머 진행을 멈춰야 한다.
				//이렇게 멈춘 타이머는 플레이어가 적의 전투반경으로 다시 들어가 타이머를 다시 작동시키기 전까지는 0이 되고 카운팅을 시작하지 않는다.
				GetWorldTimerManager().ClearTimer(AttackTimer); 
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
			if (DamageTypeClass)
			{
				//ApplyDamage함수 : 플레이어에게 데미지를 적용시키는 함수. 데미지를 받는 액터와 데미지량, 데미지를 주는 도구와 액터, UDamageType이라는 특별한 클래스를 통해 어떤 종류의 데미지인지를 골라 매개변수로 넘긴다.
				UGameplayStatics::ApplyDamage(MainCharacter, Damage, AIController, this, DamageTypeClass); //Main의 TakeDamage함수를 불러와 Damage를 DamageAmount로 매개변수로서 넘겨주도록 한다. 
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
	if (IsAlive()) //살아있다면
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
			if (AnimInstance && CombatMontage)
			{
				AnimInstance->Montage_Play(CombatMontage, 1.2f);	//CombatMontage의 애니메이션 1.2배속으로 수행

				//AttackNumber에 따라 적 NPC는 다른 모션의 공격모션을 취할 것이다.
				if (AttackNumber == 0) AnimInstance->Montage_JumpToSection(FName("Attack1"), CombatMontage); //블루프린트의 CombatMontage 애니메이션 몽타주에서 설정한 "Attack1"섹션을 FName의 파라미터로 넘겨야 함을 유의
				else if (AttackNumber == 1) AnimInstance->Montage_JumpToSection(FName("Attack2"), CombatMontage);
				else AnimInstance->Montage_JumpToSection(FName("Attack3"), CombatMontage);
			}
			if (SwingSound) //적이 무기를 휘두르는 소리 재생
			{
				UGameplayStatics::PlaySound2D(this, SwingSound);
			}
		}
	}
}

void AEnemy::EnemyAttackEnd()//적 공격이 끝났을 때.
{
	bAttacking = false;
	if (bOverlappingCombatSphere) // 공격이 끝났는데 NPC의 CombatSphere에 플레이어가 겹친다면 공격을 지속해야 함
	{
		//공격과 다음 공격 사이에 텀을 둔다.(난이도의 이유로)
		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::EnemyAttack, AttackTime);//다음 공격 전에 텀을 두도록 한다. 
		//적 객체가 공격함수를 실행하기 전에 시간 지연 최소와 최대치 사이의 랜덤한 시간값인 AttackTime 만큼 공격 사이에 텀을 두는 것이다.
		//적 전투 반경을 벗어나면 타이머 진행을 멈춰야 한다.(CombatSphereOnOverlapEnd에서 진행)
		EnemyAttack();
	}
	//그렇지 않다면 공격을 중단.
}



//아래 두 함수가 하는 일 : 쉬프트 키가 눌려져 있는지 아닌지만을 전달한다.
void AEnemy::ShiftKeyDown()
{
	bShiftKeyDown = true;
}

void AEnemy::ShiftKeyUp()
{
	bShiftKeyDown = false;
}




void AEnemy::EnemyDecrementHealth(float Amount)
{
	int32 ENYHitScene = FMath::RandRange(1, 4);
	UAnimInstance* AnimInstance2 = GetMesh()->GetAnimInstance(); //Mesh에 있는 Animation 수행
	if (EnemyHealth - Amount <= 0.f)
	{
		EnemyHealth -= Amount;	//체력이 줄어들게 함.
		EnemyDie();				//다 떨어지면 죽음. 
	}
	//체력이 다 떨어져 죽는 게 아니라면 그냥 체력이 깎이는 경우가 있음.
	else
	{
		if (bShiftKeyDown) ENYHitScene = 4;
	
		if (AnimInstance2)
		{
			AnimInstance2->Montage_Play(CombatMontage, 1.f);	//CombatMontage의 애니메이션 1배속으로 수행
			if (ENYHitScene == 1) AnimInstance2->Montage_JumpToSection(FName("Hit1"), CombatMontage); //블루프린트의 CombatMontage 애니메이션 몽타주에서 설정한 "Death"섹션을 FName의 파라미터로 넘겨야 함을 유의. 섹션 이름 정확해야 함.
			else if (ENYHitScene == 2) AnimInstance2->Montage_JumpToSection(FName("Hit2"), CombatMontage); //블루프린트의 CombatMontage 애니메이션 몽타주에서 설정한 "Death"섹션을 FName의 파라미터로 넘겨야 함을 유의. 섹션 이름 정확해야 함.
			else if (ENYHitScene == 3)AnimInstance2->Montage_JumpToSection(FName("Hit3"), CombatMontage); //블루프린트의 CombatMontage 애니메이션 몽타주에서 설정한 "Death"섹션을 FName의 파라미터로 넘겨야 함을 유의. 섹션 이름 정확해야 함.
			else AnimInstance2->Montage_JumpToSection(FName("HitHard"), CombatMontage);
		}
		EnemyHealth -= Amount;
	}
}



//적 또한 플레이어로부터 데미지를 받아야 한다. 
//Main과 달리 Enemy는 헬스포인트 증감이 구현된 함수가 있지 않으므로 직접 구현한다.
float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	////적의 체력이 공격으로 인해 0이하가 되면 죽음을 실행하는 함수를 실행. 그렇지 않으면 체력만 감소.
	//if (EnemyHealth - DamageAmount <= 0.f)
	//{
	//	EnemyHealth -= DamageAmount;
	//	EnemyDie();
	//}
	//else
	//{
	//	int32 ENYHitScene = FMath::RandRange(1, 3);
	//	UAnimInstance* AnimInstance2 = GetMesh()->GetAnimInstance(); //Mesh에 있는 Animation 수행
	//	if (AnimInstance2)
	//	{
	//		AnimInstance2->Montage_Play(CombatMontage, 1.f);	//CombatMontage의 애니메이션 1배속으로 수행
	//		if (ENYHitScene == 1) AnimInstance2->Montage_JumpToSection(FName("Hit1"), CombatMontage); //블루프린트의 CombatMontage 애니메이션 몽타주에서 설정한 "Death"섹션을 FName의 파라미터로 넘겨야 함을 유의. 섹션 이름 정확해야 함.
	//		else if (ENYHitScene == 2) AnimInstance2->Montage_JumpToSection(FName("Hit2"), CombatMontage); //블루프린트의 CombatMontage 애니메이션 몽타주에서 설정한 "Death"섹션을 FName의 파라미터로 넘겨야 함을 유의. 섹션 이름 정확해야 함.
	//		else AnimInstance2->Montage_JumpToSection(FName("Hit3"), CombatMontage); //블루프린트의 CombatMontage 애니메이션 몽타주에서 설정한 "Death"섹션을 FName의 파라미터로 넘겨야 함을 유의. 섹션 이름 정확해야 함.
	//	}
	//	EnemyHealth -= DamageAmount;
	//}
	
	EnemyDecrementHealth(DamageAmount);

	return DamageAmount;
}

//적이 죽을 때 호출되는 함수. 죽음 애니메이션을 실행시키고 적 객체를 월드에서 삭제.(혹은 그냥 그대로 둘 수도 있음)
void AEnemy::EnemyDie()
{
	UE_LOG(LogTemp, Warning, TEXT("Die Executed"))
	//적 상태 죽음상태로 전환
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_EnemyDead);
	int32 ENYDeathScene = FMath::RandRange(1, 4);
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); //Mesh에 있는 Animation 수행
	if (AnimInstance)
	{
		//AnimInstance->Montage_Play(CombatMontage, 1.f);	//CombatMontage의 애니메이션 1배속으로 수행
		//AnimInstance->Montage_JumpToSection(FName("EnemyDeath"), CombatMontage); 
		if (ENYDeathScene == 1)
		{
			AnimInstance->Montage_Play(CombatMontage, 1.f);	//CombatMontage의 애니메이션 1배속으로 수행
			AnimInstance->Montage_JumpToSection(FName("EnemyDeath"), CombatMontage); //블루프린트의 CombatMontage 애니메이션 몽타주에서 설정한 "Death"섹션을 FName의 파라미터로 넘겨야 함을 유의. 섹션 이름 정확해야 함.
		}
		else if (ENYDeathScene == 2)
		{
			AnimInstance->Montage_Play(CombatMontage, 1.f);	//CombatMontage의 애니메이션 1배속으로 수행
			AnimInstance->Montage_JumpToSection(FName("EnemyDeath2"), CombatMontage); //블루프린트의 CombatMontage 애니메이션 몽타주에서 설정한 "Death"섹션을 FName의 파라미터로 넘겨야 함을 유의. 섹션 이름 정확해야 함.
		}
		else if (ENYDeathScene == 3)
		{
			AnimInstance->Montage_Play(CombatMontage, 1.f);	//CombatMontage의 애니메이션 1배속으로 수행
			AnimInstance->Montage_JumpToSection(FName("EnemyDeath3"), CombatMontage); //블루프린트의 CombatMontage 애니메이션 몽타주에서 설정한 "Death"섹션을 FName의 파라미터로 넘겨야 함을 유의. 섹션 이름 정확해야 함.
		}
		else
		{
			AnimInstance->Montage_Play(CombatMontage, 1.f);	//CombatMontage의 애니메이션 1배속으로 수행
			AnimInstance->Montage_JumpToSection(FName("EnemyDeath4"), CombatMontage); //블루프린트의 CombatMontage 애니메이션 몽타주에서 설정한 "Death"섹션을 FName의 파라미터로 넘겨야 함을 유의. 섹션 이름 정확해야 함.
		}
	}

	//적이 죽으면 콜리젼이 비활성화 되어야 한다 : (위에서부터)무기콜리젼(무기에 달린 콜리젼)과 어그로 스피어(탐지반경), 전투콜리젼(전투시작반경), 캡슐컴포넌트(적 객체 물리적 충돌을 감지하는 캡슐)
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); //무기콜리젼(무기에 달린 콜리젼)
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); //어그로 스피어(탐지반경)
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 전투콜리젼(전투시작반경) 
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); //캡슐컴포넌트(적 객체 물리적 충돌을 감지하는 캡슐) *CapsuleComponent.h 인클루드 해야 함.
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::Disappear, DeathDelay);
}


//void AEnemy::DeathEnd()
//{
//	//적이 애니메이션의 특정 상태에서 멈춰있도록 하는 기능은 Mesh에 있다. bPauseAnims가 그것.
//	UE_LOG(LogTemp, Warning, TEXT("Death End Executed"))
//	//GetMesh()->bPauseAnims = true;
//	/*GetMesh()->bNoSkeletonUpdate = true;*/
//	// 위 두 함수는 적이 죽으면 애니메이션의 특정상태에서 메시가 멈춰있도록 하는 기능을 가진다.
//	// 적이 죽으면 이 함수가 호출됨으로써 그 상태로 고정한다.
//
//	//GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::Disappear, DeathDelay); // 타이머가 DeathDelay에 도달하면 Disappear함수를 불러 액터를 소멸시킴.
//	//Disappear();
//}


bool AEnemy::IsAlive()
{
	return GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_EnemyDead;
}

void AEnemy::Disappear()//적이 죽은 후 월드에서 사라져 메모리를 관리함.
{
	Destroy();
}