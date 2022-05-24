// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons.h"
#include "Enemy.h"
#include "MainPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Sound/SoundCue.h"


// Sets default values
AMain::AMain()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create Camera Boom(Pulls towards the player if there's a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 500.f;	// Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true;	//rotate arm based on controller

	//Set size for collision capsule
	GetCapsuleComponent()->SetCapsuleSize(45.f, 95.f);

	//Create Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	//Attach the Camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false;

	//Set our turn rates for input
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	//Don't rotate when the controller rotates 마우스 회전 시 캐릭터는 가만히 있도록 하는 것
	//Let that just affect the camera 카메라만 움직이게 하는 것
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	//캐릭터의 이동을 조종한다	
	GetCharacterMovement()->bOrientRotationToMovement = true;	//캐릭터가 입력된 방향을 바라보며 움직이도록 한다.
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 640.f, 0.0f); //괄호 안의 수만큼의 rate로 움직이도록 함. 높을 수록 빨리 회전
	GetCharacterMovement()->JumpZVelocity = 400.f; //높을 수록 캐릭터가 높게 뜀
	GetCharacterMovement()->AirControl = 0.2f;	//공중에서 사용자가 캐릭터를 제어할 수 있는 정도. 높을 수록 잘 됨.



	//ENUM으로 만든 플레이어의 초기 상태를 나타내는 변수들 초기화(체력과 스테미나).
	MaxHealth = 100.f;	
	Health = 75.f;
	MaxStamina = 150.f;
	Stamina = 150.f;
	Coins = 0;	

	RunningSpeed = 600.f;	//플레이어 이동 속도
	SprintingSpeed = 1000.f; //전력질주 속도

	bLMBDown = false;
	bShiftKeyDown = false;	//쉬프트 키는 디폴트로 눌려져 있지 않은 상태이다.

	bRMBDown = false;
	bPlayerMoving = false;

	InterpSpeed = 10.f;
	bInterpToEnemy = false;

	//ENUM으로 만든 플레이어의 초기 상태를 나타내는 변수들 초기화(스테미너 변화 및 상태)
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;
	
	//쉬프트를 누르고 있을 때 스태미너가 감소하는 정도
	StaminaDrainRate=25.f;
	//스테미너가 감소하면서 특정지점에 달했을 때 상태바의 색깔이 변해야 한다. 그 기준점. 여기서는 최대치의 3분에 1.
	MinSprintStamina=50.f;


	bHasCombatTarget = false;

	bMovingForward = false;
	bMovingRight = false;


}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay(); //게임 시작시 발생할 이벤트들

	MainPlayerController = Cast<AMainPlayerController>(GetController()); // 화면에 표시할 필요가 있을 때마다 표시를 돕기 위함
}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	/*
	적을 향해 초점을 맞추는 것은 매 틱마다 이루어져야 한다. 
	매 프레임마다 캐릭터의 회전이 부드럽게 진행되어야 하기 때문.
	캐릭터의 회전을 포함하고 있으므로 rotate 또한 사용된다.
	*/
	if (bInterpToEnemy && CombatTarget) //적의 전투 반경과 플레이어가 오버랩되어 bInterpToEnemy가 true가 되고 전투의 대상이 될 적이 유효한 경우에만 아래 과정을 실행.
	{
		//LookAtYaw는 플레이어캐릭터가 초점을 맞춰야 할 위치로, 적이 있는 방향이다. Target의 위치를 구해 매개변수로 준다.
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation()); 
		//그러나 적을 향해 매 틱마다 서서히 방향을 전환해 초점을 맞춰야 한다. 이 과정이 없으면 1/30초만에 적에게 초점을 맞출 것이다.
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);

		SetActorRotation(InterpRotation); //플레이어의 초점을 적에게 맞춤.
	}
	



	Super::Tick(DeltaTime);
	//스테미너의 증가 또는 감소는 매 Tick마다 일어나는 것이다. DeltaTime(1/30초)마다 캐릭터의 달림 상태에 따라 상태바가 변하는 것을 실시간으로 나타내야 하므로.

	if (MovementStatus == EMovementStatus::EMS_Dead) return; //매 틱마다 플레이어가 죽은 상태인지 확인, 죽었으면 아무것도 실행 못함.


	//DeltaStamina는 DeltaTime마다 증가 또는 감소해야 하는 스테미너의 변화값을 의미한다.
	float DeltaStamina = StaminaDrainRate * DeltaTime;

	/*스태미너의 값이 특정범위내이면 스테미너의 상태바가 변하고 특정 이벤트(달리기 멈춤 등)가 실행되도록 해야 한다.
	switch문을 여기에 활용할 수 있다.*/
	/*플레이어가 shift키를 누르고 있을 때 캐릭터의 동작은 전력질주하는 상태가 되어야 하지만 항상 그런 것은 아니다.
	Stamina가 특정 범위에 있으면 상태바가 변화해야 하고 만약 0이 되면 전력질주하고 싶어도 그렇게 하지 못한다.
	이런 상황들을 각각 고려해서 switch문을 만든다.
	*/
	switch (StaminaStatus)
	{
	case EStaminaStatus::ESS_Normal:
		if (bShiftKeyDown) //shift키가 눌려있다면 
		{
			if (Stamina - DeltaStamina <= MinSprintStamina) //이게 true가 되면 스태미너가 minsprintstamina를 넘어선 순간이다. BelowMinimum 상태로 전환
			{
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum); //스태미너의 상태를 BelowMinimum 상태로 전환.
				Stamina -= DeltaStamina; //스태미너 상태는 BelowMinimum을 넘어섰지만 아직 0은 아니므로 계속 진행.
			}
			else //스태미너가 아직 MinSprintStamina를 넘어서지 않음. 여전히 Normal State에 있으며, BelowMinimum 상태로 전환하지 않는다.
			{
				Stamina -= DeltaStamina;
			}
			//shift키가 눌려있으므로 움직임의 상태는 전력질주를 하는 상태여야 한다. 그러나 오른쪽 혹은 좌우로 움직이고 있는 상황이어야 전력질주가 활성화 된다.
			if (bMovingForward || bMovingRight) SetMovementStatus(EMovementStatus::EMS_Sprinting);
			else SetMovementStatus(EMovementStatus::EMS_Normal); //그렇지 않으면 일반 달리기
		}
		else //shift키가 눌려있지 않은 상태이다. 스태미너가 줄어들어있다면 회복해야 하고 가득 차있는 상태이면 그 상태를 유지하도록 해야 한다.
		{
			if (Stamina + DeltaStamina >= MaxStamina) //스태미너의 상태가 가득 찬 상태라면 
			{
				Stamina = MaxStamina;	// 스태미너는 Max인 상태로, 더이상 증가하지 않는다.
			}
			else //스태미너가 가득 차있지 않은 상태라면 
			{
				Stamina += DeltaStamina; //쉬거나 그냥 달리는 상태이므로 매tick 당 스태미너를 증가시켜야 한다.
			}
			//Shift키가 눌려있지 않은 상태이므로 움직임의 상태는 그냥 달리기를 하는 상태여야 한다.
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break; //여기까지가 스태미너가 Normal 상태. BelowMinimum이거나 고갈된 상태로 넘어간 것이 아니라면, 다음 단계로 넘어가지 않도록 break. 
	
	
	case EStaminaStatus::ESS_BelowMinimum: // 스태미너가 감소해 MinSprintStamina이하로 떨어졌을 경우이다.
		if (bShiftKeyDown) //shift키 눌렀을 때
		{
			if (Stamina - DeltaStamina <= 0.f) //스태머너가 감소해 0이 된다면
			{
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted); //움직임 상태는 그냥 달리는 상태가 되고 스태미너는 0으로 고정한다.
				Stamina = 0;
				SetMovementStatus(EMovementStatus::EMS_Normal); //스태미너 고갈됐으므로 전력질주 멈추고 그냥 뛰기 시작.
			}
			else
			{
				Stamina -= DeltaStamina; //스태미너 고갈이 아니면 아직은 뛸 수 있으므로 계속 스태미나 감소하며 전력질주.
				if (bMovingForward || bMovingRight) SetMovementStatus(EMovementStatus::EMS_Sprinting);
				else SetMovementStatus(EMovementStatus::EMS_Normal);
				 /*SetMovementStatus(EMovementStatus::EMS_Sprinting);*/
			}
		}
		else //shift키 안 눌려 있을 때
		{
			if (Stamina + DeltaStamina >= MinSprintStamina) //스태미너가 MinSprintStamina을 넘으면 Normal상태이기 때문에 그 상태로 전환해줌.
			{
				SetStaminaStatus(EStaminaStatus::ESS_Normal); //상태전환
				Stamina += DeltaStamina; //쉬거나 그냥 달리는 상태이고 스테미너가 가득 차있지 않기 때문에 매tick 당 스태미너 회복
			}
			else
			{
				Stamina += DeltaStamina; //쉬거나 그냥 달리는 상태이고 스테미너가 가득 차있지 않기 때문에 매tick 당 스태미너 회복
			}
			SetMovementStatus(EMovementStatus::EMS_Normal); // 어떤 상태이든 shift키는 눌려져 있지 않으므로 그냥 달리는 상태다.
		}
		break;
	
	
	case EStaminaStatus::ESS_Exhausted: //스태미너 다 떨어졌을 때 
		if (bShiftKeyDown) 
		{
			Stamina = 0.f; 
			/*shift키를 눌러도 스태미너가 고갈된 상태이기 때문에 움직임의 상태에 아무 변화도 일어나지 않고 스태미너의 상태는 계속 0으로 초기화된다.
			shift키를 누르지 않아야 비로소 충전이 시작되게 한다.*/
		}
		else
		{
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering); 
			/*shift키를 누르지 않은 상태로, 스태미너는 계속 회복된다.
			회복 상태에서는 상태바의 색깔이 기본 하얀색인 상태에서 노란색으로 바뀌도록 한다.*/
		}
		SetMovementStatus(EMovementStatus::EMS_Normal); //아무튼 지친 상태이므로 전력질주는 못한다. 그냥 달리기만 됨.
		break;
	

	case EStaminaStatus::ESS_ExhaustedRecovering: 
		/*체력 회복 중인 상태이다.
		이 상태에서는 스테미너 상태바 색깔을 변경하고 회복 중에 normal 상태로 복귀하는지를 판별하여 또 상태바 색깔을 변경시킨다.*/
		if (Stamina + DeltaStamina >= MinSprintStamina)
		{
			SetStaminaStatus(EStaminaStatus::ESS_Normal); //스태미너가 MinSprintStamina를 넘기면 다시 Normal상태로 회기한 것으로 판별한다.
			Stamina += DeltaStamina;
		}
		else
		{
			Stamina += DeltaStamina; //아니면 계속 스태미너를 회복한다.
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);//이 동안은 그냥 달리기만 됨.
		break;
	
	default:
		;
	}



	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation(); //적 객체의 위치를 매 틱마다 찾아 저장해두도록 한다.
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation; //매 틱마다 구해지는 적 위치를 상태바 위치에 활용할 수 있도록 한다.
		}
	}
}


