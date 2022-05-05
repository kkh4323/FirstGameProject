// Fill out your copyright notice in the Description page of Project Settings.


#include "FloatingPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

// Sets default values
AFloatingPlatform::AFloatingPlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/*���ٴϴ� �÷����� ���� ����. �翬�� �޽��� ������ �־�� �Ѵ�.
	�������� �������Ʈ���� ����*/
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	StartPoint = FVector(0.f); //��ŸƮ����Ʈ ��ġ ����
	EndPoint = FVector(0.f); //���ٴϴ� �÷����� ���ߴ� ��ġ=>StarPoint���� �����̱� ������ EndPoint���� ����

	InterpSpeed = 4.0f;	//EndPoint�� ������ �� ���� ������ �������� �� ��. ���� ���� ������ ������
	InterpTime = 1.f;
}

// Called when the game starts or when spawned
void AFloatingPlatform::BeginPlay()
{
	Super::BeginPlay();

	//���� ���� �� ���Ͱ� tlfwp ��𿡼� ������ ������ ��ġ�� �޾� StartPoint�� ����.
	StartPoint = GetActorLocation();
	//�÷����� �����̰� �ϱ� ���ؼ� EndPoint���� StartPoint��ŭ�� ���� �������� �Ѵ�.
	EndPoint += StartPoint;

	bInterping = false;

	GetWorldTimerManager().SetTimer(InterpTimer, this, &AFloatingPlatform::ToggleInterping, InterpTime);

	Distance = (EndPoint - StartPoint).Size(); //������� �Ÿ��� �̵��ߴ��� �� �� �ְ� ��
}

// Called every frame
void AFloatingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*�߰����� �־� �����÷����� �Դٰ��� �ϵ��� ���� ����. bool���� ������ �̿��� Endpoint�� �����ִٰ� �ٽ� StartPoint�� 
	�Ѿ������ �����. �̸� ���� Ÿ�̸Ӹ� ����� ��*/
	if (bInterping)
	{
		FVector CurrentLocation = GetActorLocation();
		FVector Interp = FMath::VInterpTo(CurrentLocation, EndPoint, DeltaTime, InterpSpeed);
		SetActorLocation(Interp);

		//Distance�� ���� �� �̻��̸� �ൿ�� �ٲٵ��� ��.
		float DistanceTraveled = (GetActorLocation() - StartPoint).Size();
		//�������� �ٴٸ��� ���� 0�� �� ����
		//��û ���� ��(1.x��ó��)���� ������ ������ �ٲٵ��� ��.
		if (Distance - DistanceTraveled <= 1.f)
		{
			ToggleInterping();

			GetWorldTimerManager().SetTimer(InterpTimer, this, &AFloatingPlatform::ToggleInterping, InterpTime); // ������� �ϸ� end�� start ���� swap�ؾ� ��
			SwapVectors(StartPoint, EndPoint);
		
		}
	}


}

void AFloatingPlatform::ToggleInterping()
{
	bInterping = !bInterping;	//���� true�� false�� �ٲٰ� �ݴ뵵 ����������.
}

void AFloatingPlatform::SwapVectors(FVector& VecOne, FVector& VecTwo)
{
	FVector Temp = VecOne;
	VecOne = VecTwo;
	VecTwo = Temp;
}