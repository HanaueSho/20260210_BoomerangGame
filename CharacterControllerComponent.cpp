/*
	CharacterControllerComponent.h
	20260205  hanaue sho
	キャラクター制御用コンポーネント
	PhysicsSystem と連携して移動を担う
*/
#include <cmath>
#include "CharacterControllerComponent.h"
#include "TransformComponent.h"
#include "ColliderComponent.h"
#include "PhysicsSystem.h"
#include "Manager.h"
#include "Scene.h"

namespace
{
	// cos(degree)
	float CosDeg(float deg)
	{
		return std::cos(deg * 3.1415926535f / 180.0f);
	}
	// delta の長さを maxDelta でクランプして足す
	Vector3 MoveTowardsVec(const Vector3& cur, const Vector3& target, float maxDelta)
	{
		Vector3 d = target - cur;
		float lenSq = d.lengthSq();
		if (lenSq <= maxDelta * maxDelta) return target;
		float len = std::sqrt(lenSq);
		if (len < 1e-8f) return target;
		return cur + d * (maxDelta / len);
	}
	// -PI..PI の中に収める
	float WrapPi(float a)
	{
		while(a >  3.1415926535f) a -= 2.0f * 3.1415926535f;
		while(a < -3.1415926535f) a += 2.0f * 3.1415926535f;
		return a;
	}
	// yaw を -PI..PI で MoveTowards
	float MoveTowardsAngle(float cur, float target, float maxDelta)
	{
		float d = WrapPi(target - cur);
		if (d > maxDelta)  d =  maxDelta;
		if (d < -maxDelta) d = -maxDelta;
		return cur + d;
	}
}

// ==================================================
// ライフサイクル
// ==================================================
void CharacterControllerComponent::OnAdded()
{
	m_pTransform = Owner()->Transform();
	m_pCapsuleCollider = Owner()->GetComponent<Collider>();
	m_pPhysicsSystem = &Manager::GetScene()->physicsSystem();

	// コライダーを自動で設定する
	if (!m_pCapsuleCollider)
	{
		m_pCapsuleCollider = Owner()->AddComponent<Collider>();
	}
	if (CapsuleCollision* cap = dynamic_cast<CapsuleCollision*>(m_pCapsuleCollider->Shape()))
	{
		// カプセルならそのまま
	}
	else
	{
		// カプセルセット
		m_pCapsuleCollider->SetCapsule(1.0f, 1.0f);
	}
	m_pCapsuleCollider->SetModeQuery();
}
void CharacterControllerComponent::FixedUpdate(float fixedDt)
{
	if (!m_pTransform || !m_pCapsuleCollider || !m_pPhysicsSystem) return;

	// 更新前の位置
	Vector3 prevPos = m_pTransform->Position();
	m_WasGrounded = m_Grounded; // 接地検出用

	// 同期
	m_pCapsuleCollider->UpdateWorldPose();
	m_pCapsuleCollider->UpdateWorldAABB();
	// 移動処理
	StepMovement(fixedDt);
	// 着地更新
	UpdateGrounded();

	// 更新後の位置
	Vector3 nowPos = m_pTransform->Position();
	// 現在速度の更新
	m_ActualVelocity = (nowPos - prevPos) / std::max(fixedDt, 1e-6f);

	// ----- ジャンプ関係処理 -----
	// coyote timer update
	if (m_Grounded) // 着地中
	{
		m_CoyoteTimer = m_Settings.coyoteTime;
 		m_AirJumpsUsed = 0;
	}
	else // 落下中（コヨーテタイマー消費）
	{
		if (m_WasGrounded) m_CoyoteTimer = m_Settings.coyoteTime;
		else			   m_CoyoteTimer = std::max(0.0f, m_CoyoteTimer - fixedDt);
	}
	// consume buffered jump if possible
	if (m_JumpBufferTimer > 0.0f) // バッファにジャンプ入力が残っているか
	{
		const bool canGroundJump = (m_Grounded || m_CoyoteTimer > 0.0f);
		if (canGroundJump)
		{
			// 地上ジャンプ
			DoJump(false, m_Settings.jumpSpeed);
			m_JumpBufferTimer = 0.0f;
			m_CoyoteTimer = 0.0f; // コヨーテ消費
		}
		else if (m_AirJumpsUsed < m_Settings.maxAirJumps)
		{
			// 空中ジャンプ
			DoJump(true, m_Settings.airJumpSpeed);
			m_JumpBufferTimer = 0.0f;
		}
	}
	// jump buffer timer
	if (m_JumpBufferTimer > 0.0f)
		m_JumpBufferTimer = std::max(0.0f, m_JumpBufferTimer - fixedDt);

	// ----- 向き関係処理 -----
	UpdateFacing(fixedDt);
}