// 플레이어가 적에게 초점을 맞추려면 그에 어느 방향으로 무엇을 향해 회전을 할 것인지 알려주어야 한다. 좌우로만 회전하므로 회전 방향은 Yaw이다.
FRotator AMain::GetLookAtRotationYaw(FVector Target)
{

	/*
	KismetMathLibrary의 FindLookAtRotation 함수는 시작지점(플레이어의 위치)에서 타겟이 위치한 곳의 회전값을 찾아준다. 
	적의 위치를 알고 그 방향으로 돌게 하는 데 필요함.  
	FindLookAtRotation 함수의 첫번째 매개변수는 플레이어의 위치, 두 번째 매개변수는 타겟 즉 적의 위치
	*/
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw; //결과적으로 플레이어가 쳐다보아야 할 회전값(방향)을 반환
}




// 들어오는 입력에 기능을 묶기 위한 작업들
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	//점프
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//전력질주
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);	//키 바인딩 작업1 : shift를 누르면 pressed된 것으로 됨.
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::ShiftKeyUp); // 키 바인딩 작업2 : shift를 떼면 released 된 것

	//전진
	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);

	//장비 장착 및 마우스 액션 중 공격
	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown);	
	PlayerInputComponent->BindAction("LMBt", IE_Released, this, &AMain::LMBUp); 

	//마우스 액션 중 강공격
	PlayerInputComponent->BindAction("RMB", IE_Pressed, this, &AMain::RMBDown);
	PlayerInputComponent->BindAction("RMBt", IE_Released, this, &AMain::RMBUp);

	//키보드 화살표 이용 카메라 시점 이동
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);


}

