/*
	ResultSpriteObject.h
	20260218  hanaue sho
*/
#ifndef RESULTSPRITEOBJECT_H_
#define RESULTSPRITEOBJECT_H_
#include "GameObject.h"

class ResultSpriteObject : public GameObject
{
public:
	enum class State
	{
		None,
		FadeIn,
		FadeOut,
	};
	enum class Type
	{
		Success,
		Failed
	};
private:
	Type  m_Type = Type::Success;
	State m_State = State::None;
	float m_Timer = 1.0f;
	float m_Time = 1.0f;
	float m_Speed = 1.0f;
	float m_Alpha = 0.0f;

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

	void SetType(Type type);

	void SetSpeedFade(float speed) { m_Speed = speed; }
	bool EndFadeIn() const { return m_Timer == m_Time; }
};

#endif