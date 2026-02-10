/*
	RigidbodyComponent.h
	20250902  hanaue sho
*/
#ifndef RIGIDBODYCOMPONENT_H_
#define RIGIDBODYCOMPONENT_H_
#define NOMINMAX
#include <algorithm>
#include "Component.h"
#include "Vector3.h"
#include "PhysicsSystem.h"
#include "Manager.h"
#include "Scene.h"
#undef min 
#undef max 

class Rigidbody : public Component
{
public:
	// ==================================================
	// ----- 列挙体 -----
	// ==================================================
    // --------------------------------------------------
	// ボディタイプ 
	// static    : 衝突判定のみ
	// Kinematic : 位置指定のみ反映
	// Dynamic   : 自然落下＋衝突解決全てを行う
    // --------------------------------------------------
	enum class BodyType { Static, Kinematic, Dynamic };

	// --------------------------------------------------
	// 重力設定 
	// Global : 共通の重力
	// Custom : カスタム（任意）
	// None   : 無重力
    // --------------------------------------------------
	enum class GravityMode {Global, Custom, None};

private:
	// ==================================================
	// ----- 要素 -----
	// ==================================================
    // --------------------------------------------------
	// ボディタイプ
    // --------------------------------------------------
	BodyType m_BodyType = BodyType::Dynamic;
	// --------------------------------------------------
	// 質量 
    // --------------------------------------------------
	float m_Mass	= 1.0f; // 質量
	float m_InvMass = 1.0f; // 逆質量 （Dynamic && mass>0 ならば 1/mass, それ以外は0）
	Vector3 m_CenterOfMassLocal{ 0, 0, 0 }; // 重心座標
	Vector3 m_CenterOfMassPositionWorld{ 0, 0, 0 }; // 中心座標
	// --------------------------------------------------
	// 速度・角速度
    // --------------------------------------------------
	Vector3 m_Velocity{ 0, 0, 0 }; // 線形速度
	Vector3 m_ForceAccum{ 0, 0, 0 }; // 力の積分
	Vector3 m_AngularVelocity{ 0, 0, 0 }; // 角速度
	Vector3 m_TorqueAccum{0, 0, 0}; // 累積トルク
	// --------------------------------------------------
	// 見かけの速度・角速度（Kinematic 用）
	// --------------------------------------------------
	Vector3	   m_PrevPosition{ 0, 0, 0 };	// １つ前の速度
	Quaternion m_PrevRotation{ 0, 0, 0, 1}; // １つ前の回転
	Vector3 m_KinematicVelocity{ 0, 0, 0 };
	Vector3 m_KinematicAngularVelocity{ 0, 0, 0 };
	bool m_HasPrevKinematicPose = false; // １フレーム目用
	// --------------------------------------------------
	// 回転（慣性テンソル）
    // --------------------------------------------------
	Matrix4x4 m_InertiaShapeInv  = Matrix4x4{} * 1.0f; // ローカル慣性テンソルの逆行列（ローカル主慣性軸でどの軸が回りやすいかを表す。値が大きいほど“回りやすい”）（姿勢に寄らない）
	Matrix4x4 m_InertiaBodyInv  = Matrix4x4{} * 1.0f; // オフセット回転を反映した剛体ローカル（姿勢に寄らない）
	Matrix4x4 m_InertiaWorldInv {}; // ワールド慣性逆行列（今のワールド軸に対してどれだけ回りやすいかを表す）（姿勢に寄る）
	// --------------------------------------------------
	// 反発係数
    // --------------------------------------------------
	float m_Restitution = 0.0f; // 反発係数
	// --------------------------------------------------
	// 摩擦・減衰
	// --------------------------------------------------
	float m_FrictionDynamic = 0.45f;   // 動摩擦係数μ
	float m_FrictionStatic  = 0.6f;   // 静止摩擦係数μ
	float m_LinDamping = 0.5f; // 線形減衰係数[1/s]
	float m_AngDamping = 0.5f; // 角減衰係数  [1/s]
	// --------------------------------------------------
	// 重力
	// --------------------------------------------------
	GravityMode m_GravityMode = GravityMode::Global;  // 既定：グローバルを使う
	float		m_GravityScale = 1.0f;				  // １＝通常、０＝無重力
	Vector3		m_CustomGravity{ 0.0f, -9.8f, 0.0f }; // Custom時に使う加速度

