/*
	PlayerStateJump.cpp
	20260210  hanaue sho
	プレイヤー空中の処理
*/
#include "PlayerStateJump.h"
#include "PlayerStateManagerComponent.h"
#include "Component.h"
#include "CharacterControllerComponent.h"
#include "InputSystem.h"
#include "Vector3.h"

void PlayerStateJump::Enter(PlayerStateManagerComponent& manager)
{
	manager.GetModelAnime()->PlayAnimeJump();
	manager.GetModelAnime()->SetIsLocomotion(false);
	manager.GetModelAnime()->SetSpeedAnime(2.0f);
}

void PlayerStateJump::Update(PlayerStateManagerComponent& manager, float dt)
{
	// ----- 移動処理 -----
	Vector3 input = InputSystem::GetInputMove();
	Vector3 forward = manager.GetCameraForward();
	Vector3 right = manager.GetCameraRight();
	Vector3 moveDir = right * input.x + forward * input.z;
	manager.GetCC()->SetMoveInput(moveDir);

	// ジャンプ長押し
	manager.GetCC()->SetJumpHeld(Keyboard_IsKeyDown(KK_SPACE));
	// 空中ジャンプ
	if (InputSystem::IsJumpDownTrigger())
	{
		manager.GetModelAnime()->PlayAnimeJumpAir();
		manager.GetCC()->OnJumpPressed();
	}

	// ----- 状態遷移 -----
	// 着地判定
	if (manager.GetCC()->IsGround())
	{
		manager.ChangeState(PlayerStateId::Idle);
	}
	// エイム遷移
	if (InputSystem::IsToAimDown())
	{
		manager.ChangeState(PlayerStateId::Aim);
	}
}

void PlayerStateJump::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStateJump::Exit(PlayerStateManagerComponent& manager)
{
}
