/*
	SelfDestroyComponent.h
	20260202  hanaue sho
*/
#ifndef SELFDESTROYCOMPONENT_H_
#define SELFDESTROYCOMPONENT_H_
#include "Component.h"
#include "GameObject.h"

class SelfDestroyComponent : public Component
{
private:
	bool m_IsTime = false;
	float m_Timer = 1.0f;
	float m_Height = -10.0f;

public:
	void Update(float dt) override
	{
		m_Timer -= dt;
		if (m_Timer < 0)
		{
			Owner()->RequestDestroy();
		}
		if (Owner()->Transform()->Position().y < m_Height)
		{
			Owner()->RequestDestroy();
		}
	}

	void SetTimer(float time) { m_Timer = time; }
	void SetHeight(float height) { m_Height = height; }
};




#endif