/*
	GameStage0Scene.cpp
	20260215  hanaue sho
*/
#include "GameStage0Scene.h"
#include "manager.h"
#include "renderer.h"
#include "keyboard.h"
#include "result.h"

// object
#include "Polygon.h"
#include "Field.h"
#include "Player.h"
#include "Enemy.h"
#include "LightObject.h"
#include "ModelObject.h"
#include "AppleObject.h"
#include "PlayerObject.h"
#include "SandbagObject.h"
#include "NorenObject.h"
#include "BoomerangObject.h"
#include "EnemyObject.h"
#include "TargetObject.h"
#include "TentObject.h"
#include "FenceObject.h"
#include "TreeObject.h"
#include "SkydomeObject.h"

// Component
#include "CameraFollowComponent.h"
#include "PlayerStateManagerComponent.h"
#include "BoomerangStateManagerComponent.h"
#include "Camera.h"
#include "ColliderComponent.h"

// Audio
#include "AudioSource.h"
#include "AudioBank.h"
#include "Random.h"

namespace
{
	const float myPI = 3.1415926535f;
}

void GameStage0Scene::Init()
{
	Scene::Init();

	// カメラ -----
	{
		Camera* pCamera = AddGameObject<Camera>(0);
		pCamera->Init();
		pCamera->GetComponent<CameraComponent>()->SetMode(CameraComponent::Mode::Perspective);
		pCamera->GetComponent<CameraComponent>()->
			SetPerspective(DirectX::XMConvertToRadians(60.0f), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 1000.0f);
		pCamera->Transform()->SetPosition({ 0, 5, -150 });
		pCamera->Transform()->SetEulerAngles({ 0.5, 0, 0 });
		pCamera->AddComponent<CameraFollowComponent>();

		auto* as = pCamera->AddComponent<AudioSource>();
		AudioBank::Pin("assets\\audio\\BGMresult.wav");
		as->SetClip(AudioBank::Get("assets\\audio\\BGMresult.wav"));
		as->SetVolume(0.5f);
		//as->Play();
	}
	{
		// 2Dカメラ
		Camera* pCamera = AddGameObject<Camera>(0);
		pCamera->Init();

		Polygon2D* pPolygon = AddGameObject<Polygon2D>(2);
		pPolygon->Init();
		pPolygon->Transform()->SetPosition({ 0, 0, 0 });

	}

	// プレイヤー -----
	PlayerObject* pPlayer = AddGameObject<PlayerObject>(1);
	pPlayer->Init();
	auto* psm = pPlayer->GetComponent<PlayerStateManagerComponent>();
	Camera* pCamera = GetGameObject<Camera>();
	auto* follow = pCamera->GetComponent<CameraFollowComponent>();
	follow->SetTargetObject(pPlayer);
	psm->SetCameraObject(pCamera);

	// ブーメラン
	BoomerangObject* pBoomerang = AddGameObject<BoomerangObject>(1);
	pBoomerang->Init();
	auto* state = pBoomerang->GetComponent<BoomerangStateManagerComponent>();
	state->SetPlayerObject(pPlayer);
	state->ChangeStateIdle();
	psm->SetBoomerangObject(pBoomerang); // Player Setter

	// メッシュフィールド
	Field* pField = AddGameObject<Field>(1);
	pField->Init();
	pField->Transform()->SetPosition({ -500, -5, -500 });

	// スカイドーム
	SkydomeObject* pSkydome = AddGameObject<SkydomeObject>(1);
	pSkydome->Init();

	// 柵
	CreateFences();

	// 木
	CreateTrees();

	// ライト関係
	LightApp light = {};
	light.enable = 1;
	light.diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	light.ambient = Vector4(0.2f, 0.2f, 0.2f, 1.0f);
	light.direction = Vector4(0.3f, -1.0f, 0.0f, 0.0f).normalized();
	Renderer::SetLight(light);

	// シェーダー関係
	ToonApp toon = MakeToon(ToonPreset::GravityRush2Like);
	Renderer::SetToon(toon);

	OutlineApp outline{};
	outline.outlineWidth = 0.05f;
	outline.outlineColor = Vector3(0.0f, 0.0f, 0.0f);
	Renderer::SetOutline(outline);

}