void AMain::MoveForward(float Value)
{

	bMovingForward = false; 
	if ((Controller != nullptr) && (Value != 0.0f)&&(!bAttacking)&&(!bStrongAttacking) && (MovementStatus != EMovementStatus::EMS_Dead)) //공격하고 있을 때이거나 죽어있을 땐 움직이지 않도록 한다. 
	{
		//find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
		
		bMovingForward = true; 
		//위의 조건을 모두 만족하고 움직일 때에만 true가 된다. 
		//왜? : Tick 함수에서 매번 확인하고, 이것이 false가 되면 shift 키를 눌러도 플레이어가 제자리에서 달리는 모션을 하지 않도록 하기 위함이다.
		//MoveRight에서도 마찬가지이다.
	}
}


void AMain::MoveRight(float Value)
{
	bMovingRight = false;
	if ((Controller != nullptr) && (Value != 0.0f) && (!bAttacking) && (!bStrongAttacking) && (MovementStatus != EMovementStatus::EMS_Dead))
	{
		//find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
		
		bMovingRight = true;
	}
}



void AMain::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMain::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

//왼쪽 마우스가 눌려 있는지 여부를 확인하는 함수들이다.
void AMain::LMBDown()
{
	bLMBDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead) return; // 죽은 상태라면 함수 실행 불가

	if (ActiveOverlappingItem) // 장비와 겹침상태임을 나타내는 변수가 true라면 그 장비가 무기타입인지 확인한다. 맞다면 장비를 장착하게 한다.
	{
		AWeapons* Weapons = Cast<AWeapons>(ActiveOverlappingItem);
		if (Weapons)
		{
			Weapons->Equip(this);
			SetActiveOverlappingItem(nullptr); //장비를 장착하고 나면 다시 원래대로 돌려놓는다.
		}
	}
	else if (WeaponEquipped)// 장비와 겹치지 않는다면 장비를 이비 착용하고 있는지 확인
	{
		Attack(); //이 함수의 리턴값이 true가 되면 플레이어가 공격 수행이 갸능한 것.
		//공격 수행 시 해당 애니메이션이 진행되도록 한다.
	}
}

