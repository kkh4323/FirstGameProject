// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"
#include "Enemy.h"

void UEnemyAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr) //�� NPC�� �÷��̾�� �����޴� ���� �ƴϱ� ������ nullptr�� �ȴ�.
	{
		Pawn = TryGetPawnOwner(); //�� Ŭ������ ���� Pawn�� ȣ��.
		if (Pawn)
		{
			Enemy = Cast<AEnemy>(Pawn); //Pawn�� ��ȿ�ϴٸ� ���� �ʱ�ȭ�� ���ÿ� Enemy Ŭ������ �����Ѵ�.
		}
	}
}

void UEnemyAnimInstance::UpdateAnimationProperties()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner(); //�� Ŭ������ ���� Pawn�� ȣ��.
		if (Pawn)
		{
			Enemy = Cast<AEnemy>(Pawn); //Pawn�� ��ȿ�ϴٸ� ���� �ʱ�ȭ�� ���ÿ� Enemy Ŭ������ �����Ѵ�. ���� ĳ���� �ϴ� ��.
		}
	}
	if (Pawn)
	{
		FVector Speed = Pawn->GetVelocity(); //�ʱ�ȭ�� ���ÿ� �� NPC�� �̵� �ӵ��� ����.
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		MovementSpeed = LateralSpeed.Size(); //NPC�� �ӵ�: �� �κ��� �������Ʈ���� �ٷ�.
	}
}