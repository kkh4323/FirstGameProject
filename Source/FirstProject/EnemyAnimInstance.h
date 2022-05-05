// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPROJECT_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

		
	/*
	주인공 캐릭터에 있는 몇가지 특성은 적 NPC에게도 있어야 한다.
	이동속도, 체력, 데미지 등등 몇가지 게임플레이 요소들을 추가해줄 것이다.

	*/
public:
	virtual void NativeInitializeAnimation() override;

	UFUNCTION(BlueprintCallable, Category = AnimationProperties)
	void UpdateAnimationProperties();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement) //참고로 EditAnywhere를 쓰려면 해당 함수나 변수는 Public 섹션에 있어야 한다.
	float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	class APawn* Pawn;
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	class AEnemy* Enemy;
};
