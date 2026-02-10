/*
	PlayerStateMove.h
	20260210  hanaue sho
	ƒvƒŒƒCƒ„[ˆÚ“®’†‚Ìˆ—
*/
#ifndef PLAYERSTATEMOVE_H_
#define PLAYERSTATEMOVE_H_
#include "PlayerStateInterface.h"

class PlayerStateManagerComponent;

class PlayerStateMove : public PlayerStateInterface
{
public:
	void Enter(PlayerStateManagerComponent& manager);
	void Update(PlayerStateManagerComponent& manager, float dt);
	void FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt);
	void Exit(PlayerStateManagerComponent& manager);
};

#endif