void AMain::RMBDown()
{
	bRMBDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead) return; // 죽은 상태라면 함수 실행 불가

	if (WeaponEquipped)// 장비와 겹치지 않는다면 장비를 이비 착용하고 있는지 확인
	{
		StrongAttack(); //이 함수의 리턴값이 true가 되면 플레이어가 공격 수행이 갸능한 것.
		//공격 수행 시 해당 애니메이션이 진행되도록 한다.
	}
}

void AMain::LMBUp()
{
	bLMBDown = false;
}

void AMain::RMBUp()
{
	bRMBDown = false;
}


//플레이어 캐릭터의 체력이 줄어들 때 발생할 사건을 정한다.(사망이라든지)
//void AMain::DecrementHealth(float Amount) //Amount는 적이나 장애물로부터 받는 피해량이다. 이 피해량 정도에 따라 플레이어의 hit 모션을 다르게 할 수 있다.
//{
//	if (Health - Amount <= 0.f)
//	{
//		Health -= Amount;	//체력이 줄어들게 함.
//		Die();				//다 떨어지면 죽음. 
//	}
//	//체력이 다 떨어져 죽는 게 아니라면 그냥 체력이 깎이는 경우가 있음.
//	else
//	{
//		Health -= Amount;
//	}
//}


//플레이어 캐릭터가 아이템을 얻을 때 발생할 사건의 정의(동전줍기라든지)
void AMain::IncrementCoins(int32 Amount)
{
	Coins += Amount;
}


//플레이어 체력이 0이하가 되었을 때 수행할 함수. 죽음 상태.
//공격 함수와 마찬가지로 죽을 때의 애니메이션 실행을 따로 만들고 불러와야 한다.
void AMain::Die()
{	
	if (MovementStatus == EMovementStatus::EMS_Dead) return; //이미 죽어있는 상태이면 두번 다시 죽음 애니메이션을 실행하지 않도록 한다. 이것이 없을 경우 죽음 이후에 적에게 공격받으면 또 이 애니메이션을 실행한다.
	int32 DeathAnimNum = FMath::RandRange(0, 2);
	UAnimInstance* DeathAnimInstance = GetMesh()->GetAnimInstance(); //AnimInstance.h를 필요로 한다. 
	if (DeathAnimInstance && CombatMontage)	//언리얼 에디터의 Main과 연결되어 있는 CombatMontage 를 불러옴. (DeathAnim이 CombatMontage와 같다면..)
	{
		DeathAnimInstance->Montage_Play(CombatMontage, 1.0f);
		if (DeathAnimNum == 0) DeathAnimInstance->Montage_JumpToSection(FName("Death_1"));
		else if (DeathAnimNum == 1) DeathAnimInstance->Montage_JumpToSection(FName("Death_2"));
		else DeathAnimInstance->Montage_JumpToSection(FName("Death_3"));
	}
	SetMovementStatus(EMovementStatus::EMS_Dead);
	UGameplayStatics::PlaySound2D(this, this->DeathSound);
}


