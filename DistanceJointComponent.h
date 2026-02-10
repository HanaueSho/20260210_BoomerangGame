/*
	DistanceJointComponent.h
	20251119  hanaue sho
	距離ジョイントコンポーネント
*/
#ifndef DISTANCEJOINTCOMPONENT_H_
#define DISTANCEJOINTCOMPONENT_H_
#include "Component.h"
#include "Vector3.h"
#include "GameObject.h"
#include "PhysicsSystem.h"

class Rigidbody;

class DistanceJointComponent : public Component
{
public:
	enum DistanceJointMode // 巡回インクルードのせいで今は単純なenumで定義
	{
		Rod,		// 押し引き両方
		Rope,		// 引っ張りだけ
		Compression // 押しだけ
	};

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
	// アンカーの距離
	// --------------------------------------------------
	float	m_RestLength = 1.0f;
	// --------------------------------------------------
	// アンカーの種類
	// --------------------------------------------------
	DistanceJointMode m_Mode = DistanceJointMode::Rod;
	// --------------------------------------------------
	// PhysicsSystem の採番
	// --------------------------------------------------
	int		m_JointId = -1;
public:
	// ==================================================
	// ----- セッター・ゲッター -----
	// ==================================================
	void SetOtherObject(GameObject* other) { m_pOtherObject = other; }
	void SetRestLength (float length)	   { m_RestLength = length; }
	void SetLocalAnchorA(const Vector3& anchor) { m_LocalAnchorA = anchor; }
	void SetLocalAnchorB(const Vector3& anchor) { m_LocalAnchorB = anchor; }
	void SetMode(DistanceJointMode mode) { m_Mode = mode; }

	Rigidbody* Body()		const { return m_pRigidbody; }
	Rigidbody* OtherBody()	const { return m_pOtherObject ? m_pOtherObject->GetComponent<Rigidbody>() : nullptr; }
	GameObject* OtherObject() const { return m_pOtherObject; }
	const Vector3&	 LocalAnchorA() const { return m_LocalAnchorA; }
	const Vector3&	 LocalAnchorB() const { return m_LocalAnchorB; }
	float			 RestLength()	const { return m_RestLength; }
	DistanceJointMode Mode()		const { return m_Mode; }

	int JointId() const { return m_JointId; }
	void SetJointId(int id) { m_JointId = id; }

	// ==================================================
	// ----- PhysicsSystemへの登録 -----
	// ==================================================
	void RegisterToPhysicsSystem()
	{
		// 相方などをセットしてから登録する（本当は更新する関数の方が良きかな）
		Manager::GetScene()->physicsSystem().RegisterDistanceJoint(this);
	}

	// ==================================================
	// ----- ライフサイクル -----
	// ==================================================
	void OnAdded() override
	{
		m_pRigidbody = Owner()->GetComponent<Rigidbody>();
		if (!m_pRigidbody)
		{
			assert(false && "DistanceJoint requires Rigidbody on owner");
			return;
		}
		// PhysicsSystem に登録はここでは行わない

	}
	void Uninit() override
	{
		// PhysicsSystem から削除
		Manager::GetScene()->physicsSystem().UnregisterDistanceJoint(this);
	}
};

#endif