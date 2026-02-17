/*
	PlayerStateDagame.h
	20260217  hanaue sho
	プレイヤー被ダメ中の処理
*/
#ifndef PLAYERSTATEDAMAGE_H_
#define PLAYERSTATEDAMAGE_H_
#include "PlayerStateInterface.h"

class PlayerStateManagerComponent;

class PlayerStateDamage : public PlayerStateInterface
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