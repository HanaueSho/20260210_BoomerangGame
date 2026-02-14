/*
	GameMainScene.h
	20260211  hanaue sho
*/
#include "GameMainScene.h"
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

// Component
#include "CameraFollowComponent.h"
#include "PlayerStateManagerComponent.h"
#include "BoomerangStateManagerComponent.h"
#include "Camera.h"
#include "ColliderComponent.h"

// Audio
#include "AudioSource.h"
#include "AudioBank.h"


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
		pCamera->Transform()->SetPosition({ 0, 5, -15 });
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

	LightObject* pLightObject = AddGameObject<LightObject>(1);
	pLightObject->Init();
	pLightObject->Transform()->SetPosition({ 4, 0, -4 });
	/*pLightObject = AddGameObject<LightObject>(1);
	pLightObject->Init();
	pLightObject->Transform()->SetPosition({ -7, 0, 8 });
	pLightObject = AddGameObject<LightObject>(1);
	pLightObject->Init();
	pLightObject->Transform()->SetPosition({ -7, 0, -8 });
	pLightObject = AddGameObject<LightObject>(1);
	pLightObject->Init();
	pLightObject->Transform()->SetPosition({ -7, 0, 0 });*/

	// プレイヤー -----
	PlayerObject* pPlayer = AddGameObject<PlayerObject>(1);
	pPlayer->Init();
	auto* psm = pPlayer->GetComponent<PlayerStateManagerComponent>();
	Camera* pCamera = GetGameObject<Camera>();
	auto* follow = pCamera->GetComponent<CameraFollowComponent>();
	follow->SetTargetObject(pPlayer);
	psm->SetCameraObject(pCamera);

	//// サンドバッグ
	//SandbagObject* pSandbag = AddGameObject<SandbagObject>(1);
	//pSandbag->Init();

	//// ノレン
	//NorenObject* pNoren = AddGameObject<NorenObject>(1);
	//pNoren->Init();

	// ブーメラン
	BoomerangObject* pBoomerang = AddGameObject<BoomerangObject>(1);
	pBoomerang->Init();
	auto* state = pBoomerang->GetComponent<BoomerangStateManagerComponent>();
	state->SetPlayerObject(pPlayer);
	state->ChangeStateIdle();
	psm->SetBoomerangObject(pBoomerang); // Player Setter

	// エネミー
	TargetObject* pEnemyObject = AddGameObject<TargetObject>(1);
	pEnemyObject->Init();
	pEnemyObject = AddGameObject<TargetObject>(1);
	pEnemyObject->Init();
	pEnemyObject->Transform()->SetPosition({ 0, 10, 90 });

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
	pEnemy->Transform()->SetPosition({ 5, 3, 0 });
	pEnemy->Transform()->SetEulerAngles({ 0, 0, 0 });
	pEnemy->GetComponent<Rigidbody>()->SetRestitution(0.0f);
	pEnemy->GetComponent<Rigidbody>()->SetBodyTypeDynamic();
	pEnemy->GetComponent<Collider>()->SetOffsetPosition({ 2, 0, 0 });
	pEnemy->GetComponent<Collider>()->SetOffsetRotation(Quaternion::FromEulerAngles({ 0, 1.57, 0 }));
	//pEnemy->Transform()->SetParent(pPlayer->Transform()); 

	/*auto* djc = pEnemy->AddComponent<DistanceJointComponent>();
	djc->SetOtherObject(pPlayer);
	djc->SetRestLength(3.0f);
	djc->SetMode(DistanceJointComponent::DistanceJointMode::Rope);
	djc->RegisterToPhysicsSystem();*/

	/*auto* bjc = pEnemy->AddComponent<BallJointComponent>();
	bjc->SetOtherObject(pApple0);
	bjc->SetLocalAnchorA(Vector3(0, 0, 0));
	bjc->SetLocalAnchorB(Vector3(2, 0, 0));
	bjc->RegisterToPhysicsSystem();*/

	/*auto* hjc = pEnemy->AddComponent<HingeJointComponent>();
	hjc->SetOtherObject(pPlayer);
	hjc->SetLocalAnchorA(Vector3(3, 0, 0));
	hjc->SetAxis({ 0, 1, 0 });
	hjc->SetLocalRefA({ 0, 1, 0 });
	hjc->SetEnableLimit(false);
	hjc->SetLimitMin(-1.0f);
	hjc->SetLimitMax(1.0f);
	hjc->SetEnableMotor(false);
	hjc->SetMotorSpeed(20.0f);
	hjc->SetMaxMotorTorque(100.0f);
	hjc->SetEnableSpring(false);
	hjc->SetSpringDamping(0.9f);
	hjc->SetSpringTarget(3);
	hjc->RegisterToPhysicsSystem();*/

	// 床
	pEnemy = AddGameObject<Enemy>(1);
	pEnemy->Init();
	pEnemy->Transform()->SetPosition({ 0, -10, 0 });
	pEnemy->Transform()->SetScale({ 10, 5, 10 });
	pEnemy->Transform()->SetEulerAngles({ 0, 0, 0.1f });
	pEnemy->GetComponent<Collider>()->SetBox({ 1, 1, 1 });
	pEnemy->GetComponent<Rigidbody>()->SetBodyType(Rigidbody::BodyType::Static); // 壁
	//djc = pEnemy->AddComponent<DistanceJointComponent>();
	//djc->SetOtherObject(pPlayerEx);
	//djc->SetLocalAnchorA(Vector3(0, 5, 0));
	//djc->SetRestLength(3.0f);
	//djc->SetMode(DistanceJointComponent::DistanceJointMode::Rope);
	//djc->RegisterToPhysicsSystem();

	// 壁４方
	pEnemy = AddGameObject<Enemy>(1);
	pEnemy->Init();
	pEnemy->Transform()->SetPosition({ 10.5f, -4, 0 });
	pEnemy->Transform()->SetScale({ 1, 5, 10 });
	pEnemy->GetComponent<Collider>()->SetBox({ 1, 1, 1 });
	pEnemy->GetComponent<Rigidbody>()->SetBodyType(Rigidbody::BodyType::Static); // 壁

	pEnemy = AddGameObject<Enemy>(1);
	pEnemy->Init();
	pEnemy->Transform()->SetPosition({ -10.5f, -4, 0 });
	pEnemy->Transform()->SetScale({ 1, 1, 10 });
	pEnemy->GetComponent<Collider>()->SetBox({ 1, 1, 1 });
	pEnemy->GetComponent<Rigidbody>()->SetBodyType(Rigidbody::BodyType::Static); // 壁

	pEnemy = AddGameObject<Enemy>(1);
	pEnemy->Init();
	pEnemy->Transform()->SetPosition({ 0, -4, 10.5f });
	pEnemy->Transform()->SetScale({ 10, 1, 1 });
	pEnemy->GetComponent<Collider>()->SetBox({ 1, 1, 1 });
	pEnemy->GetComponent<Rigidbody>()->SetBodyType(Rigidbody::BodyType::Static); // 壁

	pEnemy = AddGameObject<Enemy>(1);
	pEnemy->Init();
	pEnemy->Transform()->SetPosition({ 0, -4, -10.5f });
	pEnemy->Transform()->SetScale({ 10, 1, 1 });
	pEnemy->GetComponent<Collider>()->SetBox({ 1, 1, 1 });
	pEnemy->GetComponent<Rigidbody>()->SetBodyType(Rigidbody::BodyType::Static); // 壁

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
	pField->Transform()->SetPosition({ -500, -5, -500 });

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

void GameMainScene::Update(float dt)
{
	Scene::Update(dt);

	if (Keyboard_IsKeyDownTrigger(KK_ENTER))
	{
		Manager::SetScene<Result>();
	}

}

void GameMainScene::Draw()
{
	Scene::Draw();
}
