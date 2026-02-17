/*
	HealthComponent.h
	20260217  hanaue sho
*/
#ifndef HEALTHCOMPONENT_H_
#define HEALTHCOMPONENT_H_
#include "Component.h"

class HealthComponent : public Component
{
private:
	int m_Health = 6;

public:
	void TakeDamage()
	{
		m_Health--;
		if (m_Health < 1)
			m_Health = 0;
		printf("[Health]%d\n", m_Health);
	}
	bool CheckDead()
	{
		return m_Health < 1;
	}
};

#endif