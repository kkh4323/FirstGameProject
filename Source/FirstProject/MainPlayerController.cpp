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
}
