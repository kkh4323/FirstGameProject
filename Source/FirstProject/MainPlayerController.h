// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPROJECT_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()
public:

	/*�𸮾� �����Ϳ��� UMG ������ ������ �� �ֵ��� �����ϴ� �۾�*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widgets")
	TSubclassOf<class UUserWidget> HUDOverlayAsset;/*TSubclassOf �� PlayerControllerBlueprint�� �������Ʈ���� ������ �� �ֵ��� �Ѵ�.*/
	
	/*ü�¹ٿ� ���� ������ ���� ��, �װ��� ������� �ʵ��� Ȧ���ϴ� ����*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* HUDOverlay;

protected:
	virtual void BeginPlay() override; //�ٸ� Ŭ������� ��ġ�ϵ��� ����. protected�̹Ƿ� ��Ӱ����� ������ �̿� ���� ������ �����ϴ�.

};
