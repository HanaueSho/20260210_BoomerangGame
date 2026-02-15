/*
	AimObject.cpp
	20260212  hanaue sho
	エイム中に敵に狙いをつけるオブジェクト
	カメラを親として設定する
*/
#include "AimObject.h"
#include "ColliderComponent.h"
#include "AimStateManagerComponent.h"
#include "Manager.h"
#include "Scene.h"
#include "Camera.h"
#include "Keyboard.h"
#include "AimSpriteObject.h"

void AimObject::Init()
{
	GameObject::Init();

	// コライダー
	auto* col = AddComponent<Collider>();
	float range = 200.0f;
	col->SetCapsule(10, range);
	col->SetOffsetRotation(Quaternion::FromEulerAngles({ 3.1415926535f / 2.0f, 0, 0 }));
	col->SetOffsetPosition({ 0, 0, range/2 });
	col->SetModeTrigger();

	// AimStateManagerComponet
	auto* state = AddComponent<AimStateManagerComponent>();
	state->Init();

	// 親設定
	GameObject* pCamera = Manager::GetScene()->GetGameObject<Camera>();
	Transform()->SetParent(pCamera->Transform());

	// エイムスプライト
	m_pAimSprite = Manager::GetScene()->AddGameObject<AimSpriteObject>(2);
	m_pAimSprite->Init();
}

void AimObject::Update(float gameDt, float realDt)
{
	GameObject::Update(gameDt, realDt);

}

