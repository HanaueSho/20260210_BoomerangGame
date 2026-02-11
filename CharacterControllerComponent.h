/*
	CharacterControllerComponent.h
	20260205  hanaue sho
	キャラクター制御用コンポーネント
	PhysicsSystem と連携して移動を担う
*/
#ifndef CHARACTERCONTROLLERCOMPONENT_H_
#define CHARACTERCONTROLLERCOMPONENT_H_
#include "Component.h"
#include "Vector3.h"

class TransformComponent;
class Collider;
class Rigidbody;
class PhysicsSystem;
struct QueryOptions;
struct QueryHit;

class CharacterControllerComponent : public Component
{
public:
	// ==================================================
	// 構造体
	// ==================================================
	struct Settings
	{
		// ----- movement -----
		float maxSpeed = 30.0f;			// 最大移動速度
		float acceleration = 40.0f;		// 加速度 [m/s^2]
		float deceleration = 40.0f;		// 停止加速度 [m/s^2]
		float turnAcceleration = 50.0f; // ターン加速度 [m/s^2]

		// ----- jump -----
		float gravity   = -9.8f;
		float jumpSpeed = 20.0f; // ジャンプ初速度
		float gravityScaleJump = 1.5f; // 上昇中（vY > 0 かつ JumpHold 中）の重力倍率
		float gravityScaleFall = 2.0f; // 下降中（vY <= 0）の重力倍率
		float gravityScaleCut  = 2.5f; // 上昇中にボタン離しの重力倍率
		int maxAirJumps = 2; // 最大ジャンプ回数
		float airJumpSpeed = 10.0f; // 空中ジャンプの初速度
		bool resetVerticalOnAirJump = true; // 空中ジャンプ時に上書きするか（上書き推奨）
		float coyoteTime = 0.10f;	 // 崖から落ちた後も地上ジャンプ扱いにする
		float jumpBufferTime = 0.1f; // 早押しジャンプを保持する猶予

		// ----- air control -----
		float airMaxSpeedScaleJump = 0.9f; // 上昇中の空中最大速度
		float airMaxSpeedScaleFall = 1.0f; // 落下中の空中最大速度
		float airAccelScale     = 1.0f; // 空中加速度スケール
		float airTurnAccelScale = 0.3f; // 空中ターン加速度スケール
		float airDecelScale     = 0.5f; // 空中での入力無しでの減速（０だと永遠に滑る）
		bool  airJumpOverrideHorizontal = true; // 空中ジャンプ時に水平速度を上書きするか
		float airJumpOverrideSpeedScale = 1.0f; // 空中最大速度に対して何割で塗り替えるか
		float airJumpOverrideAlpha = 0.5f; // 上書き時に

		// ----- collision, ground -----
		float skinWidth = 0.02f;   // 
		float maxSlopDeg = 50.0f;  // 床扱いの最大傾斜
		float groundProbe = 0.08f; // 足元チェック

		// ----- pushing -----
		float maxPushSpeed = 50.0f; // Dynamicな剛体を押し返す最大速度
		
		// ----- solver, stepping -----
		float maxStepMove = 0.15f;	   // 1substep　の最大移動距離（抜け防止）
		int	  maxSliderIterations = 4; // ２～４で十分

		// ----- turn -----
		bool faceMoveInput = true; // 入力方向へ向く
		float turnSpeedGroundDeg = 360.0f; // 地上：１秒当たり何度回すか
		float turnSpeedAirDeg	 = 240.0f; // 空中：弱め追従
	};

private:
	// ==================================================
	// 構成要素
	// ==================================================
	TransformComponent* m_pTransform = nullptr;
	Collider*			m_pCapsuleCollider = nullptr;
	PhysicsSystem*		m_pPhysicsSystem = nullptr;

	// ==================================================
	// 設定＆状態
	// ==================================================
	Settings m_Settings{};

	// ==================================================
	// 入力＆速度
	// ==================================================
	Vector3 m_MoveInputWorld{};  // 水平方向入力
	float   m_InputDeadZone = 0.1f; // パッドのデッドゾーン
	bool  m_JumpHeld = false; // 今ジャンプボタンを押されているか（入力側から毎フレーム更新）

