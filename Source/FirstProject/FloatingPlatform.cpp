// Fill out your copyright notice in the Description page of Project Settings.


#include "FloatingPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

// Sets default values
AFloatingPlatform::AFloatingPlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/*떠다니는 플랫폼을 만들 것임. 당연히 메쉬를 설정해 주어야 한다.
	나머지는 블루프린트에서 설정*/
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	StartPoint = FVector(0.f); //스타트포인트 위치 설정
	EndPoint = FVector(0.f); //떠다니는 플랫폼이 멈추는 위치=>StarPoint에서 움직이기 시작해 EndPoint에서 멈춤

	InterpSpeed = 4.0f;	//EndPoint에 가까이 갈 수록 움직임 느려지게 할 것. 작을 수록 움직임 느려짐
	InterpTime = 1.f;
}

// Called when the game starts or when spawned
void AFloatingPlatform::BeginPlay()
{
	Super::BeginPlay();

	//게임 시작 시 액터가 tlfwp 어디에서 시작할 것인지 위치를 받아 StartPoint에 저장.
	StartPoint = GetActorLocation();
	//플랫폼이 움직이게 하기 위해서 EndPoint에는 StartPoint만큼의 값이 더해져야 한다.
	EndPoint += StartPoint;

	bInterping = false;

	GetWorldTimerManager().SetTimer(InterpTimer, this, &AFloatingPlatform::ToggleInterping, InterpTime);

	Distance = (EndPoint - StartPoint).Size(); //어느정도 거리를 이동했는지 알 수 있게 함
}

// Called every frame
void AFloatingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*중간값을 넣어 발판플랫폼이 왔다갔다 하도록 만들 것임. bool형의 변수를 이용해 Endpoint에 멈춰있다가 다시 StartPoint로 
	넘어오도록 만들것. 이를 위해 타이머를 만드는 것*/
	if (bInterping)
	{
		FVector CurrentLocation = GetActorLocation();
		FVector Interp = FMath::VInterpTo(CurrentLocation, EndPoint, DeltaTime, InterpSpeed);
		SetActorLocation(Interp);

		//Distance가 일정 값 이상이면 행동을 바꾸도록 함.
		float DistanceTraveled = (GetActorLocation() - StartPoint).Size();
		//목적지에 다다르면 거의 0이 될 것임
		//엄청 작은 값(1.x몇처럼)보다 작으면 동작을 바꾸도록 함.
		if (Distance - DistanceTraveled <= 1.f)
		{
			ToggleInterping();

			GetWorldTimerManager().SetTimer(InterpTimer, this, &AFloatingPlatform::ToggleInterping, InterpTime); // 여기까지 하면 end와 start 값을 swap해야 함
			SwapVectors(StartPoint, EndPoint);
		
		}
	}


}

void AFloatingPlatform::ToggleInterping()
{
	bInterping = !bInterping;	//만약 true면 false로 바꾸고 반대도 마찬가지로.
}

void AFloatingPlatform::SwapVectors(FVector& VecOne, FVector& VecTwo)
{
	FVector Temp = VecOne;
	VecOne = VecTwo;
	VecTwo = Temp;
}