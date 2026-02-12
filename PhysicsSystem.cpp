/*
	PhysicsSystem.cpp
	20250821  hanaue sho
	物理演算
*/
#include <assert.h>
#include <algorithm>
#include "PhysicsSystem.h"
#include "Scene.h"
#include "Component.h"
#include "TransformComponent.h"
#include "ColliderComponent.h"
#include "RigidbodyComponent.h"
#include "DistanceJointComponent.h"
#include "BallJointComponent.h"
#include "HingeJointComponent.h"
#include "Collision.h"
#include "ContactManifold.h"

#include "DebugRenderer.h"

namespace
{
	Vector3 Gravity = { 0.0f, -9.8f, 0.0f }; // 必要なら０
	const float Slop = 0.005f;				 // 許容量（大きすぎるとめり込むよ）
	const float RestThreshold = 0.2f;		 // 反発バイアス許容量
	const float Baumgarte = 0.2f;			 // 位置補正の係数

	// クランプ
	float Clamp(float value, float min, float max)
	{
		return value < min ? min : (value > max ? max : value);
	}
	// PassModeFilter
	bool PassModeFilter(const Collider* col, const QueryOptions& opt)
	{
		if (!col) return false;
		if (col->IsSimulate()) return opt.isSimulate;
		if (col->IsTrigger())  return opt.isTrigger;
		if (col->IsQuery())	   return opt.isQueryOnly;
		return false;
	}
	// 最大深度
	float GetMaxPenetration(const ContactManifold& m)
	{
		float p = 0.0f;
		for (int i = 0; i < m.count; i++)
			p = std::max(p, m.points[i].penetration);
		return p;
	}
}

// --------------------------------------------------
// 初期化、終了処理
// --------------------------------------------------
void PhysicsSystem::Init()
{
	m_Colliders.clear();
	m_Rigidbodies.clear();
	m_PrevTrigger.clear();
	m_CurrTrigger.clear();
	m_PrevCollision.clear();
	m_CurrCollision.clear();
	m_TriggerEnter.clear();
	m_TriggerExit.clear();
	m_CollisionEnter.clear();
	m_CollisionExit.clear();
	m_NextId = 0; m_FreeIds.clear(); m_ById.clear();

	for (int i = 0; i < MaxLayers; i++)
		m_CollisionMask[i] = 0xFFFFFFFFu; // とりあえず全部ON
	SetCollision(30, 31, false); // 30: Playerモデルボーン, 31: CharacterControllerComponent
	SetCollision(29, 30, false); // 29: BoomerangObject
}
void PhysicsSystem::Shutdown()
{
	m_Colliders.clear();
	m_Rigidbodies.clear();
	m_PrevTrigger.clear();
	m_CurrTrigger.clear();
	m_PrevCollision.clear();
	m_CurrCollision.clear();
	m_TriggerEnter.clear();
	m_TriggerExit.clear();
	m_CollisionEnter.clear();
	m_CollisionExit.clear();
	m_pScene = nullptr;
}
void PhysicsSystem::SetCollision(int layerA, int layerB, bool enable)
{
	assert(0 <= layerA, && layerA < MaxLayers);
	assert(0 <= layerB, && layerB < MaxLayers);

	// uint32_t(1) = 0000...0001
	// uint32_t(1) << 2 = 0000...010 （下から3ビット目だけが１）
	uint32_t bitA = (uint32_t(1) << layerA);
	uint32_t bitB = (uint32_t(1) << layerB);
	if (enable)
	{
		// |= : OR演算子 のこと
		// bitB の１が立っている場所と同じ CollisionMask の場所に１を立てる
		m_CollisionMask[layerA] |= bitB;
		m_CollisionMask[layerB] |= bitA;
	}
	else
	{
		// ~ : 1111...1 とのNAND演算
		// bitB = 0000...0100 のとき ~bitB = 1111...1011
		// &= : NOT演算子（bitB の１が立っている場所を０にする処理）
		m_CollisionMask[layerA] &= ~bitB;
		m_CollisionMask[layerB] &= ~bitA;
	}
}
bool PhysicsSystem::ShouldCollide(const Collider& a, const Collider& b) const
{
	int layerA = a.Owner()->PhysicsLayer();
	int layerB = b.Owner()->PhysicsLayer();

	uint32_t maskA = m_CollisionMask[layerA];
	uint32_t maskB = m_CollisionMask[layerB];

	// お互いのマスクに相手のビットが立っているかを見る
	bool aWants = (maskA & (uint32_t(1) << layerB)) != 0;
	bool bWants = (maskB & (uint32_t(1) << layerA)) != 0;

	// マトリクスを対称にしているなら、どちらか片方のチェックだけでもいい
	return aWants && bWants;
}
// --------------------------------------------------
// 登録、解除 
// --------------------------------------------------
int PhysicsSystem::RegisterCollider(Collider* c)
{
	assert(c);
	assert(c->m_Id == -1 && "already registered");

	int id;
	if (!m_FreeIds.empty()) { id = m_FreeIds.back(); m_FreeIds.pop_back(); }
	else { id = m_NextId++; m_ById.resize(m_NextId, nullptr); }
	c->m_Id = id;
	m_ById[id] = c; // 逆引き用に登録

	c->UpdateWorldAABB();
	m_Colliders.push_back(c);
	return id;
}
void PhysicsSystem::UnregisterCollider(Collider* c)
{
	if (!c || c->m_Id < 0) return;

	// ペア集合から除去
	const int id = c->m_Id;
	for (auto it = m_PrevTrigger.begin(); it != m_PrevTrigger.end();)
	{
		int a = KeyHigh(*it), b = KeyLow(*it);
		if (a == id || b == id) it = m_PrevTrigger.erase(it);
		else it++;
	}
	for (auto it = m_CurrTrigger.begin(); it != m_CurrTrigger.end();)
	{
		int a = KeyHigh(*it), b = KeyLow(*it);
		if (a == id || b == id) it = m_CurrTrigger.erase(it);
		else it++;
	}
	for (auto it = m_PrevCollision.begin(); it != m_PrevCollision.end();)
	{
		int a = KeyHigh(*it), b = KeyLow(*it);
		if (a == id || b == id) it = m_PrevCollision.erase(it);
		else it++;
	}
	for (auto it = m_CurrCollision.begin(); it != m_CurrCollision.end();)
	{
		int a = KeyHigh(*it), b = KeyLow(*it);
		if (a == id || b == id) it = m_CurrCollision.erase(it);
		else it++;
	}

	// 逆引き
	m_ById[id] = nullptr;
	m_FreeIds.push_back(id);

	// リストから除外
	auto it = std::find(m_Colliders.begin(), m_Colliders.end(), c);
	if (it != m_Colliders.end()) { *it = m_Colliders.back(); m_Colliders.pop_back(); }
	c->m_Id = -1;
}
void PhysicsSystem::RegisterRigidbody(Rigidbody* rb)
{
	assert(rb);
	// 登録済みチェック
	auto it = std::find(m_Rigidbodies.begin(), m_Rigidbodies.end(), rb);
	if (it != m_Rigidbodies.end()) return; // 既に登録済み

	m_Rigidbodies.push_back(rb);
}
void PhysicsSystem::UnregisterRigidbody(Rigidbody* rb)
{
	if (!rb) return;
	auto it = std::find(m_Rigidbodies.begin(), m_Rigidbodies.end(), rb);
	if (it != m_Rigidbodies.end()) { *it = m_Rigidbodies.back(); m_Rigidbodies.pop_back(); } // 最後に回して取り除く
}
int PhysicsSystem::RegisterDistanceJoint(DistanceJointComponent* joint)
{
	assert(joint);
	assert(joint->JointId() == -1 && "already registered");

	int id;
	if (!m_FreeDistanceJointIds.empty()) { id = m_FreeDistanceJointIds.back(); m_FreeDistanceJointIds.pop_back(); }
	else { id = m_DistanceJointNextId++; m_DistanceJointById.resize(m_DistanceJointNextId, nullptr); }

	joint->SetJointId(id);
	m_DistanceJointById[id] = joint; // 逆引き用に登録

	DistanceJoint dj{};
	dj.jointId = id;
	dj.pBodyA = joint->Body();
	dj.pBodyB = joint->OtherBody();
	dj.tfA = joint->Owner()->Transform();
	dj.tfB = joint->OtherObject() ? joint->OtherObject()->Transform() : nullptr;
	dj.restLength = joint->RestLength();
	dj.localAnchorA = joint->LocalAnchorA();
	dj.localAnchorB = joint->LocalAnchorB();
	dj.mode = joint->Mode();

	m_DistanceJoints.push_back(dj);
	return id;
}
void PhysicsSystem::UnregisterDistanceJoint(DistanceJointComponent* joint)
{
	if (!joint || joint->JointId() < 0) return;

	const int id = joint->JointId();

	// 逆引き
	m_DistanceJointById[id] = nullptr;
	m_FreeDistanceJointIds.push_back(id);

	// リストから除外
	auto it = std::find_if(m_DistanceJoints.begin(), m_DistanceJoints.end(), [id](const DistanceJoint& dj) { return dj.jointId == id; });
	if (it != m_DistanceJoints.end()) { *it = m_DistanceJoints.back(); m_DistanceJoints.pop_back(); }
	joint->SetJointId(-1);
}
int PhysicsSystem::RegisterBallJoint(BallJointComponent* joint)
{
	assert(joint);
	assert(joint->JointId() == -1 && "already registered");

	int id;
	if (!m_FreeBallJointIds.empty()) { id = m_FreeBallJointIds.back(); m_FreeBallJointIds.pop_back(); }
	else { id = m_BallJointNextId++; m_BallJointById.resize(m_BallJointNextId, nullptr); }

	joint->SetJointId(id);
	m_BallJointById[id] = joint; // 逆引き用に登録

	BallJoint bj{};
	bj.jointId = id;
	bj.pBodyA = joint->Body();
	bj.pBodyB = joint->OtherBody();
	bj.tfA = joint->Owner()->Transform();
	bj.tfB = joint->OtherObject() ? joint->OtherObject()->Transform() : nullptr;
	bj.localAnchorA = joint->LocalAnchorA();
	bj.localAnchorB = joint->LocalAnchorB();

	m_BallJoints.push_back(bj);
	return id;
}
void PhysicsSystem::UnregisterBallJoint(BallJointComponent* joint)
{
	if (!joint || joint->JointId() < 0) return;

	const int id = joint->JointId();

	// 逆引き
	m_BallJointById[id] = nullptr;
	m_FreeBallJointIds.push_back(id);

	// リストから除外
	auto it = std::find_if(m_BallJoints.begin(), m_BallJoints.end(), [id](const BallJoint& bj) { return bj.jointId == id; });
	if (it != m_BallJoints.end()) { *it = m_BallJoints.back(); m_BallJoints.pop_back(); }
	joint->SetJointId(-1);
}
int PhysicsSystem::RegisterHingeJoint(HingeJointComponent* joint)
{
	assert(joint);
	assert(joint->JointId() == -1 && "already registered");

	int id;
	if (!m_FreeHingeJointIds.empty()) { id = m_FreeHingeJointIds.back(); m_FreeHingeJointIds.pop_back(); }
	else { id = m_HingeJointNextId++; m_HingeJointById.resize(m_HingeJointNextId, nullptr); }

	joint->SetJointId(id);
	m_HingeJointById[id] = joint; // 逆引き用に登録

	HingeJoint hj{};
	hj.jointId = id;
	hj.pBodyA = joint->Body();
	hj.pBodyB = joint->OtherBody();
	hj.tfA = joint->Owner()->Transform();
	hj.tfB = joint->OtherObject() ? joint->OtherObject()->Transform() : nullptr;
	hj.localAnchorA = joint->LocalAnchorA();
	hj.localAnchorB = joint->LocalAnchorB();
	hj.localAxisA   = joint->Axis();
	hj.enableLimit = joint->EnableLimit();
	hj.limitMin    = joint->LimitMin();
	hj.limitMax    = joint->LimitMax();
	hj.motorSpeed     = joint->MotorSpeed();
	hj.maxMotorTorque = joint->MaxMotorTorque();
	hj.enableMotor    = joint->EnableMotor();
	hj.enableSpring  = joint->EnableSpring();
	hj.springTarget  = joint->SpringTarget();
	hj.springDamping = joint->SpringDamping();

	// ----- refA/B のセット -----
	Quaternion qA = hj.tfA->Rotation();
	Quaternion qB = hj.tfB->Rotation();

	Vector3 axisWorld = qA.Rotate(hj.localAxisA).normalized();

	// B 側のローカル軸に逆変換して保存
	hj.localAxisB = qB.Inverse().Rotate(axisWorld);
	
	// 軸と直行する適当な方向をワールドで作る
	Vector3 refWorld;
	if (fabsf(axisWorld.x) > 0.577f)
		refWorld = Vector3(axisWorld.y, -axisWorld.x, 0.0f);
	else
		refWorld = Vector3(0.0f, axisWorld.z, -axisWorld.y);
	refWorld.normalize();

	// A, B のローカルに戻して保存
	hj.localRefA = qA.Inverse().Rotate(refWorld);
	hj.localRefB = qB.Inverse().Rotate(refWorld);

	m_HingeJoints.push_back(hj);
	return id;
}
void PhysicsSystem::UnregisterHingeJoint(HingeJointComponent* joint)
{
	if (!joint || joint->JointId() < 0) return;

	const int id = joint->JointId();

	// 逆引き
	m_HingeJointById[id] = nullptr;
	m_FreeHingeJointIds.push_back(id);

	// リストから除外
	auto it = std::find_if(m_HingeJoints.begin(), m_HingeJoints.end(), [id](const HingeJoint& bj) { return bj.jointId == id; });
	if (it != m_HingeJoints.end()) { *it = m_HingeJoints.back(); m_HingeJoints.pop_back(); }
	joint->SetJointId(-1);
}

