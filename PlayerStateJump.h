/*
	PlayerStateJump.h
	20260210  hanaue sho
	プレイヤージャンプ中の処理
*/
#ifndef PLAYERSTATEJUMP_H_
#define PLAYERSTATEJUMP_H_
#include "PlayerStateInterface.h"

class PlayerStateManagerComponent;

class PlayerStateJump : public PlayerStateInterface
{
public:
	void Enter(PlayerStateManagerComponent& manager);
	void Update(PlayerStateManagerComponent& manager, float dt);
	void FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt);
	void Exit(PlayerStateManagerComponent& manager);
};

#endif