/*
	PlayerStateDance.cpp
	20260226  hanaue sho
	プレイヤーダンス中の処理
*/
#include "PlayerStateDance.h"
#include "PlayerStateManagerComponent.h"
#include "Component.h"
#include "InputSystem.h"

void PlayerStateDance::Enter(PlayerStateManagerComponent& manager)
{
	manager.GetModelAnime()->SetSpeedAnime(1.3f);
	manager.GetModelAnime()->SetIsLocomotion(false);
	manager.GetModelAnime()->PlayAnimeDance0();
	// 移動制御
	manager.GetCC()->SetMoveInput({ 0, 0, 0 });
}

void PlayerStateDance::Update(PlayerStateManagerComponent& manager, float dt)
{
	// ----- 状態遷移 -----
	// 移動遷移
	if (InputSystem::IsMoveDown())
	{
		manager.ChangeState(PlayerStateId::Move);
		manager.GetModelAnime()->SetIsLocomotion(true);
	}
	// ジャンプ遷移
	if (InputSystem::IsJumpDownTrigger())
	{
		manager.GetCC()->OnJumpPressed();
		manager.ChangeState(PlayerStateId::Jump);
		dynamic_cast<PlayerObject*>(manager.Owner())->GetAudioJump()->Play(false);
		manager.GetModelAnime()->SetIsLocomotion(false);
	}
}

void PlayerStateDance::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStateDance::Exit(PlayerStateManagerComponent& manager)
{
}