// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UCLASS()
class FIRSTPROJECT_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

	/*�⺻ ������ Collision*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item | Collision")
	class USphereComponent* CollisionVolume;

	/*���̽� �޽� ������Ʈ*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item | Mesh")
	class UStaticMeshComponent* Mesh;

	/*�����̳� ������ ������ ���� �۵� �ڽ��� �ǵ帮�ų� ȹ���� �� ��� particle system�� �߻��� �� ����. ���⳪ ���ذ��� ��.
	Idle ���¿� �ְų� ĳ���Ͱ� �� ���� ���� �۵�&ȹ���� �� ��� �߻��ϰԲ� �Ѵ�.Item Ŭ������ �������ν� ����, PickUp�� ��ӽ�Ų��.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particles")
	class UParticleSystemComponent* IdleParticlesComponent; 
	/*������Ʈ�� createdefaultsubobject�� ����� ���������� UParticleSystem�� ��� �׷� �ʿ� ����.�������Ʈ�� �˾Ƽ� ����
	Mesh�� ��쵵 ��������. => 'component'�� ���� ���� �� �Լ��� ����� �������� �װ��� ���� ��쿡�� �ƴ϶�� ��*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particles")
	class UParticleSystem* OverlapParticles;

	/*ĳ���Ͱ� ���Ϳ� ������ �ٰ����� �����̳� ������ �Բ� ������� �ϰ� �Ҹ��� ���̵� ���� �Ϸ���? USoundCue�� Ȱ���Ѵ�.
	���Ŀ� cpp ���Ͽ��� �� ���Ͱ� ĳ���Ϳ� ���� Overlap(��ħ)�Ǿ� ������� ���� �̸� Ȱ���� �Ҹ��� ���� ���� �Ѵ�*/
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Sounds")
	class USoundCue* OverlapSound;


	/*���� �����̳� ������ �� ���Ͱ� ȸ���ϴ� ȿ���� ������ �� ������ ���̴�. �׷��� Item�� ��ӹ޴� ��� Ŭ������ ȸ���� �ϸ� ����� �� �����Ƿ�
	�̸� ������ �� �ִ� boolean ���� �ش�.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | ItemProperties")
	bool bRotate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | ItemProperties")
	float RotationRate;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
