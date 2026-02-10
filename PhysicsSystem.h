/*
	PhysicsSystem.h
	20250821  hanaue sho
	物理演算
*/
#ifndef PHYSICSSYSTEM_H_
#define PHYSICSSYSTEM_H_
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <utility>
#include "ContactManifold.h"
#include "Matrix4x4.h"

class Scene;
class Collider;
class Rigidbody;
class DistanceJointComponent;
class BallJointComponent;
class HingeJointComponent;
class TransformComponent;
class DebugRenderer;

// ==================================================
// ----- 構造体 -----
// ==================================================
// --------------------------------------------------
// 接触点の構造体
// --------------------------------------------------
struct Contact
{
	Collider* A;
	Collider* B;
	ContactManifold m;
};
// --------------------------------------------------
// ジョイント関係の構造体
// --------------------------------------------------
struct DistanceJoint
{
	// DistanceJointComponent の情報
	int jointId = -1;
	Rigidbody* pBodyA = nullptr; // 自身の Rigidbody
	Rigidbody* pBodyB = nullptr; // 相方の Rigidbody
	TransformComponent* tfA = nullptr; // 自身のTransform
	TransformComponent* tfB = nullptr; // 相方のTransform

	Vector3 localAnchorA = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 localAnchorB = Vector3(0.0f, 0.0f, 0.0f);
	float restLength = 0.0f;
	int mode = 0;

	// ソルバ用のキャッシュ
	Vector3 rAworld = Vector3(0.0f, 0.0f, 0.0f); // COM からアンカーまでのベクトル
	Vector3 rBworld = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 normal  = Vector3(0.0f, 0.0f, 0.0f); // anchorA → anchorB
	float effectiveMass = 0.0f;
	float bias			= 0.0f;
	float accumImpulse	= 0.0f; // 累積インパルス
};
struct BallJoint
{
	// BallJointComponent の情報
	int jointId = -1;
	Rigidbody* pBodyA = nullptr; // 自身の Rigidbody
	Rigidbody* pBodyB = nullptr; // 相方の Rigidbody
	TransformComponent* tfA = nullptr; // 自身のTransform
	TransformComponent* tfB = nullptr; // 相方のTransform

	Vector3 localAnchorA = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 localAnchorB = Vector3(0.0f, 0.0f, 0.0f);

	// ソルバ用のキャッシュ
	Vector3 rAworld = Vector3(0.0f, 0.0f, 0.0f); // COM からアンカーまでのベクトル
	Vector3 rBworld = Vector3(0.0f, 0.0f, 0.0f);

	// ３軸分の有効質量、バイアス、累積インパルス
	Vector3 effectiveMass;
	Vector3 bias;
	Vector3 accumImpulse; // 累積インパルス
};
struct HingeJoint
{
	// HingeJointComponent の情報
	int jointId = -1;
	Rigidbody* pBodyA = nullptr; // 自身の Rigidbody
	Rigidbody* pBodyB = nullptr; // 相方の Rigidbody
	TransformComponent* tfA = nullptr; // 自身のTransform
	TransformComponent* tfB = nullptr; // 相方のTransform

	// アンカー 
	Vector3 localAnchorA = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 localAnchorB = Vector3(0.0f, 0.0f, 0.0f);

	// ソルバ用のキャッシュ
	Vector3 rAworld = Vector3(0.0f, 0.0f, 0.0f); // COM からアンカーまでのベクトル
	Vector3 rBworld = Vector3(0.0f, 0.0f, 0.0f);

	// ３軸分の有効質量、バイアス、累積インパルス
	Vector3 effectiveMassLinear;
	Vector3 biasLinear;
	Vector3 accumImpulseLinear; // 累積インパルス

	// 軸合わせ＆リミット用
	Vector3 swingAxis[2]{}; // ヒンジ軸に直交する基底
	Matrix4x4 swingMass;
	Vector3 biasSwing; // (bias1, bias2, 0)
	Vector3 accImpulseSwing; // 累積インパルス (λ1, λ2, 0)
	Vector3 swingDiff; // swing 誤差 (C1, C2, 0)

	// 軸周りの回転（ツイスト）制限、モーター用
	float effMassTwist    = 0.0f;
	float biasTwist		  = 0.0f;
	float accImpulseTwist = 0.0f;

	// ヒンジ軸・基準方向（ローカル）
	Vector3 localAxisA = Vector3(1.0f, 0.0f, 0.0f); // ヒンジ軸
	Vector3 localAxisB = Vector3(1.0f, 0.0f, 0.0f); // ヒンジ軸
	Vector3 localRefA = Vector3(1.0f, 0.0f, 0.0f); // ０度の向き
	Vector3 localRefB = Vector3(1.0f, 0.0f, 0.0f); // B側の０度の向き

