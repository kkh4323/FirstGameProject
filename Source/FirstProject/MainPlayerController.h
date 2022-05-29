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

	/*�� ü�¹� UI�� �����*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WEnemyHealthBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* EnemyHealthBar;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WPauseMenu;;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* PauseMenu;

	bool bEnemyHealthBarVisible;

	FVector EnemyLocation; //�� ������ġ���� ������ ����.
	


	//�� ü�»��¹ٸ� ���ų� ���ߴ� ������. bEnemyHealthBarVisible�� Display�� ��� true��, remove�� ��� false�� �ȴ�.
	void DisplayEnemyHealthBar();
	void RemoveEnemyHealthBar();

	bool bPauseMenuVisible;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void DisplayPauseMenu();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void RemovePauseMenu();

	void TogglePauseMenu();

	void GameModeOnly();



protected:
	virtual void BeginPlay() override; //�ٸ� Ŭ������� ��ġ�ϵ��� ����. protected�̹Ƿ� ��Ӱ����� ������ �̿� ���� ������ �����ϴ�.

	virtual void Tick(float DeltaTime) override;
};
