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
}

void PlayerStateAim::Update(PlayerStateManagerComponent& manager, float dt)
{

	// ----- 状態遷移 -----
	// 待機遷移
	if (InputSystem::IsThrowDown())
	{
		manager.ChangeState(PlayerStateId::Throw);
	}
	if (!InputSystem::IsAimDown())
	{
		manager.ChangeState(PlayerStateId::Idle);
	}
}

void PlayerStateAim::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStateAim::Exit(PlayerStateManagerComponent& manager)
{
}
