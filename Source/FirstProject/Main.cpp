// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
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
#include "SaveGameClass.h"
#include "ItemStorage.h"

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

	//Don't rotate when the controller rotates ���콺 ȸ�� �� ĳ���ʹ� ������ �ֵ��� �ϴ� ��
	//Let that just affect the camera ī�޶� �����̰� �ϴ� ��
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	//ĳ������ �̵��� �����Ѵ�	
	GetCharacterMovement()->bOrientRotationToMovement = true;	//ĳ���Ͱ� �Էµ� ������ �ٶ󺸸� �����̵��� �Ѵ�.
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 640.f, 0.0f); //��ȣ ���� ����ŭ�� rate�� �����̵��� ��. ���� ���� ���� ȸ��
	GetCharacterMovement()->JumpZVelocity = 400.f; //���� ���� ĳ���Ͱ� ���� ��
	GetCharacterMovement()->AirControl = 0.2f;	//���߿��� ����ڰ� ĳ���͸� ������ �� �ִ� ����. ���� ���� �� ��.



	//ENUM���� ���� �÷��̾��� �ʱ� ���¸� ��Ÿ���� ������ �ʱ�ȭ(ü�°� ���׹̳�).
	MaxHealth = 100.f;	
	Health = 75.f;
	MaxStamina = 150.f;
	Stamina = 150.f;
	Coins = 0;	

	RunningSpeed = 600.f;	//�÷��̾� �̵� �ӵ�
	SprintingSpeed = 1000.f; //�������� �ӵ�

	bLMBDown = false;
	bShiftKeyDown = false;	//����Ʈ Ű�� ����Ʈ�� ������ ���� ���� �����̴�.

	bRMBDown = false;
	bPlayerMoving = false;

	InterpSpeed = 10.f;
	bInterpToEnemy = false;

	//ENUM���� ���� �÷��̾��� �ʱ� ���¸� ��Ÿ���� ������ �ʱ�ȭ(���׹̳� ��ȭ �� ����)
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;
	
	//����Ʈ�� ������ ���� �� ���¹̳ʰ� �����ϴ� ����
	StaminaDrainRate=25.f;
	//���׹̳ʰ� �����ϸ鼭 Ư�������� ������ �� ���¹��� ������ ���ؾ� �Ѵ�. �� ������. ���⼭�� �ִ�ġ�� 3�п� 1.
	MinSprintStamina=50.f;


	bHasCombatTarget = false;

	bMovingForward = false;
	bMovingRight = false;

	bESCDown = false;

	AttackStandard = 0;

}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay(); //���� ���۽� �߻��� �̺�Ʈ��

	MainPlayerController = Cast<AMainPlayerController>(GetController()); // ȭ�鿡 ǥ���� �ʿ䰡 ���� ������ ǥ�ø� ���� ����

	/*LoadGameNoSwitch();*/
	
	if (MainPlayerController)
	{
		MainPlayerController->GameModeOnly();
	}
}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	/*
	���� ���� ������ ���ߴ� ���� �� ƽ���� �̷������ �Ѵ�. 
	�� �����Ӹ��� ĳ������ ȸ���� �ε巴�� ����Ǿ�� �ϱ� ����.
	ĳ������ ȸ���� �����ϰ� �����Ƿ� rotate ���� ���ȴ�.
	*/
	if (bInterpToEnemy && CombatTarget) //���� ���� �ݰ�� �÷��̾ �������Ǿ� bInterpToEnemy�� true�� �ǰ� ������ ����� �� ���� ��ȿ�� ��쿡�� �Ʒ� ������ ����.
	{
		//LookAtYaw�� �÷��̾�ĳ���Ͱ� ������ ����� �� ��ġ��, ���� �ִ� �����̴�. Target�� ��ġ�� ���� �Ű������� �ش�.
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation()); 
		//�׷��� ���� ���� �� ƽ���� ������ ������ ��ȯ�� ������ ����� �Ѵ�. �� ������ ������ 1/30�ʸ��� ������ ������ ���� ���̴�.
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);

		SetActorRotation(InterpRotation); //�÷��̾��� ������ ������ ����.
	}
	



	Super::Tick(DeltaTime);
	//���׹̳��� ���� �Ǵ� ���Ҵ� �� Tick���� �Ͼ�� ���̴�. DeltaTime(1/30��)���� ĳ������ �޸� ���¿� ���� ���¹ٰ� ���ϴ� ���� �ǽð����� ��Ÿ���� �ϹǷ�.

	if (MovementStatus == EMovementStatus::EMS_Dead) return; //�� ƽ���� �÷��̾ ���� �������� Ȯ��, �׾����� �ƹ��͵� ���� ����.


	//DeltaStamina�� DeltaTime���� ���� �Ǵ� �����ؾ� �ϴ� ���׹̳��� ��ȭ���� �ǹ��Ѵ�.
	float DeltaStamina = StaminaDrainRate * DeltaTime;

	/*���¹̳��� ���� Ư���������̸� ���׹̳��� ���¹ٰ� ���ϰ� Ư�� �̺�Ʈ(�޸��� ���� ��)�� ����ǵ��� �ؾ� �Ѵ�.
	switch���� ���⿡ Ȱ���� �� �ִ�.*/
	/*�÷��̾ shiftŰ�� ������ ���� �� ĳ������ ������ ���������ϴ� ���°� �Ǿ�� ������ �׻� �׷� ���� �ƴϴ�.
	Stamina�� Ư�� ������ ������ ���¹ٰ� ��ȭ�ؾ� �ϰ� ���� 0�� �Ǹ� ���������ϰ� �; �׷��� ���� ���Ѵ�.
	�̷� ��Ȳ���� ���� ����ؼ� switch���� �����.
	*/
	switch (StaminaStatus)
	{
	case EStaminaStatus::ESS_Normal:
		if (bShiftKeyDown) //shiftŰ�� �����ִٸ� 
		{
			if (Stamina - DeltaStamina <= MinSprintStamina) //�̰� true�� �Ǹ� ���¹̳ʰ� minsprintstamina�� �Ѿ �����̴�. BelowMinimum ���·� ��ȯ
			{
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum); //���¹̳��� ���¸� BelowMinimum ���·� ��ȯ.
				Stamina -= DeltaStamina; //���¹̳� ���´� BelowMinimum�� �Ѿ���� ���� 0�� �ƴϹǷ� ��� ����.
			}
			else //���¹̳ʰ� ���� MinSprintStamina�� �Ѿ�� ����. ������ Normal State�� ������, BelowMinimum ���·� ��ȯ���� �ʴ´�.
			{
				Stamina -= DeltaStamina;
			}
			//shiftŰ�� ���������Ƿ� �������� ���´� �������ָ� �ϴ� ���¿��� �Ѵ�. �׷��� ������ Ȥ�� �¿�� �����̰� �ִ� ��Ȳ�̾�� �������ְ� Ȱ��ȭ �ȴ�.
			if (bMovingForward || bMovingRight) SetMovementStatus(EMovementStatus::EMS_Sprinting);
			else SetMovementStatus(EMovementStatus::EMS_Normal); //�׷��� ������ �Ϲ� �޸���
		}
		else //shiftŰ�� �������� ���� �����̴�. ���¹̳ʰ� �پ����ִٸ� ȸ���ؾ� �ϰ� ���� ���ִ� �����̸� �� ���¸� �����ϵ��� �ؾ� �Ѵ�.
		{
			if (Stamina + DeltaStamina >= MaxStamina) //���¹̳��� ���°� ���� �� ���¶�� 
			{
				Stamina = MaxStamina;	// ���¹̳ʴ� Max�� ���·�, ���̻� �������� �ʴ´�.
			}
			else //���¹̳ʰ� ���� ������ ���� ���¶�� 
			{
				Stamina += DeltaStamina; //���ų� �׳� �޸��� �����̹Ƿ� ��tick �� ���¹̳ʸ� �������Ѿ� �Ѵ�.
			}
			//ShiftŰ�� �������� ���� �����̹Ƿ� �������� ���´� �׳� �޸��⸦ �ϴ� ���¿��� �Ѵ�.
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break; //��������� ���¹̳ʰ� Normal ����. BelowMinimum�̰ų� ���� ���·� �Ѿ ���� �ƴ϶��, ���� �ܰ�� �Ѿ�� �ʵ��� break. 
	
	
	case EStaminaStatus::ESS_BelowMinimum: // ���¹̳ʰ� ������ MinSprintStamina���Ϸ� �������� ����̴�.
		if (bShiftKeyDown) //shiftŰ ������ ��
		{
			if (Stamina - DeltaStamina <= 0.f) //���¸ӳʰ� ������ 0�� �ȴٸ�
			{
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted); //������ ���´� �׳� �޸��� ���°� �ǰ� ���¹̳ʴ� 0���� �����Ѵ�.
				Stamina = 0;
				SetMovementStatus(EMovementStatus::EMS_Normal); //���¹̳� �������Ƿ� �������� ���߰� �׳� �ٱ� ����.
			}
			else
			{
				Stamina -= DeltaStamina; //���¹̳� ���� �ƴϸ� ������ �� �� �����Ƿ� ��� ���¹̳� �����ϸ� ��������.
				if (bMovingForward || bMovingRight) SetMovementStatus(EMovementStatus::EMS_Sprinting);
				else SetMovementStatus(EMovementStatus::EMS_Normal);
				 /*SetMovementStatus(EMovementStatus::EMS_Sprinting);*/
			}
		}
		else //shiftŰ �� ���� ���� ��
		{
			if (Stamina + DeltaStamina >= MinSprintStamina) //���¹̳ʰ� MinSprintStamina�� ������ Normal�����̱� ������ �� ���·� ��ȯ����.
			{
				SetStaminaStatus(EStaminaStatus::ESS_Normal); //������ȯ
				Stamina += DeltaStamina; //���ų� �׳� �޸��� �����̰� ���׹̳ʰ� ���� ������ �ʱ� ������ ��tick �� ���¹̳� ȸ��
			}
			else
			{
				Stamina += DeltaStamina; //���ų� �׳� �޸��� �����̰� ���׹̳ʰ� ���� ������ �ʱ� ������ ��tick �� ���¹̳� ȸ��
			}
			SetMovementStatus(EMovementStatus::EMS_Normal); // � �����̵� shiftŰ�� ������ ���� �����Ƿ� �׳� �޸��� ���´�.
		}
		break;
	
	
	case EStaminaStatus::ESS_Exhausted: //���¹̳� �� �������� �� 
		if (bShiftKeyDown) 
		{
			Stamina = 0.f; 
			/*shiftŰ�� ������ ���¹̳ʰ� ���� �����̱� ������ �������� ���¿� �ƹ� ��ȭ�� �Ͼ�� �ʰ� ���¹̳��� ���´� ��� 0���� �ʱ�ȭ�ȴ�.
			shiftŰ�� ������ �ʾƾ� ��μ� ������ ���۵ǰ� �Ѵ�.*/
		}
		else
		{
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering); 
			/*shiftŰ�� ������ ���� ���·�, ���¹̳ʴ� ��� ȸ���ȴ�.
			ȸ�� ���¿����� ���¹��� ������ �⺻ �Ͼ���� ���¿��� ��������� �ٲ�� �Ѵ�.*/
		}
		SetMovementStatus(EMovementStatus::EMS_Normal); //�ƹ�ư ��ģ �����̹Ƿ� �������ִ� ���Ѵ�. �׳� �޸��⸸ ��.
		break;
	

	case EStaminaStatus::ESS_ExhaustedRecovering: 
		/*ü�� ȸ�� ���� �����̴�.
		�� ���¿����� ���׹̳� ���¹� ������ �����ϰ� ȸ�� �߿� normal ���·� �����ϴ����� �Ǻ��Ͽ� �� ���¹� ������ �����Ų��.*/
		if (Stamina + DeltaStamina >= MinSprintStamina)
		{
			SetStaminaStatus(EStaminaStatus::ESS_Normal); //���¹̳ʰ� MinSprintStamina�� �ѱ�� �ٽ� Normal���·� ȸ���� ������ �Ǻ��Ѵ�.
			Stamina += DeltaStamina;
		}
		else
		{
			Stamina += DeltaStamina; //�ƴϸ� ��� ���¹̳ʸ� ȸ���Ѵ�.
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);//�� ������ �׳� �޸��⸸ ��.
		break;
	
	default:
		;
	}



	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation(); //�� ��ü�� ��ġ�� �� ƽ���� ã�� �����صε��� �Ѵ�.
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation; //�� ƽ���� �������� �� ��ġ�� ���¹� ��ġ�� Ȱ���� �� �ֵ��� �Ѵ�.
		}
	}
}


