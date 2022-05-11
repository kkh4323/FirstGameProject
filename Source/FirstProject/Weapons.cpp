// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h" //���⿡ �ڽ� ������Ʈ�� �����ֹǷ� �ʿ��� ��� ����.
#include "Enemy.h"
#include "Engine/SkeletalMeshSocket.h"

AWeapons::AWeapons()
{
	//���̷�Ż �޽�������Ʈ ����
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(GetRootComponent());

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision")); //���� �� ���Ⱑ ���� ��ü�� ����� ���� �˸��� ���� ������Ʈ.
	CombatCollision->SetupAttachment(GetRootComponent());//�ݸ����� ���� �ڽ�������Ʈ ����. ���� �������Ʈ���� �� �� �ִ�.

	

	bWeaponParticle = false;

	WeaponState = EWeaponState::EWS_Pickup; 

	Damage = 25.f; //���Ⱑ ����Ʈ�� ���� ������.
}

//���� ����� �ε����� ���� �ִٸ� OverlapBegin�� OverlapEnd�Լ��� ȣ��� ���̴�.
void AWeapons::BeginPlay()
{
	Super::BeginPlay();

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeapons::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AWeapons::CombatOnOverlapEnd);
	

	//���⿡ �ݸ��� �������� �� ������ ��Ƽ���� ����Ǵ� ���� �����Ѵ�.
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); //ActivateCollision�� ȣ��Ǳ� �������� � �浹�� �����Ѵ�.
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); //�⺻�����δ� ��� �Ͱ� ������ �Ǿ �����ϵ��� �Ѵ�.
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); //Pawn�� ������ �� ������ �����ϵ��� �Ѵ�.


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
		SetInstigator(Character->GetController());//���⿡�� ������ �ý����� �����ϱ� ���� �ʿ��� '�ν�Ƽ������'(�𸮾� ���Ĺ��� ����)


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

//������ ��ü�κп� ������ ������Ʈ �ڽ��� �� NPC�� ��ü�� ������ �� �߻���ų �̺�Ʈ�� ���⼭ �����Ѵ�.	
void AWeapons::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//����� overlap�� �ٸ� ���͸� check�� ���̴�.
	if (OtherActor) //���� ����� Overlap�� �ٸ� ���Ͱ� ��ȿ�� ���̶��(���⼭�� �翬�� ���̴�. �׷��Ƿ� enemy��������� ��Ŭ��� �ؾ� �Ѵ�.)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor); //Ȯ���� OtherActor�� EnemyŸ������ Enemy��� ������ �־��ش�.
		if (Enemy)
		{
			if (Enemy->HitParticles) // Particle �� �������� ���� ���¿��� �̸� �����Ϸ� �ϸ� �����Ϳ��� ũ������ �߻��Ѵ�.
			{
				const USkeletalMeshSocket* WeaponSocket = SkeletalMesh->GetSocketByName("WeaponSocket"); //�������Ʈ �����Ϳ��� ������ �� �κп� ���� ������ �� NPC�� ������ �� ��ȿ�� Ÿ�������� ���� ���̴�.(�̷��� �� �ϸ� ���� �� �κ��� �ƴ϶� �Ϲ������� root�� �����Ǿ� �ִ� ������ ������ �κ��� Ÿ������ �Ǵ� ������ �߻��Ѵ�.)
				if (WeaponSocket)
				{
					FVector SocketLocation = WeaponSocket->GetSocketLocation(SkeletalMesh);
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->HitParticles, SocketLocation, FRotator(0.f), false);
					//HitParticle�� ��ȿ���� Ȯ�εȴٸ� Emitter�� �� ��ġ(���� Į�� �´� �ڸ�)���� Ư�� ��ƼŬ �ý����� ��Ÿ������ �� ���̴�.
					//3��° �Ű������� SocketLocation�� ��ƼŬ�ý��� Emitter�� ������ ��ġ�� �߻���ų ���̴�. 
					//������ �Ű������� ��ƼŬ �ý����� ����� ������ �ڵ����� ���������� �����ϴ� ���̴�. �ѹ��� ����� ���̹Ƿ� false�� �� ���̴�.
				}
			}
			if (Enemy->HitSound) //���� Ÿ���ϰ� ��ƼŬ���� ����Ǵ� �ͱ��� Ȯ���� �� HitSound ���
			{
				UGameplayStatics::PlaySound2D(this, Enemy->HitSound); //HitSound�� enemy������Ͽ� ������ include �����Ƿ� ���⼭�� ��� ����.
			}

			if (Enemy->ScreamingSound) //���������� ScreamingSound �� ���
			{
				UGameplayStatics::PlaySound2D(this, Enemy->ScreamingSound); 
			}

			if (DamageTypeClass) //�� ���Ϳ� ������ ����
			{
				UGameplayStatics::ApplyDamage(Enemy, Damage, WeaponInstigator, this, DamageTypeClass);
			}
		}
	}
}

void AWeapons::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}





void AWeapons::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // ActivateCollision�� ȣ��� ������ QueryOnly�� �浹�� �߻��Ѵ�.
}

void AWeapons::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); //����Ʈ�� � �浹�� �����Ѵ�.
}