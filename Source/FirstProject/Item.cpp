// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"

/*�� ������Ʈ�� ���������, ������Ʈ�� ���� �̺�Ʈ�� �߻��ϵ��� ���� ���̴�.�װ��� �������̸� �������� ȹ���ϵ��� �� ���̰� �����̸� ������
�ߵ��ǵ��� �� ���̴�.*/


// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionVolume"));
	RootComponent = CollisionVolume;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());	//�������� ������Ʈ�� �ð������� ����ȭ ��Ű�� ����.  
	//������ Ŭ������ �̷��ϹǷ� Explosive�� PickUp�� �̸� ��ӹ��� ����.


	IdleParticlesComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("IdlePariclesComponent"));
	/*"Particles/ParticleSystemComponent.h" ����� �־�� ����� ��� �����ϴ�.-> Include*/
	IdleParticlesComponent->SetupAttachment(GetRootComponent());

	bRotate = false; //default�� ȸ���� �� �Ѵ�.
	//ȸ���� �� �������� ���Ͱ� �� �������� ���ݾ� ȸ���ϴ� ���̴�. �׷��Ƿ� tick ������ �Ͼ���� �Ѵ�.
	RotationRate = 45.f; //ȸ���ӵ�
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	CollisionVolume->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnOverlapBegin);
	CollisionVolume->OnComponentEndOverlap.AddDynamic(this, &AItem::OnOverlapEnd);
	
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bRotate) //bRotate�� true�� �ȴٸ�
	{
		FRotator Rotation = GetActorRotation();	//FRotator�� Ȱ���� ���Ͱ� Rotate�ϵ��� �Ѵ�. Ư�� �ӵ��� ȸ���ϵ��� RotationRate�� �ش�.
		Rotation.Yaw += DeltaTime * RotationRate; //Yaw �������� ȸ���ϴµ�, DeltaTIme(�⺻ �ð����� : 1/30��)�� RotationRate�� ���� ���� ���Ѵ�. �׸��� �̸� SetActorRotation�� �Ű������� �־��ش�.
		SetActorRotation(Rotation); //������ �ϸ� ���� �������Ʈ�� ���� ���� ���� 'Item Properties'���� Rotate�� üũ/üũ���� �ϸ� �ȴ�. ���� ȸ�� �ϰų� �� �ϰų� �̴�.
	}

}

void AItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Super::OnOverlapBegin()"))

	/*OnOverlapBegin�� Particle System�� �����ǵ��� �� ���̴�. ��� Particle�� ��ȯ�ұ�? �ϴ� NULL�� �ƴϾ�� �Ѵ�*/
	if (OverlapParticles)//OverlapParticles�� True���
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticles, GetActorLocation(), FRotator(0.f), true);//UWorld��������� �ʿ�� �ϴ� �Լ��̴�. �� ���� ��Ŭ��� �Ѵ�.
	}
	/*���͸� ���ֹ����� ���� � ȿ���� �ְ� �ʹٸ�? �װ��� �������Ʈ���� �ؾ� �ϴ� �� : Overlap Particle ���� emit�� ȿ���� �־��־�� �Ѵ�.*/
	if (OverlapSound)
	{
		/*PlaySound2D�� ���� Context Object�̴�. UObject�� ù �Ű������� ���´�. �ι�° �Ű������� soundcue�� ����.������ �Ű������� default
		���� ���� �ֱ⿡ �� �� ���� �־��־ ������.*/
		UGameplayStatics::PlaySound2D(this, OverlapSound); //SoundCue.h�� �ʿ�� �Ѵ�. 
	}

}

void AItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("Super::OnOverlapEnd()"))
}