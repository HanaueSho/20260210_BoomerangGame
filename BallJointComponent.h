/*
	BallJointComponent.h
	20251130  hanaue sho
	ボールジョイント
*/
#ifndef BALLJOINTCOMPONENT_H_
#define BALLJOINTCOMPONENT_H_
#include "Component.h"
#include "Vector3.h"
#include "GameObject.h"
#include "PhysicsSystem.h"

class Rigidbody;

class BallJointComponent : public Component
{
private:
	// ==================================================
	// ----- 要素 -----
	// ==================================================
	// --------------------------------------------------
	// 自身のRigidbody
	// --------------------------------------------------
	Rigidbody* m_pRigidbody = nullptr;
	// --------------------------------------------------
	// 相方のポインタ
	// --------------------------------------------------
	GameObject* m_pOtherObject = nullptr;
	// --------------------------------------------------
	// 各アンカー（ローカル座標）
	// --------------------------------------------------
	Vector3 m_LocalAnchorA = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 m_LocalAnchorB = Vector3(0.0f, 0.0f, 0.0f);
	// --------------------------------------------------
	// PhysicsSystem の採番
	// --------------------------------------------------
	int		m_JointId = -1;
public:
	// ==================================================
	// ----- セッター・ゲッター -----
	// ==================================================
	void SetOtherObject(GameObject* other) { m_pOtherObject = other; }
	void SetLocalAnchorA(const Vector3& anchor) { m_LocalAnchorA = anchor; }
	void SetLocalAnchorB(const Vector3& anchor) { m_LocalAnchorB = anchor; }

	Rigidbody* Body()		const { return m_pRigidbody; }
	Rigidbody* OtherBody()	const { return m_pOtherObject ? m_pOtherObject->GetComponent<Rigidbody>() : nullptr; }
	GameObject* OtherObject() const { return m_pOtherObject; }
	const Vector3& LocalAnchorA() const { return m_LocalAnchorA; }
	const Vector3& LocalAnchorB() const { return m_LocalAnchorB; }

	int JointId() const { return m_JointId; }
	void SetJointId(int id) { m_JointId = id; }

	// ==================================================
	// ----- PhysicsSystemへの登録 -----
	// ==================================================
	void RegisterToPhysicsSystem()
	{
		// 相方などをセットしてから登録する（本当は更新する関数の方が良きかな）
		Manager::GetScene()->physicsSystem().RegisterBallJoint(this);
	}

	// ==================================================
	// ----- ライフサイクル -----
	// ==================================================
	void OnAdded() override
	{
		m_pRigidbody = Owner()->GetComponent<Rigidbody>();
		if (!m_pRigidbody)
		{
			assert(false && "BallJoint requires Rigidbody on owner");
			return;
		}
		// PhysicsSystem に登録はここでは行わない

	}
	void Uninit() override
	{
		// PhysicsSystem から削除
		Manager::GetScene()->physicsSystem().UnregisterBallJoint(this);
	}
};

#endif