/*
	EnemyObject.cpp
	20260212  hanaue sho
*/
#include "EnemyObject.h"

#include "TransformComponent.h"
#include "ColliderComponent.h"
#include "RigidbodyComponent.h"
#include "Renderer.h"
#include "Texture.h"  // Texture::Load 既存
#include "EnemyModelAnimeObject.h"
#include "CharacterControllerComponent.h"

#include "BoneManager.h"
#include "TargetObject.h"
#include "TargetStateManagerComponent.h"
#include "EnemyStateManagerComponent.h"
#include "Keyboard.h"

void EnemyObject::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0, 10, 50 });
	float s = 5.0f;
	tf->SetScale({ s,s,s });
	tf->SetEulerAngles({ 0,0,0 });

	// ModelAnimeObject
	m_pModelAnimeObject = Manager::GetScene()->AddGameObject<EnemyModelAnimeObject>(1);
	m_pModelAnimeObject->SetType(m_Type);
	m_pModelAnimeObject->Init();
	m_pModelAnimeObject->Transform()->SetParent(this->Transform());

	// 物理を働かせたいのでコライダーなどを設定
	Collider* col = AddComponent<Collider>();
	col->SetCapsule(1.5f, 1);
	col->SetModeQuery();
	col->SetOffsetPosition({ 0, 1, 0 });
	col->SetOffsetRotation(Quaternion::FromEulerAngles({ 0, 0, 0 }));

	Rigidbody* rigid = AddComponent<Rigidbody>();
	rigid->SetGravityScale(1.0f);
	rigid->SetRestitution(0.0f);
	rigid->SetMass(1.0f);
	rigid->SetBodyTypeStatic();

	// CharacterController
	auto* cc = AddComponent<CharacterControllerComponent>();

	// EnemyStateManagerComponent
	auto* state = AddComponent< EnemyStateManagerComponent>();
	state->Init();
	state->SetModelAnime(m_pModelAnimeObject);

	// ----- ターゲットオブジェクト -----
	GameObject* bone = m_pModelAnimeObject->GetBoneManager().GetBoneObject(0);
	m_pTargetObject = Manager::GetScene()->AddGameObject<TargetObject>(1);
	m_pTargetObject->Init();
	m_pTargetObject->Transform()->SetParent(bone->Transform());
	m_pTargetObject->Transform()->SetPosition({ 0, 10, 0 });
	auto* stateTarget = m_pTargetObject->GetComponent<TargetStateManagerComponent>();
	stateTarget->SetOwnerObject(this);

	state->SetTargetObject(m_pTargetObject);

	// タグ設定
	SetTag("Enemy");
}

void EnemyObject::Update(float gameDt, float realDt)
{
	GameObject::Update(gameDt, realDt);

	//if (Keyboard_IsKeyDownTrigger(KK_D1))
	//	m_pModelAnimeObject->PlayAnimeIdle();
	//if (Keyboard_IsKeyDownTrigger(KK_D2))
	//	m_pModelAnimeObject->PlayAnimeReady();
	//if (Keyboard_IsKeyDownTrigger(KK_D3))
	//	m_pModelAnimeObject->PlayAnimeAttack();
	//if (Keyboard_IsKeyDownTrigger(KK_D4))
	//	m_pModelAnimeObject->PlayAnimeDead();
}

void EnemyObject::Uninit()
{
	GameObject::Uninit();

	m_pModelAnimeObject->RequestDestroy();
	m_pTargetObject->RequestDestroy();
}

