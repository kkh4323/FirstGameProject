// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"

/*이 오브젝트와 가까워지면, 오브젝트가 가진 이벤트가 발생하도록 만들 것이다.그것이 아이템이면 아이템을 획득하도록 할 것이고 함정이면 함정이
발동되도록 할 것이다.*/


// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionVolume"));
	RootComponent = CollisionVolume;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());	//아이템의 컴포넌트를 시각적으로 형상화 시키는 과정.  
	//아이템 클래스가 이러하므로 Explosive나 PickUp은 이를 상속받을 것임.


	IdleParticlesComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("IdlePariclesComponent"));
	/*"Particles/ParticleSystemComponent.h" 헤더가 있어야 기능을 사용 가능하다.-> Include*/
	IdleParticlesComponent->SetupAttachment(GetRootComponent());

	bRotate = false; //default로 회전은 안 한다.
	//회전은 매 순간마다 액터가 한 방향으로 조금씩 회전하는 것이다. 그러므로 tick 내에서 일어나도록 한다.
	RotationRate = 45.f; //회전속도
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
	if (bRotate) //bRotate이 true가 된다면
	{
		FRotator Rotation = GetActorRotation();	//FRotator을 활용해 액터가 Rotate하도록 한다. 특정 속도로 회전하도록 RotationRate을 준다.
		Rotation.Yaw += DeltaTime * RotationRate; //Yaw 방향으로 회전하는데, DeltaTIme(기본 시간단위 : 1/30초)에 RotationRate을 곱한 값을 더한다. 그리고 이를 SetActorRotation의 매개변수로 넣어준다.
		SetActorRotation(Rotation); //컴파일 하면 이제 블루프린트로 가서 새로 생긴 'Item Properties'란의 Rotate를 체크/체크해제 하면 된다. 각각 회전 하거나 안 하거나 이다.
	}

}

void AItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Super::OnOverlapBegin()"))

	/*OnOverlapBegin에 Particle System이 스폰되도록 할 것이다. 어떻게 Particle을 소환할까? 일단 NULL이 아니어야 한다*/
	if (OverlapParticles)//OverlapParticles이 True라면
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticles, GetActorLocation(), FRotator(0.f), true);//UWorld헤더파일을 필요로 하는 함수이다. 이 또한 인클루드 한다.
	}
	/*액터를 없애버리기 전에 어떤 효과를 넣고 싶다면? 그것은 블루프린트에서 해야 하는 것 : Overlap Particle 에서 emit할 효과를 넣어주어야 한다.*/
	if (OverlapSound)
	{
		/*PlaySound2D는 월드 Context Object이다. UObject를 첫 매개변수로 같는다. 두번째 매개변수로 soundcue가 들어간다.나머지 매개변수는 default
		값을 갖고 있기에 이 두 개만 넣어주어도 괜찮다.*/
		UGameplayStatics::PlaySound2D(this, OverlapSound); //SoundCue.h를 필요로 한다. 
	}

}

void AItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("Super::OnOverlapEnd()"))
}