/*
	PlayerStatePush.h
	20260217  hanaue sho
	プレイヤープッシュ中の処理
*/
#ifndef PLAYERSTATEPUSH_H_
#define PLAYERSTATEPUSH_H_
#include "PlayerStateInterface.h"

class PlayerStateManagerComponent;

class PlayerStatePush : public PlayerStateInterface
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