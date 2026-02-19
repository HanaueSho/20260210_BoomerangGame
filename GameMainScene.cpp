/*
	GameMainScene.h
	20260211  hanaue sho
*/
#include "GameMainScene.h"
#include "manager.h"
#include "renderer.h"
#include "keyboard.h"
#include "Input.h"
#include "result.h"
#include "Random.h"

// object
#include "Polygon.h"
#include "Field.h"
#include "Player.h"
#include "Enemy.h"
#include "LightObject.h"
#include "ModelObject.h"
#include "AppleObject.h"
#include "BoxObject.h"
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
#include "FadeSpriteObject.h"
#include "WarpSceneObject.h"
#include "SignboardObject.h"
#include "DecoySwitchObject.h"
#include "OperateSpriteObject.h"
#include "RockObject.h"
#include "ChainObject.h"

// Component
#include "CameraFollowComponent.h"
#include "PlayerStateManagerComponent.h"
#include "BoomerangStateManagerComponent.h"
#include "Camera.h"
#include "ColliderComponent.h"
#include "WarpSceneComponent.h"

// Audio
#include "AudioSource.h"
#include "AudioBank.h"
#include "Random.h"
#include "GameStage0Scene.h"

namespace
{
	const float myPI = 3.1415926535f;
}

void GameMainScene::Init()
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
		AudioBank::Pin("assets\\audio\\BGM_Main.wav");
		as->SetClip(AudioBank::Get("assets\\audio\\BGM_Main.wav"));
		as->SetVolume(0.5f);
		as->Play();
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
	pField->SetHeight(2);
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
	{
		auto* warp = AddGameObject<WarpSceneObject>(1);
		warp->Init();
		warp->Transform()->SetPosition({ 180, 0, -50 });
		warp->GetComponent<WarpSceneComponent>()->SetType(WarpSceneComponent::Type::Stage0);
	}
	{
		auto* warp = AddGameObject<WarpSceneObject>(1);
		warp->Init();
		warp->Transform()->SetPosition({ 160, 0, -80 });
		warp->GetComponent<WarpSceneComponent>()->SetType(WarpSceneComponent::Type::Stage1);
	}

	//auto* decoySwitch = AddGameObject<DecoySwitchObject>(1);
	//decoySwitch->Init();
	//decoySwitch->Transform()->SetPosition({ -100, 0, -50 });

	//// ノレン
	//NorenObject* pNoren = AddGameObject<NorenObject>(1);
	//pNoren->Init();
	// 看板
	CreateSignboards();

	//デコイ
	CreateDecoies();

	// 柵
	CreateFences();

	// テント
	CreateTents();

	// 木
	CreateTrees();

	// りんご
	CreateApples();

	// 箱
	CreateBoxes();

	// 岩
	CreateRockes();

	// ライト
	CreateLights();

	// 鎖
	CreateChains();

	// ノレン
	CreateNorens();

	// ライト関係
	LightApp light = {};
	light.enable = 1;
	light.diffuse = Vector4(0.8f, 0.8f, 0.8f, 1.0f);
	light.ambient = Vector4(0.2f, 0.2f, 0.2f, 1.0f);
	light.direction = Vector4(0.3f, -1.0f, 0.2f, 0.0f).normalized();
	Renderer::SetLight(light);

	// シェーダー関係
	ToonApp toon = MakeToon(ToonPreset::GravityRush2Like);
	Renderer::SetToon(toon);

	OutlineApp outline{};
	outline.outlineWidth = 0.05f;
	outline.outlineColor = Vector3(0.0f, 0.0f, 0.0f);
	Renderer::SetOutline(outline);

}

void GameMainScene::Uninit()
{
	Scene::Uninit();
}

void GameMainScene::Update(float gameDt, float realDt)
{
	Scene::Update(gameDt, realDt);

	if (Keyboard_IsKeyDownTrigger(KK_ENTER) || Input::Pad(0).IsPressed(PadButton::START))
	{
		Manager::SetScene<Result>();
	}
}

void GameMainScene::Draw()
{
	Scene::Draw();
}


