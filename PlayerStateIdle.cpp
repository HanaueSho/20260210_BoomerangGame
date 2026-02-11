/*
	PlayerStateIdle.cpp
	20260210  hanaue sho
	プレイヤー待機中の処理
*/
#include "PlayerStateIdle.h"
#include "PlayerStateManagerComponent.h"
#include "Component.h"
#include "InputSystem.h"

void PlayerStateIdle::Enter(PlayerStateManagerComponent& manager)
{
	manager.GetModelAnime()->SetSpeedAnime(1.0f);
}

void PlayerStateIdle::Update(PlayerStateManagerComponent& manager, float dt)
{
	// ----- アニメーション管理 -----
	float speed = manager.GetCC()->ActualVelocity().lengthSq();
	float maxSpeed = manager.GetCC()->MaxMoveSpeed();
	manager.GetModelAnime()->SetBlendParam(speed/maxSpeed);

	// ----- 状態遷移 -----
	// 移動遷移
	if (InputSystem::IsMoveDown())
	{
		manager.ChangeState(PlayerStateId::Move);
	}
	// ジャンプ遷移
	if (InputSystem::IsJumpDownTrigger())
	{
		manager.GetCC()->OnJumpPressed();
		manager.ChangeState(PlayerStateId::Jump);
	}
	// 落下遷移
	if (!manager.GetCC()->IsGround())
	{
		manager.ChangeState(PlayerStateId::Jump);
	}
}

void PlayerStateIdle::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStateIdle::Exit(PlayerStateManagerComponent& manager)
{
}
