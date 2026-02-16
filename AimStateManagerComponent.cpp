/*
	AimStateManagerComponent.cpp
	20260212  hanaue sho
*/
#include "AimStateManagerComponent.h"
#include "ColliderComponent.h"
#include "Keyboard.h"


void AimStateManagerComponent::Init()
{
	Component::Init();
}

void AimStateManagerComponent::Update(float dt)
{
	Component::Update(dt);
	//printf("[size]: %d\n", (int)m_Targets.size());

	//if (Keyboard_IsKeyDown(KK_D1))
	//	Owner()->Transform()->SetParentKeepWorld(nullptr);

	// Target タグ以外を取り除く
	m_Targets.erase(remove_if(m_Targets.begin(), m_Targets.end(),
		[](const GameObject* go) {return go == nullptr || go->Tag() != "Target"; }
		),
		m_Targets.end()
	);

}

void AimStateManagerComponent::OnTriggerEnter(Collider* me, Collider* other)
{
	//if (m_IsAimming)
	{
		if (other->Owner()->Tag() == "Target")
		{
			//printf("「Targetにあたりました」\n");
			m_Targets.push_back(other->Owner());
		}
		if (other->Owner()->Tag() == "Bullet")
		{
			//printf("「Bulletにあたりました」\n");
			//m_Targets.push_back(other->Owner());
		}
	}
}

void AimStateManagerComponent::OnTriggerStay(Collider* me, Collider* other)
{
}

void AimStateManagerComponent::OnTriggerExit(Collider* me, Collider* other)
{
	//if (m_IsAimming)
	{
		if (other->Owner()->Tag() == "Target")
		{
			//printf("「Targetから外れました」\n");
			m_Targets.erase(std::remove(m_Targets.begin(), m_Targets.end(), other->Owner()), m_Targets.end());
		}
		if (other->Owner()->Tag() == "Bullet")
		{
			//printf("「Bulletから外れました」\n");
			//m_Targets.erase(std::remove(m_Targets.begin(), m_Targets.end(), other->Owner()), m_Targets.end());
		}
	}
}

void AimStateManagerComponent::SetIsAimming(bool b)
{
	m_IsAimming = b;
	if (b == false)
	{
		//m_Targets.clear();
	}
	else
	{

	}
}
