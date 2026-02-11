/*
	CameraFollowComponent.h
	20260211  hanaue sho
	‘ÎÛ‚ğ’Ç]‚·‚éƒJƒƒ‰
	“ü—Í‚Å‰ñ“]‚à‚Å‚«‚é
*/
#ifndef CAMERAFOLLOWCOMPONENT_H_
#define CAMERAFOLLOWCOMPONENT_H_
#include "Component.h"

class GameObject;

class CameraFollowComponent : public Component
{
private:
	GameObject* m_pTarget = nullptr;
	bool m_IsFollow = true;
	float m_Distance = 30.0f;
	float m_YawRadian   = 0.0f;
	float m_PitchRadian = 1.0f;

public:
	void Update(float dt) override;

	void SetTargetObject(GameObject* target)
	{
		m_pTarget = target;
	}
};

#endif