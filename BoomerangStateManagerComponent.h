/*
	BoomerangStateManagerComponent.h
	20260212  hanaue sho
*/
#ifndef BOOMERANGSTATEMANAGERCOMPONENT_H_ 
#define BOOMERANGSTATEMANAGERCOMPONENT_H_
#include <vector>
#include "Component.h"
#include "Vector3.h"

class BoomerangStateManagerComponent : public Component
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

	// ----- Idle -----
	GameObject* m_pPlayerObject = nullptr;    // Player ポインタ
	GameObject* m_pBoneSpineObject = nullptr; // 背骨ボーン
	GameObject* m_pBoneHandObject = nullptr;  // 手首ボーン
	Vector3 m_OffsetSpinePosition = { 2, 1, -0.8f }; // 背骨オフセット
	Vector3 m_OffsetSpineRotation = { 0, 0, 0 };     // 背骨オフセット
	Vector3 m_OffsetHandPosition = {  2.68f, 2.35f , 0.33f }; // 手首オフセット
	Vector3 m_OffsetHandRotation = { -0.29f, 3.23f, 4.23f };  // 手首オフセット
	// ----- Aim -----
	GameObject* m_pAimObject = nullptr;
	// ----- Throw -----
	std::vector<GameObject*> m_Targets{};
	int m_IndexTargets = 0; // Targets のインデックス
	bool m_IsApproach = false;
	Vector3 m_MoveDir = {0, 0, 0}; // 現在進行方向
	float m_Speed = 100.0f; // 飛翔速度
	float m_TurnRateFlight   = 30.0f; // １フレーム当たりの旋回角
	float m_TurnRateApproach = 180.0f * 3.1415926535f / 180.0f;  // １フレーム当たりの旋回角
	float m_TimerFlight = 0.0f;

public:
	// ライフサイクル
	void Init() override;
	void Update(float dt) override;
	void FixedUpdate(float fixedDt) override;
	
	void OnTriggerEnter(class Collider* me, class Collider* other);

	// ステートチェンジ
	void ChangeState(State newState);
	void ChangeStateIdle()  { ChangeState(State::Idle);  }
	void ChangeStateAim()   { ChangeState(State::Aim);   }
	void ChangeStateThrow() { ChangeState(State::Throw); }
	void ChangeStateBack()  { ChangeState(State::Back);  }

	void SetPlayerObject(GameObject* player);

	GameObject* GetAimObject() { return m_pAimObject; }

private:
	void SetHand();
	void SetSpine();
	// ステート内部処理
	void Throw(float dt);
};

#endif

/*
【ブチギレ備忘録】
	//if (Keyboard_IsKeyDown(KK_D1))
	//	m_OffsetHandPosition.x += -dt;
	//if (Keyboard_IsKeyDown(KK_D2))
	//	m_OffsetHandPosition.x += dt;
	//if (Keyboard_IsKeyDown(KK_D3))
	//	m_OffsetHandPosition.y += -dt;
	//if (Keyboard_IsKeyDown(KK_D4))
	//	m_OffsetHandPosition.y += dt;
	//if (Keyboard_IsKeyDown(KK_D5))
	//	m_OffsetHandPosition.z += -dt;
	//if (Keyboard_IsKeyDown(KK_D6))
	//	m_OffsetHandPosition.z += dt;
	//Owner()->Transform()->SetLocalPosition(m_OffsetHandPosition);
	//Vector3::Printf(m_OffsetSpinePosition);

*/