	// ==================================================
	// ----- 小ヘルパ関数 -----
	// ==================================================
	// --------------------------------------------------
	// 逆質量の再計算 
	// --------------------------------------------------
	void RecalcInvMass() 
	{
		if (m_BodyType == BodyType::Dynamic && m_Mass > 0) m_InvMass = 1.0f / m_Mass;
		else											   m_InvMass = 0.0f;
	}

public:
	// ==================================================
    // ----- セッター・ゲッター -----
	// ==================================================
    // --------------------------------------------------
	// ボディタイプ
    // --------------------------------------------------
	void SetBodyType(BodyType bt) { m_BodyType = bt; RecalcInvMass(); }
	void SetBodyTypeDynamic()   { m_BodyType = BodyType::Dynamic;   RecalcInvMass(); }
	void SetBodyTypeKinematic() { m_BodyType = BodyType::Kinematic; RecalcInvMass(); }
	void SetBodyTypeStatic()	{ m_BodyType = BodyType::Static;	RecalcInvMass(); }
	BodyType GetBodyType()	const { return m_BodyType; }	// BodyType
	bool  IsDynamic()		const { return m_BodyType == BodyType::Dynamic; }
	bool  IsKinematic()		const { return m_BodyType == BodyType::Kinematic; }
	bool  IsStatic()		const { return m_BodyType == BodyType::Static; }
	// --------------------------------------------------
	// 質量 
	// --------------------------------------------------	
	void  SetMass(float m) { m_Mass = std::max(m, 0.0f); RecalcInvMass();} 
	float Mass()	const  { return m_Mass; }
	float InvMass()	const  { return m_InvMass; }
	void  SetCenterOfMassLocal(const Vector3& c) { m_CenterOfMassLocal = c; }
	const Vector3& CenterOfMassLocal() const { return m_CenterOfMassLocal; }
	// --------------------------------------------------
	// 速度・角速度
    // --------------------------------------------------
	const Vector3& Velocity() const { return m_Velocity; }
	void  SetVelocity(const Vector3& v) {m_Velocity = v; }
	const Vector3& AngularVelocity() const { return m_AngularVelocity; }
	void  SetAngularVelocity(const Vector3& v) { m_AngularVelocity = v; }
	// --------------------------------------------------
	// 見かけの速度・角速度（Kinematic 用）
	// --------------------------------------------------
	const Vector3& PrevPosition() const { return m_PrevPosition; }
	void SetPrevPosition(const Vector3& pos) { m_PrevPosition = pos; }
	const Quaternion& PrevRotation() const { return m_PrevRotation; }
	void SetPrevRotation(const Quaternion& q) { m_PrevRotation = q; }
	const Vector3& KinematicVelocity() const { return m_KinematicVelocity; }
	void SetKinematicVelocity(const Vector3& v) { m_KinematicVelocity = v; }
	const Vector3& KinematicAngularVelocity() const { return m_KinematicAngularVelocity; }
	void SetKinematicAngularVelocity(const Vector3& v) { m_KinematicAngularVelocity = v; }
	bool HasPrevKinematicPose() const { return m_HasPrevKinematicPose; }
	void SetHasPrevKinematicPose(bool b) { m_HasPrevKinematicPose = b; }
	// --------------------------------------------------
	// 速度・角速度の取得API
	// Kinematic は見かけの速度を返す
	// --------------------------------------------------
	Vector3 GetLinearVelocitySolver() const
	{
		if (IsDynamic())   return Velocity();
		if (IsKinematic()) return KinematicVelocity();
		return Vector3{0, 0, 0};
	}
	Vector3 GetAngularVelocitySolver() const
	{
		if (IsDynamic())   return AngularVelocity();
		if (IsKinematic()) return KinematicAngularVelocity();
		return Vector3{0, 0, 0};
	}
	// --------------------------------------------------
	// 回転（慣性テンソル）
	// --------------------------------------------------
	void             SetInertiaShapeInv(const Matrix4x4& invLocal) { m_InertiaShapeInv = invLocal; }
	const Matrix4x4& InertiaShapeInv() const { return m_InertiaShapeInv; }
	const Matrix4x4& InertiaWorldInv() const { return m_InertiaWorldInv; }
	// --------------------------------------------------
	// 反発係数
	// --------------------------------------------------
	float Restitution() const { return m_Restitution; }
	void  SetRestitution(float e)  { m_Restitution = e; } 
	// --------------------------------------------------
	// 摩擦・減衰
	// --------------------------------------------------
	float FrictionDynamic() const { return m_FrictionDynamic; }  // 動摩擦力
	void  SetFrictionDynamic(float f) { m_FrictionDynamic = f; }
	float FrictionStatic() const { return m_FrictionStatic; }	 // 静止摩擦力
	void  SetFrictionStatic(float f) { m_FrictionStatic = f; }
	float LinDamping() const { return m_LinDamping; }	// 線形減衰
	void  SetLinDamping(float d) { m_LinDamping = d; }
	float AngDamping() const { return m_AngDamping; }	// 角速度減衰
	void  SetAngDamping(float d) { m_AngDamping = std::max(0.0f, d); }
	// --------------------------------------------------
	// 重力
	// --------------------------------------------------
	void SetGravityMode(GravityMode m) { m_GravityMode = m; }
	GravityMode GravityMode() const { return m_GravityMode; }
	void SetGravityScale(float f) { m_GravityScale = f; }
	float GravityScale() const { return m_GravityScale; }
	void SetCustomGravity(const Vector3& g) { m_CustomGravity = g; }
	Vector3 CustomGravity() const { return m_CustomGravity; }