void AMain::DeathEnd()
{
	// 애니메이션의 특정 상태에서 멈춰있도록 하는 기능은 Mesh에 있다. bPauseAnims가 그것.
	UE_LOG(LogTemp, Warning, TEXT("Death End Executed"))
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
	 /*위 두 함수는 적이 죽으면 애니메이션의 특정상태에서 메시가 멈춰있도록 하는 기능을 가진다.
	 플레이어가 죽으면 이 함수가 호출됨으로써 그 상태로 고정한다.*/

}

//Jump는 character.h에 이미 있는 함수로, main에서 상속받아 활용한다.
void AMain::Jump()
{
	if (MovementStatus != EMovementStatus::EMS_Dead) //플레이어가 살아있는 상태에서만 jump가 가능하다.
	{
		Super::Jump();
	}
}

/*캐릭터 이동 상태를 확인하는 함수*/
void AMain::SetMovementStatus(EMovementStatus Status) 
{
	MovementStatus = Status;	//매개변수로 들어온 Status가 초기 이동속도이다.
	if (MovementStatus == EMovementStatus::EMS_Sprinting) //만약 이동속도가 전력질주 속도로 달라진다면 
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed; //캐릭터의 이동은 전력질주 상태로 바뀐다. 
		//MaxWalkSpeed는 GetCharacterMovement에 있는 변수이다. 플레이어의 이동 상태를 나타내는 변수로, 선언한 SprintingSpeed 또는 RunningSpeed로 값을 조정할 수 있다.
	}
	else //만약 전력질주 하는 것이 아니라면 그냥 일반 달리기를 하는 것
	{
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}


//아래 두 함수가 하는 일 : 쉬프트 키가 눌려져 있는지 아닌지만을 전달한다.
void AMain::ShiftKeyDown()
{
	bShiftKeyDown = true;
}

void AMain::ShiftKeyUp()
{
	bShiftKeyDown = false;
}

void AMain::ShowPickupLocations()
{
	for (int32 i = 0; i < PickupLocations.Num(); i++) //아이템 획득 수 만큼 반복하여 획득 장소를 표시할 것이다.
	{
		/*레벨에서 캐릭터가 어디에 있는지 나타내는 것은 일종의 디버그이다. KismetSystem을 활용해 월드에서 캐릭터가 어디에 있는지 나타내는 기능을 만들 수 있다.*/
		/*UKismetSystemLibrary::DrawDebugSphere(this, GetActorLocation() + FVector(0, 0, 100.f), 25.f, 5, FLinearColor::Green, 10.f, .25f);*/
		/*
		Fvector의 매개변수로 넣는 것은 각각 캐릭터의 x,y,z 벡터값에 더할 값들이다. 캐릭터의 머리위(z방향)로 캐릭터 위치를 표시하는 구(sphere)를 만들어 줄 것이다. 나머지 매개변수는 DrawDebugSphere의 정의를 peeking해서 알 수 있다.
		예를 들어, 마지막 매개변수는 구의 굵기이고 마지막에서 두 번째 매개변수는 그 구가 몇 초간 나타날 것인지를 나타낸다. 여기서는 10초를 의미. Green은 구의 색깔, 5는 세그먼트 수.
		이를 이용해서 게임 시작시 사용자에게 조작법을 알려주는 디버깅 툴을 만들 수 있을 것이다.
		*/

		UKismetSystemLibrary::DrawDebugSphere(this, PickupLocations[i], 30.f, 7, FLinearColor::Green, 10.f, .25f);
		/*PickupLocations가 플레이어가 아이템을 획득한 장소이므로 굳이 "GetActorLocation() + FVector(0, 0, 100.f)"를 쓰는 것보다 PickupLocations배열의 인덱스를 사용하는 것이 낫다.*/
	}

	/*PickupLocations를 이용해 위 for문을 좀더 간단하게 만들 수는 있다. range based 반복문*/
	/*
	for (FVector Location : PickupLocations) //배열 전체를 순회하여 각 인덱스의 벡터정보를 꺼낸다.
	{
		UKismetSystemLibrary::DrawDebugSphere(this, Location, 30.f, 7, FLinearColor::Green, 10.f, .25f);
		//차이 : PickupLocation 대신에 그냥 Location 변수를 넣는다. 만약 획득한 아이템 전체를 표시하고 싶다면 유용한 방법. 그렇지 않고 몇개만 표시하고 싶다면 위의 방법이 더 적절한 방법.

	}
	*/
}

