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
}
