/*
	Title.h
	20250625  hanaue sho
*/
#include "title.h"
#include "manager.h"

#include "GameObject.h"
#include "polygon.h"
#include "camera.h"
#include "keyboard.h"
#include "game.h"
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

void Title::Update(float dt)
{
	Scene::Update(dt);
	if (Keyboard_IsKeyDownTrigger(KK_ENTER))
	{
		Manager::SetScene<Game>();
		//Manager::SetScene<PlayroomScene>();
	}
}
