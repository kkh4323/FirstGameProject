// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"
#include "Enemy.h"

void UEnemyAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr) //적 NPC는 플레이어에게 조종받는 것이 아니기 때문에 nullptr이 된다.
	{
		Pawn = TryGetPawnOwner(); //이 클래스를 갖는 Pawn을 호출.
		if (Pawn)
		{
			Enemy = Cast<AEnemy>(Pawn); //Pawn이 유효하다면 적은 초기화와 동시에 Enemy 클래스를 참조한다.
		}
	}
}

void UEnemyAnimInstance::UpdateAnimationProperties()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner(); //이 클래스를 갖는 Pawn을 호출.
		if (Pawn)
		{
			Enemy = Cast<AEnemy>(Pawn); //Pawn이 유효하다면 적은 초기화와 동시에 Enemy 클래스를 참조한다. 적을 캐스팅 하는 것.
		}
	}
	if (Pawn)
	{
		FVector Speed = Pawn->GetVelocity(); //초기화와 동시에 적 NPC의 이동 속도를 설정.
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		MovementSpeed = LateralSpeed.Size(); //NPC의 속도: 이 부분은 블루프린트에서 다룸.
	}
}