// ==================================================
// ステップの始まり
// 力の解決などを行う
// スクリプトの FixedUpdate を先に実行
// ==================================================
void PhysicsSystem::BeginStep(float fixedDt)
{
	m_CurrTrigger.clear();
	m_CurrCollision.clear();
	m_Contacts.clear();

	// ----- 外力→速度（重力など）-----
	IntegrationForce(fixedDt);

	// ステップの最初に WorldAABB を更新
	for (Collider* c : m_Colliders)
		c->UpdateWorldAABB();
	SyncCOM(); // COM の同期

	// ----- 見かけの速度更新 -----
	UpdateKinematicApparentVelocity(fixedDt);
}

// ==================================================
// ステップ
// 衝突判定、速度解決を行う
// ==================================================
void PhysicsSystem::Step(float fixedDt)
{
	// ----- 衝突判定 -----
	DetermineCollision();

	// ----- 衝突点事前計算 -----
	PreSolveContacts();
	// ----- ジョイント事前計算 -----
	PreSolveDistanceJoints(fixedDt);
	PreSolveBallJoints(fixedDt);
	PreSolveHingeJoints(fixedDt);

	// ----- 速度解決 -----
	for (int i = 0; i < 8; i++) 
	{ 
		ResolveDistanceJoints(fixedDt);
		ResolveBallJoints(fixedDt);
		ResolveHingeJoints(fixedDt);
		ResolveVelocity(fixedDt); 
		//printf("---------- resolve : %d\n", i); 
	}
}

// ==================================================
// ステップの終わり
// 減衰、速度積分、位置補正、ディスパッチ処理等を行う
// ==================================================
void PhysicsSystem::EndStep(float fixedDt)
{
	// ----- 減衰 -----
	ApplyDamping(fixedDt);

	// ----- 速度積分 -----
	IntegrationVelocity(fixedDt);

	// ----- 位置補正 -----
	for (int i = 0; i < 4; i++)	CorrectPosition();


	auto diffSets = [&](auto& prev, auto& curr, auto& outEnter, auto& outExit)
		{
			// Enter : Curr にあって Prev にない
			for (auto k: curr) 
				if (!prev.count(k))
				{
					outEnter.emplace_back(m_ById[KeyHigh(k)], m_ById[KeyLow(k)]); // Enter にいれる
				}
			// Exit : Prev にあって Curr にない
			for (auto k : prev) 
				if (!curr.count(k))
				{
					outExit.emplace_back(m_ById[KeyHigh(k)], m_ById[KeyLow(k)]); // Exit にいれる
				}
			prev.swap(curr); curr.clear();
		};

	diffSets(m_PrevTrigger  , m_CurrTrigger  , m_TriggerEnter  , m_TriggerExit);
	diffSets(m_PrevCollision, m_CurrCollision, m_CollisionEnter, m_CollisionExit);

	// ----- ディスパッチ処理 -----
	DispatchEvents(); 

	// ----- WorldPose の同期（これをしないと描画が１フレーム分ズレる） -----
	for (Collider* c : m_Colliders)
		c->UpdateWorldPose();
}

// --------------------------------------------------
// ディスパッチ処理
// --------------------------------------------------
void PhysicsSystem::DispatchEvents()
{
	// OnTriggerEnte, Exit の呼び出し
	auto callAll = [](Collider* me, Collider* other, bool enter, bool isTrigger)
		{
			if (auto* go = me->Owner())
			{
				go->ForEachComponent(
					[&](Component* c) {
						if (isTrigger)
						{
							if (enter) c->OnTriggerEnter(me, other);
							else	   c->OnTriggerExit(me, other);
						}
						else
						{
							if (enter) c->OnCollisionEnter(me, other);
							else	   c->OnCollisionExit(me, other);
						}
					}); // ”ラムダ”を渡している
			}
		};

	// TriggerEnter -----
	for (auto& p : m_TriggerEnter)
	{
		Collider* A = p.first;
		Collider* B = p.second;
		callAll(A, B, true, true);
		callAll(B, A, true, true);
	}
	// TriggerExit -----
	for (auto& p : m_TriggerExit)
	{
		Collider* A = p.first;
		Collider* B = p.second;
		callAll(A, B, false, true);
		callAll(B, A, false, true);
	}
	// CollisionEnter -----
	for (auto& p : m_CollisionEnter)
	{
		Collider* A = p.first;
		Collider* B = p.second;
		callAll(A, B, true, false);
		callAll(B, A, true, false);
	}
	// CollisionExit -----
	for (auto& p : m_CollisionExit)
	{
		Collider* A = p.first;
		Collider* B = p.second;
		callAll(A, B, false, false);
		callAll(B, A, false, false);
	}

	// 後始末
	m_TriggerEnter.clear();
	m_TriggerExit.clear();
	m_CollisionEnter.clear();
	m_CollisionExit.clear();
}