	// ==================================================
	// ----- 慣性テンソル関係 -----
	// ==================================================
	// --------------------------------------------------
	// InertiaLocalInv を現在の姿勢で更新
	// --------------------------------------------------
	void UpdateInertiaWorldInvFrom(const Quaternion& worldQ)
	{
		Matrix4x4 R = worldQ.ToMatrix().RotationNormalized(); // 純回転行列
		Matrix4x4 Rt = R.Transpose(); // 逆行列（純回転行列なので転置）
		m_InertiaWorldInv = Rt * m_InertiaBodyInv * R;
	}
	// --------------------------------------------------
	// トルク→角加速度の角項を計算するときのヘルパ（InertiaInvを適用させる）
	// --------------------------------------------------
	Vector3 ApplyInvInertiaWorld(const Vector3& v) const
	{
		if (!IsDynamic()) return Vector3();
		return m_InertiaWorldInv.TransformNormal(v); // 3x3 部分だけを掛ける
	}
	// --------------------------------------------------
	// InertiaBodyInv の計算
	// --------------------------------------------------
	void UpdateInertiaBodyInvFromOffset(const Quaternion& offsetQ)
	{
		Quaternion q = offsetQ.normalized();
		// offset 回転
		Matrix4x4 R = q.ToMatrix().RotationNormalized();
		Matrix4x4 Rt = R.Transpose();
		
		// 形状主軸(Shape) → 剛体ローカル(Body)へ回す
		m_InertiaBodyInv = Rt * m_InertiaShapeInv * R; // Ibody^(-1) = offsetR^(T) * Ishape^(-1) * offsetR
	}
	// --------------------------------------------------
	// InertiaShapeInv の計算
	// --------------------------------------------------
	void ComputeBoxInertia(const Vector3& half) { ComputeBoxInertia(half, m_Mass); }
	void ComputeBoxInertia(const Vector3& half, float mass)
	{
		float a = 2 * half.x, b = 2 * half.y, c = 2 * half.z;
		float Ix = (mass / 12.0f) * (b * b + c * c);
		float Iy = (mass / 12.0f) * (c * c + a * a);
		float Iz = (mass / 12.0f) * (a * a + b * b);
		m_InertiaShapeInv.identity();
		m_InertiaShapeInv.m[0][0] = 1.0f / Ix; // 逆数
		m_InertiaShapeInv.m[1][1] = 1.0f / Iy; 
		m_InertiaShapeInv.m[2][2] = 1.0f / Iz; 
	}
	void ComputeSphereInertia(float radius) { ComputeSphereInertia(radius, m_Mass); }
	void ComputeSphereInertia(float radius, float mass)
	{
		const float invI = 5.0f / (2.0f * mass * radius * radius);
		m_InertiaShapeInv.identity();
		m_InertiaShapeInv.m[0][0] = invI;
		m_InertiaShapeInv.m[1][1] = invI;
		m_InertiaShapeInv.m[2][2] = invI;
	}

