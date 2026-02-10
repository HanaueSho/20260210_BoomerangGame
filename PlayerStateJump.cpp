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
}

void PlayerStateJump::Update(PlayerStateManagerComponent& manager, float dt)
{
	// ----- 移動処理 -----
	Vector3 input = InputSystem::GetInputMove();
	manager.GetCC()->SetMoveInput(input);
	// ジャンプ長押し
	manager.GetCC()->SetJumpHeld(Keyboard_IsKeyDown(KK_SPACE));
	// 空中ジャンプ
	if (InputSystem::IsJumpDownTrigger())
	{
		manager.GetCC()->OnJumpPressed();
	}

	// ----- 状態遷移 -----
	// 着地判定
	if (manager.GetCC()->IsGround())
	{
		manager.ChangeState(PlayerStateId::Idle);
	}
}

void PlayerStateJump::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStateJump::Exit(PlayerStateManagerComponent& manager)
{
}
