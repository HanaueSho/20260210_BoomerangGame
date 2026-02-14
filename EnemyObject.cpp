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

//#include "EnemyStateManagerComponent.h"
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
	rigid->SetBodyTypeKinematic();

	// CharacterController
	auto* cc = AddComponent<CharacterControllerComponent>();

	// タグ設定
	SetTag("Enemy");
}

void EnemyObject::Update(float dt)
{
	GameObject::Update(dt);

	//if (Keyboard_IsKeyDownTrigger(KK_D1))
	//	m_pModelAnimeObject->PlayAnimeIdle();
	//if (Keyboard_IsKeyDownTrigger(KK_D2))
	//	m_pModelAnimeObject->PlayAnimeReady();
	//if (Keyboard_IsKeyDownTrigger(KK_D3))
	//	m_pModelAnimeObject->PlayAnimeAttack();
	//if (Keyboard_IsKeyDownTrigger(KK_D4))
	//	m_pModelAnimeObject->PlayAnimeDead();
}