// ==================================================
// 入力
// ==================================================
void CharacterControllerComponent::SetMoveInput(const Vector3& moveInput)
{
	m_MoveInputWorld = moveInput;
	m_MoveInputWorld.y = 0.0f;
}
void CharacterControllerComponent::DoJump(bool isAirJump, float jumpSpeed)
{
	// 縦速度
	if (isAirJump && !m_Settings.resetVerticalOnAirJump)
		m_VerticalVelocity += jumpSpeed;
	else
		m_VerticalVelocity = jumpSpeed;


	if (isAirJump)
	{
		// 空中ジャンプカウント
		m_AirJumpsUsed++;

		// 水平速度の上書き
		Vector3 input = m_MoveInputWorld; input.y = 0.0f;
		if (input.lengthSq() > 1e-8f)
		{
			input = input.normalized();

			float airMax = m_Settings.maxSpeed * m_Settings.airMaxSpeedScaleJump;
			Vector3 target = input * (airMax * m_Settings.airJumpOverrideSpeedScale);

			// 完全上書きではなくまずは寄せる
			float a = m_Settings.airJumpOverrideAlpha;
			m_CurrentVelocity = m_CurrentVelocity * (1.0f - a) + target * a;
		}
	}
	m_Grounded = false;
}

// ==================================================
// 歩けるか判定
// ==================================================
bool CharacterControllerComponent::IsWalkableNormal(const Vector3& n) const
{
	// up = (0, 1, 0) 前提です
	return Vector3::Dot(n, Vector3(0,1,0)) >= CosDeg(m_Settings.maxSlopDeg);
}

