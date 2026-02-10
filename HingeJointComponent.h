/*
	HingeJointComponent.h
	20251204  hanaue sho
	ヒンジジョイント
	1軸にのみ回転をゆるすジョイント
*/
#ifndef HINGEJOINTCOMPONENT_H_
#define HINGEJOINTCOMPONENT_H_
#include "Component.h"
#include "Vector3.h"
#include "GameObject.h"
#include "physicsSystem.h"

class Rigidbody;

class HingeJointComponent : public Component
{
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
	// 固定軸（ローカル座標）
	// --------------------------------------------------
	Vector3 m_Axis = Vector3(1.0f, 0.0f, 0.0f);

	// ==================================================
	// ----- 角度リミット -----
	// ==================================================
	// --------------------------------------------------
	// 角度リミット有効化
	// --------------------------------------------------
	bool m_EnableLimit = false;
	// --------------------------------------------------
	// 最小、最大角度（ラジアン角）
	// --------------------------------------------------
	float m_LimitMin = 0.0f;
	float m_LimitMax = 1.0f;
	// --------------------------------------------------
	// ローカル基底ベクトル（リミットなどに使う）
	// RefB は Register 内で自動で計算される
	// --------------------------------------------------
	Vector3 m_LocalRefA = { 0.0f, 0.0f, 0.0f };
	Vector3 m_LocalRefB = { 0.0f, 0.0f, 0.0f };

	// ==================================================
	// ----- モーター -----
	// ==================================================
	// --------------------------------------------------
	// モーター有効化
	// --------------------------------------------------
	bool m_EnableMotor = false;
	// --------------------------------------------------
	// 目標角速度[rad/s]
	// --------------------------------------------------
	float m_MotorSpeed = 0.0f;
	// --------------------------------------------------
	// 最大トルク
	// --------------------------------------------------
	float m_MaxMotorTorque = 0.0f;

	// ==================================================
	// ----- バネ -----
	// ==================================================
	// --------------------------------------------------
	// バネ有効化
	// --------------------------------------------------
	bool m_EnableSpring = false;
	// --------------------------------------------------
	// 目標角度
	// --------------------------------------------------
	float m_SpringTarget = 0.0f;
	// --------------------------------------------------
	// バネ剛性
	// --------------------------------------------------
	float m_SpringStiffness = 0.0f;
	// --------------------------------------------------
	// バネ減衰
	// --------------------------------------------------
	float m_SpringDamping = 0.0f;

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
	void SetAxis(const Vector3& axis) { m_Axis = axis.normalized(); }
	void SetEnableLimit(bool b) { m_EnableLimit = b; }
	void SetLimitMin(float min) { m_LimitMin = min; }
	void SetLimitMax(float max) { m_LimitMax = max; }
	void SetEnableMotor(bool b) { m_EnableMotor = b; }
	void SetMotorSpeed(float speed)   { m_MotorSpeed = speed; }
	void SetMaxMotorTorque(float max) { m_MaxMotorTorque = max; }
	void SetEnableSpring(bool b)   { m_EnableSpring = b; }
	void SetSpringTarget(float t)  { m_SpringTarget = t; }
	void SetSpringDamping(float d) { m_SpringDamping = d; }
	void SetSpringStiffness(float d) { m_SpringStiffness = d; }
	void SetLocalRefA(const Vector3& ref) { m_LocalRefA = ref; }
	void SetLocalRefB(const Vector3& ref) { m_LocalRefB = ref; }

	Rigidbody* Body()		const { return m_pRigidbody; }
	Rigidbody* OtherBody()	const { return m_pOtherObject ? m_pOtherObject->GetComponent<Rigidbody>() : nullptr; }
	GameObject* OtherObject() const { return m_pOtherObject; }
	const Vector3& LocalAnchorA() const { return m_LocalAnchorA; }
	const Vector3& LocalAnchorB() const { return m_LocalAnchorB; }
	const Vector3& Axis() const { return m_Axis.normalized(); }
	const bool EnableLimit() const { return m_EnableLimit; }
	const float LimitMin() const { return m_LimitMin; }
	const float LimitMax() const { return m_LimitMax; }
	const bool EnableMotor() const { return m_EnableMotor; }
	const float MotorSpeed() const { return m_MotorSpeed; }
	const float MaxMotorTorque() const { return m_MaxMotorTorque; }
	const bool EnableSpring() const { return m_EnableSpring; }
	const float SpringTarget() const { return m_SpringTarget; }
	const float SpringDamping() const { return m_SpringDamping; }
	const float SpringStiffness() const { return m_SpringStiffness; }
	const Vector3& LocalRefA() const { return m_LocalRefA; }
	const Vector3& LocalRefB() const { return m_LocalRefB; }

	int JointId() const { return m_JointId; }
	void SetJointId(int id) { m_JointId = id; }

	// ==================================================
	// ----- PhysicsSystemへの登録 -----
	// ==================================================
	void RegisterToPhysicsSystem()
	{
		// 相方などをセットしてから登録する（本当は更新する関数の方が良きかな）
		Manager::GetScene()->physicsSystem().RegisterHingeJoint(this);
	}

	// ==================================================
	// ----- ライフサイクル -----
	// ==================================================
	void OnAdded() override
	{
		m_pRigidbody = Owner()->GetComponent<Rigidbody>();
		if (!m_pRigidbody)
		{
			assert(false && "HingeJoint requires Rigidbody on owner");
			return;
		}
		// PhysicsSystem に登録はここでは行わない

	}
	void Uninit() override
	{
		// PhysicsSystem から削除
		Manager::GetScene()->physicsSystem().UnregisterHingeJoint(this);
	}
};

#endif