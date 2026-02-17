/*
	PlayerStatePush.cpp
	20260217  hanaue sho
	プレイヤープッシュ中の処理
*/
#include "PlayerStatePush.h"
#include "PlayerStateManagerComponent.h"
#include "Component.h"
#include "InputSystem.h"

void PlayerStatePush::Enter(PlayerStateManagerComponent& manager)
{
	manager.GetModelAnime()->SetSpeedAnime(1.0f);
	manager.GetModelAnime()->SetIsLocomotion(false);
	manager.GetModelAnime()->PlayAnimePush();
	// 移動制御
	manager.GetCC()->SetMoveInput({ 0, 0, 0 });

	// タイマー初期化
	m_Timer = 0.0f;
}

void PlayerStatePush::Update(PlayerStateManagerComponent& manager, float dt)
{
	m_Timer += dt;
	if (m_Timer > 0.5f)
	{
		manager.ChangeState(PlayerStateId::Idle);
	}
}

void PlayerStatePush::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStatePush::Exit(PlayerStateManagerComponent& manager)
{
}