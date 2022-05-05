// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "particles/ParticleSystemComponent.h"

AWeapons::AWeapons()
{
	//���̷�Ż �޽�������Ʈ ����
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());


	bWeaponParticle = false;

	WeaponState = EWeaponState::EWS_Pickup; 
}

void AWeapons::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	/*���⸦ ��� ���� ĳ���Ͱ� ���� �����ۿ� ������ ����Mesh�� ĳ���� Mesh�� ������ �� �߻��ؾ� �Ѵ�.*/
	if ((WeaponState == EWeaponState::EWS_Pickup) && OtherActor) //���� �÷��̾ ���� �������� ���� ������ ���. OtherActor�� ����� ���� ���Ͱ� ĳ�������� �ƴ��� Ȯ���ϴ� ��.
	{
		/*
		��������۰� ���� ���� ĳ����(Main)���� Ȯ���ϰ� �´ٸ� Equip �Լ��� ȣ��. 
		Equip�� Main�� �Ű������� �޾� �������� ������Ų��.
		*/
		AMain* Main = Cast<AMain>(OtherActor);	
		if (Main) //ĳ���Ͱ� �´��� Ȯ��
		{
			Main->SetActiveOverlappingItem(this); //���� �� �����ۿ� �������� �� overlapping �� �������� �˸���. ���⸦ �ٲ� ������, ȹ���� ������ ���� ���� �� ����� ����.
		}
	}
}
void AWeapons::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	if (OtherActor)
	{
		/*
		����� ĳ���Ͱ� �������� ���� ���¶�� overlapping ���¸� ������ �� �ִ�. 
		*/
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main) //ĳ���Ͱ� �´��� Ȯ��
		{
			Main->SetActiveOverlappingItem(nullptr); //�ƹ��͵� �������� ���� �����̱⿡ nullptr�� �Ű������� �ش�.
		}
	}
}

void AWeapons::Equip(AMain* Character)
{
	if (Character) //�Ű������� ���� ĳ���Ͱ� NULL�� �ƴϾ�� ����
	{
		//ī�޶� ���⸦ ĳ���� �޽÷� ������ ���⿡ ������ ���� �ʵ��� �����ؾ� �Ѵ�.(ī�޶� ���⸦ �����ϵ��� ������ �Ѵ�.)
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		//�÷��̾ ������ �ٸ� Pawn�鿡�Ե� ��������
		SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		//���⸦ ������ �� ����ȿ���� ����Ǹ� �÷��̾��� �հ� ���Ⱑ ��� �浹�ϴ� ������ �߻��� �� ����. �̸� ����.
		SkeletalMesh->SetSimulatePhysics(false);

		/*ĳ���� ���̷��濡 ������ �߰��Ѵ�.*/
		const USkeletalMeshSocket* Weapon_R = Character->GetMesh()->GetSocketByName("Weapon_R");
		/*
		GetSocketByName �� �̸����� ������ų �������� ������ ã�´�.
		�������Ʈ���� Weapon_R�̶�� �̸����� ������ ����� ���������Ƿ� �̸� �����Ѵ�.
		��������� �� �ڵ�� �������Ʈ���� ���� Weapon_R ������ �����ϴ� ���̴�.
		*/



		if (Weapon_R)
		{
			//���Ͽ� ������Ų ���Ϳ� ĳ���͸� ���δ�.
			Weapon_R->AttachActor(this, Character->GetMesh());
			/*
			Item Ŭ������ ����߱� ������ rotate �� Ȱ��ȭ�ϸ� ĳ������ ���̷��濡 �����Ǿ��� ������ ȸ���Ѵ�.
			ĳ���Ϳ��� �����ǰ� �� �ڿ��� ȸ������ �ʵ��� ȸ���� ��Ȱ��ȭ ��Ų��.
			*/
			bRotate = false;

			//Character->GetWeaponEquipped()->Destroy(); //�����ߴ� ���� ���� ���������Ѵ�. ��� �������� �ʰ� �ִ��� �����ϰ� ���� ����. 
			Character->SetWeaponEquipped(this); //this �� WeaponToSet����, �÷��̾ �� ���⸦ �Ű������� �����ϴ� ���̴�.
			Character->SetActiveOverlappingItem(nullptr); // �̹� ��� �����ϰ� �ƹ��͵� �������� �ʴ� ����. 
		
			WeaponState = EWeaponState::EWS_Equipped; //���� �ִϸ��̼� 1ȸ ���� �� ���Ⱑ �տ��� ������� ���� ����.

		}

		//���⸦ �����ϸ� �Ҹ��� ������ �Ѵ�.
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
//��������� Equip �Լ��� ȣ���ϸ� �������� ������ ������ �÷��̾� ���̷��濡 �ٽ� ������Ű�� ���̴�.