/*
	AimStateManagerComponent.cpp
	20260212  hanaue sho
*/
#include "AimStateManagerComponent.h"
#include "ColliderComponent.h"

void AimStateManagerComponent::Init()
{
	Component::Init();
}

void AimStateManagerComponent::Update(float dt)
{
	Component::Update(dt);
	printf("[size]: %d\n", (int)m_Targets.size());
}

void AimStateManagerComponent::OnTriggerEnter(Collider* me, Collider* other)
{
	if (m_IsAimming)
	{
		if (other->Owner()->Tag() == "Enemy")
		{
			printf("「Enemyにあたりました」\n");
			m_Targets.push_back(other->Owner());
		}
	}
}

void AimStateManagerComponent::OnTriggerExit(Collider* me, Collider* other)
{
	if (m_IsAimming)
	{
		if (other->Owner()->Tag() == "Enemy")
		{
			printf("「Enemyから外れました」\n");
			m_Targets.erase(std::remove(m_Targets.begin(), m_Targets.end(), other->Owner()), m_Targets.end());
		}
	}
}

void AimStateManagerComponent::SetIsAimming(bool b)
{
	m_IsAimming = b;
	if (b == false)
	{
		m_Targets.clear();
	}
	else
	{

	}
}
