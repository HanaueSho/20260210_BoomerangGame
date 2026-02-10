/*
	lightObject.h
	20251110 hanaue sho
*/
#pragma once
#include "GameObject.h"


class LightObject : public GameObject
{
private:
	float m_Radian = 0;
public:
	void Init() override;
	void Update(float dt) override;
};
