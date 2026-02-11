/*
	PlayerStateThrow.h
	20260210  hanaue sho
	プレイヤー待機中の処理
*/
#ifndef PLAYERSTATETHROW_H_
#define PLAYERSTATETHROW_H_
#include "PlayerStateInterface.h"

class PlayerStateManagerComponent;

class PlayerStateThrow : public PlayerStateInterface
{
public:
	void Enter(PlayerStateManagerComponent& manager);
	void Update(PlayerStateManagerComponent& manager, float dt);
	void FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt);
	void Exit(PlayerStateManagerComponent& manager);
};

#endif