	// ワールドに変換したもの
	Vector3 axisWorld = Vector3(1.0f, 0.0f, 0.0f);
	Vector3 refAWorld = Vector3(1.0f, 0.0f, 0.0f); // Aの０の方向
	Vector3 refBWorld = Vector3(1.0f, 0.0f, 0.0f); // Bの０の方向

	// リミット
	bool enableLimit = false;
	float limitMin = 0.0f;
	float limitMax = 0.0f;
	
	// モーター
	bool enableMotor = false;
	float motorSpeed = 0.0f;
	float maxMotorTorque = 0.0f;
	float accImpulseMotor = 0.0f;

	// バネ
	bool enableSpring	= false;
	float springTarget	= 0.0f; // 目標角度
	float springDamping = 0.0f;
	float springDiff	= 0.0f; // angle - target をキャッシュ
};
// --------------------------------------------------
// Query 用の構造体
// --------------------------------------------------
struct QueryOptions
{
	bool isSimulate = true;
	bool isTrigger  = false;
	bool isQueryOnly = false;
	float slop = 0.0f; // isOverlap の slop
};
struct QueryHit
{
	Collider* other = nullptr; // 当たった相手
	Vector3 normal{};		   // A -> B（A=自分）
	float penetration = 0.0f;  // めり込み量
	Vector3 mtd{};			   // 自分を押し出すベクトル
	ContactManifold manifold{};
};

// ==================================================
// ----- クラス本体 -----
// ==================================================
class PhysicsSystem
{
private:
	// ==================================================
	// ----- 要素 -----
	// ==================================================
	// --------------------------------------------------
	// 親シーン
    // --------------------------------------------------
	Scene* m_pScene = nullptr; // 現在のシーン
	
	// --------------------------------------------------
	// 登録コライダー、ボディ
    // --------------------------------------------------
	std::vector<Collider*>  m_Colliders;   // 登録した Collider を管理する配列
	std::vector<Rigidbody*> m_Rigidbodies; // 登録した Rigidbody を管理する配列

	// --------------------------------------------------
	// 登録ジョイント
	// --------------------------------------------------
	std::vector<DistanceJoint> m_DistanceJoints;
	int m_DistanceJointNextId = 0;
	std::vector<int> m_FreeDistanceJointIds;
	std::vector<DistanceJointComponent*> m_DistanceJointById; //id → Joint* （逆引き用）

	std::vector<BallJoint> m_BallJoints;
	int m_BallJointNextId = 0;
	std::vector<int> m_FreeBallJointIds;
	std::vector<BallJointComponent*> m_BallJointById; //id → Joint* （逆引き用）

	std::vector<HingeJoint> m_HingeJoints;
	int m_HingeJointNextId = 0;
	std::vector<int> m_FreeHingeJointIds;
	std::vector<HingeJointComponent*> m_HingeJointById; //id → Joint* （逆引き用）

	// --------------------------------------------------
	// コライダー用採番 
    // --------------------------------------------------
	int m_NextId = 0;
	std::vector<int> m_FreeIds;	   // 解放IDの再利用
	std::vector<Collider*> m_ById; //id → collider* （逆引き用）

	// ==================================================
	// ----- 小ヘルパ -----
	// ==================================================
	// --------------------------------------------------
	// ペア集合
    // --------------------------------------------------
	uint64_t MakePairKey(uint32_t a, uint32_t b) noexcept
	{
		if (b < a) std::swap(a, b); // 順序を正規化
		return (uint64_t(a) << 32) | uint64_t(b); // 64bit の中に、小さい方を上位ビット、大きい方を下位ビットとして詰める
	}
	static inline int KeyHigh(uint64_t k) { return int(uint32_t(k >> 32)); } // 上位ビットを int に変換
	static inline int KeyLow (uint64_t k) { return int(uint32_t(k & 0xffffffffu)); } // 下位ビットを int に変換

	// --------------------------------------------------
	// １ステップ前と現在の衝突ペア
	// --------------------------------------------------
	std::unordered_set<uint64_t> m_PrevTrigger,   m_CurrTrigger;
	std::unordered_set<uint64_t> m_PrevCollision, m_CurrCollision;

	// --------------------------------------------------
	// 発火キュー
    // --------------------------------------------------
	std::vector<std::pair<Collider*, Collider*>> m_TriggerEnter;
	std::vector<std::pair<Collider*, Collider*>> m_TriggerExit;
	std::vector<std::pair<Collider*, Collider*>> m_CollisionEnter;
	std::vector<std::pair<Collider*, Collider*>> m_CollisionExit;

	// --------------------------------------------------
	// 今ステップの衝突点のコンテナ
    // --------------------------------------------------
	std::vector<Contact> m_Contacts; 

