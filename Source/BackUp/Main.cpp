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
	Stamina = 120.f;
	Coins = 0;	

	RunningSpeed = 600.f;
	SprintingSpeed = 1000.f;

	bLMBDown = false;
	bShiftKeyDown = false;	//����Ʈ Ű�� ����Ʈ�� ������ ���� ���� �����̴�.

	bRMBDown = false;


	//ENUM���� ���� �÷��̾��� �ʱ� ���¸� ��Ÿ���� ������ �ʱ�ȭ(���׹̳� ��ȭ �� ����)
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;
	
	//����Ʈ�� ������ ���� �� ���¹̳ʰ� �����ϴ� ����
	StaminaDrainRate=25.f;
	//���׹̳ʰ� �����ϸ鼭 Ư�������� ������ �� ���¹��� ������ ���ؾ� �Ѵ�. �� ������. ���⼭�� �ִ�ġ�� 3�п� 1.
	MinSprintStamina=50.f;
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay(); //���� ���۽� �߻��� �̺�Ʈ��

}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//���׹̳��� ���� �Ǵ� ���Ҵ� �� Tick���� �Ͼ�� ���̴�. DeltaTime(1/30��)���� ĳ������ �޸� ���¿� ���� ���¹ٰ� ���ϴ� ���� �ǽð����� ��Ÿ���� �ϹǷ�.

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
			//shiftŰ�� ���������Ƿ� �������� ���´� �������ָ� �ϴ� ���¿��� �Ѵ�.
			SetMovementStatus(EMovementStatus::EMS_Sprinting);
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
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
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
}

// ������ �Է¿� ����� ���� ���� �۾���
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	//����
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
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

	//Ű���� ȭ��ǥ �̿� ī�޶� ���� �̵�
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);


}

void AMain::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f)&&(!bAttacking)) //�����ϰ� ���� �� �������� �ʵ��� �Ѵ�.
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

//���� ���콺�� ���� �ִ��� ���θ� Ȯ���ϴ� �Լ����̴�.
void AMain::LMBDown()
{
	bLMBDown = true;

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

void AMain::LMBUp()
{
	bLMBDown = false;
}



//�÷��̾� ĳ������ ü���� �پ�� �� �߻��� ����� ���Ѵ�.(����̶����)
void AMain::DecrementHealth(float Amount) 
{
	if (Health - Amount <= 0.f)
	{
		Health -= Amount;	//ü���� �پ��� ��.
		Die();				//�� �������� ����. 
	}
	//ü���� �� ������ �״� �� �ƴ϶�� �׳� ü���� ���̴� ��찡 ����.
	else
	{
		Health -= Amount;
	}
}


//�÷��̾� ĳ���Ͱ� �������� ���� �� �߻��� ����� ����(�����ݱ�����)
void AMain::IncrementCoins(int32 Amount)
{
	Coins += Amount;
}


void AMain::Die()
{

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
	if (!bAttacking)//���� ���� �ÿ��� ���콺�� �ٽ� Ŭ���ϴ��� �� ������ ������ ������ ������ ����ȴ�.
	{
		bAttacking = true;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); //AnimInstance.h�� �ʿ�� �Ѵ�.
		if (AnimInstance && CombatMontage)
		{

			/*
			�÷��̾�� ���� ������ �ο��� �� �ִ�.(�޺� �ý��� ����)
			�޺��� ���� ��ŭ case���� ����.
			*/
			int32 Section = FMath::RandRange(0, 3); //RandRange�� �Ű������� ���� �� ������ ������ ���ڴ�� �ִϸ��̼��� �����Ѵ�. 0,1,2 �ϳ��� ������ �׿� �ش��ϴ� case���� �ִϸ��̼��� �����ϵ��� �ϴ� ���̴�.
			switch (Section)
			{
			case 0:
				AnimInstance->Montage_Play(CombatMontage, 1.35f);//�� ��° �μ��� �ִϸ��̼��� ����ӵ��� �����Ѵ�.
				AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage); //�������Ʈ���� ���� ���ϸ��̼��� �̸��� �μ��� �־��ָ� �ش� ���ϸ��̼� ȣ�� �� ����.
				break;

			case 1:
				AnimInstance->Montage_Play(CombatMontage, 1.f);//�� ��° �μ��� �ִϸ��̼��� ����ӵ��� �����Ѵ�.
				AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage); //�������Ʈ���� ���� ���ϸ��̼��� �̸��� �μ��� �־��ָ� �ش� ���ϸ��̼� ȣ�� �� ����.
				break;

			case 2:
				AnimInstance->Montage_Play(CombatMontage, 0.7f);//�� ��° �μ��� �ִϸ��̼��� ����ӵ��� �����Ѵ�.
				AnimInstance->Montage_JumpToSection(FName("Attack_3"), CombatMontage); //�������Ʈ���� ���� ���ϸ��̼��� �̸��� �μ��� �־��ָ� �ش� ���ϸ��̼� ȣ�� �� ����.
				break;


			case 3:
				AnimInstance->Montage_Play(CombatMontage, 0.7f);//�� ��° �μ��� �ִϸ��̼��� ����ӵ��� �����Ѵ�.
				AnimInstance->Montage_JumpToSection(FName("Attack_4"), CombatMontage); //�������Ʈ���� ���� ���ϸ��̼��� �̸��� �μ��� �־��ָ� �ش� ���ϸ��̼� ȣ�� �� ����.
				break;

			default:
				;
			}
		}
	}
}



void AMain::AttackEnd() //���� ��
{
	//���� ���콺 ��ư�� 1ȸ Ŭ���� 1ȸ ���ݸ� �����ϰ� �� ������ ������ ��� �����ϵ��� �ϰ� �ʹٸ�
	if (bLMBDown)
	{
		Attack();
	}
	bAttacking = false; 
	/*
	������ �ϰ� ���� �ٽ� ���콺�� ������ �������� ������ �ؼ��� �� �ȴ�.if�� �Ʒ��� �ƴ϶� ���� �ִٸ� ��� LMB�� �ٿ�� ���·� �νĵǱ� ������ ���콺�� ������ �ʾƵ� ������ ��� �����Ѵ�.
	�׷��� if�� �Ʒ��� ��ġ���־�� �ϴ� ���̴�. �����ϰ� false�� �ٽ� �ٲ�������.
	*/
}