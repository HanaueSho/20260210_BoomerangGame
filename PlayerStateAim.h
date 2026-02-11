/*
	PlayerStateAim.h
	20260210  hanaue sho
	プレイヤー待機中の処理
*/
#ifndef PLAYERSTATEAIM_H_
#define PLAYERSTATEAIM_H_
#include "PlayerStateInterface.h"

class PlayerStateManagerComponent;

class PlayerStateAim : public PlayerStateInterface
{
public:
	void Enter(PlayerStateManagerComponent& manager);
	void Update(PlayerStateManagerComponent& manager, float dt);
	void FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt);
	void Exit(PlayerStateManagerComponent& manager);
};

#endif