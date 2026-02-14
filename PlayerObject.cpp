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
#include "BoneManager.h"


void PlayerObject::Init()
{
	auto* tf = GetComponent<TransformComponent>();
	float s = 1.0f;
	tf->SetPosition({ 0, 5, -150 });
	tf->SetScale({ s, s, s });
	tf->SetEulerAngles({ 0, 0, 0 });

	// ModelAnimeObject
	m_pModelAnimeObject = Manager::GetScene()->AddGameObject<ModelAnimeObject>(1);
	m_pModelAnimeObject->Init();
	m_pModelAnimeObject->Transform()->SetParentKeepWorld(this->Transform());

	// コライダー
	auto* col = AddComponent<Collider>();
	col->SetCapsule(1.5f, 8);
	col->SetModeQuery();
	col->SetOffsetPosition({0, 0, 0});
	col->SetOffsetRotation(Quaternion::FromEulerAngles({ 0, 0, 0}));

	// CharacterController
	auto* cc = AddComponent<CharacterControllerComponent>();

	// StatePattern
	auto* state = AddComponent<PlayerStateManagerComponent>();
	state->Init();
	state->SetModelAnime(m_pModelAnimeObject);
	state->SetStateInitial(PlayerStateId::Idle);

	// レイヤー
	SetPhysicsLayer(31);
	SetTag("Player");
}

void PlayerObject::Uninit()
{
	GameObject::Uninit();
}

void PlayerObject::Update(float dt)
{
	GameObject::Update(dt);
	
}

GameObject* PlayerObject::GetBoneObject(int index)
{
	return m_pModelAnimeObject->GetBoneManager().GetBoneObject(index);
}
