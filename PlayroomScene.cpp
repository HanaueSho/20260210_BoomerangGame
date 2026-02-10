/*
	PlayroomScene.cpp
	20260202  hanaue sho
*/
#include "PlayroomScene.h"
#include "manager.h"
#include "renderer.h"
#include "keyboard.h"
#include "result.h"

#include "polygon.h"
#include "field.h"
#include "camera.h"
#include "player.h"
#include "enemy.h"
#include "LightObject.h"
#include "ModelObject.h"
#include "AppleObject.h"
#include "PlayerObject.h"
#include "SandbagObject.h"
#include "NorenObject.h"
#include "CubeObject.h"
#include "AppleGenerator.h"
#include "AutoRotateComponent.h"
#include "KomaComponent.h"

#include "AudioSource.h"
#include "AudioBank.h"

#include "DistanceJointComponent.h"
#include "BallJointComponent.h"
#include "HingeJointComponent.h"

void PlayroomScene::Init()
{
	Scene::Init();

	// カメラ -----
	Camera* pCamera = AddGameObject<Camera>(0);
	pCamera->Init();
	pCamera->GetComponent<CameraComponent>()->SetMode(CameraComponent::Mode::Perspective);
	pCamera->GetComponent<CameraComponent>()->
		SetPerspective(DirectX::XMConvertToRadians(60.0f), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 1000.0f);
	pCamera->Transform()->SetPosition({ 0, 10, -15 });
	pCamera->Transform()->SetEulerAngles({ 0.5, 0, 0 });
	auto* as = pCamera->AddComponent<AudioSource>();
	AudioBank::Pin("assets\\audio\\BGMresult.wav");
	as->SetClip(AudioBank::Get("assets\\audio\\BGMresult.wav"));
	as->SetVolume(0.5f);
	//as->Play();

	

	CreateSection_0();
	CreateSection_1();
	CreateSection_2();
	CreateSection_4();
	CreateSection_5();

	// りんご-----
	AppleObject* pApple = AddGameObject<AppleObject>(1);
	pApple->Init();
	pApple->Transform()->SetPosition({ 3, -3, 3 });
	pApple->Transform()->SetEulerAngles({ 0, 0, 0 });

	AppleObject* pApple0;
	AppleObject* pAppleEx = pApple;
	for (int x = 0; x < 4; x++)
	{
		for (int z = 0; z < 2; z++)
		{
			for (int y = 0; y < 1; y++)
			{
				pApple0 = AddGameObject<AppleObject>(1);
				pApple0->Init();
				pApple0->Transform()->SetPosition({ -6.0f + x * 2.0f, y * 1.0f, -8.0f + z * 2.0f });

				//auto* djc = pPlayer0->AddComponent<DistanceJointComponent>();
				//djc->SetOtherObject(pPlayer);
				//djc->SetRestLength(3.0f);
				//djc->SetMode(DistanceJointComponent::DistanceJointMode::Rope);
				//djc->RegisterToPhysicsSystem();
				//pPlayer = pPlayer0;
			}
		}
	}

	// エネミー -----
	Enemy* pEnemy = AddGameObject<Enemy>(1);
	pEnemy->Init();
	pEnemy->Transform()->SetPosition({ 0, 3, 0 });
	pEnemy->Transform()->SetEulerAngles({ 0, 0, 0 });
	pEnemy->GetComponent<Rigidbody>()->SetRestitution(0.0f);
	pEnemy->GetComponent<Rigidbody>()->SetBodyTypeDynamic();
	pEnemy->GetComponent<Collider>()->SetOffsetPosition({ 0, 0, 0 });
	pEnemy->GetComponent<Collider>()->SetOffsetRotation(Quaternion::FromEulerAngles({ 0, 1.57, 0 }));
	//pEnemy->Transform()->SetParent(pPlayer->Transform()); 



	// ライト関係
	LightApp light = {};
	light.enable = 1;
	light.diffuse = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	light.ambient = Vector4(0.2f, 0.2f, 0.2f, 1.0f);
	light.direction = Vector4(0.3f, -1.0f, 0.0f, 0.0f).normalized();
	Renderer::SetLight(light);

	// メッシュフィールド
	Field* pField = AddGameObject<Field>(1);
	pField->Init();
	pField->Transform()->SetPosition({ -40, -7, -40 });

	// シェーダー関係
	ToonApp toon = MakeToon(ToonPreset::GravityRush2Like);
	Renderer::SetToon(toon);

	OutlineApp outline{};
	outline.outlineWidth = 0.05f;
	outline.outlineColor = Vector3(0.0f, 0.0f, 0.0f);
	Renderer::SetOutline(outline);
}

void PlayroomScene::Uninit()
{
	Scene::Uninit();
}