// ==================================================
// ----- Query 関係 -----
// ==================================================
// --------------------------------------------------
// １）いまの位置で重なっているか（最も深い１件を返す）
// --------------------------------------------------
bool PhysicsSystem::QueryOverlapBest(const Collider* self, QueryHit& outHit, const QueryOptions& opt) const
{
	if (!self) return false;
	const Collision* shapeA = self->Shape();
	if (!shapeA) return false;

	// 自分の現在姿勢でAABBを作る
	const ColliderPose& poseA = self->m_WorldPose;
	const AABB aabbA = shapeA->ComputeWorldAABB(poseA);

	bool found = false;
	float bestPenetration = -FLT_MAX;

	const int n = (int)m_Colliders.size();
	for (int i = 0; i < n; i++)
	{
		Collider* other = m_Colliders[i];
		if (!other) continue;
		if (other == self) continue;

		// Mode フィルタ
		if (!PassModeFilter(other, opt)) continue;

		// レイヤー判定
		if (!ShouldCollide(*self, *other)) continue;

		const Collision* shapeB = other->Shape();
		if (!shapeB) continue;

		// Broad
		if (!aabbA.isOverlap(other->WorldAABB()))continue;

		// Narrow
		ContactManifold m{};
		if (!shapeA->isOverlap(poseA, *shapeB, other->m_WorldPose, m, opt.slop)) continue;
		if (!m.touching || m.count <= 0) continue;

		const float pene = GetMaxPenetration(m);
		if (!found || pene >= bestPenetration)
		{
			found = true;
			bestPenetration = pene;

			outHit.other = other;
			outHit.normal = m.normal; 
			outHit.penetration = pene;
			outHit.mtd = -m.normal * pene; 
			outHit.manifold = m;
		}
	}

	return found;
}
// --------------------------------------------------
// ２）MTD（最小押し出し）だけ欲しい場合
// --------------------------------------------------
bool PhysicsSystem::QueryComputeMTD(const Collider* self, Vector3& outMTD, QueryHit* outHit, const QueryOptions& opt) const
{
	QueryHit hit{};

	if (!QueryOverlapBest(self, hit, opt))
	{
		outMTD = Vector3{};
		if (outHit) *outHit = hit;
		return false;
	}

	outMTD = hit.mtd;
	if (outHit) *outHit = hit;
	return true;
}
// --------------------------------------------------
// ３）疑似 CapsuleCast: delta 方向に動かしたときに最初に当たる位置を二分探索で探す
// --------------------------------------------------
bool PhysicsSystem::QueryCapsuleCastBinary(const Collider* self, const Vector3& delta, float skinWidth, QueryHit& outHit, const QueryOptions& opt, int binaryIterations) const
{
	if (!self) return false;
	const Collision* shapeA = self->Shape();
	if (!shapeA) return false;

	// まず、終点で重ならないなら「途中でも当たらない」とみなす
	ColliderPose poseStart = self->m_WorldPose; // 始点（現フレーム）
	ColliderPose poseEnd = poseStart; // 終点（次のフレーム）
	poseEnd.position += delta;

	// 終点でのOvelapチェック
	auto OverlapBestAtPose = [&](const ColliderPose& poseA, QueryHit& hit)->bool
		{
			const AABB aabbA = shapeA->ComputeWorldAABB(poseA);

			bool found = false;
			float bestPenetration = -FLT_MAX;

			const int n = (int)m_Colliders.size();
			for (int i = 0; i < n; i++)
			{
				Collider* other = m_Colliders[i];
				if (!other) continue;
				if (other == self) continue;

				if (!PassModeFilter(other, opt)) continue;
				if (!ShouldCollide(*self, *other)) continue;

				const Collision* shapeB = other->Shape();
				if (!shapeB) continue;

				// ブロード
				if (!aabbA.isOverlap(other->WorldAABB()))continue;

				// Narrow
				ContactManifold m{};
				if (!shapeA->isOverlap(poseA, *shapeB, other->m_WorldPose, m, opt.slop)) continue;
				if (!m.touching || m.count <= 0) continue;

				const float pene = GetMaxPenetration(m);
				if (!found || pene >= bestPenetration)
				{
					found = true;
					bestPenetration = pene;

					hit.other = other;
					hit.normal = m.normal;
					hit.penetration = pene;
					hit.mtd = -m.normal * pene;
					hit.manifold = m;
				}
			}
			return found;
		};

	QueryHit endHit{};
	if (!OverlapBestAtPose(poseEnd, endHit))
	{
		return false;
	}

	// 二分探索：最初に重なるｔを探す
	float lo = 0.0f;
	float hi = 1.0f;

	QueryHit midHit{};
	for (int it = 0; it < binaryIterations; it++)
	{
		const float mid = (lo + hi) * 0.5f;
		ColliderPose poseMid = poseStart;
		poseMid.position += delta * mid;

		QueryHit h{};
		if (OverlapBestAtPose(poseMid, h)) // 真ん中で重なるかチェック
		{
			// 重なる⇒もっと手前へ
			hi = mid;
			midHit = h;
		}
		else
		{
			lo = mid;
		}
	}
	outHit = midHit; // 結果、
	return true;
}

// --------------------------------------------------
// 力反映
// m_Rigidbodies
// --------------------------------------------------
void PhysicsSystem::IntegrationForce(float dt)
{
	for (Rigidbody* rb : m_Rigidbodies)
	{
		if (!rb || !rb->IsDynamic()) continue;

		// ここで InertiaWorldInv の更新 -----
		rb->UpdateInertiaWorldInvFrom(rb->Owner()->Transform()->Rotation());

		// 重力加速度の決定
		Vector3 gravity{ 0.0f, 0.0f, 0.0f };
		switch (rb->GravityMode())
		{
		case Rigidbody::GravityMode::Global: // グローバル重力
			gravity = Gravity * rb->GravityScale();
			break;
		case Rigidbody::GravityMode::Custom: // カスタム重力
			gravity = rb->CustomGravity();
			break;
		case Rigidbody::GravityMode::None: // 無重力
			gravity = {0.0f, 0.0f, 0.0f};
			break;
		}

		// a = g + F/m
		Vector3 force = rb->ConsumeForces();
		Vector3 acc = gravity + force * rb->InvMass();
		rb->SetVelocity(rb->Velocity() + acc * dt);

		// トルク反映
		// ω += InertiaWorld^-1 * t * dt
		Vector3 tau = rb->ConsumeTorques();
		Vector3 omega = rb->ApplyInvInertiaWorld(tau) * dt;
		rb->SetAngularVelocity(rb->AngularVelocity() + omega);
	}
}

// --------------------------------------------------
// 衝突判定
// m_Colliders
// --------------------------------------------------
void PhysicsSystem::DetermineCollision()
{
	// 当たり判定
	const int n = (int)m_Colliders.size();
	for (int ia = 0; ia < n; ia++)
	{
		// Collider 取得
		Collider* colA = m_Colliders[ia];
		if (colA->IsQuery()) continue; // QueryOnly は早期リターン
		const Collision* shapeA = colA->Shape();
		if (!shapeA) continue;

		for (int ib = ia + 1; ib < n; ib++)
		{
			Collider* colB = m_Colliders[ib];
			if (colB->IsQuery()) continue; // QueryOnly は早期リターン

			// レイヤー判定
			if (!ShouldCollide(*colA, *colB)) continue;

			// ブロード（AABB）
			if (!colA->WorldAABB().isOverlap(colB->WorldAABB())) continue; // AABB で当たっているか大まかに判定

			// ナロー
			const Collision* shapeB = colB->Shape();
			if (!shapeB) continue;

			ContactManifold m; 
			if (!shapeA->isOverlap(colA->m_WorldPose, *shapeB, colB->m_WorldPose, m, 0.0f)) continue;

			if (colA->IsTrigger() || colB->IsTrigger()) m_CurrTrigger.insert(MakePairKey(colA->Id(), colB->Id()));
			else // どっちも IsTrigger が off だったら Collision イベントへ			  
			{
				m_CurrCollision.insert(MakePairKey(colA->Id(), colB->Id()));
				if (m.touching && m.count > 0)
					m_Contacts.push_back({ colA, colB, m });
			}
		}
	}
}

// --------------------------------------------------
// 衝突点の事前計算
// m_Contacts
// --------------------------------------------------
void PhysicsSystem::PreSolveContacts()
{
	// 衝突ペア
	for (auto& c : m_Contacts)
	{
		if (c.m.touching && c.m.count > 0) // 触れているかつ衝突点あり
		{
			// Rigidbody 取得
			auto* rbA = c.A->Owner()->GetComponent<Rigidbody>();
			auto* rbB = c.B->Owner()->GetComponent<Rigidbody>();

			const float invA = (rbA && rbA->IsDynamic()) ? rbA->InvMass() : 0.0f; // 逆質量取得
			const float invB = (rbB && rbB->IsDynamic()) ? rbB->InvMass() : 0.0f; // 逆質量取得

			// 反発係数
			const float eCandidate = std::max(rbA ? rbA->Restitution() : 0.0f,
											  rbB ? rbB->Restitution() : 0.0f);

			// 各衝突点
			for (int i = 0; i < c.m.count; i++)
			{
				float biasRest = 0.0f;

				if (invA + invB > 0.0f && eCandidate > 0.0f)
				{
					const Vector3 p = (c.m.points[i].pointOnA + c.m.points[i].pointOnB) * 0.5f; //接触点の中点を求める
					const Vector3 xA = rbA ? rbA->WorldCOM() : Vector3(); // 重心座標
					const Vector3 xB = rbB ? rbB->WorldCOM() : Vector3(); // 重心座標
					const Vector3 rA = p - xA; // COM からのベクトル（相対位置）
					const Vector3 rB = p - xB; // COM からのベクトル（相対位置）

					// 速度（角成分込み）
					const Vector3 vA = rbA ? rbA->Velocity() : Vector3{};
					const Vector3 wA = rbA ? rbA->AngularVelocity() : Vector3{};
					const Vector3 vB = rbB ? rbB->Velocity() : Vector3{};
					const Vector3 wB = rbB ? rbB->AngularVelocity() : Vector3{};

					// 接触点の相対速度 vRel = (vB + wB×rB) - (vA + wA×rA) [relative velocity]
					const Vector3 vRel = (vB + Vector3::Cross(wB, rB)) - (vA + Vector3::Cross(wA, rA)); // 相対速度（Aから見たBの相対速度）
					const float relVelN = Vector3::Dot(vRel, c.m.normal); // 法線方向の相対速度 [normal relative velocity]
					if (relVelN < -RestThreshold)
					{
						biasRest = eCandidate * relVelN;
					}
				}
				c.m.points[i].restitutionBias = biasRest;
			}
		}
	}
}

