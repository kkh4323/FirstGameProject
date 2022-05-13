// Fill out your copyright notice in the Description page of Project Settings.

/*�� ������Ʈ�� ���������, ������ �ߵ��ǵ��� �� ���̴�*/

#include "Explosive.h"
#include "Main.h"	//Main��������� ��������ν� Mainĳ������ ������ �����´�.
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"

AExplosive::AExplosive()
{
	Damage = 50.f;	//Explosive�� �ִ� ������.
}



/*OnOverlapBegin�� OnOverlapEnd �� ���� �� Ŭ������ �ν��Ͻ��� �ٸ� ���Ͱ� �پ��� ��, ������ ���� Ȯ���ϴ� �۾��̴�.
	Explosive�� ��� ���Ͱ� ��ü�� ������ �����ϰ� �������� �÷��̾� ĳ���Ϳ��� �������� �־�� �Ѵ�. �װ��� ��� �����ұ�?*/

void AExplosive::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	//UE_LOG(LogTemp, Warning, TEXT("Explosive::OnOverlapBegin()"))
	
	/*�� �Լ� ������ ���� �Ű������� � ���Ͱ� ����� �� �� ������ ������ �������ִ� �͵��̴�.
	�׷��� ó���� Ȯ�����־�� �ϴ� ���� �� �ٸ� ���Ͱ� valid�� ������(NULL�� �ƴ���) Ȯ���ϴ� ���̴�.*/
	if (OtherActor)
	{
		AMain* Main = Cast<AMain>(OtherActor); //OtherActor�� Main���� ��ȯ�� Main�� �����Ѵ�. �� OtherActor�� AMain�� �ƴ϶�� Main�� NULL�� �� ���̴�.
		if (Main) //���� ������ �Ǵ� ���� ĳ���Ͷ��
		{

			if (OverlapParticles)//OverlapParticles�� True���
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticles, GetActorLocation(), FRotator(0.f), true);//UWorld��������� �ʿ�� �ϴ� �Լ��̴�. �� ���� ��Ŭ��� �Ѵ�.
			}
			//���͸� ���ֹ����� ���� � ȿ���� �ְ� �ʹٸ�? �װ��� �������Ʈ���� �ؾ� �ϴ� �� : Overlap Particle ���� emit�� ȿ���� �־��־�� �Ѵ�.
			if (OverlapSound)
			{
				//PlaySound2D�� ���� Context Object�̴�. UObject�� ù �Ű������� ���´�. �ι�° �Ű������� soundcue�� ����.������ �Ű������� default ���� ���� �ֱ⿡ �� �� ���� �־��־ ������.
				UGameplayStatics::PlaySound2D(this, OverlapSound); //SoundCue.h�� �ʿ�� �Ѵ�. 
			}


			Main->DecrementHealth(Damage);//cast�� ����� Main ���� �´ٸ� : Main�� explosive�� ��ü�� ������ ��(�� ������ �ɷ��� ��) ü���� 50 ���̵��� �Ѵ�.
		
			/*
			�����̳� ���߹� �������� ĳ���Ͱ� �ٰ����� �۵��ϸ鼭 �������� �ϴ� ��ҵ��� �� ������ ���� �Լ��� ���� �ִ�. : Destroy
			���Ϳ� ���� ������ ��� ���� ���ֹ����� ��. �ڵ����� �޸𸮸� �����ϴ� �𸮾��� ��� �� �ϳ��̴�.
			��, ����� ���� ������ �ؾ��Ѵٵ����� ������ ���������� �� �Ǵ� �������� ��� �����̴�.
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