// ==================================================
// 速度処理
// ==================================================
void CharacterControllerComponent::StepMovement(float fixedDt)
{
	// --------------------------------------------------
	// ----- input velocity -----
	// --------------------------------------------------
	Vector3 input = m_MoveInputWorld;
	input.y = 0.0f;
	float inputLenSq = input.lengthSq();
	if (inputLenSq > 1e-8f) input = input.normalized(); // 入力あり
	else					input = Vector3(0, 0, 0);	// 入力なし
	const bool hasInput = (inputLenSq > 1e-8f);

	// --------------------------------------------------
	// ----- air/ground params -----
	// --------------------------------------------------
	const bool inAir = !m_Grounded;
	float maxSpeed = m_Settings.maxSpeed;
	if (inAir)
	{
		// 空中の最大速度に調整
		const bool rising = (m_VerticalVelocity > 0.0f);
		const float scale = rising ? m_Settings.airMaxSpeedScaleJump : m_Settings.airMaxSpeedScaleFall;
		maxSpeed *= scale;
	}
	// desired
	Vector3 des = input * maxSpeed; des.y = 0.0f;
	m_DesiredVelocity = des; // 目標速度

	// --------------------------------------------------
	// ----- current velocity -----
	// --------------------------------------------------
	Vector3 cur = m_CurrentVelocity; cur.y = 0.0f;

	// 加速度スケール
	float accelScale = 1.0f;
	float turnAccelScale = 1.0f;
	float decelScale = 1.0f;
	if (inAir)
	{
		// 空中スケール
		accelScale	   = m_Settings.airAccelScale;
		turnAccelScale = m_Settings.airTurnAccelScale;
		decelScale     = m_Settings.airDecelScale;
	}

	// どういう加速度を使うかを決める
	float  accel = 0.0f;
	if (!hasInput)
	{
		// 入力無し：止める
		accel = m_Settings.deceleration * decelScale;
		des = Vector3(0, 0, 0);
	}
	else
	{
		// 入力あり
		// 方向転換が大きいなら turnAcceleration、そうでなければ acceleration を使う
		float curLenSq = cur.lengthSq();
		if (curLenSq > 1e-8f)
		{
			Vector3 curDir = cur / std::sqrt(curLenSq);
			Vector3 desDir = des.normalized();
			float d = Vector3::Dot(curDir, desDir); // -1..1
			// 反転／大きく曲がるときは強め
			accel = (d < 0.5f) ? m_Settings.turnAcceleration * turnAccelScale
							   : m_Settings.acceleration	 * accelScale;
		}
		else
			accel = m_Settings.acceleration * accelScale;
	}
	float maxDelta = accel * fixedDt;
	Vector3 newCur = MoveTowardsVec(cur, des, maxDelta);

	// 空中クランプ（保険）
	if (inAir)
	{
		float speedSq = newCur.lengthSq();
		float maxSq = maxSpeed * maxSpeed;
		if (speedSq > maxSq && speedSq > 1e-8f)
		{
			float sp = std::sqrt(speedSq);
			newCur *= (maxSpeed / sp);
		}
	}
	m_CurrentVelocity = newCur;

	// --------------------------------------------------
	// ----- vertical velocity -----
	// --------------------------------------------------
	// 接地中の下向き速度は潰す
	if (m_Grounded && m_VerticalVelocity < 0.0f)
		m_VerticalVelocity = 0.0f;
	// 上昇下降カットで重力スケールを切り替え
	float gScale = 1.0f;
	if (m_VerticalVelocity > 0.0f) // 上昇中
		gScale = (m_JumpHeld ? m_Settings.gravityScaleJump : m_Settings.gravityScaleCut);
	else
		gScale = m_Settings.gravityScaleFall;

	m_VerticalVelocity += (m_Settings.gravity * gScale) * fixedDt;

	// --------------------------------------------------
	// ----- displacement（位置増分）を作る -----
	// --------------------------------------------------
	Vector3 displacement = m_CurrentVelocity;
	displacement.y = m_VerticalVelocity;
	displacement *= fixedDt;

	// 合成して Move＆Slide
	MoveAndSlide_Substep(displacement, fixedDt);
}
void CharacterControllerComponent::MoveAndSlide_Substep(const Vector3& displacement, float fixedDt)
{
	Vector3 remaining = displacement; // この Step で「まだ動きたい移動量（位置の増分）」
	float len = std::sqrt(remaining.lengthSq()); // 移動量
	if (len < 1e-8f) return;

	// １度で大きく移動するとトンネリングしてしまう
	// そこで「maxStepMove ずつ」に分割して複数回に分ける（サブステップ）
	int steps = (int)std::ceil(len / m_Settings.maxStepMove); // ceil: 小数点切り上げ
	steps = std::max(1, steps);

	// Query対象を組み立てる
	QueryOptions opt{};
	BuildQueryOpt(opt);

	// --------------------------------------------------
	// ----- サブステップループ -----
	// remaining を少しずつ消費しながら Move&Slide を繰り返す
	// --------------------------------------------------
	for (int s = 0; s < steps; s++)
	{
		// --------------------------------------------------
		// もう残りがほぼ無ければ終了
		// --------------------------------------------------
		if (remaining.lengthSq() < 1e-8f) break;

		// --------------------------------------------------
		// このサブステップで動かす量 stepDelta を決める
		// remaining を masStepMove 以内にクランプしたものを stepDelta とする
		// --------------------------------------------------
		Vector3 stepDelta = remaining; 
		{
			float stepLenSq = stepDelta.lengthSq();
			float maxLen = m_Settings.maxStepMove;
			if (stepLenSq > maxLen * maxLen)
			{
				float stepLen = std::sqrt(stepLenSq);
				stepDelta *= (maxLen / stepLen);
			}
		}
		// 今回の理想として消費する量
		const Vector3 stepDeltaPlanned = stepDelta; 

		// --------------------------------------------------
		// いったん移動させる
		// --------------------------------------------------
		m_pTransform->SetPosition(m_pTransform->Position() + stepDelta);

		// 自分のコライダーを同期
		m_pCapsuleCollider->UpdateWorldPose();
		m_pCapsuleCollider->UpdateWorldAABB();

		// --------------------------------------------------
		// めり込み解消　＋　スライド
		// --------------------------------------------------
		bool hitSomething = false; // 今は使ってないが、今後のデバッグなどで使える
		for (int it = 0; it < m_Settings.maxSliderIterations; it++)
		{
			Vector3 mtd{};
			QueryHit hit{};
			if (!m_pPhysicsSystem->QueryComputeMTD(m_pCapsuleCollider, mtd, &hit, opt))
				break;
			hitSomething = true;

			// Dynamic 剛体を押し返す処理
			TryPushDynamic(hit, mtd, fixedDt);

			// 押し出し（skin を少し足す：再めり込み抑制）
			Vector3 push = mtd;
			float pushLenSq = push.lengthSq();
			if (pushLenSq > 1e-12f)
			{
				Vector3 n = (-mtd).normalized(); // mtd = - normal * pen
				push += -n * m_Settings.skinWidth; // mtd の向きが逆のせいで -n　にしています（本来は push += n * m_Settings.skinWidth）
			}
			m_pTransform->SetPosition(m_pTransform->Position() + push);

			// 同期
			m_pCapsuleCollider->UpdateWorldPose();
			m_pCapsuleCollider->UpdateWorldAABB();

			// --------------------------------------------------
			// スライド
			// 残り移動 remaining を”壁面に沿うように”変形する
			// --------------------------------------------------
			const bool isWalkable = IsWalkableNormal(hit.normal);
			
			float vn = Vector3::Dot(remaining, hit.normal); // 壁に沿うようなベクトルだけ残す
			if (vn < 0.0f)
				remaining = remaining - hit.normal * vn;

			if (isWalkable)
				if (remaining.y < 0.0f)
					remaining.y = 0.0f;
			// --------------------------------------------------
			// 速度側にも反映：壁方向に押し続ける速度を削る
			// これをたらないと、毎フレーム同じ方向に突っ込む→MTD押し出し…が繰り返されてしまう
			// --------------------------------------------------
			{
				Vector3 v = m_CurrentVelocity;
				v.y = 0.0f;
				float vN = Vector3::Dot(v, hit.normal);
				if (vN < 0.0f)
				{
					v = v - hit.normal * vN;
					m_CurrentVelocity.x = v.x;
					m_CurrentVelocity.z = v.z;
				}
			}
		}
		// --------------------------------------------------
		// このサブステップで“計画した分”を remaining から引く
		// --------------------------------------------------
		remaining -= stepDeltaPlanned;

		// ここでremaining がもうほぼ無いなら終わる
		if (remaining.lengthSq() < 1e-8f) break;
	}
}
void CharacterControllerComponent::UpdateGrounded()
{
	m_Grounded = false;
	m_GroundNormal = Vector3(0, 1, 0);

	// 上昇中はスナップしない
	if (m_VerticalVelocity > 0.0f)
		return;


	// 足元チェック（本当はpose指定Queryが欲しい）
	const Vector3 pos0 = m_pTransform->Position();
	const Vector3 down = Vector3(0, -m_Settings.groundProbe, 0);

	m_pTransform->SetPosition(pos0 + down);
	m_pCapsuleCollider->UpdateWorldPose();
	m_pCapsuleCollider->UpdateWorldAABB();

	QueryOptions opt{};
	BuildQueryOpt(opt);

	QueryHit hit{};
	if (m_pPhysicsSystem->QueryOverlapBest(m_pCapsuleCollider, hit, opt))
	{
		hit.normal = -hit.normal; // 整合性を保つため（なぜかQueryOverlapBest内で補正するとif文を通らなくなる）
		if (IsWalkableNormal(hit.normal))
		{
			m_Grounded = true;
			m_GroundNormal = hit.normal;

			// 地面にめり込んでいるなら少し押し戻す
			Vector3 snap = hit.mtd;
			Vector3 up = { 0, 1, 0 }; // ワールド上ベクトル
			float upMove = Vector3::Dot(snap, up);
			if (upMove > 1e-12f)
			{
				// skinWidth 分持ち上げる
				upMove += m_Settings.skinWidth; 
				// めり込んだ分だけ戻す
				m_pTransform->SetPosition(m_pTransform->Position() +  up * upMove);
			}

			// 同期
			m_pCapsuleCollider->UpdateWorldPose();
			m_pCapsuleCollider->UpdateWorldAABB();

			// groundedなら落下速度を止める
			if (m_Grounded && m_VerticalVelocity < 0.0f)
				m_VerticalVelocity = 0.0f;

			return;
		}
	}

	// 接地してないなら元に戻す
	m_pTransform->SetPosition(pos0);
	m_pCapsuleCollider->UpdateWorldPose();
	m_pCapsuleCollider->UpdateWorldAABB();
}

