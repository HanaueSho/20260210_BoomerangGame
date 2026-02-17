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
#include "PlayerHeartObject.h"
#include "HealthComponent.h"
#include "HealthSpriteObject.h"
#include "HelpSpriteObject.h"

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

	// HeartObject
	auto* heart = Manager::GetScene()->AddGameObject<PlayerHeartObject>(1);
	heart->Init();
	heart->Transform()->SetParent(Transform());
	heart->SetOwnerPlayer(this);

	// HealthComponent
	auto* health = AddComponent<HealthComponent>();

	// HelpSpriteObject
	auto* help = Manager::GetScene()->AddGameObject<HelpSpriteObject>(1);
	help->Init();

	// HealthSprite
	for (int i = 0; i < 6; i++)
	{
		m_pHealthSprite[i] = Manager::GetScene()->AddGameObject<HealthSpriteObject>(2);
		m_pHealthSprite[i]->Init();
		m_pHealthSprite[i]->Transform()->SetPosition({50 + 30.0f * i, 50, 0});
	}


	// レイヤー
	SetPhysicsLayer(31);
	SetTag("Player");
}

void PlayerObject::Uninit()
{
	for (int i = 0; i < 6; i++)
	{
		if (m_pHealthSprite[i])
			m_pHealthSprite[i]->Uninit();
	}
	GameObject::Uninit();
}

void PlayerObject::Update(float gameDt, float realDt)
{
	GameObject::Update(gameDt, realDt);

}

void PlayerObject::TakeDamage()
{
	if (m_pHealthSprite[m_IndexHealth])
	{
		m_pHealthSprite[m_IndexHealth]->RequestDestroy();
		m_pHealthSprite[m_IndexHealth] = nullptr;
	}
	m_IndexHealth--;
	if (m_IndexHealth < 0)
		m_IndexHealth = 0;
}

GameObject* PlayerObject::GetBoneObject(int index)
{
	return m_pModelAnimeObject->GetBoneManager().GetBoneObject(index);
}
