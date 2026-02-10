/*
	PlayerStateIdle.h
	20260210  hanaue sho
	プレイヤー待機中の処理
*/
#ifndef PLAYERSTATEIDLE_H_
#define PLAYERSTATEIDLE_H_
#include "PlayerStateInterface.h"

class PlayerStateManagerComponent;

class PlayerStateIdle : public PlayerStateInterface
{
public:
	void Enter(PlayerStateManagerComponent& manager);
	void Update(PlayerStateManagerComponent& manager, float dt);
	void FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt);
	void Exit(PlayerStateManagerComponent& manager);
};

#endif