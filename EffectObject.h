/*
	EffectObject.h
	20260215  hanaue sho
*/
#ifndef EFFECTOBJECT_H_
#define EFFECTOBJECT_H_
#include "GameObject.h"
#include "SpriteAnimationComponent.h"

class EffectObject : public GameObject
{
private:
	SpriteClip m_Clip;
	float m_Timer = 0.0f;

public:
	void Init() override;
	void Update(float gameDt, float realDt) override;

};

#endif