// �÷��̾ ������ ������ ���߷��� �׿� ��� �������� ������ ���� ȸ���� �� ������ �˷��־�� �Ѵ�. �¿�θ� ȸ���ϹǷ� ȸ�� ������ Yaw�̴�.
FRotator AMain::GetLookAtRotationYaw(FVector Target)
{

	/*
	KismetMathLibrary�� FindLookAtRotation �Լ��� ��������(�÷��̾��� ��ġ)���� Ÿ���� ��ġ�� ���� ȸ������ ã���ش�. 
	���� ��ġ�� �˰� �� �������� ���� �ϴ� �� �ʿ���.  
	FindLookAtRotation �Լ��� ù��° �Ű������� �÷��̾��� ��ġ, �� ��° �Ű������� Ÿ�� �� ���� ��ġ
	*/
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw; //��������� �÷��̾ �Ĵٺ��ƾ� �� ȸ����(����)�� ��ȯ
}




// ������ �Է¿� ����� ���� ���� �۾���
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	//����
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//��������
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);	//Ű ���ε� �۾�1 : shift�� ������ pressed�� ������ ��.
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::ShiftKeyUp); // Ű ���ε� �۾�2 : shift�� ���� released �� ��

	//����
	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);

	//��� ���� �� ���콺 �׼� �� ����
	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown);	
	PlayerInputComponent->BindAction("LMBt", IE_Released, this, &AMain::LMBUp); 

	//���콺 �׼� �� ������
	PlayerInputComponent->BindAction("RMB", IE_Pressed, this, &AMain::RMBDown);
	PlayerInputComponent->BindAction("RMBt", IE_Released, this, &AMain::RMBUp);

	//Ű���� ȭ��ǥ �̿� ī�޶� ���� �̵�
	PlayerInputComponent->BindAxis("Turn", this, &AMain::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMain::LookUp);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);

	//ESC Ű 
	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::ESCDown);
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &AMain::ESCUp);


}

