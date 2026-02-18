/*
	TargetStateMaanger.cpp
	20260215  hanaue sho
*/
#include "TargetStateManagerComponent.h"
#include "MaterialComponent.h"
#include "GameObject.h"
#include "EnemyStateManagerComponent.h"
#include "LightComponent.h"

void TargetStateManagerComponent::Init()
{
}

void TargetStateManagerComponent::Update(float dt)
{
	switch(m_State)
	{
	case State::Default:
		break;
	case State::Invincible:
		break;
	}
}

void TargetStateManagerComponent::ChangeState(State newState)
{
	// 終了処理
	switch (m_State)
	{
	case State::Default:
		break;
	case State::Invincible:
		break;
	}

	m_State = newState;
	// 初期化処理
	switch (m_State)
	{
	case State::Default:
		break;
	case State::Invincible:
		break;
	}
}

void TargetStateManagerComponent::TakeDamage()
{
	if (m_State == State::Invincible) return;

	// フラグ立て
	m_IsBroken = true;

	// 色変え
	auto* mat = Owner()->GetComponent<MaterialComponent>();
	// マテリアルセット
	MaterialApp m{};
	float d = 0.2f;
	m.diffuse = Vector4(d, d, d, 1.0f);
	m.ambient = Vector4(1, 1, 1, 1);
	m.specular = Vector4(0, 0, 0, 1);
	m.textureEnable = false;
	mat->SetMaterial(m);

	// 条件でデッド
	auto* state = m_pOwnerObject->GetComponent<EnemyStateManagerComponent>();
	state->ChangeStateDead();

	if (auto* lc = Owner()->GetComponent<LightComponent>())
	{
		Owner()->RemoveComponent(lc);
	}
}
