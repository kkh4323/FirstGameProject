// Fill out your copyright notice in the Description page of Project Settings.

//�������� �ֿ� �� �ش� �̺�Ʈ�� �߻��ϴ� ���� �˸����� �� ���̴�.

#include "PickUp.h"
#include "Main.h"

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
		AMain* Main = Cast<AMain>(OtherActor); //OtherActor�� Main���� ��ȯ�� Main�� �����Ѵ�. �� OtherActor�� AMain�� �ƴ϶�� Main�� NULL�� �� ���̴�.
		if (Main)
		{
			Main->IncrementCoins(CoinCount);//cast�� ����� Main(ĳ����) �� �´ٸ� : Main�� PickUp�� ��ü�� ������ ��(�� �������� ȹ������ ��) ������ ������ �þ���� �Ѵ�.
			/*�������� ȹ������ ��, �� ��ġ�� ������ ǥ���ϵ��� �Ѵ�. ������ ȹ�� ����� ��ġ�� �޾� main.h�� Tarray�� �̸� �����Ѵ�.*/
			Main->PickupLocations.Add(GetActorLocation());//PickupLocation�� �ڷ����� TArray�̴�. PickupLocations.Add()�� �Ű������δ� ȹ���� �������� FVector, �� �÷��̾��� ���� ��ġ�� ����.
			/*�������� ȹ���� ������ ����ױ�(sphere)�� �� ��ġ�� ��Ÿ���� �Ѵ�.*/

			/*
			�����̳� ���߹� �������� ĳ���Ͱ� �ٰ����� �۵��ϸ鼭 �������� �ϴ� ��ҵ��� �� ������ ���� �Լ��� ���� �ִ�. : Destroy
			���Ϳ� ���� ������ ��� ���� ���ֹ����� ��. �ڵ����� �޸𸮸� �����ϴ� �𸮾��� ��� �� �ϳ��̴�.
			*/
			Destroy();
		
		}
	}

}

void APickUp::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	UE_LOG(LogTemp, Warning, TEXT("PickUp::OnOverlapEnd()"))
}