bool AMain::CanMove(float Value) //�÷��̾ �������� �������� ���ǿ� ���� �Ǻ��ϴ� �Լ�.
{
	if (MainPlayerController)
	{
		return (Controller != nullptr) &&	//�ʹ� ��� �Ʒ��� �������� �� �ִ�. ����� ��Ʃ��� Ư¡.
			(Value != 0.0f) && (!bAttacking) &&
			(!bStrongAttacking) &&
			(MovementStatus != EMovementStatus::EMS_Dead) &&
			!MainPlayerController->bPauseMenuVisible;
	}
	return false;
}


void AMain::MoveForward(float Value)
{

	bMovingForward = false; 
	if ((Controller != nullptr) && (Value != 0.0f) && (!bAttacking) && (!bStrongAttacking) && (MovementStatus != EMovementStatus::EMS_Dead)) //�����ϰ� ���� ���̰ų� �׾����� �� �������� �ʵ��� �Ѵ�. 
	{
		//find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
		
		bMovingForward = true; 
		//���� ������ ��� �����ϰ� ������ ������ true�� �ȴ�. 
		//��? : Tick �Լ����� �Ź� Ȯ���ϰ�, �̰��� false�� �Ǹ� shift Ű�� ������ �÷��̾ ���ڸ����� �޸��� ����� ���� �ʵ��� �ϱ� �����̴�.
		//MoveRight������ ���������̴�.
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

void AMain::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AMain::Turn(float Value)
{
	AddControllerYawInput(Value);
	//if (CanMove(Value))
	//{
		//AddControllerYawInput(Value);
	//}
}

void AMain::TurnAtRate(float Rate)
{
	//if (CanMove(Value))
	//{
		//AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	//}
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());

}

void AMain::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

//���� ���콺�� ���� �ִ��� ���θ� Ȯ���ϴ� �Լ����̴�.
void AMain::LMBDown()
{
	bLMBDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead) return; // ���� ���¶�� �Լ� ���� �Ұ�

	if (MainPlayerController)
	{
		if (MainPlayerController->bPauseMenuVisible) return; //����ȭ���� �����ִٸ� ���콺 ��ư�� �� ������. ������ ���콺 ��ư�� �����ϰ� ���ش�.
	}

	if (ActiveOverlappingItem) // ���� ��ħ�������� ��Ÿ���� ������ true��� �� ��� ����Ÿ������ Ȯ���Ѵ�. �´ٸ� ��� �����ϰ� �Ѵ�.
	{
		AWeapons* Weapons = Cast<AWeapons>(ActiveOverlappingItem);
		if (Weapons)
		{
			Weapons->Equip(this);
			SetActiveOverlappingItem(nullptr); //��� �����ϰ� ���� �ٽ� ������� �������´�.
		}
	}
	else if (WeaponEquipped)// ���� ��ġ�� �ʴ´ٸ� ��� �̺� �����ϰ� �ִ��� Ȯ��
	{
		Attack(); //�� �Լ��� ���ϰ��� true�� �Ǹ� �÷��̾ ���� ������ ������ ��.
		//���� ���� �� �ش� �ִϸ��̼��� ����ǵ��� �Ѵ�.
	}
}

