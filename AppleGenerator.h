/*
	AppleObejct.h
	20260201 hanaue sho
*/
#ifndef APPLEGENERATOR_H_
#define APPLEGENERATOR_H_
#include "gameObject.h"

class AppleGenerator : public GameObject
{
private:
	float m_Timer = 0.0f;
	float m_Interval = 1.0f;
	Vector3 m_BasePosition{};
	Vector3 m_Move{};
	float m_Radian = 0.0f;
	int m_Sign = 1;

	float m_AppleScale = 1.0f;
public:
	void Init() override;
	void Update(float dt) override;

	void SetMoveVector(const Vector3& vect) { m_Move = vect; }
	void SetInterval(float interval) { m_Interval = interval; }
	void SetAppleScale(float s) { m_AppleScale = s; }
};

#endif