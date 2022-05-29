// Fill out your copyright notice in the Description page of Project Settings.


/*PlayerController : �÷��̾ ����ϴ� Pawn�� ��� ����ϴ� �����̴�.
���� �ϳ��� C++Ŭ�����̴�. Actor�� Characteró�� PlayerController��� Ŭ������ �𸮾��� ���� ���� �����ϴ�.
PlayerController�� Pawn�� ������ �ش�. ���⿡ ������ ������ �θ� Pawn�� PlayerController�� ������ �� ������ ǥ�õȴ�.

HUD�� �÷��̾� ����(ü�¹� ��)�� �κ��丮 �� ȭ�鿡 ��Ÿ���� ��� �����������̽��� ���Ѵ�.
HUD�� ǥ���ϴ� �����, HUD�� ���õ� ��� �͵��� PlayerController Ŭ������ �ִ� ���̴�.*/

#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"	//UserWidget�� ���Ƿ� �� ��������� ����

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDOverlayAsset) //�������Ʈ���� OverlayAsset�� Ȱ��ȭ��Ų�ٸ� �����ؾ� �� ��
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
	}
	HUDOverlay->AddToViewport();//�������Ʈ�� ����Ʈ�� HUDOverlay�� �߰��Ѵ�.
	HUDOverlay->SetVisibility(ESlateVisibility::Visible);	//�ڵ�� �ϸ� �����ڰ� ���ϴ� ��� Ư�� HUD�� ���̰ų� ������ �ʰ� ������ �� �ִ�. ESlateVisibility�� �׷� �뵵.

	
	if (WEnemyHealthBar) // �ｺ�� Ȱ��ȭ(��ȿ���� Ȯ��)
	{
		EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);
		if (EnemyHealthBar)
		{
			EnemyHealthBar->AddToViewport(); //�� �ｺ�ٸ� �������Ʈ ����Ʈ�� �߰�.
			EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden); //����Ʈ���� ������ �������� �ʵ��� ��.
		}
		FVector2D Alignment(0.f, 0.f);
		EnemyHealthBar->SetAlignmentInViewport(Alignment );
	}

	if (WPauseMenu) // �����޴� Ȱ��ȭ
	{
		PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);
		if (PauseMenu)
		{
			PauseMenu->AddToViewport();
			PauseMenu->SetVisibility(ESlateVisibility::Hidden);
		}

	}

}

void AMainPlayerController::DisplayEnemyHealthBar() //�ｺ�ٸ� ���� �Լ�. 
{
	if (EnemyHealthBar)
	{
		bEnemyHealthBarVisible = true;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Visible); 
	}
}
void AMainPlayerController::RemoveEnemyHealthBar()
{
	if (EnemyHealthBar) //�ｺ�ٸ� �����ϴ� �Լ�.
	{
		bEnemyHealthBarVisible = false;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
	}
}


void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);//ƽ �Լ� ���. �ｺ�ٸ� ���� ƽ ������ �ʿ��ϱ⿡ ���� ������ش�.
	if (EnemyHealthBar)
	{
		//3���� �������� ���� ��ġ�� ������ 2���� ������ ��ǥ�� ��ȯ��Ų��.
		FVector2D PositionInViewport;
		ProjectWorldLocationToScreen(EnemyLocation,PositionInViewport);//3���� ������ Ư�� ��ǥ�� �����´�. �� ��� ���� ��ǥ. �׷���� 2���� ��ǥ�� ��ȯ�Ѵ�.
		PositionInViewport.Y -= 70.f; //���¹� ��ġ ���.������ �Ʒ������� �����̹Ƿ� ���� �÷��ַ��� �����.
		
		FVector2D SizeInViewport = FVector2D(300.f, 25.f); //ü�¹� ǥ�� ũ��. ���� ��ܿ��� �����Ѵ�. ù��° �Ű������� ���������� �ŭ�� ũ������, �ι�° �Ű������� �Ʒ������� �ŭ�� ũ�������� ��Ÿ����.
		//FVector2D SizeInViewport(200.f, 25.f); => �̷� ����� �ִ�.

		EnemyHealthBar->SetPositionInViewport(PositionInViewport);
		EnemyHealthBar->SetDesiredSizeInViewport(SizeInViewport);
	}

}



void AMainPlayerController::DisplayPauseMenu_Implementation()	//_Implementation�� ���� �Լ��� �������Ʈ ���� �Լ��̴�. C++���� �װ��� ����Ϸ��� _Implementation�� �ٿ��� �Ѵ�.
//���� ������ ����� ������ �׳� �������Ʈ ���� �Լ��� ���⿣ ��� �׷���. �׳� �����ص� ��� ����.
{
	if (PauseMenu)
	{
		bPauseMenuVisible = true;
		PauseMenu->SetVisibility(ESlateVisibility::Visible);

		//�����޴��� ȣ������ �� �Է���ġ(���콺 Ŀ��)�� ��Ÿ������ �Ѵ�.
		FInputModeGameAndUI InputModeGameAndUI;
		bShowMouseCursor = true; //���콺 Ŀ���� ���̵��� ��ġ.(������ ���� �ִ� �Һ����̴�.)
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