void AMain::RMBDown()
{
	bRMBDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead) return; // ���� ���¶�� �Լ� ���� �Ұ�
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;
	if (WeaponEquipped)// ���� ��ġ�� �ʴ´ٸ� ��� �̺� �����ϰ� �ִ��� Ȯ��
	{
		StrongAttack(); //�� �Լ��� ���ϰ��� true�� �Ǹ� �÷��̾ ���� ������ ������ ��.
		//���� ���� �� �ش� �ִϸ��̼��� ����ǵ��� �Ѵ�.
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

void AMain::ESCUp()
{
	bESCDown = false;
}

void AMain::ESCDown()
{
	bESCDown = true;

	if (MainPlayerController)
	{
		MainPlayerController->TogglePauseMenu();
	}

}


//�÷��̾� ĳ������ ü���� �پ�� �� �߻��� ����� ���Ѵ�.(����̶����)
//void AMain::DecrementHealth(float Amount) //Amount�� ���̳� ��ֹ��κ��� �޴� ���ط��̴�. �� ���ط� ������ ���� �÷��̾��� hit ����� �ٸ��� �� �� �ִ�.
//{
//	if (Health - Amount <= 0.f)
//	{
//		Health -= Amount;	//ü���� �پ��� ��.
//		Die();				//�� �������� ����. 
//	}
//	//ü���� �� ������ �״� �� �ƴ϶�� �׳� ü���� ���̴� ��찡 ����.
//	else
//	{
//		Health -= Amount;
//	}
//}


//�÷��̾� ĳ���Ͱ� �������� ���� �� �߻��� ����� ����(�����ݱ�����)
void AMain::IncrementCoins(int32 Amount)
{
	Coins += Amount;
}

//�÷��̾� ĳ���Ͱ� Ư�� �������� ���� �� �߻��� ����� ����(�����ݱ�����) : ���⼭�� ü�·� ����
void AMain::IncrementHealth(float Amount)
{
	if (Health + Amount >= MaxHealth)
	{
		Health = MaxHealth;
	}
	else Health += Amount;
}


//�÷��̾� ü���� 0���ϰ� �Ǿ��� �� ������ �Լ�. ���� ����.
//���� �Լ��� ���������� ���� ���� �ִϸ��̼� ������ ���� ����� �ҷ��;� �Ѵ�.
void AMain::Die()
{	
	if (MovementStatus == EMovementStatus::EMS_Dead) return; //�̹� �׾��ִ� �����̸� �ι� �ٽ� ���� �ִϸ��̼��� �������� �ʵ��� �Ѵ�. �̰��� ���� ��� ���� ���Ŀ� ������ ���ݹ����� �� �� �ִϸ��̼��� �����Ѵ�.
	int32 DeathAnimNum = FMath::RandRange(0, 2);
	UAnimInstance* DeathAnimInstance = GetMesh()->GetAnimInstance(); //AnimInstance.h�� �ʿ�� �Ѵ�. 
	if (DeathAnimInstance && CombatMontage)	//�𸮾� �������� Main�� ����Ǿ� �ִ� CombatMontage �� �ҷ���. (DeathAnim�� CombatMontage�� ���ٸ�..)
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
	// �ִϸ��̼��� Ư�� ���¿��� �����ֵ��� �ϴ� ����� Mesh�� �ִ�. bPauseAnims�� �װ�.
	UE_LOG(LogTemp, Warning, TEXT("Death End Executed"))
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
	 /*�� �� �Լ��� ���� ������ �ִϸ��̼��� Ư�����¿��� �޽ð� �����ֵ��� �ϴ� ����� ������.
	 �÷��̾ ������ �� �Լ��� ȣ������ν� �� ���·� �����Ѵ�.*/

}

//Jump�� character.h�� �̹� �ִ� �Լ���, main���� ��ӹ޾� Ȱ���Ѵ�.
void AMain::Jump()
{
	if (MainPlayerController) if (MainPlayerController->bPauseMenuVisible) return;

	if (MovementStatus != EMovementStatus::EMS_Dead) //�÷��̾ ����ִ� ���¿����� jump�� �����ϴ�.
	{
		Super::Jump();
		UGameplayStatics::PlaySound2D(this, this->JumpSound);
	}
}

/*ĳ���� �̵� ���¸� Ȯ���ϴ� �Լ�*/
void AMain::SetMovementStatus(EMovementStatus Status) 
{
	MovementStatus = Status;	//�Ű������� ���� Status�� �ʱ� �̵��ӵ��̴�.
	if (MovementStatus == EMovementStatus::EMS_Sprinting) //���� �̵��ӵ��� �������� �ӵ��� �޶����ٸ� 
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed; //ĳ������ �̵��� �������� ���·� �ٲ��. 
		//MaxWalkSpeed�� GetCharacterMovement�� �ִ� �����̴�. �÷��̾��� �̵� ���¸� ��Ÿ���� ������, ������ SprintingSpeed �Ǵ� RunningSpeed�� ���� ������ �� �ִ�.
	}
	else //���� �������� �ϴ� ���� �ƴ϶�� �׳� �Ϲ� �޸��⸦ �ϴ� ��
	{
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}


//�Ʒ� �� �Լ��� �ϴ� �� : ����Ʈ Ű�� ������ �ִ��� �ƴ������� �����Ѵ�.
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
	for (int32 i = 0; i < PickupLocations.Num(); i++) //������ ȹ�� �� ��ŭ �ݺ��Ͽ� ȹ�� ��Ҹ� ǥ���� ���̴�.
	{
		/*�������� ĳ���Ͱ� ��� �ִ��� ��Ÿ���� ���� ������ ������̴�. KismetSystem�� Ȱ���� ���忡�� ĳ���Ͱ� ��� �ִ��� ��Ÿ���� ����� ���� �� �ִ�.*/
		/*UKismetSystemLibrary::DrawDebugSphere(this, GetActorLocation() + FVector(0, 0, 100.f), 25.f, 5, FLinearColor::Green, 10.f, .25f);*/
		/*
		Fvector�� �Ű������� �ִ� ���� ���� ĳ������ x,y,z ���Ͱ��� ���� �����̴�. ĳ������ �Ӹ���(z����)�� ĳ���� ��ġ�� ǥ���ϴ� ��(sphere)�� ����� �� ���̴�. ������ �Ű������� DrawDebugSphere�� ���Ǹ� peeking�ؼ� �� �� �ִ�.
		���� ���, ������ �Ű������� ���� �����̰� ���������� �� ��° �Ű������� �� ���� �� �ʰ� ��Ÿ�� �������� ��Ÿ����. ���⼭�� 10�ʸ� �ǹ�. Green�� ���� ����, 5�� ���׸�Ʈ ��.
		�̸� �̿��ؼ� ���� ���۽� ����ڿ��� ���۹��� �˷��ִ� ����� ���� ���� �� ���� ���̴�.
		*/

		UKismetSystemLibrary::DrawDebugSphere(this, PickupLocations[i], 30.f, 7, FLinearColor::Green, 10.f, .25f);
		/*PickupLocations�� �÷��̾ �������� ȹ���� ����̹Ƿ� ���� "GetActorLocation() + FVector(0, 0, 100.f)"�� ���� �ͺ��� PickupLocations�迭�� �ε����� ����ϴ� ���� ����.*/
	}

	/*PickupLocations�� �̿��� �� for���� ���� �����ϰ� ���� ���� �ִ�. range based �ݺ���*/
	/*
	for (FVector Location : PickupLocations) //�迭 ��ü�� ��ȸ�Ͽ� �� �ε����� ���������� ������.
	{
		UKismetSystemLibrary::DrawDebugSphere(this, Location, 30.f, 7, FLinearColor::Green, 10.f, .25f);
		//���� : PickupLocation ��ſ� �׳� Location ������ �ִ´�. ���� ȹ���� ������ ��ü�� ǥ���ϰ� �ʹٸ� ������ ���. �׷��� �ʰ� ��� ǥ���ϰ� �ʹٸ� ���� ����� �� ������ ���.

	}
	*/
}