// ==================================================
// Dynamic 剛体を押し返す
// ※Dynamic剛体を押してはいるが、位置を押し込んでいるわけでは無い。さらに、こちらのMTDは補正しているのでDynamic剛体に触れると減速する
// ==================================================
bool CharacterControllerComponent::TryPushDynamic(const QueryHit& hit, const Vector3& mtd, float dt)
{
	if (!hit.other) return false;

	auto* otherRb = hit.other->Owner()->GetComponent<Rigidbody>();
	if (!otherRb || !otherRb->IsDynamic()) return false;

	// 床は押さないので判定
	const float upDot = Vector3::Dot(hit.normal, Vector3(0, 1, 0));
	if (upDot >= CosDeg(m_Settings.maxSlopDeg)) return false;

	// 押す強さ（とりあえず「めり込み量／ｄｔ」）
	const float pen = mtd.length();
	if (pen <= 1e-6f) return false;

	Vector3 dir = (-mtd).normalized(); // 相手を押す方向
	float speed = std::min(pen / std::max(dt, 1e-6f), m_Settings.maxPushSpeed);

	// 相手を押す
	Vector3 v = otherRb->Velocity();
	float vn = Vector3::Dot(v, dir);
	if (vn < speed) otherRb->SetVelocity(v + dir * (speed - vn));
	return true;
}