void GameMainScene::CreateSignboards()
{
	// 案内板
	{
		auto* signboard = AddGameObject<SignboardObject>(1);
		signboard->Init();
		signboard->Transform()->SetPosition({ 0, -3, -120 });
		signboard->Transform()->SetEulerAngles({ 3.141592f / 2 * 3, 3.141592f , 0 });

		auto* sprite = AddGameObject<OperateSpriteObject>(1);
		sprite->Init();
		sprite->SetTypeTexture(OperateSpriteObject::Type::Movement);
		sprite->Transform()->SetPosition({ 0, 15, -120 });
		sprite->Transform()->SetEulerAngles({ 0, 0 , 0 });
	}

	// 攻撃チュートリアル
	{
		auto* signboard = AddGameObject<SignboardObject>(1);
		signboard->Init();
		signboard->Transform()->SetPosition({ -70, -3, -40 });
		signboard->Transform()->SetEulerAngles({ 3.141592f / 2 * 3, 3.141592f / 3 * 3.5f , 0 });

		auto* sprite = AddGameObject<OperateSpriteObject>(1);
		sprite->Init();
		sprite->SetTypeTexture(OperateSpriteObject::Type::Throw);
		sprite->Transform()->SetPosition({ -70, 15, -40 });
		sprite->Transform()->SetEulerAngles({ 0, 3.141592f / 3 * 0.5f, 0 });
	}

	// ステージ０
	{
		auto* signboard = AddGameObject<SignboardObject>(1);
		signboard->Init();
		signboard->Transform()->SetPosition({ 190, -3, -50 });
		signboard->Transform()->SetEulerAngles({ 3.141592f / 2 * 3, 3.141592f / 2 * 3 , 0 });

		auto* sprite = AddGameObject<OperateSpriteObject>(1);
		sprite->Init();
		sprite->SetTypeTexture(OperateSpriteObject::Type::Stage1);
		sprite->Transform()->SetPosition({ 190, 20, -50 });
		sprite->Transform()->SetEulerAngles({ 0, 3.141592f / 2 * 1, 0 });
	}
	 
	// ステージ１
	{
		auto* signboard = AddGameObject<SignboardObject>(1);
		signboard->Init();
		signboard->Transform()->SetPosition({ 170, -3, -80 });
		signboard->Transform()->SetEulerAngles({ 3.141592f / 2 * 3, 3.141592f / 2 * 3 , 0 });

		auto* sprite = AddGameObject<OperateSpriteObject>(1);
		sprite->Init();
		sprite->SetTypeTexture(OperateSpriteObject::Type::Stage2);
		sprite->Transform()->SetPosition({ 170, 20, -80 });
		sprite->Transform()->SetEulerAngles({ 0, 3.141592f / 2 * 1, 0 });
	}

}

void GameMainScene::CreateDecoies()
{
	// エネミー（デコイ）
	for (int i = 0; i < 5; i++)
	{
		m_pDecoies[i] = AddGameObject<EnemyObject>(1);
		m_pDecoies[i]->SetType(Type::Decoy);
		m_pDecoies[i]->Init();
		m_pDecoies[i]->Transform()->SetPosition({ -170.0f + 30.0f * i, 0.0f, -30 + 30.0f * i });
	}
}

void GameMainScene::DestroyDecoies()
{
	for (int i = 0; i < 5; i++)
	{
		m_pDecoies[i]->RequestDestroy();
	}
}

void GameMainScene::CreateFences()
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
		float x = cosf(rad) * i *  30;
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
		if (i == 4) continue;
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

void GameMainScene::CreateTents()
{
	TentObject* pTent = AddGameObject<TentObject>(1);
	pTent->Init();
	float s = 7.0f;
	pTent->Transform()->SetScale({s, s, s});
	pTent->Transform()->SetPosition({220, -5, 0});
	pTent->Transform()->RotateAxis({0, 1, 0}, PI / 2 * 3);
}

void GameMainScene::CreateTrees()
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
			float x = cosf(rad) * radius +(50.0f - 100.0f * Random::Random01());
			float z = sinf(rad) * radius +(50.0f - 100.0f * Random::Random01());
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

void GameMainScene::CreateApples()
{
	// スタートから左へ
	{
		Vector3 position = { 0, 3, -150 };
		Vector3 vect = { -1, 0, 1 }; vect.normalize();
		for (int i = 0; i < 10; i++)
		{
			Vector3 scale = { 2, 2, 2 };
			float random = Random::Random01() * 0.5f;
			scale.x += random;
			scale.y += random;
			scale.z += random;
			position += vect * 10.0f;
			auto* apple = AddGameObject<AppleObject>(1);
			apple->Init();
			apple->Transform()->SetPosition(position);
			apple->Transform()->SetScale(scale);
		}
	}

	// 奥
	{
		Vector3 position = { 130, 10, 80 };
		for (int i = 0; i < 10; i++)
		{
			Vector3 scale = { 2, 2, 2 };
			float random = Random::Random01() * 0.5f;
			scale.x += random;
			scale.y += random;
			scale.z += random;
			position.x += random;
			position.z += random;
			auto* apple = AddGameObject<AppleObject>(1);
			apple->Init();
			apple->Transform()->SetPosition(position);
			apple->Transform()->SetScale(scale);
		}
	}
}

