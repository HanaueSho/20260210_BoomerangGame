/*
	BoomerangStateManagerComponent.cpp
	20260212  hanaue sho
*/
#include "BoomerangStateManagerComponent.h"
#include "AimStateManagerComponent.h"
#include "BoneManager.h"
#include "PlayerObject.h"
#include "GameObject.h"
#include "Keyboard.h"
#include "AimObject.h"
#include "ColliderComponent.h"

#include "Manager.h"
#include "Scene.h"

namespace
{
	// -PI..PI の中に収める
	float WrapPi(float a)
	{
		while (a > 3.1415926535f) a -= 2.0f * 3.1415926535f;
		while (a < -3.1415926535f) a += 2.0f * 3.1415926535f;
		return a;
	}
}

void BoomerangStateManagerComponent::Init()
{
	// AimObject の生成
	m_pAimObject = Manager::GetScene()->AddGameObject<AimObject>(1);
	m_pAimObject->Init();
}

void BoomerangStateManagerComponent::Update(float dt)
{
	switch (m_State)
	{
	case State::Idle:
		break;
	case State::Aim:
		break;
	case State::Throw:
		Throw(dt);
		break;
	case State::Back:
		break;
	}

}

void BoomerangStateManagerComponent::FixedUpdate(float fixedDt)
{
}

void BoomerangStateManagerComponent::OnTriggerEnter(Collider* me, Collider* other)
{
	switch (m_State)
	{
	case State::Idle:
		break;
	case State::Aim:
		break;
	case State::Throw:
	{
		if (other->Owner()->Tag() == "Enemy")
		{
			m_IndexTargets++;
			m_IsApproach = true;
		}
	}
		break;
	case State::Back:
		break;
	}
}

void BoomerangStateManagerComponent::ChangeState(State newState)
{
	// ----- 終了処理 -----
	switch (m_State)
	{
	case State::Idle:
		break;
	case State::Aim:
		break;
	case State::Throw:
		break;
	case State::Back:
		break;
	}

	// ----- 初期化処理 -----
	m_State = newState;
	switch (newState)
	{
	case State::Idle:
	{
		SetSpine();
		GetAimObject()->GetComponent<AimStateManagerComponent>()->SetIsAimming(false);
	}
		break;
	case State::Aim:
	{
		SetHand();
		GetAimObject()->GetComponent<AimStateManagerComponent>()->SetIsAimming(true);
	}
		break;
	case State::Throw:
	{
		// Target をコピー
		m_Targets = GetAimObject()->GetComponent<AimStateManagerComponent>()->Targets();
		if (m_Targets.size() < 1)
		{
			ChangeState(State::Idle); // 投げれない
			break;
		}

		// 飛翔方向算出
		m_MoveDir  = m_pPlayerObject->Transform()->Forward(); // 正面ベクトル
		m_MoveDir += m_pPlayerObject->Transform()->Right(); // 右側へ補正
		m_MoveDir.y += 0.2f; // 上へも少し補正
		m_MoveDir.normalize();
		// 親子関係を切る
		Owner()->Transform()->SetParentKeepWorld(nullptr);
		// フラグ初期化
		m_IsApproach = false;
		// インデックス初期化
		m_IndexTargets = 0;
	}
		break;
	case State::Back:
		break;
	}
}

void BoomerangStateManagerComponent::SetPlayerObject(GameObject* player)
{
	m_pPlayerObject = player;
	// 背骨ボーン
	m_pBoneSpineObject = dynamic_cast<PlayerObject*>(m_pPlayerObject)->GetBoneObject(2);
	m_pBoneHandObject = dynamic_cast<PlayerObject*>(m_pPlayerObject)->GetBoneObject(24);
}

void BoomerangStateManagerComponent::SetHand()
{
	if (!m_pBoneHandObject) return;
	Owner()->Transform()->SetParent(m_pBoneHandObject->Transform());
	Owner()->Transform()->SetLocalPosition(m_OffsetHandPosition);
	Owner()->Transform()->SetLocalEulerAngles(m_OffsetHandRotation);
}

