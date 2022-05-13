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

	/*언리얼 에디터에서 UMG 에셋을 참조할 수 있도록 설정하는 작업*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widgets")
	TSubclassOf<class UUserWidget> HUDOverlayAsset;/*TSubclassOf 는 PlayerControllerBlueprint를 블루프린트에서 선택할 수 있도록 한다.*/
	
	/*체력바와 같은 위젯을 만든 후, 그것이 사라지지 않도록 홀드하는 변수*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* HUDOverlay;

	/*적 체력바 UI를 만든다*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WEnemyHealthBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* EnemyHealthBar;

	bool bEnemyHealthBarVisible;

	FVector EnemyLocation; //적 백터위치값을 가지는 변수.
	
	//적 체력상태바를 띄우거나 감추는 제어기들. bEnemyHealthBarVisible은 Display의 경우 true가, remove의 경우 false가 된다.
	void DisplayEnemyHealthBar();
	void RemoveEnemyHealthBar();



protected:
	virtual void BeginPlay() override; //다른 클래스들과 일치하도록 설정. protected이므로 상속관계일 때에나 이에 대한 접근이 가능하다.

	virtual void Tick(float DeltaTime) override;
};
