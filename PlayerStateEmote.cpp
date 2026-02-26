/*
	PlayerStateEmote.cpp
	20260217  hanaue sho
	プレイヤーエモート中の処理
*/
#include "PlayerStateEmote.h"
#include "PlayerStateManagerComponent.h"
#include "Component.h"
#include "InputSystem.h"

void PlayerStateEmote::Enter(PlayerStateManagerComponent& manager)
{
	manager.GetModelAnime()->SetSpeedAnime(1.0f);
	manager.GetModelAnime()->SetIsLocomotion(false);
	manager.GetModelAnime()->PlayAnimeEmote0();
	// 移動制御
	manager.GetCC()->SetMoveInput({ 0, 0, 0 });

	// タイマー初期化
	m_Timer = 0.0f;

}

void PlayerStateEmote::Update(PlayerStateManagerComponent& manager, float dt)
{
	m_Timer += dt;
	if (m_Timer > 1.0f)
	{
		manager.ChangeState(PlayerStateId::Idle);
	}
}

void PlayerStateEmote::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStateEmote::Exit(PlayerStateManagerComponent& manager)
{
}