//새로운 장비를 장착할 때, 기존에 장착하고 있던 장비가 있으면 완전삭제한다.
void AMain::SetWeaponEquipped(AWeapons* WeaponToSet)
{
	if (WeaponEquipped)//장비를 장착하고 있다면 완전삭제
	{
		WeaponEquipped->Destroy();
	}
	WeaponEquipped = WeaponToSet; //장착하고 있는 장비가 없었다면 새로 줍는 장비로 장착
}

void AMain::Attack() //공격 진행 여부를 판단하고 해당 에니메이션을 불러와 진행한다.
{
	//if (!bAttacking && !bStrongAttacking)//공격 진행 시에는 마우스를 다시 클릭하더라도 그 공격이 끝나고 나서야 공격이 진행된다.
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead) // 보다 빠른 액션 전환을 원하면 조건 하나를 없애는 게 나을 수도 있다.(타격은 없을 수도) 또한 플레이어 상태가 죽음 상태가 아니어야 공격을 진행할 수 있다.
	{
		SetInterpToEnemy(true);//공격을 시작함과 동시에 적에게 초점을 맞추도록 한다.
		bAttacking = true;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); //AnimInstance.h를 필요로 한다.
		if (AnimInstance && CombatMontage)
		{

			/*
			플레이어에게 공격 패턴을 부여할 수 있다.(콤보 시스템 형성)
			콤보의 개수 만큼 case문을 생성.
			*/
			int32 Section = FMath::RandRange(0, 1); //RandRange의 매개변수로 들어가는 수 사이의 무작위 숫자대로 애니메이션을 실행한다. 0,1,2 하나가 나오고 그에 해당하는 case문의 애니메이션을 실행하도록 하는 것이다.
			
			//if (bPlayerMoving) Section = 1;
			if (bShiftKeyDown) Section = 2;
			

			switch (Section)
			{
			case 0:
				AnimInstance->Montage_Play(CombatMontage, 0.9f);//두 번째 인수는 애니메이션의 진행속도를 결정한다.
				AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage); //블루프린트에서 정한 에니메이션의 이름을 인수로 넣어주면 해당 에니메이션 호출 및 진행.
				break;

			case 1:
				AnimInstance->Montage_Play(CombatMontage, 0.9f);//두 번째 인수는 애니메이션의 진행속도를 결정한다.
				AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage); //블루프린트에서 정한 에니메이션의 이름을 인수로 넣어주면 해당 에니메이션 호출 및 진행.
				break;

			case 2:
				AnimInstance->Montage_Play(CombatMontage, 0.7f);//두 번째 인수는 애니메이션의 진행속도를 결정한다.
				AnimInstance->Montage_JumpToSection(FName("Attack_3"), CombatMontage); //블루프린트에서 정한 에니메이션의 이름을 인수로 넣어주면 해당 에니메이션 호출 및 진행.
				break;


			default:
				;
			}
		}
		//if (WeaponEquipped->SwingSound) //무기를 장비하고 있는 상태라면 무기를 휘두를 때마다 휘두르는 소리가 나도록 한다.
		//{
		//	//UGameplayStatics::PlaySound2D(this, WeaponEquipped->SwingSound);//이러한 sound 를 사용하려면 그에 맞는 헤더 파일을 부를 수 있어야 한다.
		//}
	}
}



