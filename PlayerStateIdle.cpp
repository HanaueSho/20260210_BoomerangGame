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
	manager.GetModelAnime()->SetIsLocomotion(true);
	manager.GetModelAnime()->PlayAnimeIdle();
	// カメラ制御
	manager.SetCameraStateFollow();
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
		dynamic_cast<PlayerObject*>(manager.Owner())->GetAudioJump()->Play(false);
	}
	// 落下遷移
	if (!manager.GetCC()->IsGround())
	{
		manager.ChangeState(PlayerStateId::Jump);
	}
	// エイム遷移
	if (InputSystem::IsToAimDown())
	{
		if (manager.GetBoomerang()->IsStateIdle())
			manager.ChangeState(PlayerStateId::Aim);
	}
	// キック遷移
	if (InputSystem::IsKickTrigger())
	{
		manager.ChangeState(PlayerStateId::Kick);
	}
	// プッシュ遷移
	if (InputSystem::IsPushTrigger())
	{
		manager.ChangeState(PlayerStateId::Push);
	}
	// エモート遷移
	if (InputSystem::IsEmoteTrigger())
	{
		manager.ChangeState(PlayerStateId::Emote);
	}
	// ダンス遷移
	if (InputSystem::IsDanceTrigger())
	{
		manager.ChangeState(PlayerStateId::Dance);
	}

}

void PlayerStateIdle::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStateIdle::Exit(PlayerStateManagerComponent& manager)
{
}