// --------------------------------------------------
// 速度解決
// m_Contacts
// 逐次インパルス法（接触、摩擦、ジョイントなどの速度拘束で即座に速度を更新する手法）
// --------------------------------------------------
void PhysicsSystem::ResolveVelocity(float dt)
{
	for (auto& c : m_Contacts)
	{
		auto* rbA = c.A->Owner()->GetComponent<Rigidbody>();
		auto* rbB = c.B->Owner()->GetComponent<Rigidbody>();

		const float invA = (rbA && rbA->IsDynamic()) ? rbA->InvMass() : 0.0f; // 逆質量取得
		const float invB = (rbB && rbB->IsDynamic()) ? rbB->InvMass() : 0.0f; // 逆質量取得
		if (invA + invB == 0.0f) continue; // 両方固定
		// 法線
		const Vector3 n = c.m.normal; // A→B

		// 摩擦係数（現在はRigidbody）
		const float fricDynamicA = rbA ? rbA->FrictionDynamic() : 0.0f;
		const float fricDynamicB = rbB ? rbB->FrictionDynamic() : 0.0f;
		const float fricStaticA = rbA ? rbA->FrictionStatic() : 0.0f;
		const float fricStaticB = rbB ? rbB->FrictionStatic() : 0.0f;
		auto Mix = [](float a, float b) {return (a > 0.0f && b > 0.0f) ? sqrtf(a * b) : std::max(a, b); };
		float fricD = Mix(fricDynamicA, fricDynamicB);
		float fricS = Mix(fricStaticA, fricStaticB);
		fricD = std::max(0.0f, fricD);
		fricS = std::max(0.0f, std::max(fricS, fricD)); // fricS >= fricD

		// ----- 角項の小ヘルパ -----
		// d : 評価方向（法線または接線）
		// 接触点に方向ｄのインパルスを 1[N・s]入れたときの「角運動による逆加速度寄与」をスカラーで返す
		// 接触点速度(v = ω×r)にインパルス(P = d * j)を入れたときの角速度変化(Δω = I^-1(r×P))を接触点速度へ戻した寄与((I^-1(r×d))×r)をｄで射影したもの（？）
		auto AngTerm = [](Rigidbody* rb, const Vector3& r, const Vector3& d) -> float
			{
				// n・[(I^-1 (r×d))×r] = dot(d, (r×d))×r)を展開してもいいが可読性重視でこちら
				return  Vector3::Dot(d, Vector3::Cross(rb->ApplyInvInertiaWorld(Vector3::Cross(r, d)), r));
			};

		// ======== PHASE 1: 全接点の「法線」だけ解く =========
		// 各接触点について法線インパルス
		for (int i = 0; i < c.m.count; i++)
		{
			// 接触点と COM 差分を取る
			const Vector3 p = (c.m.points[i].pointOnA + c.m.points[i].pointOnB) * 0.5f; //接触点の中点を求める
			const Vector3 pA = c.m.points[i].pointOnA;
			const Vector3 pB = c.m.points[i].pointOnB;
			const Vector3 xA = rbA ? rbA->WorldCOM() : Vector3(); // 重心座標
			const Vector3 xB = rbB ? rbB->WorldCOM() : Vector3(); // 重心座標
			const Vector3 rA = p - xA; // COM からのベクトル（相対位置）
			const Vector3 rB = p - xB; // COM からのベクトル（相対位置）
			
			// 速度（角成分込み）
			const Vector3 vA = rbA->GetLinearVelocitySolver();
			const Vector3 wA = rbA->GetAngularVelocitySolver();
			const Vector3 vB = rbB->GetLinearVelocitySolver();
			const Vector3 wB = rbB->GetAngularVelocitySolver();

			// 接触点の相対速度 vRel = (vB + wB×rB) - (vA + wA×rA) [relative velocity]
			const Vector3 vRel = (vB + Vector3::Cross(wB, rB)) - (vA + Vector3::Cross(wA, rA)); // 相対速度（Aから見たBの相対速度）
			const float relVelN = Vector3::Dot(vRel, n); // 法線方向の相対速度 [normal relative velocity]
			if (relVelN > 0.0f && c.m.points[i].penetration <= Slop) continue; // 離れていく方向 && めり込みが無い なので不要


			// ----- 法線インパルス（角項込みの有効質量） -----
			// 有効質量の分母（法線）
			float denomN = invA + invB; // 角項込みの法線方向の有効質量の逆 [normal effective mass]
			if (rbA && invA > 0.0f) denomN += AngTerm(rbA, rA, n); // 接触点速度の“ｄ方向成分”がどれだけ変わるかを表す係数↓
			if (rbB && invB > 0.0f) denomN += AngTerm(rbB, rB, n); // 分母 k = (invA + invB + AngTermA + AngTermB)
			if (denomN < 1e-12f) continue;

			// 位置バイアス
			const float pen = c.m.points[i].penetration;
			const float bias_pos = pen > Slop ? Baumgarte * (pen - Slop) / dt : 0.0f;
			// 反発バイアス
			const float bias_rest = c.m.points[i].restitutionBias; // ここで反発処理が含まれる

			// 増分インパルス（インパルス量）
			float deltaImpulseN = -(relVelN + bias_pos + bias_rest) / denomN;
			// delta = relVelN / denomN; // 今の相対速度を０にするインパルス
			// delota += bias_rest / denomN; // 反発後の速度にするインパルス


			// 累積インパルス
			float old = c.m.points[i].accumN;
			c.m.points[i].accumN = std::max(0.0f, old + deltaImpulseN);
			deltaImpulseN = c.m.points[i].accumN - old; // 実際に適用する増分

			const Vector3 impulseVector = n * deltaImpulseN; // ベクトル化したインパルス [normal impulse vector]

			// v と ω に即時適用 -----
			if (rbA && invA > 0.0f)
			{
				rbA->SetVelocity(vA - impulseVector * invA);
				rbA->SetAngularVelocity(wA - rbA->ApplyInvInertiaWorld(Vector3::Cross(rA, impulseVector)));
			}
			if (rbB && invB > 0.0f)
			{
				rbB->SetVelocity(vB + impulseVector * invB);
				rbB->SetAngularVelocity(wB + rbB->ApplyInvInertiaWorld(Vector3::Cross(rB, impulseVector)));
			}
		}
		
		// ======== PHASE 2: 全接点の「摩擦」だけ解く =========
		// 各接触点について摩擦
		for (int i = 0; i < c.m.count; i++)
		{
			// 接触点と COM 差分を取る
			const Vector3 p = (c.m.points[i].pointOnA + c.m.points[i].pointOnB) * 0.5f; //接触点の中点を求める
			const Vector3 pA = c.m.points[i].pointOnA;
			const Vector3 pB = c.m.points[i].pointOnB;
			const Vector3 xA = rbA ? rbA->WorldCOM() : Vector3(); // 重心座標
			const Vector3 xB = rbB ? rbB->WorldCOM() : Vector3(); // 重心座標
			const Vector3 rA = p - xA; // COM からのベクトル（相対位置）
			const Vector3 rB = p - xB; // COM からのベクトル（相対位置）

			// ----- 動摩擦力（法線更新後の相対速度で評価） -----
			const Vector3 vA2 = rbA->GetLinearVelocitySolver();
			const Vector3 wA2 = rbA->GetAngularVelocitySolver();
			const Vector3 vB2 = rbB->GetLinearVelocitySolver();
			const Vector3 wB2 = rbB->GetAngularVelocitySolver();
			const Vector3 vRel2 = (vB2 + Vector3::Cross(wB2, rB)) - (vA2 + Vector3::Cross(wA2, rA)); // 相対速度

			// ----- 希望する接線のインパルス -----
			Vector3 t1;
			if (fabsf(n.x) > 0.57735f) t1 = Vector3{ -n.y, n.x, 0.0f };
			else					   t1 = Vector3{ 0.0f, -n.z, n.y };
			t1 = t1.normalized();
			Vector3 t2 = Vector3::Cross(n, t1); // 接線方向（２軸目）

			// 有効質量の分母
			float denomT1 = invA + invB; // 接線方向の有効質量の分母 [tangential impulse mass]
			if (rbA && invA > 0.0f) denomT1 += AngTerm(rbA, rA, t1);
			if (rbB && invB > 0.0f) denomT1 += AngTerm(rbB, rB, t1);
			if (denomT1 < 1e-12f) continue;
			float denomT2 = invA + invB; // 接線方向の有効質量の分母 [tangential impulse mass]
			if (rbA && invA > 0.0f) denomT2 += AngTerm(rbA, rA, t2);
			if (rbB && invB > 0.0f) denomT2 += AngTerm(rbB, rB, t2);
			if (denomT2 < 1e-12f) continue;

			// 望ましい増分（静止摩擦ターゲット vt' = 0）
			float v_t1 = Vector3::Dot(vRel2, t1);
			float v_t2 = Vector3::Dot(vRel2, t2);
			float deltaImpulseT_desired_1 = -(v_t1) / denomT1; // [tangential impulse scalar]
			float deltaImpulseT_desired_2 = -(v_t2) / denomT2; // [tangential impulse scalar]

			// ----- Coulomb 円錐（半径 = μ * jn）でクランプ -----
			float jtMaxS = fricS * c.m.points[i].accumN; // 静止摩擦半径　μ * jn （累積）
			float jtMaxD = fricD * c.m.points[i].accumN; // 動摩擦半径　μ * jn （累積）
			if (jtMaxD <= 0.0f) continue; // 法線反力がない≒摩擦が無い

			Vector3 oldImpWorld = c.m.points[i].accumImpulseT; // ワールド累積
			float oldImp1 = Vector3::Dot(oldImpWorld, t1); // t1 成分取り出し
			float oldImp2 = Vector3::Dot(oldImpWorld, t2); // t2 成分取り出し

			Vector3 starImp = Vector3{ oldImp1 + deltaImpulseT_desired_1, oldImp2 + deltaImpulseT_desired_2, 0.0f };
			float deltaImpulseT = starImp.length(); // 実際に適用するインパルス

			Vector3 newImp;
			if (deltaImpulseT <= jtMaxS) // static : 完全停止
			{
				newImp = starImp;
			}
			else // slip : 動摩擦円錐に張り付け
			{ 
				if (deltaImpulseT > 1e-12f) newImp = starImp * (jtMaxD / deltaImpulseT);
				else						newImp = Vector3();
			}

			Vector3 delta = newImp - Vector3{ oldImp1, oldImp2, 0.0f };

			c.m.points[i].accumImpulseT = t1 * newImp.x + t2 * newImp.y; // 累積は世界ベクトルで保存する

			const Vector3 impulseTVector = t1 * delta.x + t2 * delta.y; // 接線方向に掛けるインパルスベクトル [tangential impulse vector]
			if (rbA && invA > 0.0f)
			{
				rbA->SetVelocity(rbA->Velocity() - impulseTVector * invA);
				rbA->SetAngularVelocity(rbA->AngularVelocity() - rbA->ApplyInvInertiaWorld(Vector3::Cross(rA, impulseTVector)));
			}
			if (rbB && invB > 0.0f)
			{
				rbB->SetVelocity(rbB->Velocity() + impulseTVector * invB);
				rbB->SetAngularVelocity(rbB->AngularVelocity() + rbB->ApplyInvInertiaWorld(Vector3::Cross(rB, impulseTVector)));
			}
		}
	}
}