void GameMainScene::CreateBoxes()
{
	// 奥の方
	{
		Vector3 position = { 20, 5, -20 };
		Vector3 vect = { -1, 0, 1 }; vect.normalize();
		for (int i = 0; i < 6; i++)
		{
			Vector3 scale = { 8, 8, 8 };
			float random = Random::Random01() * 3.0f;
			scale.x += random;
			scale.y += random;
			scale.z += random;
			position.x += random;
			position.z += random;
			auto* box = AddGameObject<BoxObject>(1);
			box->Init();
			box->Transform()->SetPosition(position);
			box->Transform()->SetScale(scale);
		}
	}
}

void GameMainScene::CreateRockes()
{
	// 奥の方
	{
		Vector3 position = { 100, -6, 0 };
		Vector3 scale = { 100, 6, 100 };
		auto* rock = AddGameObject<RockObject>(1);
		rock->Init();
		rock->Transform()->SetPosition(position);
		rock->Transform()->SetScale(scale);
	}
	// 斜め
	{
		Vector3 position = { 100, -10, 50 };
		Vector3 scale = { 70, 20, 60 };
		auto* rock = AddGameObject<RockObject>(1);
		rock->Init();
		rock->Transform()->SetPosition(position);
		rock->Transform()->SetScale(scale);
		rock->Transform()->Rotate({0, 3.141592f/2, -0.3f });
	}
	{
		Vector3 position = { 150, -6, 70 };
		Vector3 scale = { 70, 20, 60 };
		auto* rock = AddGameObject<RockObject>(1);
		rock->Init();
		rock->Transform()->SetPosition(position);
		rock->Transform()->SetScale(scale);
		rock->Transform()->Rotate({-0.0f, 0, 0.4f});
	}
	{
		Vector3 position = { 130, 3, 100 };
		Vector3 scale = { 70, 20, 60 };
		auto* rock = AddGameObject<RockObject>(1);
		rock->Init();
		rock->Transform()->SetPosition(position);
		rock->Transform()->SetScale(scale);
		rock->Transform()->Rotate({0.0f, 0.2f, 0.0f});
	}
	{
		Vector3 position = { 90, 3, 100 };
		Vector3 scale = { 70, 30, 40 };
		auto* rock = AddGameObject<RockObject>(1);
		rock->Init();
		rock->Transform()->SetPosition(position);
		rock->Transform()->SetScale(scale);
		rock->Transform()->Rotate({0.0f, 0.0f, -0.6f});
	}
	{
		Vector3 position = { 90, 3, 130 };
		Vector3 scale = { 70, 30, 40 };
		auto* rock = AddGameObject<RockObject>(1);
		rock->Init();
		rock->Transform()->SetPosition(position);
		rock->Transform()->SetScale(scale);
		rock->Transform()->Rotate({0.0f, 0.0f, -0.9f});
	}
}

void GameMainScene::CreateLights()
{
	{
		auto* light = AddGameObject<LightObject>(1);
		light->Init();
		light->Transform()->SetPosition({ -150.0f, 10, 10 });
		light->SetColorWhite();
	}
	{
		auto* light = AddGameObject<LightObject>(1);
		light->Init();
		light->Transform()->SetPosition({ -120.0f, 10, 40 });
		light->SetColorRed();
	}
	{
		auto* light = AddGameObject<LightObject>(1);
		light->Init();
		light->Transform()->SetPosition({ -90.0f, 10, 70 });
		light->SetColorBlue();
	}
	{
		auto* light = AddGameObject<LightObject>(1);
		light->Init();
		light->Transform()->SetPosition({ -60.0f, 10, 100 });
		light->SetColorGreen();
	}


}

void GameMainScene::CreateChains()
{
	for(int i = 0; i < 0; i++)
	{
		auto* chain = AddGameObject<ChainObject>(1);
		chain->Init();
		chain->Transform()->SetPosition({ -90 - 5.0f * i, 17, -110 - 5.0f * i });
		chain->Transform()->SetScale({ 1, 1, 1 });
		chain->CreateChains(5);
	}
}

void GameMainScene::CreateNorens()
{
	for (int i = 0; i < 0; i++)
	{
		auto* noren = AddGameObject<NorenObject>(1);
		noren->Init();
		noren->Transform()->SetPosition({ -95 + 6.0f * i, 10, -95 + 6.0f * i });
		noren->Transform()->RotateAxis({ 0,1,0 }, 3.1415926535f / 4);
		noren->SetupBones();
	}
}
