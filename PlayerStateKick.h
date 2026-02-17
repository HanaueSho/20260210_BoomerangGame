/*
	PlayerStateKick.h
	20260217  hanaue sho
	プレイヤーキック中の処理
*/
#ifndef PLAYERSTATEKICK_H_
#define PLAYERSTATEKICK_H_
#include "PlayerStateInterface.h"

class PlayerStateManagerComponent;

class PlayerStateKick : public PlayerStateInterface
{
private:
	float m_Timer = 0.0f;
public:
	void Enter(PlayerStateManagerComponent& manager);
	void Update(PlayerStateManagerComponent& manager, float dt);
	void FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt);
	void Exit(PlayerStateManagerComponent& manager);
};

#endif