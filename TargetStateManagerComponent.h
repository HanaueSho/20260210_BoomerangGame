/*
	TargetStateMaanger.h
	20260215  hanaue sho
*/
#ifndef TARGETSTATEMANAGERCOMPONENT_H_
#define TARGETSTATEMANAGERCOMPONENT_H_
#include "Component.h"

class GameObject;

class TargetStateManagerComponent : public Component
{
public:
	enum class State
	{
		Default, // デフォルト
		Invincible, // 無敵
	};
private:
	State m_State = State::Default;
	bool m_IsBroken = false; // 壊れた

	GameObject* m_pOwnerObject = nullptr;

public:
	void Init() override;
	void Update(float dt) override;
	
	void ChangeState(State newState);

	void SetOwnerObject(GameObject* owner)
	{
		m_pOwnerObject = owner;
	}

	// 被ダメージ時呼出し処理
	void TakeDamage();

};

#endif