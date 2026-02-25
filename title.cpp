/*
	Title.h
	20250625  hanaue sho
*/
#include "Title.h"
#include "Manager.h"

#include "GameObject.h"
#include "Polygon.h"
#include "Camera.h"
#include "Keyboard.h"
#include "Input.h"
#include "InputSystem.h"
#include "Game.h"
#include "Result.h"
#include "GameMainScene.h"
#include "PlayroomScene.h"
#include "GameStage0Scene.h"
#include "FadeSpriteObject.h"
#include "Main.h"


void Title::Init()
{
	Scene::Init();

	Camera* pCamera = AddGameObject<Camera>(0);
	pCamera->Init();
	// titleTexture
	{
		Polygon2D* pPolygon = AddGameObject<Polygon2D>(2);
		pPolygon->Init();
		pPolygon->SetTexture("assets\\texture\\titleTexture.png");
		pPolygon->SetSize(SCREEN_WIDTH, SCREEN_HEIGHT, false);
	}
	{
		Polygon2D* pPolygon = AddGameObject<Polygon2D>(2);
		pPolygon->Init();
		pPolygon->SetTexture("assets\\texture\\pressAnyButton.png");
		pPolygon->SetSize(500.0f, 200.0f, true);
		pPolygon->SetBlink(true);
		pPolygon->Transform()->SetPosition({ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT /2.0f + 200.0f, 0 });
	}


	auto* fade = AddGameObject<FadeSpriteObject>(2);
	fade->Init();
	fade->FadeOut();
}

void Title::Uninit()
{
	Scene::Uninit();
}

void Title::Update(float gameDt, float realDt)
{
	Scene::Update(gameDt, realDt);

	// フェード確認
	auto* fade = GetGameObject<FadeSpriteObject>();


	if (Keyboard_IsKeyDownTrigger(KK_ENTER) || InputSystem::AnyButtonDown())
	{
		fade->FadeIn();
		m_IsFadeChangeScene = true;
		//Manager::SetScene<GameStage0Scene>();
		//Manager::SetScene<GameMainScene>();
		//Manager::SetScene<Game>();
		//Manager::SetScene<PlayroomScene>();
	}

	if (m_IsFadeChangeScene)
	{
		if (fade->EndFadeIn())
		{
			//Manager::SetScene<GameStage0Scene>();
			Manager::SetScene<GameMainScene>();
			//Manager::SetScene<Game>();
			//Manager::SetScene<PlayroomScene>();
			//Manager::SetScene<Result>();
		}
	}

}
