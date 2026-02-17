/*
	FadeSpriteObject.h
	20260217  hanaue sho
*/
#ifndef FADESPRITEOBJECT_H_
#define FADESPRITEOBJECT_H_
#include "GameObject.h"

#include "SpriteAnimationComponent.h"

class FadeSpriteObject : public GameObject
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
	float m_Timer = 1.0f;
	float m_Time  = 1.0f;
	float m_Speed = 1.0f;
	float m_Alpha = 1.0f;

public:
	void Init() override;
	void Update(float gameDt, float realDt) override;

	void FadeIn()
	{
		m_State = State::FadeIn;
	}
	void FadeOut()
	{
		m_State = State::FadeOut;
	}

	void SetSpeedFade(float speed) { m_Speed = speed; }
	bool EndFadeIn() const { return m_Timer == m_Time; }
};

#endif