	// --------------------------------------------------
	// コリジョンマスク
	// --------------------------------------------------
	static constexpr int MaxLayers = 32;
	uint32_t m_CollisionMask[MaxLayers] = {};
public:
	// ==================================================
	// ----- コンストラクタ -----
	// ==================================================
	PhysicsSystem(Scene& scene) : m_pScene(&scene) {}
	~PhysicsSystem() = default;

	// ==================================================
	// ----- ライフサイクル -----
	// ==================================================
	// --------------------------------------------------
	// 初期化、終了処理
	// --------------------------------------------------
	void Init();
	void Shutdown();
	// --------------------------------------------------
	// レイヤーマスク
	// --------------------------------------------------
	void SetCollision(int layerA, int layerB, bool enable);
	bool ShouldCollide(const Collider& a, const Collider& b) const;
	// --------------------------------------------------
	// 登録、解除 
    // --------------------------------------------------
	int  RegisterCollider  (Collider* c);
	void UnregisterCollider(Collider* c);
	void RegisterRigidbody  (Rigidbody* rb);
	void UnregisterRigidbody(Rigidbody* rb);
	int  RegisterDistanceJoint  (DistanceJointComponent* joint);
	void UnregisterDistanceJoint(DistanceJointComponent* joint);
	int  RegisterBallJoint	(BallJointComponent* joint);
	void UnregisterBallJoint(BallJointComponent* joint);
	int  RegisterHingeJoint  (HingeJointComponent* joint);
	void UnregisterHingeJoint(HingeJointComponent* joint);
	// --------------------------------------------------
	// ステップ
    // --------------------------------------------------
	void BeginStep(float fixedDt); 
	void Step(float fixedDt); 
	void EndStep(float fixedDt);

	void DispatchEvents(); // Trigger, Collision 配信

	// ==================================================
	// ----- Query 関係 -----
	// ==================================================
	// --------------------------------------------------
	// １）いまの位置で重なっているか（最も深い１件を返す）
    // --------------------------------------------------
	bool QueryOverlapBest(const Collider* self, QueryHit& outHit, const QueryOptions& opt) const;
	// --------------------------------------------------
	// ２）MTD（最小押し出し）だけ欲しい場合
	// --------------------------------------------------
	bool QueryComputeMTD(const Collider* self, Vector3& outMTD, QueryHit* outHit, const QueryOptions& opt) const;
	// --------------------------------------------------
	// ３）疑似 CapsuleCast: delta 方向に動かしたときに最初に当たる位置を二分探索で探す
	// （今は使われていないよ）
	// --------------------------------------------------
	bool QueryCapsuleCastBinary(const Collider* self, const Vector3& delta, float skinWidth, QueryHit& outHit, const QueryOptions& opt, int binaryIterations = 10) const;

private:
	// ==================================================
	// ----- 基本物理演算 -----
	// ==================================================
	// --------------------------------------------------
	// 力反映
	// m_Rigidbodies
    // --------------------------------------------------
	void IntegrationForce(float dt);

	// --------------------------------------------------
	// 衝突判定 
	// m_Colliders
    // --------------------------------------------------
	void DetermineCollision();

	// --------------------------------------------------
	// 衝突点の事前計算
	// m_Contacts
	// --------------------------------------------------
	void PreSolveContacts();

	// --------------------------------------------------
	// 速度解決
	// m_Contacts
    // --------------------------------------------------
	void ResolveVelocity(float dt);
	
	// --------------------------------------------------
	// 減衰処理（指数減衰）
	// m_Rigidbodies
    // --------------------------------------------------
	void ApplyDamping(float dt);

	// --------------------------------------------------
	// 位置補正
	// m_Contacts
    // --------------------------------------------------
	void CorrectPosition();

	// --------------------------------------------------
	// 速度反映
	// m_Rigidbodies
    // --------------------------------------------------
	void IntegrationVelocity(float dt);

	// ==================================================
	// ----- ジョイント -----
	// ==================================================
	// --------------------------------------------------
	// ジョイントの事前計算
	// m_Joints
	// --------------------------------------------------
	void PreSolveDistanceJoints(float dt);
	void PreSolveBallJoints(float dt);
	void PreSolveHingeJoints(float dt);

	// --------------------------------------------------
	// ジョイントの解決
	// m_Joints
	// --------------------------------------------------
	void ResolveDistanceJoints(float dt);
	void ResolveBallJoints(float dt);
	void ResolveHingeJoints(float dt);

	// ==================================================
	// ----- 見かけの速度更新 -----
	// ==================================================
	void UpdateKinematicApparentVelocity(float dt);

	// ==================================================
	// ----- 同期 -----
	// ==================================================
	// --------------------------------------------------
	// COM の同期
	// --------------------------------------------------
	void SyncCOM();

public:
	// ==================================================
	// ----- コライダーデバッグ描画関係 -----
	// ==================================================
	void DrawDebug(DebugRenderer& dr);
};

#endif