//���ο� ��� ������ ��, ������ �����ϰ� �ִ� ��� ������ ���������Ѵ�.
void AMain::SetWeaponEquipped(AWeapons* WeaponToSet)
{
	if (WeaponEquipped)//��� �����ϰ� �ִٸ� ��������
	{
		WeaponEquipped->Destroy();
	}
	WeaponEquipped = WeaponToSet; //�����ϰ� �ִ� ��� �����ٸ� ���� �ݴ� ���� ����
}

void AMain::Attack() //���� ���� ���θ� �Ǵ��ϰ� �ش� ���ϸ��̼��� �ҷ��� �����Ѵ�.
{
	//if (!bAttacking && !bStrongAttacking)//���� ���� �ÿ��� ���콺�� �ٽ� Ŭ���ϴ��� �� ������ ������ ������ ������ ����ȴ�.
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead) // ���� ���� �׼� ��ȯ�� ���ϸ� ���� �ϳ��� ���ִ� �� ���� ���� �ִ�.(Ÿ���� ���� ����) ���� �÷��̾� ���°� ���� ���°� �ƴϾ�� ������ ������ �� �ִ�.
	{
		SetInterpToEnemy(true);//������ �����԰� ���ÿ� ������ ������ ���ߵ��� �Ѵ�.
		bAttacking = true;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); //AnimInstance.h�� �ʿ�� �Ѵ�.
		if (AnimInstance && CombatMontage)
		{
			/*
			�÷��̾�� ���� ������ �ο��� �� �ִ�.(�޺� �ý��� ����)
			�޺��� ���� ��ŭ case���� ����.
			*/
			int32 Section = FMath::RandRange(0, 1); //RandRange�� �Ű������� ���� �� ������ ������ ���ڴ�� �ִϸ��̼��� �����Ѵ�. 0,1,2 �ϳ��� ������ �׿� �ش��ϴ� case���� �ִϸ��̼��� �����ϵ��� �ϴ� ���̴�.
			//if (bPlayerMoving) Section = 1;
			if (AttackStandard == 0) Section = 0;
			else if (AttackStandard == 1) Section = 1; //1,2�� ������ ������ ���� �ϱ� ���� ��ġ

			if (bShiftKeyDown) Section = 2;

			switch (Section)
			{
			case 0:
				UGameplayStatics::PlaySound2D(this, this->BattleCry1);
				AnimInstance->Montage_Play(CombatMontage, 0.9f);//�� ��° �μ��� �ִϸ��̼��� ����ӵ��� �����Ѵ�.
				AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage); //�������Ʈ���� ���� ���ϸ��̼��� �̸��� �μ��� �־��ָ� �ش� ���ϸ��̼� ȣ�� �� ����.
				AttackStandard = 1;
				break;

			case 1:
				UGameplayStatics::PlaySound2D(this, this->BattleCry2);
				AnimInstance->Montage_Play(CombatMontage, 0.9f);//�� ��° �μ��� �ִϸ��̼��� ����ӵ��� �����Ѵ�.
				AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage); //�������Ʈ���� ���� ���ϸ��̼��� �̸��� �μ��� �־��ָ� �ش� ���ϸ��̼� ȣ�� �� ����.
				AttackStandard = 0;
				break;

			case 2:
				UGameplayStatics::PlaySound2D(this, this->BattleCry3);
				AnimInstance->Montage_Play(CombatMontage, 0.7f);//�� ��° �μ��� �ִϸ��̼��� ����ӵ��� �����Ѵ�.
				AnimInstance->Montage_JumpToSection(FName("Attack_3"), CombatMontage); //�������Ʈ���� ���� ���ϸ��̼��� �̸��� �μ��� �־��ָ� �ش� ���ϸ��̼� ȣ�� �� ����.
				break;


			default:
				;
			}
		}
		//if (WeaponEquipped->SwingSound) //���⸦ ����ϰ� �ִ� ���¶�� ���⸦ �ֵθ� ������ �ֵθ��� �Ҹ��� ������ �Ѵ�.
		//{
		//	//UGameplayStatics::PlaySound2D(this, WeaponEquipped->SwingSound);//�̷��� sound �� ����Ϸ��� �׿� �´� ��� ������ �θ� �� �־�� �Ѵ�.
		//}
		
	}
}



