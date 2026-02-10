/*
	KomaComponent.h
	20260202  hanaue sho
*/
#ifndef KOMACOMPONENT_H_
#define KOMACOMPONENT_H_
#include "Component.h"
#include "RigidbodyComponent.h"
#include "GameObject.h"

class KomaComponent : public Component
{
private:
	float m_Torque = 500.0f;

public:
	void Update(float dt) override
	{
		if (auto* rigid = Owner()->GetComponent<Rigidbody>())
		{
			rigid->AddTorque({ 0, 500 * dt, 0 });
		}
	}

	void SetTorquePower(float t) { m_Torque = t; }
};


#endif