// --------------------------------------------------
// 減衰
// m_Rigidbodies
// --------------------------------------------------
void PhysicsSystem::ApplyDamping(float dt)
{
	if (dt <= 0.0f) return;

	for (Rigidbody* rb : m_Rigidbodies)
	{
		if (!rb || !rb->IsDynamic()) continue;

		const float a = std::exp(-rb->LinDamping() * dt);
		const float b = std::exp(-rb->AngDamping() * dt);

		// 線形減衰
		rb->SetVelocity(rb->Velocity() * a);
		rb->SetAngularVelocity(rb->AngularVelocity() * b);
	}
}

// --------------------------------------------------
// 位置補正
// m_Contacts
// --------------------------------------------------
void PhysicsSystem::CorrectPosition()
{
	for (auto& c : m_Contacts)
	{
		auto* rbA = c.A->Owner()->GetComponent<Rigidbody>();
		auto* rbB = c.B->Owner()->GetComponent<Rigidbody>();
		const float invA = (rbA && rbA->IsDynamic()) ? rbA->InvMass() : 0.0f;
		const float invB = (rbB && rbB->IsDynamic()) ? rbB->InvMass() : 0.0f;
		if (invA + invB == 0.0f) continue;

		const Vector3 n = c.m.normal; // A→B

		float maxPen = 0.0f;
		for (int i = 0; i < c.m.count; i++)
			maxPen = std::max(maxPen, c.m.points[i].penetration); // 最大の貫入深度を採用
		
		// slop で判定
		const float C = std::max(0.0f, maxPen - Slop);
		if (C <= 0.0f) continue;

		// じわっと直す
		const float s = Baumgarte * C / (invA + invB); // 逆質量比
		const Vector3 corrA = -n * (s * invA);
		const Vector3 corrB =  n * (s * invB);
		
		// ----- 位置反映 -----
		auto anyNonZero = [](const Vector3& v) { return v.lengthSq() > 1e-12f; };

		// ----- A: COM を押して Transform を復元 -----
		if (rbA && invA > 0.0f && anyNonZero(corrA))
		{
			rbA->SetWorldCOM(rbA->WorldCOM() + corrA); // COM へ反映
			auto* tfA = c.A->Owner()->Transform();
			if (tfA)
			{
				Quaternion qA = tfA->Rotation();
				const Vector3 rA = qA.Rotate(rbA->CenterOfMassLocal());
				tfA->SetPosition(rbA->WorldCOM() - rA);
			}
		}
		// ----- B: COM を押して Transform を復元 -----
		if (rbB && invB > 0.0f && anyNonZero(corrB))
		{
			rbB->SetWorldCOM(rbB->WorldCOM() + corrB); // COM へ反映
			auto* tfB = c.B->Owner()->Transform();
			if (tfB)
			{
				Quaternion qB = tfB->Rotation();
				const Vector3 rB = qB.Rotate(rbB->CenterOfMassLocal());
				tfB->SetPosition(rbB->WorldCOM() - rB);
			}
		}
	}
}

// --------------------------------------------------
// 速度反映
// m_Rigidbodies
// --------------------------------------------------
void PhysicsSystem::IntegrationVelocity(float dt)
{
	for (Rigidbody* rb : m_Rigidbodies)
	{
		if (!rb || !rb->IsDynamic()) continue;

		auto* tfc = rb->Owner()->Transform();

		// COM を積分 -----
		rb->SetWorldCOM(rb->WorldCOM() + rb->Velocity() * dt);

		// 姿勢：ωで積分(rad/s) -----
		Quaternion q = tfc->Rotation().normalized(); // 現在姿勢
		Vector3 w = rb->AngularVelocity(); // 角速度取得
		float wlen = w.length();
		if (wlen > 1e-8f)
		{
			float theta = wlen * dt; // 角速度[rad/s] * 時間[s] = 角度[rad]
			Vector3 axis = w * (1.0f / wlen); // 回転軸の単位ベクトル
			float s = sinf(0.5f * theta), c = cosf(0.5f * theta); 
			Quaternion dq(axis.x * s, axis.y * s, axis.z * s, c); // クォータニオン生成
			q = (dq * q).normalized(); // 回転（右手）
			tfc->SetRotation(q);
		}

		// ----- Trnasform 原点を COM から復元 -----
		// origin = COM - R * COMLocal
		const Vector3 COMLocal = rb->CenterOfMassLocal();
		const Vector3 r = q.Rotate(COMLocal);
		tfc->SetPosition(rb->WorldCOM() - r);
	}
}

