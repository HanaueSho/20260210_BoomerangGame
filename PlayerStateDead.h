/*
	PlayerStateDead.h
	20260217  hanaue sho
	ƒvƒŒƒCƒ„[€–S’†‚Ìˆ—
*/
#ifndef PLAYERSTATEDEAD_H_
#define PLAYERSTATEDEAD_H_
#include "PlayerStateInterface.h"

class PlayerStateManagerComponent;

class PlayerStateDead : public PlayerStateInterface
{
private:

public:
	void Enter(PlayerStateManagerComponent& manager);
	void Update(PlayerStateManagerComponent& manager, float dt);
	void FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt);
	void Exit(PlayerStateManagerComponent& manager);
};

#endif