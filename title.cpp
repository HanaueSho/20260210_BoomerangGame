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
#include "Game.h"
#include "GameMainScene.h"
#include "PlayroomScene.h"


void Title::Init()
{
	Scene::Init();

	Camera* pCamera = AddGameObject<Camera>(0);
	pCamera->Init();
	Polygon2D* pPolygon = AddGameObject<Polygon2D>(2);
	pPolygon->Init();

}

void Title::Uninit()
{
	Scene::Uninit();
}

void Title::Update(float gameDt, float realDt)
{
	Scene::Update(gameDt, realDt);
	if (Keyboard_IsKeyDownTrigger(KK_ENTER))
	{
		Manager::SetScene<GameMainScene>();
		//Manager::SetScene<Game>();
		//Manager::SetScene<PlayroomScene>();
	}

	if (Input::Pad(0).IsDown(PadButton::A))
		printf("%f\n", Input::Pad(0).LX());

}