// --------------------------------------------------
// ジョイントの事前計算
// m_Joints
// --------------------------------------------------
void PhysicsSystem::PreSolveDistanceJoints(float dt)
{
	const float beta = 0.2f;

	// Joints の更新
	for (auto& joint : m_DistanceJoints)
	{
		// Rigidbody 取得
		auto* rbA = joint.pBodyA;
		auto* rbB = joint.pBodyB;
		TransformComponent* tfA = joint.tfA;
		TransformComponent* tfB = joint.tfB;
		if (!tfA || !tfB) continue;
		if (!rbA && !rbB) continue;

		// アンカー計算
		Vector3 xA = rbA->WorldCOM(); // tfA->Position()
		Vector3 xB = rbB->WorldCOM(); // tfB->Position()

		Quaternion qA = tfA->Rotation();
		Quaternion qB = tfB->Rotation();

		Vector3 rA = qA.Rotate(joint.localAnchorA);
		Vector3 rB = qB.Rotate(joint.localAnchorB);

		joint.rAworld = rA; // COM からアンカーへのベクトル
		joint.rBworld = rB;

		Vector3 pA = xA + rA;
		Vector3 pB = xB + rB;
		float len = (pB - pA).length();

		// 法線ベクトル
		Vector3 n = Vector3();
		n = pB - pA; // アンカー間
		n.normalize();
		joint.normal = n;

 		// 有効質量
		float invA = (rbA && rbA->IsDynamic()) ? rbA->InvMass() : 0.0f;
		float invB = (rbB && rbB->IsDynamic()) ? rbB->InvMass() : 0.0f;
		float k = invA + invB;
		
		auto AngTerm = [](Rigidbody* rb, const Vector3& r, const Vector3& dir) -> float
			{
				// dir・[(I^-1 (r×dir))×r]
				return Vector3::Dot(dir, Vector3::Cross(rb->ApplyInvInertiaWorld(Vector3::Cross(r, dir)), r));
			};
		if (len > 1e-6f)
		{
			if (rbA && rbA->IsDynamic()) k += AngTerm(rbA, rA, n);
			if (rbB && rbB->IsDynamic()) k += AngTerm(rbB, rB, n);
		}

		joint.effectiveMass = (k > 1e-8f) ? 1.0f / k : 0.0f;

		// Baumgarte バイアス（ C = 現在の長さ - 目標の長さ）
		float C = len - joint.restLength;
		joint.bias = beta * C / dt;
		
		// 累積初期化
		joint.accumImpulse = 0.0f; // 今はウォームスタート無し
	}
}
void PhysicsSystem::PreSolveBallJoints(float dt)
{
	const float beta = 0.2f;

	for (auto& joint : m_BallJoints)
	{
		// Rigidbody 取得
		auto* rbA = joint.pBodyA;
		auto* rbB = joint.pBodyB;
		TransformComponent* tfA = joint.tfA;
		TransformComponent* tfB = joint.tfB;
		if (!tfA || !tfB) continue;
		if (!rbA && !rbB) continue;

		// アンカー計算
		Vector3 xA = rbA->WorldCOM(); // tfA->Position()
		Vector3 xB = rbB->WorldCOM(); // tfB->Position()

		Quaternion qA = tfA->Rotation();
		Quaternion qB = tfB->Rotation();
		
		Vector3 rA = qA.Rotate(joint.localAnchorA);
		Vector3 rB = qB.Rotate(joint.localAnchorB);

		joint.rAworld = rA;
		joint.rBworld = rB;

		Vector3 pA = xA + rA;
		Vector3 pB = xB + rB;

		// 目標の位置
		Vector3 C = pB - pA;

		// 有効質量計算（３本方向）
		float invA = (rbA && rbA->IsDynamic()) ? rbA->InvMass() : 0.0f;
		float invB = (rbB && rbB->IsDynamic()) ? rbB->InvMass() : 0.0f;

		auto AngTerm = [](Rigidbody* rb, const Vector3& r, const Vector3& dir) -> float
			{
				// dir・[(I^-1 (r×dir))×r]
				return Vector3::Dot(dir, Vector3::Cross(rb->ApplyInvInertiaWorld(Vector3::Cross(r, dir)), r));
			};

		const Vector3 axes[3] =
		{
			{1.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 1.0f}
		};

		for (int i = 0; i < 3; i++)
		{
			const Vector3 dir = axes[i];
			float k = invA + invB;

			if (rbA && invA > 0.0f) k += AngTerm(rbA, rA, dir);
			if (rbB && invB > 0.0f) k += AngTerm(rbB, rB, dir);

			float eff = (k > 1e-8f) ? 1.0f / k : 0.0f;

			joint.effectiveMass[i] = eff;
		}

		// Baumgarte バイアス
		joint.bias = beta * C / dt;

		// 累積初期化
		joint.accumImpulse = Vector3(0.0f, 0.0f, 0.0f);
	}
}
void PhysicsSystem::PreSolveHingeJoints(float dt)
{
	const float betaLinear = 0.2f;

	for (auto& joint : m_HingeJoints)
	{
		// Rigidbody 取得
		auto* rbA = joint.pBodyA;
		auto* rbB = joint.pBodyB;
		TransformComponent* tfA = joint.tfA;
		TransformComponent* tfB = joint.tfB;
		if (!tfA || !tfB) continue;
		if (!rbA && !rbB) continue;

		// アンカー計算
		Vector3 xA = rbA->WorldCOM(); // tfA->Position()
		Vector3 xB = rbB->WorldCOM(); // tfB->Position()

		Quaternion qA = tfA->Rotation();
		Quaternion qB = tfB->Rotation();

		Vector3 rA = qA.Rotate(joint.localAnchorA);
		Vector3 rB = qB.Rotate(joint.localAnchorB);

		joint.rAworld = rA;
		joint.rBworld = rB;

		Vector3 pA = xA + rA;
		Vector3 pB = xB + rB;

		// 目標の位置
		Vector3 C = pB - pA;

		// 有効質量計算（３本方向）
		float invA = (rbA && rbA->IsDynamic()) ? rbA->InvMass() : 0.0f;
		float invB = (rbB && rbB->IsDynamic()) ? rbB->InvMass() : 0.0f;

		auto AngTerm = [](Rigidbody* rb, const Vector3& r, const Vector3& dir) -> float
			{
				// dir・[(I^-1 (r×dir))×r]
				return Vector3::Dot(dir, Vector3::Cross(rb->ApplyInvInertiaWorld(Vector3::Cross(r, dir)), r));
			};

		const Vector3 axes[3] =
		{
			{1.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 1.0f}
		};

		// 直交基底の有効質量
		for (int i = 0; i < 3; i++)
		{
			const Vector3 dir = axes[i];
			float k = invA + invB;

			if (rbA && invA > 0.0f) k += AngTerm(rbA, rA, dir);
			if (rbB && invB > 0.0f) k += AngTerm(rbB, rB, dir);

			float eff = (k > 1e-8f) ? 1.0f / k : 0.0f;

			joint.effectiveMassLinear[i] = eff;
		}

		// Baumgarte バイアス
		joint.biasLinear = betaLinear * C / dt;

		// 累積初期化
		joint.accumImpulseLinear = Vector3(0.0f, 0.0f, 0.0f);

		// ----- ワールドのヒンジ軸を求める -----
		Vector3 axisWorld = qA.Rotate(joint.localAxisA).normalized();
		joint.axisWorld = axisWorld;

		// 直交基底を作る
		Vector3 t1, t2;
		if (fabsf(axisWorld.x) > 0.577f)
			t1 = Vector3( axisWorld.y, -axisWorld.x, 0);
		else
			t1 = Vector3(0, axisWorld.z, -axisWorld.y);
		t1.normalize();
		t2 = Vector3::Cross(axisWorld, t1);
		t2.normalize();
		joint.swingAxis[0] = t1;
		joint.swingAxis[1] = t2;

		// ----- 軸補正用の biasSwing 計算 -----
		// A, B 両方のヒンジ軸をワールドで
		Vector3 axisA = qA.Rotate(joint.localAxisA).normalized();
		Vector3 axisB = qB.Rotate(joint.localAxisB).normalized();

		// t1, t2 方向にどれだけズレているか
		float C1 = Vector3::Dot(axisB, t1);
		float C2 = Vector3::Dot(axisB, t2);

		joint.swingDiff = Vector3(C1, C2, 0);

		// Baumgarte で角度誤差→速度バイアスへ
		const float betaSwing = 0.005f;
		joint.biasSwing = betaSwing * joint.swingDiff / dt;

		// 角度拘束の有効質量を計算
		auto AngMix = [&](const Vector3& a, const Vector3& b)
			{
				float k = 0.0f;
				if (rbA && rbA->IsDynamic()) k += Vector3::Dot(a, rbA->ApplyInvInertiaWorld(b));
				if (rbB && rbB->IsDynamic()) k += Vector3::Dot(a, rbB->ApplyInvInertiaWorld(b));
				return k;
			};
		// K = J M^-1 J^T （2×2）
		float k11 = AngMix(t1, t1);
		float k12 = AngMix(t1, t2);
		float k21 = AngMix(t2, t1);
		float k22 = AngMix(t2, t2);

		// 数値誤差で非対称になりやすいので対称化しておく
		float off = 0.5f * (k12 + k21);
		k12 = off; k21 = off;

		// 逆行列 swingMat を計算
		float det = k11 * k22 - k12 * k21;
		if (det > 1e-8f)
		{
			float invDet = 1.0f / det;
			joint.swingMass.m[0][0] =  invDet * k22;
			joint.swingMass.m[0][1] = -invDet * k12;
			joint.swingMass.m[1][0] = -invDet * k21;
			joint.swingMass.m[1][1] =  invDet * k11;
		}
		else // ほぼ特異なら拘束を弱くするor無視する
		{
			joint.swingMass.m[0][0] = 0.0f;
			joint.swingMass.m[0][1] = 0.0f;
			joint.swingMass.m[1][0] = 0.0f;
			joint.swingMass.m[1][1] = 0.0f;
		}
		joint.accImpulseSwing = { 0.0f, 0.0f, 0.0f };

		// ----- 基準ベクトルをワールドに変換 -----
		Vector3 refA = tfA->Rotation().Rotate(joint.localRefA);
		Vector3 refB = tfB->Rotation().Rotate(joint.localRefB);

		// 軸成分を落として、軸と直行させる
		refA -= joint.axisWorld * Vector3::Dot(refA, joint.axisWorld);
		refB -= joint.axisWorld * Vector3::Dot(refB, joint.axisWorld);
		if (refA.length() < 1e-8f)
		{
			if (fabsf(joint.axisWorld.x) > 0.577f)
				refA = Vector3(joint.axisWorld.y, -joint.axisWorld.x, 0.0f);
			else
				refA = Vector3(0.0f, joint.axisWorld.z, -joint.axisWorld.y);
		}
		if (refB.length() < 1e-8f)
		{
			if (fabsf(joint.axisWorld.x) > 0.577f)
				refB = Vector3(joint.axisWorld.y, -joint.axisWorld.x, 0.0f);
			else
				refB = Vector3(0.0f, joint.axisWorld.z, -joint.axisWorld.y);
		}
		refA.normalize();
		refB.normalize();

		joint.refAWorld = refA;
		joint.refBWorld = refB;

		// 現在のヒンジ角θを計算
		float cosTheta = Vector3::Clamp(Vector3::Dot(refA, refB), -1.0f, 1.0f);
		float sinTheta = Vector3::Dot(joint.axisWorld, Vector3::Cross(refA, refB));
		float angle = atan2f(sinTheta, cosTheta);
		
		// twist の有効質量を計算
		auto AngMass = [](Rigidbody* rb, const Vector3& axisDir)
			{
				// axisDir・(I^-1 axisDir)
				Vector3 v = rb->ApplyInvInertiaWorld(axisDir);
				return Vector3::Dot(axisDir, v);
			};
		float kTwist = 0.0f;
		if (rbA && rbA->IsDynamic()) kTwist += AngMass(rbA, joint.axisWorld);
		if (rbB && rbB->IsDynamic()) kTwist += AngMass(rbB, joint.axisWorld);
		joint.effMassTwist = (kTwist > 1e-8f) ? 1.0f / kTwist : 0.0f;

		// ----- biasTwist ----- 
		joint.biasTwist = 0.0f;
		joint.accImpulseTwist = 0.0f;

		if (joint.enableLimit && joint.effMassTwist > 0.0f)
		{
			float Ctwist = 0.0f;
			const float betaTwist = 0.001f;
			if (angle < joint.limitMin) // 下限より小さい→下限まで押し戻したい
				Ctwist = angle - joint.limitMin; // 負の値
			else if (angle > joint.limitMax) // 上限より大きい→上限まで押し戻したい
				Ctwist = angle - joint.limitMax; // 正の値
			else
				Ctwist = 0.0f; // 許容範囲なら角度拘束は何もしない

			if (Ctwist != 0.0f)
			{
				// vn * βC = 0 の βC に相当
				joint.biasTwist += -betaTwist * Ctwist / dt;
			}
		}

		// ----- Spring -----
		joint.springDiff = 0.0f;
		if (joint.enableSpring && joint.effMassTwist > 0.0f)
		{
			joint.springDiff = angle - joint.springTarget; // 差分をキャッシュ
		}

		// ----- Motor -----
		joint.accImpulseMotor = 0.0f;

		// ----- biasSpring -----
		if (joint.enableSpring && joint.effMassTwist > 0.0f)
		{
			float C = angle - joint.springTarget;
			const float betaSpring = 0.001f;
			joint.biasTwist += -betaSpring * C / dt;
		}
	}
}