void AMain::StrongAttack() //���� ���� ���θ� �Ǵ��ϰ� �ش� ���ϸ��̼��� �ҷ��� �����Ѵ�.
{
	float WieldingSpeed1 = 1.15f, WieldingSpeed2 = 1.1f; //���⸦ �ֵθ��� �ӵ�, �� �ִϸ��̼� ���� �ӵ�
	if (!bStrongAttacking)//���� ���� �ÿ��� ���콺�� �ٽ� Ŭ���ϴ��� �� ������ ������ ������ ������ ����ȴ�. *����� �߿��� ���������� ��ȯ�� �����ϳ� �� �ݴ�� �� �ȴ�.
	{
		SetInterpToEnemy(true);//������ �����԰� ���ÿ� ������ ������ ���ߵ��� �Ѵ�.
		bStrongAttacking = true;


		UAnimInstance* AnimInstance2 = GetMesh()->GetAnimInstance(); //AnimInstance.h�� �ʿ�� �Ѵ�.
		if (AnimInstance2 && CombatMontage_StrongAtck)
		{

			/*
			�÷��̾�� ���� ������ �ο��� �� �ִ�.(�޺� �ý��� ����)
			�޺��� ���� ��ŭ case���� ����.
			*/
			int32 Section = FMath::RandRange(0, 1); //RandRange�� �Ű������� ���� �� ������ ������ ���ڴ�� �ִϸ��̼��� �����Ѵ�. 0,1,2 �ϳ��� ������ �׿� �ش��ϴ� case���� �ִϸ��̼��� �����ϵ��� �ϴ� ���̴�.
			Section = 1;
			if (bShiftKeyDown) Section = 0;
			switch (Section)
			{
			case 0:
				if (Stamina <= 50) WieldingSpeed1 = 0.5f;//�÷��̾� ���׹̳ʰ� 50 ���϶�� ������ �ӵ��� ���������� �Ѵ�. (�ƴϸ� �̵� �ӵ� ������ �ǵ�� �̵��ӵ��� ���������� �� ���� �ִ�.)

				AnimInstance2->Montage_Play(CombatMontage_StrongAtck, WieldingSpeed1);//�� ��° �μ��� �ִϸ��̼��� ����ӵ��� �����Ѵ�.
				AnimInstance2->Montage_JumpToSection(FName("Attack_Heavy1"), CombatMontage_StrongAtck); //�������Ʈ���� ���� ���ϸ��̼��� �̸��� �μ��� �־��ָ� �ش� ���ϸ��̼� ȣ�� �� ����.
				UGameplayStatics::PlaySound2D(this, this->BattleCry4);
				Stamina -= 30.f; //������ �� ���¹̳� 30 �Ҹ�

				break;

			case 1:
				if (Stamina <= 50) WieldingSpeed2 = 0.5f;

				AnimInstance2->Montage_Play(CombatMontage_StrongAtck, WieldingSpeed2);//�� ��° �μ��� �ִϸ��̼��� ����ӵ��� �����Ѵ�.
				AnimInstance2->Montage_JumpToSection(FName("Attack_Heavy2"), CombatMontage_StrongAtck); //�������Ʈ���� ���� ���ϸ��̼��� �̸��� �μ��� �־��ָ� �ش� ���ϸ��̼� ȣ�� �� ����.
				UGameplayStatics::PlaySound2D(this, this->BattleCry5);
				Stamina -= 30.f; //������ �� ���¹̳� 30 �Ҹ�
				break;


			default:
				;
			}
		}
		//if (WeaponEquipped->SwingSound) //���⸦ ����ϰ� �ִ� ���¶�� ���⸦ �ֵθ� ������ �ֵθ��� �Ҹ��� ������ �Ѵ�.
		//{
		//	//UGameplayStatics::PlaySound2D(this, WeaponEquipped->SwingSound);//�̷��� sound �� ����Ϸ��� �׿� �´� ��� ������ �θ� �� �־�� �Ѵ�.
		//}
	}
}


void AMain::StrongAttackEnd() //���� ��
{
	if (bRMBDown)
	{
		StrongAttack();
	}
	//�������� �ƴ϶�� ������ ���� ���ߴ� ���� �ߴ�.
	SetInterpToEnemy(false);
	bStrongAttacking = false;
	bAttacking = false;
}


void AMain::AttackEnd() //���� ��
{
	//���� ���콺 ��ư�� 1ȸ Ŭ���� 1ȸ ���ݸ� �����ϰ� �� ������ ������ ��� �����ϵ��� �ϰ� �ʹٸ�
	if (bLMBDown)
	{
		Attack();
	}
	bAttacking = false;
	//�������� �ƴ϶�� ������ ���� ���ߴ� ���� �ߴ�.
	SetInterpToEnemy(false);
	bStrongAttacking = false;
	/*
	������ �ϰ� ���� �ٽ� ���콺�� ������ �������� ������ �ؼ��� �� �ȴ�.if�� �Ʒ��� �ƴ϶� ���� �ִٸ� ��� LMB�� �ٿ�� ���·� �νĵǱ� ������ ���콺�� ������ �ʾƵ� ������ ��� �����Ѵ�.
	�׷��� if�� �Ʒ��� ��ġ���־�� �ϴ� ���̴�. �����ϰ� false�� �ٽ� �ٲ�������.
	*/

}