void AMain::StrongAttack() //공격 진행 여부를 판단하고 해당 에니메이션을 불러와 진행한다.
{
	float WieldingSpeed1 = 1.15f, WieldingSpeed2 = 1.1f; //무기를 휘두르는 속도, 즉 애니메이션 진행 속도
	if (!bStrongAttacking)//공격 진행 시에는 마우스를 다시 클릭하더라도 그 공격이 끝나고 나서야 공격이 진행된다. *약공격 중에는 강공격으로 전환이 가능하나 그 반대는 안 된다.
	{
		SetInterpToEnemy(true);//공격을 시작함과 동시에 적에게 초점을 맞추도록 한다.
		bStrongAttacking = true;


		UAnimInstance* AnimInstance2 = GetMesh()->GetAnimInstance(); //AnimInstance.h를 필요로 한다.
		if (AnimInstance2 && CombatMontage_StrongAtck)
		{

			/*
			플레이어에게 공격 패턴을 부여할 수 있다.(콤보 시스템 형성)
			콤보의 개수 만큼 case문을 생성.
			*/
			int32 Section = FMath::RandRange(0, 1); //RandRange의 매개변수로 들어가는 수 사이의 무작위 숫자대로 애니메이션을 실행한다. 0,1,2 하나가 나오고 그에 해당하는 case문의 애니메이션을 실행하도록 하는 것이다.
			Section = 1;
			if (bShiftKeyDown) Section = 0;
			switch (Section)
			{
			case 0:
				if (Stamina <= 50) WieldingSpeed1 = 0.5f;//플레이어 스테미너가 50 이하라면 강공격 속도가 느려지도록 한다. (아니면 이동 속도 변수를 건드려 이동속도가 느려지도록 할 수도 있다.)

				AnimInstance2->Montage_Play(CombatMontage_StrongAtck, WieldingSpeed1);//두 번째 인수는 애니메이션의 진행속도를 결정한다.
				AnimInstance2->Montage_JumpToSection(FName("Attack_Heavy1"), CombatMontage_StrongAtck); //블루프린트에서 정한 에니메이션의 이름을 인수로 넣어주면 해당 에니메이션 호출 및 진행.
				Stamina -= 30.f; //강공격 시 스태미너 30 소모

				break;

			case 1:
				if (Stamina <= 50) WieldingSpeed2 = 0.5f;

				AnimInstance2->Montage_Play(CombatMontage_StrongAtck, WieldingSpeed2);//두 번째 인수는 애니메이션의 진행속도를 결정한다.
				AnimInstance2->Montage_JumpToSection(FName("Attack_Heavy2"), CombatMontage_StrongAtck); //블루프린트에서 정한 에니메이션의 이름을 인수로 넣어주면 해당 에니메이션 호출 및 진행.
				Stamina -= 30.f; //강공격 시 스태미너 30 소모
				break;


			default:
				;
			}
		}
		//if (WeaponEquipped->SwingSound) //무기를 장비하고 있는 상태라면 무기를 휘두를 때마다 휘두르는 소리가 나도록 한다.
		//{
		//	//UGameplayStatics::PlaySound2D(this, WeaponEquipped->SwingSound);//이러한 sound 를 사용하려면 그에 맞는 헤더 파일을 부를 수 있어야 한다.
		//}
	}
}


void AMain::StrongAttackEnd() //공격 끝
{
	if (bRMBDown)
	{
		StrongAttack();
	}
	//공격중이 아니라면 적에게 초점 맞추는 것을 중단.
	SetInterpToEnemy(false);
	bStrongAttacking = false;
	bAttacking = false;
}


void AMain::AttackEnd() //공격 끝
{
	//왼쪽 마우스 버튼을 1회 클릭당 1회 공격만 가능하고 꾹 누르고 있으면 계속 공격하도록 하고 싶다면
	if (bLMBDown)
	{
		Attack();
	}
	bAttacking = false;
	//공격중이 아니라면 적에게 초점 맞추는 것을 중단.
	SetInterpToEnemy(false);
	bStrongAttacking = false;
	/*
	공격을 하고 나면 다시 마우스를 누르기 전까지는 공격을 해서는 안 된다.if문 아래가 아니라 위에 있다면 계속 LMB가 다운된 상태로 인식되기 때문에 마우스를 누르지 않아도 공격을 계속 진행한다.
	그래서 if문 아래에 배치해주어야 하는 것이다. 공격하고 false로 다시 바꿔지도록.
	*/

}

