/*
	BulletStateManagerComponent.h
	20260215  hanaue sho
*/
#ifndef BULLETSTATEMANAGERCOMPONENT_H
#define BULLETSTATEMANAGERCOMPONENT_H
#include "Component.h"
#include "Vector3.h"

class BulletStateManagerComponent : public Component
{
private:
	float m_Speed = 30.0f;
	Vector3 m_Vect = { 0, 0, 0 };
	float m_Timer = 0.0f;

public:
	void Init() override;
	void Update(float dt) override;
	void OnTriggerEnter(class Collider* me, class Collider* other) override;

	void DestroyThis();

	void SetVect(Vector3 vect)
	{
		m_Vect = vect;
	}
};

#endif