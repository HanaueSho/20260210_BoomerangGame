/*
	AutoRotateComponent.h
	20260202  hanaue sho
*/
#ifndef AUTOROTATECOMPONENT_H_
#define AUTOROTATECOMPONENT_H_
#include "Component.h"
#include "GameObject.h"
#include "Vector3.h"

class AutoRotateComponent : public Component
{
private:
	Vector3 m_Axis = { 0, 1, 0 };
	float m_Speed = 1.0f;

public:
	void Update(float dt) override
	{
		Owner()->Transform()->RotateAxis(m_Axis, m_Speed * dt);
	}

	void SetAxis(const Vector3& axis) { m_Axis = axis; }
	void SetSpeed(float s) { m_Speed = s; }
};

#endif