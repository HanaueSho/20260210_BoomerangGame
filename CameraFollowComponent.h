/*
	CameraFollowComponent.h
	20260211  hanaue sho
	対象を追従するカメラ
	入力で回転もできる
*/
#ifndef CAMERAFOLLOWCOMPONENT_H_
#define CAMERAFOLLOWCOMPONENT_H_
#include "Component.h"
#include "Vector3.h"

class GameObject;

class CameraFollowComponent : public Component
{
public:
	enum class State
	{
		None,
		Follow,
		Aim,
	};
private:
	State m_State = State::Follow;
	// ----- Follow -----
	GameObject* m_pTarget = nullptr;
	float m_Distance = 30.0f;
	float m_YawRadian   = 0.0f;
	float m_PitchRadian = 0.5f;
	float m_LookK = 14.0f; // 大きいほど速く追従

	// ----- Aim -----
	Vector3 m_Offset = { 2.5f, 6.0f, -2.0f };

	// ----- シェイク -----
	Vector3 m_ShakeOffset = { 0, 0, 0 };
	float m_ShakeTime  = 0.0f;
	float m_ShakeTimer = 0.0f;
	Vector3 m_ShakeValue = { 0, 1, 0 };
	float m_ShakeScale = 1.0f;
	bool m_IsShake = false;
	int m_ShakeSign = 1;

public:
	UpdateClock Clock() const noexcept override { return UpdateClock::Real; }
	void Update(float dt) override;

	void SetTargetObject(GameObject* target)
	{
		m_pTarget = target;
	}

	void ChangeState(State newState);
	// ----- StateSetter -----
	void SetStateFollow() { ChangeState(State::Follow); }
	void SetStateAim()	  { ChangeState(State::Aim); }

	// 画面シェイク
	void Shake(float time, float scale)
	{
		m_IsShake = true;
		m_ShakeTime = time;
		m_ShakeScale = scale;
	}

private:
	void Follow(float dt);
	void Aim(float dt);
	void AddShake(float dt);
};

#endif