	// --------------------------------------------------
	// CenterOfMass をワールドへ変換（現在はOnAddedでキャッシュしてる）
	// --------------------------------------------------
	void SetWorldCOM(const Vector3& pos) { m_CenterOfMassPositionWorld = pos; }
	Vector3 WorldCOM() const
	{
		return m_CenterOfMassPositionWorld;
	}

	// ==================================================
	// ----- Force 関係 -----
	// ==================================================
	// --------------------------------------------------
	// 力を加える
	// --------------------------------------------------
	void AddForce(const Vector3& f) 
	{ 
		m_ForceAccum += f; 
	}
	// --------------------------------------------------
	// インパルス
	// --------------------------------------------------
	void ApplyImpulse(const Vector3& P)
	{
		if (!IsDynamic()) return;
		m_Velocity += P * m_InvMass;
	}
	// --------------------------------------------------
	// インパルス（座標指定）
	// --------------------------------------------------
	void ApplyImpulseAtPoint(const Vector3& P, const Vector3& worldP)
	{
		if (!IsDynamic()) return;
		m_Velocity += P * m_InvMass;
		const Vector3 r = worldP - WorldCOM();
		m_AngularVelocity += ApplyInvInertiaWorld(Vector3::Cross(r, P));
	}
	// --------------------------------------------------
	// 力の消費
	// --------------------------------------------------
	Vector3 ConsumeForces() 
	{ 
		Vector3 f = m_ForceAccum; m_ForceAccum = { 0, 0, 0 }; return f; 
	} 

	// ==================================================
	// ----- Torque 関係 -----
	// ==================================================
	// --------------------------------------------------
	// 回転を与える
	// --------------------------------------------------
	void AddTorque(const Vector3& tau) 
	{
		if (!IsDynamic()) return;
		m_TorqueAccum += tau;
	}
	// --------------------------------------------------
	// 回転を与える
	// F : 世界座標の力ベクトル[N]
	// WorldP : その力を加える世界座標上の点[m]
	// --------------------------------------------------
	void AddForceAtPoint(const Vector3& F, const Vector3& worldP)
	{
		if (!IsDynamic()) return;

		// 線形
		m_ForceAccum += F;

		// トルク
		const Vector3 r = worldP - WorldCOM(); // 中心からのベクトル
		const Vector3 tau = Vector3::Cross(r, F); // 距離に依存してトルクの大きさが変わる
		m_TorqueAccum += tau;
	}
	// --------------------------------------------------
	// トルクの消費
	// --------------------------------------------------
	Vector3 ConsumeTorques() 
	{ 
		Vector3 t = m_TorqueAccum; m_TorqueAccum = { 0, 0, 0 }; return t; 
	} 

	// ==================================================
	// ----- ライフサイクル -----
	// ==================================================
	void OnAdded() override
	{
		// 登録する
		Manager::GetScene()->physicsSystem().RegisterRigidbody(this);

		auto* tr = Owner()->GetComponent<TransformComponent>();
		m_CenterOfMassPositionWorld = tr->Position() + tr->Rotation().Rotate(m_CenterOfMassLocal);
		//tr->WorldMatrix().TransformPoint(m_CenterOfMassLocal); // 旧
	}
	void FixedUpdate(float dt) override
	{
		// ここで Transform と同期
		//if (!IsDynamic())
		{
			auto* tr = Owner()->GetComponent<TransformComponent>();
			m_CenterOfMassPositionWorld = tr->Position() + tr->Rotation().Rotate(m_CenterOfMassLocal);
		}
	}
	void Uninit() override
	{
		// 削除処理
		Manager::GetScene()->physicsSystem().UnregisterRigidbody(this);
	}
};

#endif