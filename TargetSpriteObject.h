/*
	TargetSpriteObject.h
	20260215  hanaue sho
*/
#ifndef TARGETSPRITEOBJECT_H_
#define TARGETSPRITEOBJECT_H_
#include "GameObject.h"

#include "SpriteAnimationComponent.h"

class TargetSpriteObject : public GameObject
{
	enum class State
	{
		None,
		FadeIn,
		FadeOut,
	};
private:
	State m_State = State::None;
	SpriteClip m_Clip;
	float m_Timer = 0.0f;
	float m_Time = 1.0f;
	float m_Speed = 5.0f;
	float m_Alpha = 1.0f;

public:
	void Init() override;
	void Update(float dt) override;

	void FadeIn()
	{
		m_State = State::FadeIn;
	}
	void FadeOut()
	{
		m_State = State::FadeOut;
	}
};

#endif