/*
	CameraMove.h
	20250820  hanaue sho
*/
#pragma once
#include "Component.h"
#include "TransformComponent.h"
#include "GameObject.h"

class CameraMove : public Component
{
private:
	TransformComponent* m_pTransform = nullptr;
public:
	void OnAdded() override
	{
		m_pTransform = Owner()->GetComponent<TransformComponent>();
	}
	void Update(float dt) override;
};



