/*
	PlayerStateInterface.h
	20260210  hanaue sho
	ステートパターンのインターフェース
*/
#ifndef PLAYERSTATEINTERFACE_H_
#define PLAYERSTATEINTERFACE_H_

class PlayerStateManagerComponent;

// StateInterface
class PlayerStateInterface
{
public:
	virtual ~PlayerStateInterface() = default;
	virtual void Enter(PlayerStateManagerComponent& manager) = 0;
	virtual void Update(PlayerStateManagerComponent& manager, float dt) = 0;
	virtual void FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt) = 0;
	virtual void Exit(PlayerStateManagerComponent& manager) = 0;
};

#endif

/*
【テンプレート】
#ifndef PLAYERSTATE@@@_H_
#define PLAYERSTATE@@@_H_
#include "PlayerStateInterface.h"

class PlayerStateManagerComponent;

class PlayerState@@@ : public PlayerStateInterface
{
public:
	void Enter(PlayerStateManagerComponent& manager);
	void Update(PlayerStateManagerComponent& manager, float dt);
	void FixedUpdate(PlayerStateManagerComponent& manager, float fixedDt);
	void Exit(PlayerStateManagerComponent& manager);
};

#endif


*/
