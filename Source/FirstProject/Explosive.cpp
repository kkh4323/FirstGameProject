// Fill out your copyright notice in the Description page of Project Settings.

/*이 오브젝트와 가까워지면, 함정이 발동되도록 할 것이다*/

#include "Explosive.h"
#include "Main.h"	//Main헤더파일을 끌어옴으로써 Main캐릭터의 정보를 가져온다.
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"

AExplosive::AExplosive()
{
	Damage = 50.f;	//Explosive가 주는 데미지.
}



/*OnOverlapBegin과 OnOverlapEnd 는 각각 이 클래스의 인스턴스에 다른 액터가 다았을 때, 떨어질 때를 확인하는 작업이다.
	Explosive의 경우 액터가 객체에 닿으면 폭발하고 떨어지면 플레이어 캐릭터에게 데미지를 주어야 한다. 그것을 어떻게 구현할까?*/

void AExplosive::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	//UE_LOG(LogTemp, Warning, TEXT("Explosive::OnOverlapBegin()"))
	
	/*두 함수 각각에 들어가는 매개변수는 어떤 액터가 닿았을 때 그 액터의 정보를 전달해주는 것들이다.
	그래서 처음에 확인해주어야 하는 것은 그 다른 액터가 valid한 것인지(NULL이 아닌지) 확인하는 것이다.*/
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor); //OtherActor을 Main으로 변환해 Main에 저장한다. 이 OtherActor가 AMain이 아니라면 Main은 NULL이 될 것이다.
		if (Main) //만약 오버랩 되는 것이 캐릭터라면
		{

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


			Main->DecrementHealth(Damage);//cast한 결과가 Main 형이 맞다면 : Main이 explosive의 객체와 겹쳤을 때(즉 함정에 걸렸을 때) 체력이 50 깎이도록 한다.
		
			/*
			코인이나 폭발물 함정같이 캐릭터가 다가서면 작동하면서 없어져야 하는 요소들은 그 동작을 위한 함수가 따로 있다. : Destroy
			액터와 액터 내부의 모든 것을 없애버리는 것. 자동으로 메모리를 관리하는 언리얼의 기능 중 하나이다.
			단, 무기와 같이 장착을 해야한다든지의 이유로 없어져서는 안 되는 아이템의 경우 예외이다.
			*/
			Destroy();
		}
	}

	 
}

void AExplosive::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	UE_LOG(LogTemp, Warning, TEXT("Explosive::OnOverlapEnd()"))
}