// --------------------------------------------------
// ジョイントの解決
// m_Joints
// --------------------------------------------------
void PhysicsSystem::ResolveDistanceJoints(float dt)
{
	for (auto& joint : m_DistanceJoints)
	{
		Rigidbody* rbA = joint.pBodyA;
		Rigidbody* rbB = joint.pBodyB;
		if (!rbA && !rbB) continue;
		if (joint.effectiveMass <= 0.0f) continue;

		const Vector3 n = joint.normal;
		const Vector3 rA = joint.rAworld;
		const Vector3 rB = joint.rBworld;

		// 現在速度（無い方は０）
		Vector3 vA = rbA ? rbA->GetLinearVelocitySolver()   : Vector3();
		Vector3 wA = rbA ? rbA->GetAngularVelocitySolver() : Vector3();
		Vector3 vB = rbB ? rbB->GetLinearVelocitySolver()	 : Vector3();
		Vector3 wB = rbB ? rbB->GetAngularVelocitySolver() : Vector3();

		// 相対速度
		Vector3 vRel = (vB + Vector3::Cross(wB, rB)) - (vA + Vector3::Cross(wA, rA));
		float relVelN = Vector3::Dot(vRel, n);

		float deltaImpulse = -(relVelN + joint.bias) * joint.effectiveMass;

		// 必要なら張り網にするクランプ
		float oldImpulse = joint.accumImpulse;
		float candidate = oldImpulse + deltaImpulse;

		switch (joint.mode)
		{
		case 0: // Rod
			// クランプなし
			joint.accumImpulse = candidate;
			break;
		case 1: // Rope
			// 引っ張りだけ
			joint.accumImpulse = std::min(0.0f, candidate);
			break;
		case 2: // compression
			// 押し出しだけ
			joint.accumImpulse = std::max(0.0f, candidate);
			break;
		}
		deltaImpulse = joint.accumImpulse - oldImpulse;

		// インパルスベクトル
		Vector3 impulseVector = n * deltaImpulse;

		float invA = (rbA && rbA->IsDynamic()) ? rbA->InvMass() : 0.0f;
		float invB = (rbB && rbB->IsDynamic()) ? rbB->InvMass() : 0.0f;
		if (invA > 0.0f)
		{
			rbA->SetVelocity(vA - impulseVector * invA);
			rbA->SetAngularVelocity(wA - rbA->ApplyInvInertiaWorld(Vector3::Cross(rA, impulseVector)));
		}
		if (invB > 0.0f)
		{
			rbB->SetVelocity(vB + impulseVector * invB);
			rbB->SetAngularVelocity(wB + rbB->ApplyInvInertiaWorld(Vector3::Cross(rB, impulseVector)));
		}
	}
}
void PhysicsSystem::ResolveBallJoints(float dt)
{
	const Vector3 axes[3] =
	{
		{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f}
	};

	for (auto& joint : m_BallJoints)
	{
		Rigidbody* rbA = joint.pBodyA;
		Rigidbody* rbB = joint.pBodyB;
		if (!rbA && !rbB) continue;
		if (joint.effectiveMass.lengthSq() <= 1e-8f) continue;

		Vector3 rA = joint.rAworld;
		Vector3 rB = joint.rBworld;

		// 現在速度
		Vector3 vA = rbA ? rbA->GetLinearVelocitySolver()  : Vector3();
		Vector3 wA = rbA ? rbA->GetAngularVelocitySolver() : Vector3();
		Vector3 vB = rbB ? rbB->GetLinearVelocitySolver()  : Vector3();
		Vector3 wB = rbB ? rbB->GetAngularVelocitySolver() : Vector3();

		// 相対速度
		Vector3 vRel = (vB + Vector3::Cross(wB, rB)) - (vA + Vector3::Cross(wA, rA));

		// 合計インパルスを算出
		Vector3 totalImpulse = {0.0f, 0.0f, 0.0f};

		// 位置拘束（３軸分）
		for (int i = 0; i < 3; i++)
		{
			float eff = joint.effectiveMass[i];
			if (eff <= 0.0f) continue; // 拘束になっていない軸

			const Vector3 dir = axes[i];
			float vRelK = Vector3::Dot(vRel, dir); // 軸方向の速度
			float biasK = joint.bias[i];

			float deltaImpulse = -(vRelK + biasK) * eff;

			// 累積インパルス
			float oldImpulse = joint.accumImpulse[i];
			float newImpulse = oldImpulse + deltaImpulse;

			// BallJoint は基本クランプ不要
			joint.accumImpulse[i] = newImpulse;

			deltaImpulse = newImpulse - oldImpulse;

			// その軸のインパルスを加算する
			totalImpulse += dir * deltaImpulse;
		}

		// ３軸全部０なら何もしない
		if (totalImpulse.lengthSq() <= 1e-8f) continue;

		float invA = (rbA && rbA->IsDynamic()) ? rbA->InvMass() : 0.0f;
		float invB = (rbB && rbB->IsDynamic()) ? rbB->InvMass() : 0.0f;

		if (invA > 0.0f)
		{
			rbA->SetVelocity(vA - totalImpulse * invA);
			rbA->SetAngularVelocity(wA - rbA->ApplyInvInertiaWorld(Vector3::Cross(rA, totalImpulse)));
		}
		if (invB > 0.0f)
		{
			rbB->SetVelocity(vB + totalImpulse * invB);
			rbB->SetAngularVelocity(wB + rbB->ApplyInvInertiaWorld(Vector3::Cross(rB, totalImpulse)));
		}
	}
}
void PhysicsSystem::ResolveHingeJoints(float dt)
{
	const Vector3 axes[3] =
	{
		{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f}
	};

	for (auto& joint : m_HingeJoints)
	{
		Rigidbody* rbA = joint.pBodyA;
		Rigidbody* rbB = joint.pBodyB;
		if (!rbA && !rbB) continue;

		Vector3 rA = joint.rAworld;
		Vector3 rB = joint.rBworld;

		// 現在速度
		Vector3 vA = rbA ? rbA->GetLinearVelocitySolver()   : Vector3();
		Vector3 wA = rbA ? rbA->GetAngularVelocitySolver() : Vector3();
		Vector3 vB = rbB ? rbB->GetLinearVelocitySolver()	 : Vector3();
		Vector3 wB = rbB ? rbB->GetAngularVelocitySolver() : Vector3();

		// ----- 位置拘束 -----
		// いずれかの軸が有効ならやる
		bool isLinear = joint.effectiveMassLinear.x > 0.0f ||
					    joint.effectiveMassLinear.y > 0.0f ||
					    joint.effectiveMassLinear.z > 0.0f;

		if (isLinear)
		{
			// 相対速度
			Vector3 vRel = (vB + Vector3::Cross(wB, rB)) - (vA + Vector3::Cross(wA, rA));

			// 合計インパルスを算出
			Vector3 totalLinearImpulse = { 0.0f, 0.0f, 0.0f };

			// 位置拘束（３軸分）
			for (int i = 0; i < 3; i++)
			{
				float eff = joint.effectiveMassLinear[i];
				if (eff <= 0.0f) continue; // 拘束になっていない軸

				const Vector3 dir = axes[i];
				float vRelK = Vector3::Dot(vRel, dir); // 軸方向の速度
				float biasK = joint.biasLinear[i];

				float deltaImpulse = -(vRelK + biasK) * eff;

				// 累積インパルス
				float oldImpulse = joint.accumImpulseLinear[i];
				float newImpulse = oldImpulse + deltaImpulse;

				// 位置拘束なので今はクランプなし
				joint.accumImpulseLinear[i] = newImpulse;

				deltaImpulse = newImpulse - oldImpulse;

				// その軸のインパルスを加算する
				totalLinearImpulse += dir * deltaImpulse;
			}

			// ３軸全部０なら線形拘束をスキップ
			if (totalLinearImpulse.lengthSq() > 1e-8f)
			{
				float invA = (rbA && rbA->IsDynamic()) ? rbA->InvMass() : 0.0f;
				float invB = (rbB && rbB->IsDynamic()) ? rbB->InvMass() : 0.0f;

				if (invA > 0.0f)
				{
					rbA->SetVelocity(vA - totalLinearImpulse * invA);
					rbA->SetAngularVelocity(wA - rbA->ApplyInvInertiaWorld(Vector3::Cross(rA, totalLinearImpulse)));
				}
				if (invB > 0.0f)
				{
					rbB->SetVelocity(vB + totalLinearImpulse * invB);
					rbB->SetAngularVelocity(wB + rbB->ApplyInvInertiaWorld(Vector3::Cross(rB, totalLinearImpulse)));
				}
			}
		}

		// 相対角速度を返す関数
		auto GetRelativeOmega = [&](const Vector3& wA, const Vector3& wB)
			{
				if (rbA && rbB) return wB - wA;
				else if (rbB)   return wB;
				else			return -wA;
			};

		// ----- swing の角拘束処理 -----
		// 更新した現在速度を再取得
		wA = rbA ? rbA->AngularVelocity() : Vector3();
		wB = rbB ? rbB->AngularVelocity() : Vector3();

		// 相対角速度
		Vector3 wRel = GetRelativeOmega(wA, wB);

		// 基底t1, t2
		const Vector3& t1 = joint.swingAxis[0];
		const Vector3& t2 = joint.swingAxis[1];

		// Cdot = ( wRel・t1, wRel・t2 )
		Vector3 Cdot;
		Cdot.x = Vector3::Dot(wRel, t1);
		Cdot.y = Vector3::Dot(wRel, t2);

		// rhs = Cdot + bias
		Vector3 rhs = Cdot + joint.biasSwing;

		// Δλ = - swingMass * rhs
		Vector3 deltaImpulse;
		const Matrix4x4& mass = joint.swingMass;
		deltaImpulse.x = -(mass.m[0][0] * rhs.x + mass.m[0][1] * rhs.y);
		deltaImpulse.y = -(mass.m[1][0] * rhs.x + mass.m[1][1] * rhs.y);

		// 累積
		Vector3 oldImpulse = joint.accImpulseSwing;
		Vector3 newImpulse = oldImpulse + deltaImpulse;
		joint.accImpulseSwing = newImpulse;

		// 実際に適用する分
		deltaImpulse = newImpulse - oldImpulse;

		// トルクインパルス = Δλ.x * t1 + Δλ.y * t2
		Vector3 totalAngularImpulse = deltaImpulse.x * t1 + deltaImpulse.y * t2;

		if (totalAngularImpulse.lengthSq() > 1e-8f) 
		{
			float invA = (rbA && rbA->IsDynamic()) ? rbA->InvMass() : 0.0f;
			float invB = (rbB && rbB->IsDynamic()) ? rbB->InvMass() : 0.0f;

			if (invA > 0.0f)
			{
				rbA->SetAngularVelocity(wA - rbA->ApplyInvInertiaWorld(totalAngularImpulse));
			}
			if (invB > 0.0f)
			{
				rbB->SetAngularVelocity(wB + rbB->ApplyInvInertiaWorld(totalAngularImpulse));
			}
		}

		// ----- twist の処理 -----
		if ((joint.enableLimit || joint.enableSpring) && joint.effMassTwist >= 1e-8f)
		{
			// 更新した現在速度を再取得
			wA = rbA ? rbA->AngularVelocity() : Vector3();
			wB = rbB ? rbB->AngularVelocity() : Vector3();

			// 相対角速度
			if (rbA && rbB) wRel = wB - wA;
			else if (rbB)	wRel = wB;
			else		    wRel = -wA;

			// ヒンジ軸成分だけ取り出す
			float wRelTwist = Vector3::Dot(wRel, joint.axisWorld);

			// ----- spring 処理 -----
			// Cdot = wRelTwist + damp * springDiff
			float Cdot = wRelTwist;
			if (joint.enableSpring)
				Cdot += joint.springDamping * joint.springDiff;

			float biasTwist = joint.biasTwist;

			// 角速度拘束: wRelTwist + bias = 0
			float deltaImpluse = -(Cdot + biasTwist) * joint.effMassTwist;

			float oldImpluse = joint.accImpulseTwist;
			float newImpluse = oldImpluse + deltaImpluse;

			// ----- limit 処理（クランプ） -----
			if (joint.enableLimit)
			{
				if (biasTwist > 0.0f) // 下限を超えたとき：　正のインパルスだけ許可
					newImpluse = std::max(0.0f, newImpluse);
				else if (biasTwist < 0.0f) // 上限を超えたとき：　負のインパルスだけ許可
					newImpluse = std::min(0.0f, newImpluse);
			}

			deltaImpluse = newImpluse - oldImpluse;
			joint.accImpulseTwist = newImpluse;

			if (fabsf(deltaImpluse) > 1e-8f)
			{
				// ヒンジ軸周りのトルクインパルス
				Vector3 impulseTwist = joint.axisWorld * deltaImpluse;

				if (impulseTwist.lengthSq() > 1e-8f)
				{
					float invA = (rbA && rbA->IsDynamic()) ? rbA->InvMass() : 0.0f;
					float invB = (rbB && rbB->IsDynamic()) ? rbB->InvMass() : 0.0f;

					if (invA > 0.0f)
					{
						rbA->SetAngularVelocity(wA - rbA->ApplyInvInertiaWorld(impulseTwist));
					}
					if (invB > 0.0f)
					{
						rbB->SetAngularVelocity(wB + rbB->ApplyInvInertiaWorld(impulseTwist));
					}
				}
			}

		}

		// ----- Motor 処理 -----
		if (joint.enableMotor && joint.effMassTwist > 0.0f && joint.maxMotorTorque > 0.0f)
		{
			// 更新した現在速度を再取得
			wA = rbA ? rbA->AngularVelocity() : Vector3();
			wB = rbB ? rbB->AngularVelocity() : Vector3();

			// 相対角速度
			if (rbA && rbB) wRel = wB - wA;
			else if (rbB)	wRel = wB;
			else		    wRel = -wA;

			// ヒンジ軸の成分だけ取り出す
			float wRelTwist = Vector3::Dot(wRel, joint.axisWorld);

			// 目標速度： wRelTwist == motorSpeed
			float Cdot = wRelTwist - joint.motorSpeed;

			// Cdot + λ * M-1 = 0 → λ = -Cdot * effMass
			float deltaImpulse = -Cdot * joint.effMassTwist;

			// 累積とクランプ
			float oldImpulse = joint.accImpulseMotor;
			float maxImpulse = joint.maxMotorTorque * dt; // N・m * s = N・m・s

			float newImpulse = oldImpulse + deltaImpulse;
			// -maxImpulse ~ maxImpulse にクランプ
			if (newImpulse >  maxImpulse) newImpulse =  maxImpulse;
			if (newImpulse < -maxImpulse) newImpulse = -maxImpulse;

			deltaImpulse = newImpulse - oldImpulse;
			joint.accImpulseMotor = newImpulse;

			if (fabsf(deltaImpulse) > 1e-8f)
			{
				Vector3 impulseMotor = joint.axisWorld * deltaImpulse;
				Vector3::Printf(impulseMotor, "impulseMotor");
				if (impulseMotor.lengthSq() > 1e-8f)
				{
					float invA = (rbA && rbA->IsDynamic()) ? rbA->InvMass() : 0.0f;
					float invB = (rbB && rbB->IsDynamic()) ? rbB->InvMass() : 0.0f;

					if (invA > 0.0f)
					{
						rbA->SetAngularVelocity(wA - rbA->ApplyInvInertiaWorld(impulseMotor));
					}
					if (invB > 0.0f)
					{
						rbB->SetAngularVelocity(wB + rbB->ApplyInvInertiaWorld(impulseMotor));
					}
				}
			}
		}
		//Vector3::Printf(joint.axisWorld);
	}
}