//TakeDamage�Լ� : �÷��̾ �޴� ���������� �÷��̾�� �������� �����ϴ� �Լ� �� ������ ���� ��ɵ��� �� �Լ��� �����ȴ�.
float AMain::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) 
{
	//DecrementHealth(DamageAmount);//Health ������ ������ �ִ� DecrementHealth �Լ��� ȣ��. ���������� �Ű������� ����. �� �Լ��� �ᵵ ������ ���⼭�� �߰��� DamageCauser�� �����ϱ� ���� Ǯ� ����.

	if (Health - DamageAmount <= 0.f)
	{
		Health -= DamageAmount;	//ü���� �پ��� ��.
		Die();				//�� �������� ����. 


		//DamageCauser�� �������� ������ �Ǵ� ��, �� ���⼭�� �� NPC�̴�.
		if (DamageCauser)
		{
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);
			if (Enemy)
			{
				Enemy->bHasValidTarget = false;
			}
		}
	}
	//ü���� �� ������ �״� �� �ƴ϶�� �׳� ü���� ���̴� ��찡 ����.
	else
	{
		int32 HitAnimNum = FMath::RandRange(0, 2);
		UAnimInstance* HitAnimInstance = GetMesh()->GetAnimInstance(); //AnimInstance.h�� �ʿ�� �Ѵ�. 
		if (DamageAmount >= 100) HitAnimNum = 3; //200�̻��� �������� ������ �ڷ� ƨ���� ���� HitHard �ִϸ��̼� ���
		else if (DamageAmount < 60)
		{
			Health -= DamageAmount;
			return 0; // ���� �������� 60�̸��̸� �ƹ��� ���� �� ��.
		}
		if (HitAnimInstance && CombatMontage)	//�𸮾� �������� Main�� ����Ǿ� �ִ� CombatMontage �� �ҷ���. (DeathAnim�� CombatMontage�� ���ٸ�..)
		{

			UGameplayStatics::PlaySound2D(this, this->SmashingSound);	//�´� �Ҹ�
			
			

			HitAnimInstance->Montage_Play(CombatMontage, 2.f);
			if (HitAnimNum == 0)
			{
				HitAnimInstance->Montage_JumpToSection(FName("Hit1"));
				UGameplayStatics::PlaySound2D(this, this->PainSound);
			}

			else if (HitAnimNum == 1)
			{
				HitAnimInstance->Montage_JumpToSection(FName("Hit2"));
				UGameplayStatics::PlaySound2D(this, this->PainSound2);
			}
			else if (HitAnimNum == 2)
			{
				HitAnimInstance->Montage_JumpToSection(FName("Hit3"));
				UGameplayStatics::PlaySound2D(this, this->PainSound3);
			}
			else if (HitAnimNum == 3)
			{
				HitAnimInstance->Montage_JumpToSection(FName("HitHard"));
				UGameplayStatics::PlaySound2D(this, this->GreaterPainSound);
			}

			bStrongAttacking = false;	//Hit �� �� ���� �����ϴ� ���� ������ �ڵ�
			bAttacking = false;			//Hit �� �� ���� �����ϴ� ���� ������ �ڵ�
			SetInterpToEnemy(false);	//Hit �� �� ���� �����ϴ� ���� ������ �ڵ�
		}
		Health -= DamageAmount;
	}



	return DamageAmount; // �ް� �Ǵ� ���������� ��ȯ

}

//ApplyDamage�Լ� : �÷��̾�� �������� �����Ű�� �Լ�. �������� �޴� ���Ϳ� ��������, �������� �ִ� ������ ����, UDamageType�̶�� Ư���� Ŭ������ ���� � ������ ������������ ��� �Ű������� �ѱ��.
//���� ������ ���� �� �÷��̾�� �������� �� ���̹Ƿ� enemy.cpp���� �� ���� ���� �� �����ϵ��� �Ѵ�.



void AMain::SwingingSound() 
{
	if(WeaponEquipped) UGameplayStatics::PlaySound2D(this, WeaponEquipped->SwingSound); //���⸦ ����ϰ� �ִ� ���¶�� ���⸦ �ֵθ� ������ �ֵθ��� �Ҹ��� ������ �Ѵ�.
}


void AMain::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}


void AMain::RefreshTarget() //Ÿ�� ���� ���� : ���� ĳ���Ϳ� �������ϴ� ��� ���͸� Ÿ������ ����. (���� �� Enemy���� Ÿ������ ������ ��)
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, TargetingActor);//��ġ�� ���� �� �� ��° �Ű������� �� Ŭ������ ��ü�� ���͸� �����ϰ� ��� ���͸� �Ѵ�. ���⼭ Enemy Ŭ������ ã�Ƴ����� �ϴ� ��.
	
	//ĳ���Ϳ� ���� ����� ���� Ÿ������ ���� ���̴�. ĳ���Ϳ� �� ���� �Ÿ��� ����ؾ� ��.


	if (OverlappingActors.Num() == 0)	//�÷��̾�� �������ϴ� ���� ��ȿ�ؾ� �Ѵ�.(0�̶�� ���� ���� ��. �ּ��� �� ���� ���� �־�� ��)
	{
		//�÷��̾ ���� ���̰� �������Ǵ� �ٸ� ���� ���ٸ� ���� �ｺ�ٳ� ������ ����Ʈ���� ��� ���ش�.
		if (MainPlayerController)
		{
			MainPlayerController->RemoveEnemyHealthBar();
		}
		return; // ��ġ�� ���� �����Ƿ�(0�̹Ƿ�) �Լ� ����.
	}
	AEnemy* EnemyInClosestRange = Cast<AEnemy>(OverlappingActors[0]); //���� ����� ������ ������ �� ��ü�� �����ϴ� ������ �迭�� ����
	if (EnemyInClosestRange) //���� �������ϴ� ���� ������ EnemyŬ������ ���� Ȯ�εȴٸ�(���� �������� �׻� ����)
	{	
		FVector PlayerLocation = GetActorLocation(); //�÷��̾� ��ġ

		float EnemyDistance = (EnemyInClosestRange->GetActorLocation() - PlayerLocation).Size();//���� ����� �Ÿ��� �ִ� ������ �Ÿ� : ���� ��ġ���� �� ĳ������ ��ġ�� �� ����� �����ش�.

		for (auto Actor : OverlappingActors) //OverlappingActors �迭�� ��� ���Ҹ� ��ȸ�Ѵ�.(�� �ϳ��ϳ��� �÷��̾���� �Ÿ��� ������ �÷��̾�� �� ������ �����ϱ� ����)
		{
			//����[0]���� �迭 �� ����[?]���� �ϳ��� ��� �Ÿ� ������ ������ ��.
			AEnemy* Enemy = Cast<AEnemy>(Actor); //��, �� Ŭ������ ���� Ȯ�ε� ������ �����Ѵ�.
			if (Enemy)
			{
				float EnemiesDistance = (Enemy->GetActorLocation() - PlayerLocation).Size();
				if (EnemiesDistance < EnemyDistance)
				{
					EnemyDistance = EnemiesDistance;	
					EnemyInClosestRange = Enemy;

					//���� �迭 ���� �ٸ� ���� �Ÿ��� ���� ���� ª�� �Ÿ��� �ִ� ���� �Ÿ����� ª�� ��� '���� ª�� �Ÿ�'���� �� ������ ���� ����� ���� �����ϴ� ���̴�.
				}
			} 
		}
		if (MainPlayerController)
		{
			MainPlayerController->DisplayEnemyHealthBar();	//�� ü��â ���
		}
		SetCombatTarget(EnemyInClosestRange);	// ���� ����� ���� �����̿� �ִ� ��(���� �Ǹ�� �� ��ü �� �ϳ���)�� Ÿ������ ����
		bHasCombatTarget = true;
	}

}