void BoomerangStateManagerComponent::SetSpine()
{
	if (!m_pBoneSpineObject) return;
	Owner()->Transform()->SetParent(m_pBoneSpineObject->Transform());
	Owner()->Transform()->SetLocalPosition(m_OffsetSpinePosition);
	Owner()->Transform()->SetLocalEulerAngles(m_OffsetSpineRotation);
}

void BoomerangStateManagerComponent::Throw(float dt)
{
	// ----- 飛翔処理 -----
	if (m_IsApproach)
	{
		// ターゲット位置
		Vector3 target = {0, 0, 0};
		if (m_IndexTargets >= m_Targets.size())
		{
			// プレイヤーに飛ぶ
			target = m_pPlayerObject->Transform()->Position();
		}
		else
		{
			// エネミーに飛ぶ
			target = m_Targets[m_IndexTargets]->Transform()->Position();
		}

		Vector3 vect = target - Owner()->Transform()->Position(); 
		float dist = vect.length();
		vect.normalize();
		// ----- 水平成分 -----
		// 水平成分のみ取り出す
		Vector3 vectHolizontal = Vector3(vect.x, 0, vect.z);
		Vector3 dirHolizontal  = Vector3(m_MoveDir.x, 0, m_MoveDir.z);
		vectHolizontal.normalize();
		dirHolizontal.normalize();
		// 水平成分の角度を求める
		float yawCur  = atan2f(dirHolizontal.x, dirHolizontal.z);
		float yawVect = atan2f(vectHolizontal.x, vectHolizontal.z);
		float yawDiff = WrapPi(yawVect - yawCur); // 差分
		// 最大旋回角にクランプ
		float maxTurn = m_IsApproach ? m_TurnRateApproach * dt : m_TurnRateFlight * dt; // １フレーム内の最大旋回角
		// 近距離は maxTurn を増加
		float addTurn = 90 * std::clamp((30.0f - dist) / 30.0f, 0.0f, 1.0f) * dt;
		maxTurn += addTurn;

		float yawStep = std::clamp(yawDiff, -maxTurn, maxTurn);
		float yawNew = yawCur + yawStep;

		// yawNew から水平方向を復元
		Vector3 dirNew = {sinf(yawNew), 0.0f, cosf(yawNew)};
		dirNew.normalize();

		// ----- 垂直成分 -----
		float diffVertical = target.y - Owner()->Transform()->Position().y;
		diffVertical *= 1.0f * dt;
		// 成分合成
		dirNew.y += diffVertical;
		dirNew.normalize();

		// 位置更新
		Vector3 position = Owner()->Transform()->Position();
		position += dirNew * (m_Speed * dt);
		Owner()->Transform()->SetPosition(position);

		// 進行方向更新
		m_MoveDir = dirNew;
	}
	else
	{
		Vector3 vect = m_MoveDir;
		// 位置更新（直進）
		Vector3 position = Owner()->Transform()->Position();
		position += vect * (m_Speed * dt);
		Owner()->Transform()->SetPosition(position);
		// 時間更新
		m_TimerFlight += dt;
		if (m_TimerFlight > 1.0f)
		{
			m_TimerFlight = 0.0f;
			m_IsApproach = true;
		}
	}
	
	//if (!m_IsApproach) m_TimerFlight += dt;
	//if (m_TimerFlight > 1.0f)
	//{
	//	m_TimerFlight = 0.0f;
	//	m_IsApproach = true;
	//}

	// ----- 回転処理 -----
	Vector3 lookPos = Owner()->Transform()->Position();
	lookPos += Vector3::Cross(m_MoveDir, { 0, 1, 0 }); // 進行方向の内側を向くようにする
	
	Owner()->Transform()->LookAt(lookPos, Owner()->Transform()->Up()); // 内側へ向ける
	Vector3 axis = Owner()->Transform()->Forward();
	Owner()->Transform()->RotateAxis(axis, -50.0f * dt); // 回転させる

}
