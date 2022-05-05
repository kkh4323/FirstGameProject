// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Main.h"


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
	CombatSphere->InitSphereRadius(100.f); //적의 플레이어 공격 반경 초기화

	bOverlappingCombatSphere = false;
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
		AMain* Main = Cast<AMain>(OtherActor);
		{
			if (Main) //그 겹친 무엇인가가 주인공 캐릭터라면
			{
				CombatTarget = Main;
				bOverlappingCombatSphere = true;
				SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking); //NPC의 동작 상태를 공격상태로 전환
			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor) //만약 CombatSphere로부터 플레이어가 떨어진다면(도망친다든지)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		{
			if (Main) 
			{
				bOverlappingCombatSphere = false;
				if (EnemyMovementStatus != EEnemyMovementStatus::EMS_Attacking) //공격을 하고 있는 중이라면 공격을 계속 끝까지 진행하도록 하겠지만 그것이 아니라면 플레이어에게 이동
				{
					MoveToTarget(Main);
					CombatTarget = nullptr;
				}
			}
		}
	}
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
		MoveRequest.SetAcceptanceRadius(15.0f); // 적 NPC의 루트 컴포넌트와 플레이어 캐릭터 사이의 루트 컴포넌트가 닿지 않도록 서로 띄워놓는 거리.

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