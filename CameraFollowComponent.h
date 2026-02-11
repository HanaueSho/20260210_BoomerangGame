/*
	CameraFollowComponent.h
	20260211  hanaue sho
	‘ÎÛ‚ğ’Ç]‚·‚éƒJƒƒ‰
	“ü—Í‚Å‰ñ“]‚à‚Å‚«‚é
*/
#ifndef CAMERAFOLLOWCOMPONENT_H_
#define CAMERAFOLLOWCOMPONENT_H_
#include "Component.h"
#include "Vector3.h"

class GameObject;

class CameraFollowComponent : public Component
{
public:
	enum class State
	{
		None,
		Follow,
		Aim,
	};
private:
	State m_State = State::Follow;
	// ----- Follow -----
	GameObject* m_pTarget = nullptr;
	float m_Distance = 30.0f;
	float m_YawRadian   = 0.0f;
	float m_PitchRadian = 1.0f;

	// ----- Aim -----
	Vector3 m_Offset = { 1.5f, 6.0f, -2.0f };

public:
	void Update(float dt) override;

	void SetTargetObject(GameObject* target)
	{
		m_pTarget = target;
	}

	// ----- StateSetter -----
	void SetStateFollow() { m_State = State::Follow; }
	void SetStateAim() { m_State = State::Aim; }

private:
	void Follow();
	void Aim();
};

#endif