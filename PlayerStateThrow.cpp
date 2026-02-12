/*
	PlayerStateThrow.cpp
	20260210  hanaue sho
	プレイヤー待機中の処理
*/
#include "PlayerStateThrow.h"
#include "PlayerStateManagerComponent.h"
#include "Component.h"
#include "InputSystem.h"

void PlayerStateThrow::Enter(PlayerStateManagerComponent& manager)
{
	manager.GetModelAnime()->SetSpeedAnime(2.0f);
	manager.GetModelAnime()->SetIsLocomotion(false);
	manager.GetModelAnime()->PlayAnimeThrow();

	// ブーメラン制御
	manager.GetBoomerang()->ChangeStateThrow();
}

void PlayerStateThrow::Update(PlayerStateManagerComponent& manager, float dt)
{

	// ----- 状態遷移 -----
	// 移動遷移
	if (InputSystem::IsMoveDown())
	{
		manager.ChangeState(PlayerStateId::Idle);
	}
}

void PlayerStateThrow::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStateThrow::Exit(PlayerStateManagerComponent& manager)
{
}
