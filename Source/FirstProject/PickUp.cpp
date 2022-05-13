// Fill out your copyright notice in the Description page of Project Settings.

//아이템을 주울 때 해당 이벤트가 발생하는 것을 알리도록 할 것이다.

#include "PickUp.h"
#include "Main.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"

APickUp::APickUp()
{
	CoinCount = 1;
}


void APickUp::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	UE_LOG(LogTemp, Warning, TEXT("PickUp::OnOverlapBegin()"))

	if (OtherActor) 
	{
		AMain* Main = Cast<AMain>(OtherActor); //OtherActor을 Main으로 변환해 Main에 저장한다. 이 OtherActor가 AMain이 아니라면 Main은 NULL이 될 것이다.
		if (Main)
		{
			Main->IncrementCoins(CoinCount);//cast한 결과가 Main(캐릭터) 이 맞다면 : Main이 PickUp의 객체와 겹쳤을 때(즉 아이템을 획득했을 때) 아이템 개수가 늘어나도록 한다.
			/*아이템을 획득했을 때, 그 위치를 저장해 표시하도록 한다. 아이템 획득 당시의 위치를 받아 main.h의 Tarray에 이를 저장한다.*/
			Main->PickupLocations.Add(GetActorLocation());//PickupLocation의 자료형은 TArray이다. PickupLocations.Add()의 매개변수로는 획득한 아이템의 FVector, 즉 플레이어의 현재 위치가 들어간다.
			/*아이템을 획득할 때마다 디버그구(sphere)가 그 위치에 나타나게 한다.*/


			if (OverlapParticles)//OverlapParticles이 True라면
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticles, GetActorLocation(), FRotator(0.f), true);//UWorld헤더파일을 필요로 하는 함수이다. 이 또한 인클루드 한다.
			}
			//액터를 없애버리기 전에 어떤 효과를 넣고 싶다면? 그것은 블루프린트에서 해야 하는 것 : Overlap Particle 에서 emit할 효과를 넣어주어야 한다.
			if (OverlapSound)
			{
				//PlaySound2D는 월드 Context Object이다. UObject를 첫 매개변수로 같는다. 두번째 매개변수로 soundcue가 들어간다.나머지 매개변수는 default 값을 갖고 있기에 이 두 개만 넣어주어도 괜찮다.
				UGameplayStatics::PlaySound2D(this, OverlapSound); //SoundCue.h를 필요로 한다. 
			}

			/*
			코인이나 폭발물 함정같이 캐릭터가 다가서면 작동하면서 없어져야 하는 요소들은 그 동작을 위한 함수가 따로 있다. : Destroy
			액터와 액터 내부의 모든 것을 없애버리는 것. 자동으로 메모리를 관리하는 언리얼의 기능 중 하나이다.
			*/
			Main->Health+=10; //<시험적인 코드. 안 되면 반드시 지울 것!>Main.cpp에 있는 플레이어 캐릭터의 체력을 가져온 것이다. 체력 10 회복. 된다!
			/*
			★다른 클래스에서 원하는 변수 또는 함수를 사용할 때 위와 같이 사용할 수 있다. 
			C++ 클래스, 헤더 개념 복습 필요.
			*/


			Destroy();// 아이템 사라짐.
		}
	}

}

void APickUp::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	UE_LOG(LogTemp, Warning, TEXT("PickUp::OnOverlapEnd()"))
}