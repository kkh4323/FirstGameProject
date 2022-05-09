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


/*
���� �̵� ������ ũ�� 3�����̴�.
1. ���̵� ����(�÷��̾� �߰߸���)
2. ���� ����(�÷��̾� �߰� �� ����)
3. ���� ����(�÷��̾�� �����Ÿ� ���Ϸ� �ٰ��� ����)
*/


// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/*
	��� �Ϳ��� root ������Ʈ�� �پ��ִ�. �Ϸʷ� ĳ�������� ���Ϳ��� ĸ������ ������Ʈ�� �پ��ִ�.
	NPC�� ���Ǹ� ���� AgroSphere�� CombatSphere�� ��Ʈ ������Ʈ�� �ٿ��־�� �Ѵ�.
	�� �� ����� ��� OverlapBegin�� ����ȴ�.
	*/
	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(600.f); //���� �÷��̾� �ν� �ݰ� �ʱ�ȭ
	
	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(50.f); //���� �÷��̾� ���� �ݰ� �ʱ�ȭ

	bOverlappingCombatSphere = false;

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision")); //���� ���Ⱑ �÷��̾ ���� ���� ��ȿ Ÿ�� �ڽ�
	//���� Ư�� ��ġ�� �� �ڽ��� ����� �ʹٸ�, �� ��ġ�� ������ ���̰� �ű⿡ CombatCollision�� ������Ű�� ���� ����. AttachToComponent�� �� ������Ʈ�� �ٸ� ������Ʈ�� ������Ű�� �Լ��̴�. �� ��쿡 ���.
	CombatCollision->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("weapon_rSocket"));

	Health = 75.f; //�� �⺻ ü��
	MaxHealth = 200.f; //�� �ְ� ü��
	Damage = 20.f; //�� ������
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AAIController>(GetController()); //AAIController�� ���� ��������� 'AIController.h'�̴�.

	//AgroSphere, CombatSphere�� BeginOverlap ���ε�
	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	//Weapons.cpp�� �ִ� �Ķ���͸� �״�� ������ ��. NPC�� �÷��̾ ������ ������ �� �ݴ�� ���� ��İ� �������� �����ؾ� �ϱ� ����.
	//���⿡ �ݸ��� �������� �� ������ ��Ƽ���� ����Ǵ� ���� �����Ѵ�.
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); //ActivateCollision�� ȣ��Ǳ� �������� � �浹�� �����Ѵ�.
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); //�⺻�����δ� ��� �Ͱ� ������ �Ǿ �����ϵ��� �Ѵ�.
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); //Pawn�� ������ �� ������ �����ϵ��� �Ѵ�.
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



//��׷� �ݰ�� ���� ���� �ݰ濡 �÷��̾ '��'������ �˸�.
void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)	//���� �ν� �ݰ濡 ����
	{
		AMain* Main = Cast<AMain>(OtherActor); //Main �� ���ΰ� ĳ����(����ȯ) 
		if (Main)	//���� ��ü�� ���ΰ� ĳ�����̸�
		{
			MoveToTarget(Main); //Main���� �̵�
		}
	}
}

void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor) //���� �÷��̾ ����ģ�ٵ��� �ؼ� AgroSphere ������ �����ٸ�
	{
		AMain* Main = Cast<AMain>(OtherActor);
		{
			if (Main)
			{
				SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle); //NPC�� �Ͽ��� �÷��̾ �� �̻� �������� �ʰ� Idle ���·� ���ư��� ��
				if (AIController)
				{
					AIController->StopMovement(); //NPC�� �� �ϵ� ���߰� �ϴ� �Լ�.
				}
			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor) //���𰡰� CombatSphere�� ������ ��
	{
		AMain* Main = Cast<AMain>(OtherActor);
		{
			if (Main) //�� ��ģ �����ΰ��� ���ΰ� ĳ���Ͷ��
			{
				CombatTarget = Main;
				bOverlappingCombatSphere = true;
				SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking); //NPC�� ���� ���¸� ���ݻ��·� ��ȯ
			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor) //���� CombatSphere�κ��� �÷��̾ �������ٸ�(����ģ�ٵ���)
	{
		AMain* Main = Cast<AMain>(OtherActor);
		{
			if (Main) 
			{
				bOverlappingCombatSphere = false;
				//if (EnemyMovementStatus != EEnemyMovementStatus::EMS_Attacking) //������ �ϰ� �ִ� ���̶�� ������ ��� ������ �����ϵ��� �ϰ����� �װ��� �ƴ϶�� �÷��̾�� �̵�
				if (EnemyMovementStatus == EEnemyMovementStatus::EMS_Attacking)
				{
					MoveToTarget(Main);
					CombatTarget = nullptr;
				}
			}
		}
	}
}




void AEnemy::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//NPC�� ����� overlap�� �ٸ� ���͸� check�� ���̴�. ���⼭�� �翬�� �÷��̾� ĳ������ main�� �� ���̴�. enemy�� �Ǵ� weapons.cpp�����ʹ� �ٸ���.
	if (OtherActor) //���� ����� Overlap�� �ٸ� ���Ͱ� ��ȿ�� ���̶��(���⼭�� �翬�� �÷��̾� ĳ�����̴�. �׷��Ƿ� Main��������� ��Ŭ��� �ؾ� �Ѵ�.)
	{
		AMain* MainCharacter = Cast<AMain>(OtherActor); //Ȯ���� OtherActor�� MainŸ������ MainCharacter��� ������ �־��ش�.
		if (MainCharacter) //���� ���� ���� ���ΰ� ĳ���Ͷ�� �Ʒ� ����.
		{
			if (MainCharacter->HitParticles) // Particle �� �������� ���� ���¿��� �̸� �����Ϸ� �ϸ� �����Ϳ��� ũ������ �߻��Ѵ�.
			{
				const USkeletalMeshSocket* HitPoint = GetMesh()->GetSocketByName("HitPoint"); //�������Ʈ �����Ϳ��� ������ �� �κп� ���� ������ �� NPC�� ������ �� ��ȿ�� Ÿ�������� ���� ���̴�.(�̷��� �� �ϸ� ���� �� �κ��� �ƴ϶� �Ϲ������� root�� �����Ǿ� �ִ� ������ ������ �κ��� Ÿ������ �Ǵ� ������ �߻��Ѵ�.) HitPoint��� ������ �������Ʈ���� ������ ���̴�.
				if (HitPoint)
				{
					FVector SocketLocation = HitPoint->GetSocketLocation(GetMesh());
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MainCharacter->HitParticles, SocketLocation, FRotator(0.f), false);
					//HitParticle�� ��ȿ���� Ȯ�εȴٸ� Emitter�� �� ��ġ(���� Į�� �´� �ڸ�)���� Ư�� ��ƼŬ �ý����� ��Ÿ������ �� ���̴�.
					//3��° �Ű������� SocketLocation�� ��ƼŬ�ý��� Emitter�� ������ ��ġ�� �߻���ų ���̴�. 
					//������ �Ű������� ��ƼŬ �ý����� ����� ������ �ڵ����� ���������� �����ϴ� ���̴�. �ѹ��� ����� ���̹Ƿ� false�� �� ���̴�.
				}
			}
			if (MainCharacter->HitSound) //���� Ÿ���ϰ� ��ƼŬ���� ����Ǵ� �ͱ��� Ȯ���� �� HitSound ���
			{
				UGameplayStatics::PlaySound2D(this, MainCharacter->HitSound); //HitSound�� enemy������Ͽ� ������ include �����Ƿ� ���⼭�� ��� ����.(�� soundcue ��������� ���� �߰����־�� �Ѵ�. ����!)
			}

			if (MainCharacter->ScreamingSound) //���������� ScreamingSound �� ���
			{
				UGameplayStatics::PlaySound2D(this, MainCharacter->ScreamingSound);
			}
		}
	}
}

