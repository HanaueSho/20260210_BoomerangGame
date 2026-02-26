/*
	PlayerStateAim.cpp
	20260210  hanaue sho
	プレイヤー移動中の処理
*/
#include "PlayerStateAim.h"
#include "PlayerStateManagerComponent.h"
#include "Component.h"
#include "CharacterControllerComponent.h"
#include "InputSystem.h"
#include "Input.h"
#include "Vector3.h"
#include "Manager.h"

void PlayerStateAim::Enter(PlayerStateManagerComponent& manager)
{
	// アニメ処理
	manager.GetModelAnime()->PlayAnimeAim();
	manager.GetModelAnime()->SetIsLocomotion(false);
	manager.GetModelAnime()->SetSpeedAnime(1.0f);
	// 移動制御
	manager.GetCC()->SetMoveInput({0, 0, 0});

	// カメラ方向を向くように
	Vector3 vect = manager.GetCameraForward(); Vector3::Printf(vect);
	vect.y = 0.0f; 
	Vector3 targetPos = manager.Owner()->Transform()->Position() + vect * 10.0f;
	manager.Owner()->Transform()->LookAt(targetPos);
	// カメラ制御
	manager.SetCameraStateAim();

	// ブーメラン制御
	manager.GetBoomerang()->ChangeStateAim();

	// スローモーション
	Manager::SetTimeScale(0.2f);

	// SE
	dynamic_cast<PlayerObject*>(manager.Owner())->GetAudioAim()->Play(false);
}

void PlayerStateAim::Update(PlayerStateManagerComponent& manager, float dt)
{
	// ターゲット補足
	if (InputSystem::IsTargetDownTrigger())
	{
		// ターゲットを追加
		manager.GetBoomerang()->AddTarget();
	}

	// 回転処理S
	if (Input::Pad(0).IsConnected()) // コントローラー優先
	{
		float yawRadianDelta = Input::Pad(0).RX() * dt * -2.0f;
		manager.Owner()->Transform()->RotateAxis({ 0, 1, 0 }, -yawRadianDelta);
	}

	// ----- 状態遷移 -----
	// 待機遷移
	if (InputSystem::IsThrowUp())
	{
		if (manager.GetBoomerang()->GetTargetsSize() > 0)
		{
			manager.ChangeState(PlayerStateId::Throw);
			dynamic_cast<PlayerObject*>(manager.Owner())->GetAudioThrow()->Play(false);
		}
		else
		{
			manager.GetBoomerang()->ChangeStateIdle();
			manager.ChangeState(PlayerStateId::Idle);
		}
	}
}

void PlayerStateAim::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStateAim::Exit(PlayerStateManagerComponent& manager)
{
	// スローモーション解除
	Manager::SetTimeScale(1.0f);
}
