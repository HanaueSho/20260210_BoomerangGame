/*
	PlayerStateDance.h
	20260226  hanaue sho
	プレイヤーダンス中の処理
*/
#ifndef PLAYERSTATEDANCE_H_
#define PLAYERSTATEDANCE_H_
#include "PlayerStateInterface.h"

class PlayerStateManagerComponent;

class PlayerStateDance : public PlayerStateInterface
{
private:
public:
	void Enter(PlayerStateManagerComponent& manager);
	void Update(PlayerStateManagerComponent& manager, float dt);
	void FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt);
	void Exit(PlayerStateManagerComponent& manager);
};

#endif