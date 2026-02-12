/*
	AimStateManagerComponent.h
	20260212  hanaue sho
*/
#ifndef AIMSTATEMANAGERCOMPONENT_H_
#define AIMSTATEMANAGERCOMPONENT_H_
#include <vector>
#include "Component.h"

class AimStateManagerComponent : public Component
{
private:
	bool m_IsAimming = false;
	std::vector<GameObject*> m_Targets{};

public:
	void Init() override;
	void Update(float dt) override;

	void OnTriggerEnter(class Collider* me, class Collider* other) override;
	void OnTriggerExit(class Collider* me, class Collider* other)  override; 

	void SetIsAimming(bool b);
	std::vector<GameObject*> Targets() const { return m_Targets; }
};	

#endif