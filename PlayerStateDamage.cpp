/*
	PlayerStateDamage.cpp
	20260217  hanaue sho
	プレイヤー被ダメ中の処理
*/
#include "PlayerStateDamage.h"
#include "PlayerStateManagerComponent.h"
#include "Component.h"
#include "InputSystem.h"

void PlayerStateDamage::Enter(PlayerStateManagerComponent& manager)
{
	manager.GetModelAnime()->SetSpeedAnime(1.0f);
	manager.GetModelAnime()->SetIsLocomotion(false);
	manager.GetModelAnime()->PlayAnimeDamage();
	// 移動制御
	manager.GetCC()->SetMoveInput({ 0, 0, 0 });

	// タイマー初期化
	m_Timer = 0.0f;

	// 無敵状態に
	manager.SetInvinsible(true);

	// ダメージ処理
	manager.Owner()->GetComponent<HealthComponent>()->TakeDamage();

	// 点滅処理
	manager.GetModelAnime()->StartBlinkColor();
}

void PlayerStateDamage::Update(PlayerStateManagerComponent& manager, float dt)
{
	m_Timer += dt;
	if (m_Timer > 0.5f)
	{
		manager.ChangeState(PlayerStateId::Idle);
	}
}

void PlayerStateDamage::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStateDamage::Exit(PlayerStateManagerComponent& manager)
{
}