/*
	PlayerStateDead.cpp
	20260217  hanaue sho
	ƒvƒŒƒCƒ„[Ž€–S’†‚Ìˆ—
*/
#include "PlayerStateDead.h"
#include "PlayerStateManagerComponent.h"
#include "Component.h"
#include "InputSystem.h"

void PlayerStateDead::Enter(PlayerStateManagerComponent& manager)
{
	manager.GetModelAnime()->SetSpeedAnime(1.0f);
	manager.GetModelAnime()->SetIsLocomotion(false);
	manager.GetModelAnime()->PlayAnimeDead();
	// ˆÚ“®§Œä
	manager.GetCC()->SetMoveInput({ 0, 0, 0 });

}

void PlayerStateDead::Update(PlayerStateManagerComponent& manager, float dt)
{
}

void PlayerStateDead::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStateDead::Exit(PlayerStateManagerComponent& manager)
{
}