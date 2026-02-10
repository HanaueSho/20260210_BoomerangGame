/*
	PlayerStateIdle.cpp
	20260210  hanaue sho
	ƒvƒŒƒCƒ„[‘Ò‹@’†‚Ìˆ—
*/
#include "PlayerStateIdle.h"
#include "PlayerStateManagerComponent.h"
#include "Component.h"
#include "InputSystem.h"

void PlayerStateIdle::Enter(PlayerStateManagerComponent& manager)
{
}

void PlayerStateIdle::Update(PlayerStateManagerComponent& manager, float dt)
{
	// ----- ó‘Ô‘JˆÚ -----
	// ˆÚ“®‘JˆÚ
	if (InputSystem::IsMoveDown())
	{
		manager.ChangeState(PlayerStateId::Move);
	}
	// ƒWƒƒƒ“ƒv‘JˆÚ
	if (InputSystem::IsJumpDownTrigger())
	{
		manager.GetCC()->OnJumpPressed();
		manager.ChangeState(PlayerStateId::Jump);
	}
	// —Ž‰º‘JˆÚ
	if (!manager.GetCC()->IsGround())
	{
		manager.ChangeState(PlayerStateId::Jump);
	}
}

void PlayerStateIdle::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStateIdle::Exit(PlayerStateManagerComponent& manager)
{
}
