// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapons.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"


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
	Stamina = 120.f;
	Coins = 0;	

	RunningSpeed = 600.f;
	SprintingSpeed = 1000.f;

	bLMBDown = false;
	bShiftKeyDown = false;	//쉬프트 키는 디폴트로 눌려져 있지 않은 상태이다.

	bRMBDown = false;


	//ENUM으로 만든 플레이어의 초기 상태를 나타내는 변수들 초기화(스테미너 변화 및 상태)
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;
	
	//쉬프트를 누르고 있을 때 스태미너가 감소하는 정도
	StaminaDrainRate=25.f;
	//스테미너가 감소하면서 특정지점에 달했을 때 상태바의 색깔이 변해야 한다. 그 기준점. 여기서는 최대치의 3분에 1.
	MinSprintStamina=50.f;
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay(); //게임 시작시 발생할 이벤트들

}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//스테미너의 증가 또는 감소는 매 Tick마다 일어나는 것이다. DeltaTime(1/30초)마다 캐릭터의 달림 상태에 따라 상태바가 변하는 것을 실시간으로 나타내야 하므로.

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
			//shift키가 눌려있으므로 움직임의 상태는 전력질주를 하는 상태여야 한다.
			SetMovementStatus(EMovementStatus::EMS_Sprinting);
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
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
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
}

// 들어오는 입력에 기능을 묶기 위한 작업들
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	//점프
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
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

	//키보드 화살표 이용 카메라 시점 이동
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);


}

void AMain::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f)&&(!bAttacking)) //공격하고 있을 땐 움직이지 않도록 한다.
	{
		//find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}


void AMain::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f) && (!bAttacking))
	{
		//find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
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

void AMain::LMBUp()
{
	bLMBDown = false;
}



//플레이어 캐릭터의 체력이 줄어들 때 발생할 사건을 정한다.(사망이라든지)
void AMain::DecrementHealth(float Amount) 
{
	if (Health - Amount <= 0.f)
	{
		Health -= Amount;	//체력이 줄어들게 함.
		Die();				//다 떨어지면 죽음. 
	}
	//체력이 다 떨어져 죽는 게 아니라면 그냥 체력이 깎이는 경우가 있음.
	else
	{
		Health -= Amount;
	}
}


//플레이어 캐릭터가 아이템을 얻을 때 발생할 사건의 정의(동전줍기라든지)
void AMain::IncrementCoins(int32 Amount)
{
	Coins += Amount;
}


void AMain::Die()
{

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
	if (!bAttacking)//공격 진행 시에는 마우스를 다시 클릭하더라도 그 공격이 끝나고 나서야 공격이 진행된다.
	{
		bAttacking = true;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); //AnimInstance.h를 필요로 한다.
		if (AnimInstance && CombatMontage)
		{

			/*
			플레이어에게 공격 패턴을 부여할 수 있다.(콤보 시스템 형성)
			콤보의 개수 만큼 case문을 생성.
			*/
			int32 Section = FMath::RandRange(0, 3); //RandRange의 매개변수로 들어가는 수 사이의 무작위 숫자대로 애니메이션을 실행한다. 0,1,2 하나가 나오고 그에 해당하는 case문의 애니메이션을 실행하도록 하는 것이다.
			switch (Section)
			{
			case 0:
				AnimInstance->Montage_Play(CombatMontage, 1.35f);//두 번째 인수는 애니메이션의 진행속도를 결정한다.
				AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage); //블루프린트에서 정한 에니메이션의 이름을 인수로 넣어주면 해당 에니메이션 호출 및 진행.
				break;

			case 1:
				AnimInstance->Montage_Play(CombatMontage, 1.f);//두 번째 인수는 애니메이션의 진행속도를 결정한다.
				AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage); //블루프린트에서 정한 에니메이션의 이름을 인수로 넣어주면 해당 에니메이션 호출 및 진행.
				break;

			case 2:
				AnimInstance->Montage_Play(CombatMontage, 0.7f);//두 번째 인수는 애니메이션의 진행속도를 결정한다.
				AnimInstance->Montage_JumpToSection(FName("Attack_3"), CombatMontage); //블루프린트에서 정한 에니메이션의 이름을 인수로 넣어주면 해당 에니메이션 호출 및 진행.
				break;


			case 3:
				AnimInstance->Montage_Play(CombatMontage, 0.7f);//두 번째 인수는 애니메이션의 진행속도를 결정한다.
				AnimInstance->Montage_JumpToSection(FName("Attack_4"), CombatMontage); //블루프린트에서 정한 에니메이션의 이름을 인수로 넣어주면 해당 에니메이션 호출 및 진행.
				break;

			default:
				;
			}
		}
	}
}



void AMain::AttackEnd() //공격 끝
{
	//왼쪽 마우스 버튼을 1회 클릭당 1회 공격만 가능하고 꾹 누르고 있으면 계속 공격하도록 하고 싶다면
	if (bLMBDown)
	{
		Attack();
	}
	bAttacking = false; 
	/*
	공격을 하고 나면 다시 마우스를 누르기 전까지는 공격을 해서는 안 된다.if문 아래가 아니라 위에 있다면 계속 LMB가 다운된 상태로 인식되기 때문에 마우스를 누르지 않아도 공격을 계속 진행한다.
	그래서 if문 아래에 배치해주어야 하는 것이다. 공격하고 false로 다시 바꿔지도록.
	*/
}