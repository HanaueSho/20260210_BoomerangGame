/*
	PlayerStateMove.cpp
	20260210  hanaue sho
	プレイヤー移動中の処理
*/
#include "PlayerStateMove.h"
#include "PlayerStateManagerComponent.h"
#include "Component.h"
#include "CharacterControllerComponent.h"
#include "InputSystem.h"
#include "Vector3.h"

void PlayerStateMove::Enter(PlayerStateManagerComponent& manager)
{
}

void PlayerStateMove::Update(PlayerStateManagerComponent& manager, float dt)
{
	// ----- 移動処理 -----
	Vector3 input = InputSystem::GetInputMove();
	manager.GetCC()->SetMoveInput(input);


	// ----- アニメーション管理 -----
	float speed = manager.GetCC()->ActualVelocity().length();
	float maxSpeed = manager.GetCC()->MaxMoveSpeed();
	manager.GetModelAnime()->SetBlendParam(speed / maxSpeed);
	manager.GetModelAnime()->SetSpeedAnime(speed / maxSpeed * 1.5f);

	// ----- 状態遷移 -----
	// ジャンプ遷移
	if (InputSystem::IsJumpDownTrigger())
	{
		manager.GetCC()->OnJumpPressed();
		manager.ChangeState(PlayerStateId::Jump);
	}
	// 待機遷移
	if (!InputSystem::IsMoveDown())
	{
		manager.ChangeState(PlayerStateId::Idle);
	}
	// 落下遷移
	if (!manager.GetCC()->IsGround())
	{
		manager.ChangeState(PlayerStateId::Jump);
	}
}

void PlayerStateMove::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStateMove::Exit(PlayerStateManagerComponent& manager)
{
}