void AMain::SwitchLevel(FName LevelName)
{
	//LevelName�� �� �� �̸��� ���忡 �ִ�. �׷��Ƿ� ���忡�� �ҷ��;� �Ѵ�.
	UWorld* World = GetWorld();
	if (World)
	{
		FString CurrentLevel = World->GetMapName();//GetMapName�� �����Լ���, ���� ���� �̸��� �ҷ��´�. 
		//�ҷ������� ���� �̸��� ���� ���� �̸��� ���Ƽ��� �� �ȴ�.
		FName CurrentLevelName(*CurrentLevel); //FName�� Fstring���� �ʱ�ȭ�� �� ����. *�����ڴ� �̷� ��� FName�� ���ڿ��� �ʱ�ȭ�� �� �ְ� �Ѵ�.
		if (CurrentLevelName != LevelName)
		{
			//LevelName���� ���� ������ �����Ѵ�.
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	
	}
}


void AMain::SaveGame()
{
	USaveGameClass*SaveGameInstance = Cast<USaveGameClass>(UGameplayStatics::CreateSaveGameObject(USaveGameClass::StaticClass()));

	SaveGameInstance->CharacterStatus.PlayerHealth = Health;
	SaveGameInstance->CharacterStatus.PlayerMaxHealth = MaxHealth;
	SaveGameInstance->CharacterStatus.PlayerStamina = Stamina;
	SaveGameInstance->CharacterStatus.PlayerMaxStamina = MaxStamina;

	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	UE_LOG(LogTemp, Warning, TEXT("MapName: %s"), *MapName)

	SaveGameInstance->CharacterStatus.LevelName = MapName;

	if (WeaponEquipped)
	{
		SaveGameInstance->CharacterStatus.WeaponName = WeaponEquipped->Name;
	}

	SaveGameInstance->CharacterStatus.Coins = Coins;
	SaveGameInstance->CharacterStatus.PlayerLocation = GetActorLocation();
	SaveGameInstance->CharacterStatus.PlayerRotation = GetActorRotation();

	//������ ���Կ� �����ϴ� �Լ�. ���̺������� ���Կ� �ҷ����� ���̺���� �ν��Ͻ��� �� �޸� ���ο� �����͸� ��ǻ�� �޸� ��򰡿� �����Ѵ�.
	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);

	GEngine->AddOnScreenDebugMessage(0, 3.f, FColor::Green, FString::Printf(TEXT("Game Saved!")));
}

void AMain::LoadGame(bool SetPosition)
{
	USaveGameClass* LoadGameInstance = Cast<USaveGameClass>(UGameplayStatics::CreateSaveGameObject(USaveGameClass::StaticClass()));   

	LoadGameInstance = Cast<USaveGameClass>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStatus.PlayerHealth;
	MaxHealth = LoadGameInstance->CharacterStatus.PlayerMaxHealth;

	Stamina = LoadGameInstance->CharacterStatus.PlayerStamina;
	MaxStamina = LoadGameInstance->CharacterStatus.PlayerMaxStamina;

	Coins = LoadGameInstance->CharacterStatus.Coins;

	if (WeaponStorage)
	{
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons)
		{
			FString WeaponName = LoadGameInstance->CharacterStatus.WeaponName;

			if (Weapons->WeaponMap.Contains(WeaponName))
			{
				AWeapons* WeaponToEquip = GetWorld()->SpawnActor<AWeapons>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}
		}
	}
 
	if (SetPosition)
	{
		SetActorLocation(LoadGameInstance->CharacterStatus.PlayerLocation);
		SetActorRotation(LoadGameInstance->CharacterStatus.PlayerRotation);
	}

	SetMovementStatus(EMovementStatus::EMS_Normal); //�̰��� ���� ��� : ĳ���Ͱ� ���� �������� ������ ����������� �ε����� �� ĳ���Ͱ� ������ �׾��ִ� ���°� �ȴ�.
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;		//DeathEnd�Լ��� ��Ȱ��ȭ�Ǿ��� �ִϸ��̼ǰ� ���̷��� Ȱ��ȭ

	if (LoadGameInstance->CharacterStatus.LevelName != TEXT(""))
	{
		FName LevelName(*LoadGameInstance->CharacterStatus.LevelName);

		SwitchLevel(LevelName);
	}

}


void AMain::LoadGameNoSwitch()
{
	USaveGameClass* LoadGameInstance = Cast<USaveGameClass>(UGameplayStatics::CreateSaveGameObject(USaveGameClass::StaticClass()));

	LoadGameInstance = Cast<USaveGameClass>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStatus.PlayerHealth;
	MaxHealth = LoadGameInstance->CharacterStatus.PlayerMaxHealth;

	Stamina = LoadGameInstance->CharacterStatus.PlayerStamina;
	MaxStamina = LoadGameInstance->CharacterStatus.PlayerMaxStamina;

	Coins = LoadGameInstance->CharacterStatus.Coins;


	if (WeaponStorage)
	{
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons)
		{
			FString WeaponName = LoadGameInstance->CharacterStatus.WeaponName;

			if (Weapons->WeaponMap.Contains(WeaponName))
			{
				AWeapons* WeaponToEquip = GetWorld()->SpawnActor<AWeapons>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}
		}
	}

	SetMovementStatus(EMovementStatus::EMS_Normal); //�̰��� ���� ��� : ĳ���Ͱ� ���� �������� ������ ����������� �ε����� �� ĳ���Ͱ� ������ �׾��ִ� ���°� �ȴ�.
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;		//DeathEnd�Լ��� ��Ȱ��ȭ�Ǿ��� �ִϸ��̼ǰ� ���̷��� Ȱ��ȭ
}