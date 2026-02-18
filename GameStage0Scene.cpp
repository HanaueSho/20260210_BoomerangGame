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
#include "EnemyAttackObject.h"
#include "FadeSpriteObject.h"
#include "ResultSpriteObject.h"
#include "WarpSceneObject.h"

// Component
#include "CameraFollowComponent.h"
#include "PlayerStateManagerComponent.h"
#include "BoomerangStateManagerComponent.h"
#include "Camera.h"
#include "ColliderComponent.h"
#include "EnemyModelAnimeObject.h""
#include "WarpSceneComponent.h""
#include "EnemyStateManagerComponent.h"

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

	// フェード
	auto* fade = AddGameObject<FadeSpriteObject>(2);
	fade->Init();
	fade->FadeOut();

	// ワープ
	auto* warp = AddGameObject<WarpSceneObject>(1);
	warp->Init();
	warp->Transform()->SetPosition({ 0, 0, -400 });
	warp->GetComponent<WarpSceneComponent>()->SetType(WarpSceneComponent::Type::Main);

	// リザルト
	m_pResultSprite = AddGameObject<ResultSpriteObject>(2);
	m_pResultSprite->Init();

	// 柵
	CreateFences();

	// 木
	CreateTrees();

	// エネミー（最初の一体目）
	m_pEnemyMeleeFirst = AddGameObject<EnemyObject>(1);
	m_pEnemyMeleeFirst->SetType(Type::Melee);
	m_pEnemyMeleeFirst->Init();
	m_pEnemyMeleeFirst->Transform()->SetPosition({ 30.0f, 10, 50.0f  });

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

void GameStage0Scene::Update(float gameDt, float realDt)
{
	Scene::Update(gameDt, realDt);

	if (Keyboard_IsKeyDownTrigger(KK_ENTER))
	{
		Manager::SetScene<GameMainScene>();
	}

	switch (m_State)
	{
	case State::First:
		if (m_pEnemyMeleeFirst->GetComponent<EnemyStateManagerComponent>()->GetState() == EnemyStateManagerComponent::State::Dead)
		{
			ChangeState(State::Second);
		}
		break;
	case State::Second:
		if (m_pEnemyShotSecond->GetComponent<EnemyStateManagerComponent>()->GetState() == EnemyStateManagerComponent::State::Dead)
		{
			ChangeState(State::Third);
		}
		break;
	case State::Third:
	{
		bool isClear = true;
		for (int i = 0; i < 3; i++)
		{
			if (m_pEnemyMeleeThird[i]->GetComponent<EnemyStateManagerComponent>()->GetState() != EnemyStateManagerComponent::State::Dead)
			{
				isClear = false;
				break;
			}
			if (m_pEnemyShotThird[i]->GetComponent<EnemyStateManagerComponent>()->GetState() != EnemyStateManagerComponent::State::Dead)
			{
				isClear = false;
				break;
			}
		}
		if (isClear) ChangeState(State::GameClear);
	}
		break;
	case State::GameClear:
		m_ClearTimer += realDt;
		if (m_ClearTimer > 3.0f)
		{
			auto* warp = GetGameObject<WarpSceneObject>();
			warp->GetComponent<WarpSceneComponent>()->ChangeScene(); // フェード
		}
		break;
	case State::GameOver:
		m_ClearTimer += realDt;
		if (m_ClearTimer > 3.0f)
		{
			auto* warp = GetGameObject<WarpSceneObject>();
			warp->GetComponent<WarpSceneComponent>()->ChangeScene(); // フェード
		}
		break;
	}

	auto* player = GetGameObject<PlayerObject>();
	if (player->GetComponent<PlayerStateManagerComponent>()->GetStateId() == PlayerStateId::Dead)
	{
		ChangeState(State::GameOver);
	}
}

void GameStage0Scene::Draw()
{
	Scene::Draw();
}

void GameStage0Scene::ChangeState(State newState)
{
	// 終了処理
	switch (m_State)
	{
	case State::First:
		break;
	case State::Second:
		break;
	case State::Third:
		break;
	case State::GameClear:
		break;
	case State::GameOver:
		break;
	}

	m_State = newState;
	// 開始処理
	switch (m_State)
	{
	case State::First:
		break;
	case State::Second:
	{
		m_pEnemyShotSecond = AddGameObject<EnemyObject>(1);
		m_pEnemyShotSecond->SetType(Type::Shot);
		m_pEnemyShotSecond->Init();
		m_pEnemyShotSecond->Transform()->SetPosition({ 30.0f, 10, 50.0f });
	}
		break;
	case State::Third:
	{
		for (int i = 0; i < 3; i++)
		{
			m_pEnemyMeleeThird[i] = AddGameObject<EnemyObject>(1);
			m_pEnemyMeleeThird[i]->SetType(Type::Melee);
			m_pEnemyMeleeThird[i]->Init();
			m_pEnemyMeleeThird[i]->Transform()->SetPosition({ 30.0f, 10, 50.0f + 20 * i });
			m_pEnemyShotThird[i] = AddGameObject<EnemyObject>(1);
			m_pEnemyShotThird[i]->SetType(Type::Shot);
			m_pEnemyShotThird[i]->Init();
			m_pEnemyShotThird[i]->Transform()->SetPosition({ -30.0f + 20 * i, 10, 50.0f + 20 * i });
		}
	}
		break;
	case State::GameClear:
		m_pResultSprite->SetType(ResultSpriteObject::Type::Success);
		m_pResultSprite->FadeIn();
		break;
	case State::GameOver:
		m_pResultSprite->SetType(ResultSpriteObject::Type::Failed);
		m_pResultSprite->FadeIn();
	break;
	}
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

	// 横置き
	for (int i = 0; i < 4; i++)
	{
		float x = 50 + 20.0f * i;
		float z = 50;
		Vector3 position = { x, -5, z };

		FenceObject* pFence = AddGameObject<FenceObject>(1);
		pFence->Init();
		pFence->Transform()->SetPosition(position);
		pFence->Transform()->SetScale(scale);
		pFence->Transform()->RotateAxis({ 0, 1, 0 }, 0);
		pFence->GetComponent<Collider>()->SetBox({ 6, 0.5f, 5 });
		pFence->GetComponent<Collider>()->SetOffsetPosition({ 0, 0.0f, 0 });
	}
	for (int i = 1; i < 4; i++)
	{
		float x = -50 - 20.0f * i;
		float z = -50;
		Vector3 position = { x, -5, z };

		FenceObject* pFence = AddGameObject<FenceObject>(1);
		pFence->Init();
		pFence->Transform()->SetPosition(position);
		pFence->Transform()->SetScale(scale);
		pFence->Transform()->RotateAxis({ 0, 1, 0 }, 0);
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
