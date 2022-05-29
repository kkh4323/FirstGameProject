// Fill out your copyright notice in the Description page of Project Settings.


/*PlayerController : 플레이어가 사용하는 Pawn의 제어를 담당하는 액터이다.
역시 하나의 C++클래스이다. Actor나 Character처럼 PlayerController라는 클래스를 언리얼을 통해 생성 가능하다.
PlayerController는 Pawn에 영향을 준다. 여기에 정보를 저장해 두면 Pawn에 PlayerController에 저장해 둔 내용이 표시된다.

HUD는 플레이어 상태(체력바 등)나 인벤토리 등 화면에 나타나는 모든 유저인터페이스를 말한다.
HUD를 표시하는 방법은, HUD와 관련된 모든 것들을 PlayerController 클래스에 넣는 것이다.*/

#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"	//UserWidget을 쓰므로 이 헤더파일을 포함

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDOverlayAsset) //블루프린트에서 OverlayAsset을 활성화시킨다면 수행해야 할 것
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
	}
	HUDOverlay->AddToViewport();//블루프린트의 뷰포트에 HUDOverlay를 추가한다.
	HUDOverlay->SetVisibility(ESlateVisibility::Visible);	//코드로 하면 개발자가 원하는 대로 특정 HUD를 보이거나 보이지 않게 조정할 수 있다. ESlateVisibility는 그런 용도.

	
	if (WEnemyHealthBar) // 헬스바 활성화(유효한지 확인)
	{
		EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);
		if (EnemyHealthBar)
		{
			EnemyHealthBar->AddToViewport(); //적 헬스바를 블루프린트 뷰포트에 추가.
			EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden); //뷰포트에는 있으나 보이지는 않도록 함.
		}
		FVector2D Alignment(0.f, 0.f);
		EnemyHealthBar->SetAlignmentInViewport(Alignment );
	}

	if (WPauseMenu) // 정지메뉴 활성화
	{
		PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);
		if (PauseMenu)
		{
			PauseMenu->AddToViewport();
			PauseMenu->SetVisibility(ESlateVisibility::Hidden);
		}

	}

}

void AMainPlayerController::DisplayEnemyHealthBar() //헬스바를 띄우는 함수. 
{
	if (EnemyHealthBar)
	{
		bEnemyHealthBarVisible = true;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Visible); 
	}
}
void AMainPlayerController::RemoveEnemyHealthBar()
{
	if (EnemyHealthBar) //헬스바를 제거하는 함수.
	{
		bEnemyHealthBarVisible = false;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
	}
}


void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);//틱 함수 상속. 헬스바를 위한 틱 연산이 필요하기에 따로 만들어준다.
	if (EnemyHealthBar)
	{
		//3차원 공간에서 적의 위치를 가져와 2차원 공간의 좌표로 변환시킨다.
		FVector2D PositionInViewport;
		ProjectWorldLocationToScreen(EnemyLocation,PositionInViewport);//3차원 공간의 특정 좌표를 가져온다. 이 경우 적의 좌표. 그러고는 2차원 좌표로 변환한다.
		PositionInViewport.Y -= 70.f; //상태바 위치 상승.위에서 아래방향이 증가이므로 위로 올려주려면 빼쥰다.
		
		FVector2D SizeInViewport = FVector2D(300.f, 25.f); //체력바 표기 크기. 왼쪽 상단에서 시작한다. 첫번째 매개변수는 오른쪽으로 몇만큼의 크기인지, 두번째 매개변수는 아래쪽으로 몇만큼의 크기인지를 나타낸다.
		//FVector2D SizeInViewport(200.f, 25.f); => 이런 방법도 있다.

		EnemyHealthBar->SetPositionInViewport(PositionInViewport);
		EnemyHealthBar->SetDesiredSizeInViewport(SizeInViewport);
	}

}



void AMainPlayerController::DisplayPauseMenu_Implementation()	//_Implementation이 붙은 함수는 블루프린트 전용 함수이다. C++에서 그것을 사용하려면 _Implementation을 붙여야 한다.
//붉은 밑줄이 생기는 이유는 그냥 블루프린트 전용 함수라 여기엔 없어서 그렇다. 그냥 진행해도 상관 없다.
{
	if (PauseMenu)
	{
		bPauseMenuVisible = true;
		PauseMenu->SetVisibility(ESlateVisibility::Visible);

		//정지메뉴를 호출했을 때 입력장치(마우스 커서)가 나타나도록 한다.
		FInputModeGameAndUI InputModeGameAndUI;
		bShowMouseCursor = true; //마우스 커서가 보이도록 조치.(엔진에 원래 있는 불변수이다.)
		SetInputMode(InputModeGameAndUI);
	}
	
}

void AMainPlayerController::RemovePauseMenu_Implementation()
{
	if (PauseMenu)
	{

		bShowMouseCursor = false;
		bPauseMenuVisible = false;
		GameModeOnly();
	}
}

void AMainPlayerController::TogglePauseMenu()
{
	if (bPauseMenuVisible)
	{
		RemovePauseMenu();
	}
	else
	{
		DisplayPauseMenu();
	}
}

void AMainPlayerController::GameModeOnly()
{
	FInputModeGameOnly InputModeGameOnly;
	SetInputMode(InputModeGameOnly);
}