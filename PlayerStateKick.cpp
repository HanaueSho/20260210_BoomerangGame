/*
	PlayerStateKick.cpp
	20260217  hanaue sho
	プレイヤーキック中の処理
*/
#include "PlayerStateKick.h"
#include "PlayerStateManagerComponent.h"
#include "Component.h"
#include "InputSystem.h"

void PlayerStateKick::Enter(PlayerStateManagerComponent& manager)
{
	manager.GetModelAnime()->SetSpeedAnime(1.0f);
	manager.GetModelAnime()->SetIsLocomotion(false);
	manager.GetModelAnime()->PlayAnimeKick();
	// 移動制御
	manager.GetCC()->SetMoveInput({ 0, 0, 0 });

	// タイマー初期化
	m_Timer = 0.0f;
}

void PlayerStateKick::Update(PlayerStateManagerComponent& manager, float dt)
{
	m_Timer += dt;
	if (m_Timer > 0.5f)
	{
		manager.ChangeState(PlayerStateId::Idle);
	}
}

void PlayerStateKick::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStateKick::Exit(PlayerStateManagerComponent& manager)
{
}