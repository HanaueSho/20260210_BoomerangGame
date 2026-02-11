/*
	BoomerangObject.h
	20260211  hanaue sho
	ブーメランのオブジェクト
*/
#ifndef BOOMERANGOBJECT_H_
#define BOOMERANGOBJECT_H_
#include "GameObject.h"
#include "ModelLoader.h"


class BoomerangObject : public GameObject
{
public:
	enum class State
	{
		Idle,  // 待機
		Aim,   // 狙う
		Throw, // 飛翔
		Back   // 返り
	};
private:
	State m_State = State::Idle;
	GameObject* m_pPlayerObject = nullptr;
	GameObject* m_pBoneSpineObject = nullptr;
	GameObject* m_pBoneHandObject = nullptr;

	Vector3 m_OffsetSpine = { 2, 1, -0.8f };
	Vector3 m_OffsetHandPosition  = { 2.5f, 3 , 0.5f };
	Vector3 m_OffsetHandRotation = { 0.78f, 7.7f, 0.36f };

public:
	void Init() override;
	void Update(float dt) override;

	void ChangeState(State newState);

	void SetPlayerObject(GameObject* player);
	void SetHand();
	void SetSpine();
};


#endif