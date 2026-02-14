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
	Vector3 forward = manager.GetCameraForward(); 
	Vector3 right = manager.GetCameraRight(); 
	Vector3 moveDir = right * input.x + forward * input.z;
	manager.GetCC()->SetMoveInput(moveDir);


	// ----- アニメーション管理 -----
	float speed = manager.GetCC()->ActualVelocity().length();
	float maxSpeed = manager.GetCC()->MaxMoveSpeed();
	manager.GetModelAnime()->SetBlendParam(speed / maxSpeed);
	manager.GetModelAnime()->SetSpeedAnime(speed / maxSpeed * 2.0f);

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
	// エイム遷移
	if (InputSystem::IsToAimDown())
	{
		manager.ChangeState(PlayerStateId::Aim);
	}
}

void PlayerStateMove::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStateMove::Exit(PlayerStateManagerComponent& manager)
{
}
