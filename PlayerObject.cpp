/*
	PlayerObejct.cpp
	20260201 hanaue sho
*/
#include "PlayerObject.h"

#include "RigidbodyComponent.h"
#include "ColliderComponent.h"
#include "Keyboard.h"
#include "CharacterControllerComponent.h"
#include "PlayerStateManagerComponent.h"
#include "ModelAnimeObject.h"
#include "Manager.h"
#include "Scene.h"


void PlayerObject::Init()
{
	auto* tf = GetComponent<TransformComponent>();
	float s = 1.0f;
	tf->SetPosition({ 0, 10, 0 });
	tf->SetScale({ s, s, s });
	tf->SetEulerAngles({ 0, 0, 0 });

	// ModelAnimeObject
	m_pModelAnimeObject = Manager::GetScene()->AddGameObject<ModelAnimeObject>(1);
	m_pModelAnimeObject->Init();
	m_pModelAnimeObject->Transform()->SetParent(this->Transform());

	// コライダー
	auto* col = AddComponent<Collider>();
	col->SetCapsule(1.5f, 8);
	col->SetModeTrigger();
	col->SetOffsetPosition({0, 0, 0});
	col->SetOffsetRotation(Quaternion::FromEulerAngles({ 0, 0, 0}));

	// CharacterController
	auto* cc = AddComponent<CharacterControllerComponent>();
	cc->SetMaxMoveSpeed(10.0f);

	// StatePattern
	auto* state = AddComponent<PlayerStateManagerComponent>();
	state->Init();
	state->SetStateInitial(PlayerStateId::Idle);

}

void PlayerObject::Uninit()
{
	GameObject::Uninit();
}

void PlayerObject::Update(float dt)
{
	GameObject::Update(dt);

	// ----- 移動処理 -----
	//auto* cc = GetComponent<CharacterControllerComponent>();
	//Vector3 targetVector = { 0, 0, 0 };
	//if (Keyboard_IsKeyDown(KK_W))
	//{
	//	targetVector.z += 1;
	//}
	//if (Keyboard_IsKeyDown(KK_S))
	//{
	//	targetVector.z += -1;
	//}
	//if (Keyboard_IsKeyDown(KK_A))
	//{
	//	targetVector.x += -1;
	//}
	//if (Keyboard_IsKeyDown(KK_D))
	//{
	//	targetVector.x += 1;
	//}
	//if (Keyboard_IsKeyDownTrigger(KK_SPACE))
	//{
	//	cc->OnJumpPressed();
	//}
	//// ジャンプ長押し
	//cc->SetJumpHeld(Keyboard_IsKeyDown(KK_SPACE));
	
	//cc->SetMoveInput(targetVector);

	
}