// ==================================================
// ----- 見かけの速度更新 -----
// ==================================================
void PhysicsSystem::UpdateKinematicApparentVelocity(float dt)
{
	if (dt <= 0.0f) return;

	for (Rigidbody* rb : m_Rigidbodies)
	{
		if (!rb) continue;
		if (!rb->IsKinematic()) continue;

		auto* tf = rb->Owner()->Transform();
		if (!tf) continue;

		const Vector3 curPos = tf->Position();
		Quaternion curRot = tf->Rotation().normalized();

		// １フレーム目のPrevが無い時用の処理
		if (!rb->HasPrevKinematicPose())
		{
			rb->SetPrevPosition(curPos);
			rb->SetPrevRotation(curRot);
			rb->SetKinematicVelocity(Vector3());
			rb->SetKinematicAngularVelocity(Vector3());
			rb->SetHasPrevKinematicPose(true);
			continue;
		}

		// ----- Veclocity -----
		rb->SetKinematicVelocity((curPos - rb->PrevPosition()) * (1.0f / dt));

		// ----- Angular -----
		Quaternion dq = curRot * rb->PrevRotation().Conjugate();
		dq = dq.normalized();
		if (dq.w < 0.0f) dq = dq * -1.0f; // 最短経路

		float w = std::clamp(dq.w, -1.0f, 1.0f);
		float angle = 2.0f * acosf(w);
		float s = sqrtf(std::max(0.0f, 1.0f - w * w));

		Vector3 axis;
		if (s < 1e-6f || angle < 1e-6f)
			axis = Vector3(0, 0, 0);
		else
			axis = Vector3(dq.x, dq.y, dq.z) * (1.0f / s);

		rb->SetKinematicAngularVelocity(axis * (angle / dt));

		// Prev 更新
		rb->SetPrevPosition(curPos);
		rb->SetPrevRotation(curRot);
	}
}

// --------------------------------------------------
// 同期
// --------------------------------------------------
void PhysicsSystem::SyncCOM()
{
	// Rigidbody の m_CenterOfMassLocal と Collider の m_OffsetPosition を同期させる
	for (auto* rb : m_Rigidbodies)
	{
		auto* col = rb->Owner()->GetComponent<Collider>();
		if (col)
		{
			if (col->OffsetDirty())
			{
				// offsetPosition 反映
				rb->SetCenterOfMassLocal(col->m_OffsetPositionLocal); // 同期
				auto* tr = rb->Owner()->GetComponent<TransformComponent>();
				rb->SetWorldCOM(tr->Position() + tr->Rotation().Rotate(rb->CenterOfMassLocal()));

				// offsetRotation 反映
				rb->UpdateInertiaBodyInvFromOffset(col->m_OffsetRotationLocal);
				rb->UpdateInertiaWorldInvFrom(rb->Owner()->Transform()->Rotation());
				col->ConsumeOffsetDirty(); // フラグを折る
			}
		}
	}
}

// ==================================================
// ----- コライダーデバッグ描画関係 -----
// ==================================================
// ワールド座標を渡す必要あり
void PhysicsSystem::DrawDebug(DebugRenderer& dr)
{
	Vector4 colorTrigger = { 0.0f, 0.0f, 1.0f, 1.0f }; // Triggerなら青

	for (int i = 0; i < m_Colliders.size(); i++)
	{
		Collider* col = m_Colliders[i];
		if (!col) continue;

		const ColliderPose& pose = col->WorldPose();

		// Trigger か判定
		Vector4 color = col->IsTrigger() ? colorTrigger : Vector4{1.0f, 1.0f, 0.0f, 1.0f}; // 黄色

		switch (col->m_Type)
		{
		case ColliderType::Box:
		{
			auto* shape = static_cast<BoxCollision*>(col->Shape());
			assert(shape);
			Vector3 center = pose.position;
			Vector3 halfSize = shape->HalfSize() * pose.scale;
			Matrix4x4 rot = pose.rotation.ToMatrix();

			dr.DrawBox(center, halfSize, rot, color);
			break;
		}
		case ColliderType::Sphere:
		{
			auto* shape = static_cast<SphereCollision*>(col->Shape());
			assert(shape);
			Vector3 center = pose.position;
			float   radius = shape->Radius() * pose.scale.x;
			Matrix4x4 rot = pose.rotation.ToMatrix();

			dr.DrawSphere(center, radius, rot,  color);
			break;
		}
		case ColliderType::Capsule:
		{
			auto* shape = static_cast<CapsuleCollision*>(col->Shape());
			assert(shape);
			Vector3 center = pose.position;
			float   radius = shape->Radius()		 * pose.scale.x;
			float   height = shape->CylinderHeight() * pose.scale.y;
			Matrix4x4 rot  = pose.rotation.ToMatrix();

			dr.DrawCapsule(center, radius, height, rot, color);
			break;
		}
		}
	}
}
