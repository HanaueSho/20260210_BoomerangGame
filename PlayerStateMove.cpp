/*
	PlayerStateMove.cpp
	20260210  hanaue sho
	ƒvƒŒƒCƒ„[ˆÚ“®’†‚Ìˆ—
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
	// ----- ˆÚ“®ˆ— -----
	Vector3 input = InputSystem::GetInputMove();
	manager.GetCC()->SetMoveInput(input);



	// ----- ó‘Ô‘JˆÚ -----
	// ƒWƒƒƒ“ƒv‘JˆÚ
	if (InputSystem::IsJumpDownTrigger())
	{
		manager.GetCC()->OnJumpPressed();
		manager.ChangeState(PlayerStateId::Jump);
	}
	// ‘Ò‹@‘JˆÚ
	if (!InputSystem::IsMoveDown())
	{
		manager.ChangeState(PlayerStateId::Idle);
	}
	// —Ž‰º‘JˆÚ
	if (!manager.GetCC()->IsGround())
	{
		manager.ChangeState(PlayerStateId::Jump);
	}
}

void PlayerStateMove::FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt)
{
}

void PlayerStateMove::Exit(PlayerStateManagerComponent& manager)
{
}