void GameStage0Scene::Uninit()
{
	Scene::Uninit();
}

void GameStage0Scene::Update(float dt)
{
	Scene::Update(dt);

	if (Keyboard_IsKeyDownTrigger(KK_ENTER))
	{
		Manager::SetScene<Result>();
	}

}

void GameStage0Scene::Draw()
{
	Scene::Draw();
}

void GameStage0Scene::CreateFences()
{
	float s = 2;
	Vector3 scale = { s, s, s };

	float radius = 200.0f;

	// n 角形を作る
	const int numVertex = 8 * 6;
	for (int i = 0; i < numVertex; i++)
	{
		float rad = myPI * 2.0f / numVertex * i;
		float x = cosf(rad) * radius;
		float z = sinf(rad) * radius;
		Vector3 position = { x, -5, z };

		FenceObject* pFence = AddGameObject<FenceObject>(1);
		pFence->Init();
		pFence->Transform()->SetPosition(position);
		pFence->Transform()->SetScale(scale);
		pFence->Transform()->RotateAxis({ 0, 1, 0 }, -rad + myPI / 2.0f);
	}

	// 横断
	for (int i = 0; i < 7; i++)
	{
		float rad = -myPI / 4.0f;
		float x = cosf(rad) * i * 30;
		float z = sinf(rad) * i * -30;
		Vector3 position = { x, -5, z };

		FenceObject* pFence = AddGameObject<FenceObject>(1);
		pFence->Init();
		pFence->Transform()->SetPosition(position);
		pFence->Transform()->SetScale(scale);
		pFence->Transform()->RotateAxis({ 0, 1, 0 }, -myPI / 4.0f);
		pFence->GetComponent<Collider>()->SetBox({ 6, 0.5f, 5 });
		pFence->GetComponent<Collider>()->SetOffsetPosition({ 0, 0.0f, 0 });
	}
	for (int i = 1; i < 7; i++)
	{
		float rad = -myPI / 4.0f;
		float x = cosf(rad) * i * -30;
		float z = sinf(rad) * i * 30;
		Vector3 position = { x, -5, z };

		FenceObject* pFence = AddGameObject<FenceObject>(1);
		pFence->Init();
		pFence->Transform()->SetPosition(position);
		pFence->Transform()->SetScale(scale);
		pFence->Transform()->RotateAxis({ 0, 1, 0 }, -myPI / 4.0f);
		pFence->GetComponent<Collider>()->SetBox({ 6, 0.5f, 5 });
		pFence->GetComponent<Collider>()->SetOffsetPosition({ 0, 0.0f, 0 });
	}

}

void GameStage0Scene::CreateTents()
{
	TentObject* pTent = AddGameObject<TentObject>(1);
	pTent->Init();
	float s = 7.0f;
	pTent->Transform()->SetScale({ s, s, s });
	pTent->Transform()->SetPosition({ 220, -5, 0 });
	pTent->Transform()->RotateAxis({ 0, 1, 0 }, PI / 2 * 3);
}

void GameStage0Scene::CreateTrees()
{

	float radiusBase = 280.0f;

	// n 角形を作る
	const int numVertex = 8 * 6;
	for (int j = 0; j < 5; j++)
	{
		float radius = radiusBase + 50 * Random::Random01() + j * 30.0f;
		float s = 3 - Random::Random01();
		for (int i = 0; i < numVertex; i++)
		{
			float rad = myPI * 2.0f / numVertex * i; rad += j * 2;
			float x = cosf(rad) * radius + 50.0f * Random::Random01();
			float z = sinf(rad) * radius + 50.0f * Random::Random01();
			Vector3 position = { x, -5, z };

			Vector3 scale = { s, s, s };

			TreeObject* pTree = AddGameObject<TreeObject>(1);
			pTree->Init();
			pTree->Transform()->SetPosition(position);
			pTree->Transform()->SetScale(scale);
			pTree->Transform()->RotateAxis({ 0, 1, 0 }, Random::RandomRange(0.0f, 6.0f));
		}
	}
}