	Vector3 m_DesiredVelocity{}; // 入力由来の目標速度( m_MoveInputWorld * maxSpeed )
	Vector3 m_CurrentVelocity{}; // 慣性後の水平速度
	float   m_VerticalVelocity = 0.0f;  // 垂直速度
	Vector3 m_ActualVelocity{};  // 実移動速度
	Vector3 m_PrevPosition{};	 // actualVelocity 計算用

	// ==================================================
	// 着地関係
	// ==================================================
	bool m_Grounded = false;
	Vector3 m_GroundNormal = Vector3(0, 1, 0);
	int m_AirJumpsUsed = 0; // 空中ジャンプ何回したか
	bool m_WasGrounded = false;
	float m_CoyoteTimer = 0.0f;
	float m_JumpBufferTimer = 0.0f;
	bool m_JumpPressedThisFrame = false; // 入力トリガー（入力側から毎フレーム更新）

	// ==================================================
	// 向き関係
	// ==================================================
	Vector3 m_FacingDirWorld = Vector3(0, 0, 1);
public:
	// ==================================================
	// コンストラクタ
	// ==================================================
	CharacterControllerComponent() = default;

	// ==================================================
	// ライフサイクル
	// ==================================================
	void OnAdded() override;
	void FixedUpdate(float fixedDt) override;

	// ==================================================
	// 外部入力
	// ==================================================
	void SetMoveInput(const Vector3& moveInput);
	void SetJumpHeld(bool held) { m_JumpHeld = held; }
	void OnJumpPressed() { m_JumpBufferTimer = m_Settings.jumpBufferTime; }
	void AddHorizontalVelocity(const Vector3& v)
	{
		m_CurrentVelocity.x += v.x;
		m_CurrentVelocity.y += v.y;
	}
	void AddVerticalVelocity(float vy) { m_VerticalVelocity += vy; }

	// ==================================================
	// 状態参照
	// ==================================================
	bool IsGround() const { return m_Grounded; }
	const Vector3& GroundNormal() const { return m_GroundNormal; }
	const Vector3& ActualVelocity() const { return m_ActualVelocity; }
	float MaxMoveSpeed() const { return m_Settings.maxSpeed; }

	// ==================================================
	// セッター
	// ==================================================
	void SetSettings(const Settings& set) { m_Settings = set; }
	void SetMaxMoveSpeed(float speed) { m_Settings.maxSpeed = speed; }
	void SetFaceMoveInput(bool b) { m_Settings.faceMoveInput = b; }
	void SetHorizontalVelocity(const Vector3& v)
	{
		m_CurrentVelocity.x = v.x;
		m_CurrentVelocity.y = v.y;
	}
	void SetVerticalVelocity(float vy) { m_VerticalVelocity = vy; }

private:
	// ==================================================
	// 歩けるか判定
	// ==================================================
	bool IsWalkableNormal(const Vector3& n) const;

	// ==================================================
	// 速度処理
	// ==================================================
	void StepMovement(float fixedDt);
	void MoveAndSlide_Substep(const Vector3& displacement, float fixedDt);
	void UpdateGrounded();
	// ==================================================
	// Dynamic 剛体を押し返す
	// ==================================================
	bool TryPushDynamic(const QueryHit& hit, const Vector3& mtd, float fixedDt);

	// ==================================================
	// Query用
	// ==================================================
	void BuildQueryOpt(QueryOptions& opt) const;

	// ==================================================
	// ジャンプ処理用
	// ==================================================
	void DoJump(bool isAirJump, float jumpSpeed);

	// ==================================================
	// 向き処理用
	// ==================================================
	void UpdateFacing(float fixedDt);
};

#endif

/*
【備忘録】
QueryOverlapBestによる当たり判定で、hit.normalの向きが逆のせいでの不都合を整えるための符号変換が
いくつかちりばめられています。（skinWidthとgroundNormal辺り）
*/