void PlayroomScene::Update(float dt)
{
	Scene::Update(dt);

	if (Keyboard_IsKeyDownTrigger(KK_ENTER))
	{
		Manager::SetScene<Result>();
	}
}

void PlayroomScene::Draw()
{
	Scene::Draw();
}

void PlayroomScene::CreateSection_0()
{
	Vector3 worldOffset = { 50, 0, 0 };
	Vector3 position = { 0, 0, 0 };
	// プレイヤー配置 -----
	PlayerObject* pPlayer = AddGameObject<PlayerObject>(1);
	position = Vector3(0, 0, 0) + worldOffset;
	pPlayer->Transform()->SetPosition(position);
	pPlayer->Init();
	// リンゴ
	//AppleObject* pApple = AddGameObject<AppleObject>(1);
	//position = Vector3(0, 5, 0) + worldOffset;
	//pApple->Init();
	//pApple->Transform()->SetPosition(position);
	//pApple->Transform()->SetScale({1, 1, 1});
	//pApple->Transform()->SetEulerAngles({1, 0.5, 1});
	//pApple->GetComponent<Rigidbody>()->SetBodyTypeKinematic();


	// サンドバッグ
	SandbagObject* pSandbag = AddGameObject<SandbagObject>(1);
	position = Vector3(-10, 15, 0) + worldOffset;
	pSandbag->Transform()->SetPosition(position);
	pSandbag->Init();

	// ノレン
	NorenObject* pNoren = AddGameObject<NorenObject>(1);
	position = Vector3(0, 13, 5) + worldOffset;
	pNoren->Transform()->SetPosition(position);
	pNoren->Init();
	pNoren = AddGameObject<NorenObject>(1);
	position = Vector3(4, 13, 5) + worldOffset;
	pNoren->Transform()->SetPosition(position);
	pNoren->Init();

	// カプセル
	for (int i = 0; i < 5; i ++)
	{
		LightObject* pLightObject = AddGameObject<LightObject>(1);
		pLightObject->Init();
		position = Vector3(0 + i * 2, 0, 0) + worldOffset;
		pLightObject->Transform()->SetPosition(position);
	}

	// 床
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(0, -1, 0) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 40, 2, 40 });
		auto* rigid = pCube->GetComponent<Rigidbody>();
		rigid->SetBodyTypeStatic();
	}
	// 奥壁
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(0, -1, 20) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 40, 20, 1 });
		auto* rigid = pCube->GetComponent<Rigidbody>();
		rigid->SetBodyTypeStatic();
	}
	// 手前壁
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(0, -1, -20) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 40, 20, 1 });
		auto* rigid = pCube->GetComponent<Rigidbody>();
		rigid->SetBodyTypeStatic();
	}
	// 右壁
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(20, -1, 0) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 1, 20, 40 });
		auto* rigid = pCube->GetComponent<Rigidbody>();
		rigid->SetBodyTypeStatic();
	}
	// 左壁
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(-20, -1, 0) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 1, 20, 40 });
		auto* rigid = pCube->GetComponent<Rigidbody>();
		rigid->SetBodyTypeStatic();
	}
}
// 坂道
void PlayroomScene::CreateSection_1()
{
	Vector3 worldOffset = { 20, 10, 30 };
	Vector3 position = { 0, 0, 0 };
	// 坂道配置
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(0, 20, 0) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 20, 1, 10 });
		pCube->Transform()->SetEulerAngles({ 0, 0, -0.7 });
		pCube->GetComponent<Rigidbody>()->SetBodyType(Rigidbody::BodyType::Kinematic); // 壁
	}
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(10, 10, 0) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 20, 1, 10 });
		pCube->Transform()->SetEulerAngles({ 0, 0, 0.7 });
		pCube->GetComponent<Rigidbody>()->SetBodyType(Rigidbody::BodyType::Kinematic); // 壁
	}
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(0, 0, 0) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 20, 1, 10 });
		pCube->Transform()->SetEulerAngles({ 0, 0, -0.7 });
		pCube->GetComponent<Rigidbody>()->SetBodyType(Rigidbody::BodyType::Kinematic); // 壁
	}
	// ジェネレーター
	{
		AppleGenerator* pGenerator = AddGameObject<AppleGenerator>(1);
		position = Vector3(0, 25, -3) + worldOffset;
		pGenerator->Transform()->SetPosition(position);
		pGenerator->Init();
		pGenerator->SetMoveVector({ 0, 0, 5 });
		pGenerator->SetInterval(1.5f);
	}
}
// 回転バー
void PlayroomScene::CreateSection_2()
{
	Vector3 worldOffset = { 0, 0, 40 };
	Vector3 position = { 0, 0, 0 };
	// 回転体
	auto* pCube = AddGameObject<CubeObject>(1);
	pCube->Init();
	position = Vector3(0, 0, 0) + worldOffset;
	pCube->Transform()->SetPosition(position);
	pCube->Transform()->SetScale({ 15, 1, 5 });
	pCube->GetComponent<Rigidbody>()->SetBodyType(Rigidbody::BodyType::Kinematic); // 壁
	auto* ar = pCube->AddComponent<AutoRotateComponent>();
	ar->SetAxis({ 0, 0, 1 });
	ar->SetSpeed(5);

	// ジェネレーター
	{
		AppleGenerator* pGenerator = AddGameObject<AppleGenerator>(1);
		position = Vector3(5, 10, -3) + worldOffset;
		pGenerator->Transform()->SetPosition(position);
		pGenerator->Init();
		pGenerator->SetMoveVector({ 0, 0, 4 });
		pGenerator->SetInterval(3);
	}
}
// メッシュフィールド
void PlayroomScene::CreateSection_3()
{
}
// こま
void PlayroomScene::CreateSection_4()
{
	Vector3 worldOffset = { 10, 0, -40 };
	Vector3 position = { 0, 0, 0 };

	// 独楽
	for (int i = 0; i < 4; i++)
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(0 + i * 5, 10, 0) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 2, 2, 2 });
		pCube->Transform()->SetEulerAngles({ 1, 1, 1 });
		auto* rigid = pCube->GetComponent<Rigidbody>();
		rigid->SetBodyTypeDynamic();
		auto* koma = pCube->AddComponent<KomaComponent>();
		koma->SetTorquePower(800);
	}
	
	// 床
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(0, -1, 0) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 40, 2, 40 });
		auto* rigid = pCube->GetComponent<Rigidbody>();
		rigid->SetBodyTypeStatic();
	}
	// 奥壁
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(0, -1, 20) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 40, 20, 1 });
		auto* rigid = pCube->GetComponent<Rigidbody>();
		rigid->SetBodyTypeStatic();
	}
	// 手前壁
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(0, -1, -20) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 40, 20, 1 });
		auto* rigid = pCube->GetComponent<Rigidbody>();
		rigid->SetBodyTypeStatic();
	}
	// 右壁
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(20, -1, 0) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 1, 20, 40 });
		auto* rigid = pCube->GetComponent<Rigidbody>();
		rigid->SetBodyTypeStatic();
	}
	// 左壁
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(-20, -1, 0) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 1, 20, 40 });
		auto* rigid = pCube->GetComponent<Rigidbody>();
		rigid->SetBodyTypeStatic();
	}
}
// 跳ねる床
void PlayroomScene::CreateSection_5()
{
	Vector3 worldOffset = { -40, 0, 0 };
	Vector3 position = { 0, 0, 0 };

	// 床
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(0, -1, 0) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 40, 2, 40 });
		auto* rigid = pCube->GetComponent<Rigidbody>();
		rigid->SetBodyTypeStatic();
		rigid->SetRestitution(2.0f);
	}
	// 奥壁
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(0, 5, 20) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 40, 20, 1 });
		pCube->Transform()->SetEulerAngles({ 1.0f, 0, 0 });
		auto* rigid = pCube->GetComponent<Rigidbody>();
		rigid->SetBodyTypeStatic();
		rigid->SetRestitution(2.0f);
	}
	// 手前壁
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(0, 5, -20) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 40, 20, 1 });
		pCube->Transform()->SetEulerAngles({ -1.0f, 0, 0 });
		auto* rigid = pCube->GetComponent<Rigidbody>();
		rigid->SetBodyTypeStatic();
		rigid->SetRestitution(2.0f);
	}
	// 右壁
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(20, 5, 0) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 1, 20, 40 });
		pCube->Transform()->SetEulerAngles({ 0, 0, -1.0f });
		auto* rigid = pCube->GetComponent<Rigidbody>();
		rigid->SetBodyTypeStatic();
		rigid->SetRestitution(2.0f);
	}
	// 左壁
	{
		auto* pCube = AddGameObject<CubeObject>(1);
		pCube->Init();
		position = Vector3(-20, 5, 0) + worldOffset;
		pCube->Transform()->SetPosition(position);
		pCube->Transform()->SetScale({ 1, 20, 40 });
		pCube->Transform()->SetEulerAngles({ 0, 0, 1.0f });
		auto* rigid = pCube->GetComponent<Rigidbody>();
		rigid->SetBodyTypeStatic();
		rigid->SetRestitution(2.0f);
	}
	// ジェネレーター
	{
		AppleGenerator* pGenerator = AddGameObject<AppleGenerator>(1);
		position = Vector3(20, 30, -20) + worldOffset;
		pGenerator->Transform()->SetPosition(position);
		pGenerator->Init();
		pGenerator->SetMoveVector({ 0, 0, 40 });
		pGenerator->SetAppleScale(2);
		pGenerator->SetInterval(1);
	}
}