//TakeDamage함수 : 플레이어가 받는 데미지량과 플레이어에게 데미지를 적용하는 함수 등 데미지 관련 기능들이 이 함수에 내포된다.
float AMain::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) 
{
	//DecrementHealth(DamageAmount);//Health 변수에 영향을 주는 DecrementHealth 함수를 호출. 데미지량을 매개변수로 받음. 이 함수를 써도 되지만 여기서는 추가로 DamageCauser를 구현하기 위해 풀어서 쓴다.

	if (Health - DamageAmount <= 0.f)
	{
		Health -= DamageAmount;	//체력이 줄어들게 함.
		Die();				//다 떨어지면 죽음. 


		//DamageCauser은 데미지의 원인이 되는 것, 즉 여기서는 적 NPC이다.
		if (DamageCauser)
		{
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);
			if (Enemy)
			{
				Enemy->bHasValidTarget = false;
			}
		}
	}
	//체력이 다 떨어져 죽는 게 아니라면 그냥 체력이 깎이는 경우가 있음.
	else
	{
		int32 HitAnimNum = FMath::RandRange(0, 2);
		UAnimInstance* HitAnimInstance = GetMesh()->GetAnimInstance(); //AnimInstance.h를 필요로 한다. 
		if (DamageAmount >= 100) HitAnimNum = 3; //200이상의 데미지를 받으면 뒤로 튕겨져 가는 HitHard 애니메이션 재생
		else if (DamageAmount < 50)
		{
			Health -= DamageAmount;
			return 0; // 받은 데미지가 50이하이면 아무런 반응 안 함.
		}
		if (HitAnimInstance && CombatMontage)	//언리얼 에디터의 Main과 연결되어 있는 CombatMontage 를 불러옴. (DeathAnim이 CombatMontage와 같다면..)
		{

			UGameplayStatics::PlaySound2D(this, this->SmashingSound);	//비명과 맞는 소리
			UGameplayStatics::PlaySound2D(this, this->PainSound);

			HitAnimInstance->Montage_Play(CombatMontage, 1.5f);
			if (HitAnimNum == 0) HitAnimInstance->Montage_JumpToSection(FName("Hit1"));
			else if (HitAnimNum == 1) HitAnimInstance->Montage_JumpToSection(FName("Hit2"));
			else if (HitAnimNum == 2)HitAnimInstance->Montage_JumpToSection(FName("Hit3"));
			else if (HitAnimNum == 3)HitAnimInstance->Montage_JumpToSection(FName("HitHard"));

			bStrongAttacking = false;	//Hit 된 후 공격 중지하는 에러 방지용 코드
			bAttacking = false;			//Hit 된 후 공격 중지하는 에러 방지용 코드
			SetInterpToEnemy(false);	//Hit 된 후 공격 중지하는 에러 방지용 코드
		}
		Health -= DamageAmount;
	}



	return DamageAmount; // 받게 되는 데미지량을 반환

}

//ApplyDamage함수 : 플레이어에게 데미지를 적용시키는 함수. 데미지를 받는 액터와 데미지량, 데미지를 주는 도구와 액터, UDamageType이라는 특별한 클래스를 통해 어떤 종류의 데미지인지를 골라 매개변수로 넘긴다.
//적이 공격을 가한 뒤 플레이어에게 데미지를 줄 것이므로 enemy.cpp에서 적 공격 시행 후 적용하도록 한다.



void AMain::SwingingSound() 
{
	if(WeaponEquipped) UGameplayStatics::PlaySound2D(this, WeaponEquipped->SwingSound); //무기를 장비하고 있는 상태라면 무기를 휘두를 때마다 휘두르는 소리가 나도록 한다.
}


void AMain::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}


void AMain::RefreshTarget() //타겟 새로 설정 : 메인 캐릭터와 오버랩하는 모든 액터를 타겟으로 설정. (액터 중 Enemy만을 타겟으로 설정할 것)
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, TargetingActor);//겹치는 액터 중 두 번째 매개변수로 들어간 클래스의 객체인 액터만 제외하고 모두 필터링 한다. 여기서 Enemy 클래스만 찾아내도록 하는 것.
	
	//캐릭터와 가장 가까운 적을 타겟으로 정할 것이다. 캐릭터와 적 사이 거리를 계산해야 함.


	if (OverlappingActors.Num() == 0) return;	//플레이어와 오버랩하는 적이 유효해야 한다.(OverlappingActors 배열에 

}