void AEnemy::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}


//�������Ʈ�� CombaMontage���� AnimNotifier�� ���� Collision�� Ȱ��ȭ �Ǵ� ��Ȱ��ȭ ��ų�� ������ ���ߴ�. 
//Weapons.cpp���� ������ ��ó�� ���� ���⿡�� �÷��̾ �浹�ϰ� ������ �ؾ� �Ѵ�. �ݸ����� Ȱ��ȭ,��Ȱ��ȭ ��Ű�� ���� �� �۾��� �ʿ��ϴ�.
void AEnemy::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // ActivateCollision�� ȣ��� ������ QueryOnly�� �浹�� �߻��Ѵ�.
}

void AEnemy::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); //����Ʈ�� � �浹�� �����Ѵ�.
}



//Ÿ������ �̵��ϵ��� �ϴ� �Լ� : �÷��̾� ĳ���Ͱ� NPC�� �ν� �ݰ濡 ���ϴ� ���� ȣ��ȴ� = AgroSphereOnOverlapBegin.
void AEnemy::MoveToTarget(class AMain* Target)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

	if (AIController)
	{
		/*UE_LOG(LogTemp, Warning, TEXT("MoveToTarget()"));*/ //������ �� Ǯ�·α׿� ����� ��
		FAIMoveRequest MoveRequest; 
		MoveRequest.SetGoalActor(Target); //GoalActor�� MoveRequest ����ü�� ����Լ���, Ÿ�� ����, �� ���⼭�� ����ĳ���Ͱ� �ȴ�.
		MoveRequest.SetAcceptanceRadius(1.0f); // �� NPC�� ��Ʈ ������Ʈ�� �÷��̾� ĳ���� ������ ��Ʈ ������Ʈ�� ���� �ʵ��� ���� ������� �Ÿ�.

		FNavPathSharedPtr NavPath;
		AIController->MoveTo(MoveRequest, &NavPath); //MoveTo�� AI�� ����� ���õ� �Լ��� �Ű������� ���� ���� ������ ����. �̴� �𸮾� ���� ���Ĺ����� �����ϵ���.
		//AI�� ĳ���Ͱ� OverlapBegin�� �Ǹ鼭 MoveToTarget�� ȣ��ǰ� MoveTo�� ����ǰ� Target�� Main, �� ���ΰ�ĳ���ͷ� �����鼭 ������� �Ǵ� ���̴�.
		
		//TArray<FNavPathPoint> PathPoints = NavPath->GetPathPoints();
		
		//AgroSphere�� ��ġ�� ���ڸ��� ǥ������.
		/*auto PathPoints = NavPath->GetPathPoints();
		for (auto Point : PathPoints)
		{
			FVector Location = Point.Location;

			UKismetSystemLibrary::DrawDebugSphere(this, Location, 30.f, 7, FLinearColor::Green, 10.f, 1.25f);
		}*/
	}
}

void AEnemy::Attack()//�� ���ݽ� ������ ����
{
	if (AIController)
	{
		AIController->StopMovement();
		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking); //AIController�� ��ȿ�ϴٸ�(�׻� ��ȿ��) ������ ����
	}
	if (!bAttacking)	//���� ���ݸ���� �����ϰ� �ִ� ���� �ƴ϶�� ���ݸ���� ������ �� ����
	{
		bAttacking = true;
	}
}

void AEnemy::AttackEnd()//�� ������ ������ ��.
{
	bAttacking = false;
}