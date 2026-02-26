/*
	PlayerStateEmote.h
	20260226  hanaue sho
	プレイヤーエモート中の処理
*/
#ifndef PLAYERSTATEEMOTE_H_
#define PLAYERSTATEEMOTE_H_
#include "PlayerStateInterface.h"

class PlayerStateManagerComponent;

class PlayerStateEmote : public PlayerStateInterface
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