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
#include "Vector3.h"

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
}

void PlayerStateAim::Update(PlayerStateManagerComponent& manager, float dt)
{
	// ターゲット補足
	if (InputSystem::IsAimDownTrigger())
	{
		// ターゲットを追加
		manager.GetBoomerang()->AddTarget();
	}

	// ----- 状態遷移 -----
	// 待機遷移
	if (InputSystem::IsThrowUp())
	{
		if (manager.GetBoomerang()->GetTargetsSize() > 0)
		{
			manager.ChangeState(PlayerStateId::Throw);
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
}
