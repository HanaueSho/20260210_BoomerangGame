/*
	BulletStateManagerComponent.cpp
	20260215  hanaue sho
*/
#include "BulletStateManagerComponent.h"
#include "ColliderComponent.h"
#include "GameObject.h"
#include "PlayerStateManagerComponent.h"

void BulletStateManagerComponent::Init()
{

}

void BulletStateManagerComponent::Update(float dt)
{
	// 移動処理
	Vector3 position = Owner()->Transform()->Position();
	position += m_Vect * m_Speed * dt;
	Owner()->Transform()->SetPosition(position);

	// 回転処理
	float r = 1 * dt;
	Owner()->Transform()->Rotate({ r, r, r });

	// 時間経過
	m_Timer += dt;
	if (m_Timer > 10.0f)
	{
		// 自身を消す処理
		Owner()->RequestDestroy();
	}
}

void BulletStateManagerComponent::OnTriggerEnter(Collider* me, Collider* other)
{
	if (other->Owner()->Tag() == "PlayerHeart")
	{
		//auto* player = other->Owner()->GetComponent<PlayerStateManagerComponent>();
		// ダメージ判定

		// 自身を消す処理
		Owner()->RequestDestroy();
	}

}

void BulletStateManagerComponent::DestroyThis()
{
	Owner()->RequestDestroy();
}
