/*
	ColliderComponent.h
	20250821  hanaue sho
	コライダー系のコンポーネント
*/
#ifndef COLLIDERCOMPONENT_H_
#define COLLIDERCOMPONENT_H_
#include <memory>
#include "Component.h"
#include "TransformComponent.h"
#include "GameObject.h"
#include "Collision.h"
#include "Vector3.h"
#include "manager.h"
#include "Scene.h"
#include "PhysicsSystem.h"
#include "ColliderPose.h"
#include "RigidbodyComponent.h" // 循環参照になりかねないからあんまりよくないケド


// ==================================================
// 構造体
// ==================================================
enum class ColliderType
{
	Box,
	Sphere,
	Capsule,
	MeshField,
};

class Collider : public Component
{
public:
	enum class Mode
	{
		Simulate,  // 通常
		Trigger,  // トリガー
		QueryOnly // Solve 対象外
	};

private:
	// ==================================================
	// ----- 本体要素 -----
	// ==================================================
	// --------------------------------------------------
	// シェープ
	// --------------------------------------------------
	std::unique_ptr<Collision> m_pShape = nullptr;
	// --------------------------------------------------
	// オフセット
	// --------------------------------------------------
	Vector3 m_OffsetPositionLocal{0, 0, 0};
	Quaternion m_OffsetRotationLocal = Quaternion();
	bool m_OffsetDirty = true;
	// --------------------------------------------------
	// オフセット込みの Transform
	// --------------------------------------------------
	ColliderPose m_WorldPose{};
	// --------------------------------------------------
	// 広域探査用のAABB
	// --------------------------------------------------
	AABB m_WorldAABB{}; // 広域探査で使う
	// --------------------------------------------------
	// PhyisicsSystem 用の採番
	// 初期値は-1
	// --------------------------------------------------
	int m_Id = -1; // PhysicsSystem 採番
	// --------------------------------------------------
	// モード
	// Simulate: 接触解決に入れる
	// Trigger:  イベント生成
	// Query:	 
	// --------------------------------------------------
	Mode m_Mode = Mode::Simulate;
	// --------------------------------------------------
	// 有効化フラグ
	// 現在は使われていない？
	// --------------------------------------------------
	bool m_IsEnable = true;

	friend PhysicsSystem;
public:
	// ==================================================
	// ----- セッターゲッター -----
	// ==================================================
	const Collision* Shape() const noexcept { return m_pShape.get(); }
		  Collision* Shape()	   noexcept { return m_pShape.get(); }
	const ColliderPose& WorldPose() const { return m_WorldPose; }
	const AABB& WorldAABB() const { return m_WorldAABB; }
	int  Id() const { return m_Id; }
	void SetMode(Mode mode) { m_Mode = mode; }
	void SetModeSimulate() { m_Mode = Mode::Simulate; }
	void SetModeTrigger()  { m_Mode = Mode::Trigger; }
	void SetModeQuery()	   { m_Mode = Mode::QueryOnly; }
	bool IsSimulate() const { return m_Mode == Mode::Simulate; }
	bool IsTrigger()  const { return m_Mode == Mode::Trigger; }
	bool IsQuery()	  const { return m_Mode == Mode::QueryOnly; }
	void SetOffsetPosition(const Vector3& offset) { m_OffsetPositionLocal = offset; UpdateWorldPose(); m_OffsetDirty = true; }
	void SetOffsetRotation(const Quaternion& offset) { m_OffsetRotationLocal = offset;UpdateWorldPose(); m_OffsetDirty = true; }
	bool OffsetDirty() { return m_OffsetDirty; }
	void ConsumeOffsetDirty() { m_OffsetDirty = false; }

	void UpdateWorldAABB() { m_WorldAABB = m_pShape->ComputeWorldAABB(m_WorldPose); }
	void UpdateWorldPose() 
	{
		auto* trans = Owner()->GetComponent<TransformComponent>();
		if (!trans) return;

		// Owner のワールド
		const Vector3 ownerPos = trans->Position();
		const Quaternion ownerRot = trans->Rotation();
		const Vector3 ownerScale = trans->LossyScale();

		// offset 回転を念のため正規化
		Quaternion offsetRot = m_OffsetRotationLocal;
		offsetRot = offsetRot.normalized();

		// offset 位置を owner 回転で回して加算
		const Vector3 offsetPosWorld = ownerRot.Rotate(m_OffsetPositionLocal);
		m_WorldPose.position = ownerPos + offsetPosWorld;

		// 回転の合成
		m_WorldPose.rotation = ownerRot * offsetRot;
		m_WorldPose.rotation = m_WorldPose.rotation.normalized(); // 正規化

		// スケールはとりあえずそのまま
		m_WorldPose.scale = ownerScale;
	}

	// ----- 形状セッター -----
	void SetShape  (std::unique_ptr<Collision> shp)		{ m_pShape = std::move(shp); UpdateWorldAABB(); }
	void SetBox	   (const Vector3& halfSize)			{ auto ptr = std::make_unique<BoxCollision>(halfSize);					 SetShape(std::move(ptr)); m_Type = ColliderType::Box; }
	void SetSphere (float radius)						{ auto ptr = std::make_unique<SphereCollision>(radius);					 SetShape(std::move(ptr)); m_Type = ColliderType::Sphere; }
	void SetCapsule(float radius, float cylinderHeight) { auto ptr = std::make_unique<CapsuleCollision>(radius, cylinderHeight); SetShape(std::move(ptr)); m_Type = ColliderType::Capsule; }
	void SetMeshField(int width, int depth, float cellSizeX, float cellSizeZ, std::vector<float> heights) { auto ptr = std::make_unique<HeightMapCollision>(width, depth, cellSizeX, cellSizeZ, heights); SetShape(std::move(ptr)); m_Type = ColliderType::MeshField; }

	// ==================================================
	// ----- ライフサイクル -----
	// ==================================================
	void OnAdded() override
	{
		//RegisterColiderの中でUpdateWorldAABBを呼ぶので先に初期化する
		SetSphere(1);

		// ここで World 姿勢の同期
		UpdateWorldPose();
		 
		// ここで PhysicsSystem に登録（ID採番）
		m_Id = Manager::GetScene()->physicsSystem().RegisterCollider(this);
	}
	void Uninit() override
	{
		// 削除処理
		Manager::GetScene()->physicsSystem().UnregisterCollider(this);
	}
	void FixedUpdate(float dt) override
	{
		UpdateWorldAABB(); // 形状の同期
		UpdateWorldPose(); // 形状の同期
	}

	void OnTriggerEnter(class Collider* me, class Collider* other) override
	{
		printf("当たったよ");
	}
	void OnTriggerExit(class Collider* me, class Collider* other)
	{
		printf("離れたよ");		
	}

public:
	ColliderType m_Type = ColliderType::Box;
};

#endif