// ==================================================
// Query用
// ==================================================
void CharacterControllerComponent::BuildQueryOpt(QueryOptions& opt) const
{
	opt.isSimulate  = true;
	opt.isTrigger   = false; // 通常 Trigger では止まらない
	opt.isQueryOnly = false; // 他の QueryOnly には当たらない
	opt.slop = 0.0f;
}

// ==================================================
// 向き処理用
// ==================================================
void CharacterControllerComponent::UpdateFacing(float fixedDt)
{
	if (!m_Settings.faceMoveInput) return;
	if (!m_pTransform) return;

	// 入力方向
	Vector3 dir = m_MoveInputWorld;
	dir.y = 0.0f;
	if (dir.lengthSq() < 1e-8f) return;

	dir = dir.normalized();
	m_FacingDirWorld = dir; // 一応保持（外部へ出力できるから）

	// 地上/空中で回転速度を変える
	const float turnSpeedDeg = m_Grounded ? m_Settings.turnSpeedGroundDeg 
										  : m_Settings.turnSpeedAirDeg;
	const float maxDelta = (turnSpeedDeg * 3.1415926535f / 180.0f) * fixedDt;

	// 現在の yaw と目標 yaw
	const Vector3 f = m_pTransform->Forward();
	float curYaw = std::atan2(f.x, f.z);
	float tgtYaw = std::atan2(dir.x, dir.z);

	float newYaw = MoveTowardsAngle(curYaw, tgtYaw, maxDelta);

	// yaw のみ回転に反映
	Quaternion q = Quaternion::FromAxisAngle(Vector3(0, 1, 0), newYaw);
	m_pTransform->SetRotation(q);
}