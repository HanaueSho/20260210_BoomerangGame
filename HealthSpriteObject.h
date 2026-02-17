/*
	HealthSpriteObject.h
	20260217  hanaue sho
*/
#ifndef HEALTHSPRITEOBJECT_H_
#define HEALTHSPRITEOBJECT_H_
#include "GameObject.h"

class HealthSpriteObject : public GameObject
{
private:
	float m_Alpha = 1.0f;

public:
	void Init() override;
	void Update(float gameDt, float realDt) override;
	
};

#endif