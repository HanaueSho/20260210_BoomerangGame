/*
	EnemyStateMaanger.cpp
	20260215  hanaue sho
*/
#include "EnemyStateManagerComponent.h"
#include "GameObject.h"
#include "CharacterControllerComponent.h"
#include "AnimatorComponent.h"
#include "Manager.h"
#include "Scene.h"
#include "PlayerObject.h"
#include "ColliderComponent.h"
#include "BulletObject.h"
#include "BulletStateManagerComponent.h"
#include "TargetObject.h"
#include "MarkSpriteObject.h"
#include "TargetSpriteObject.h"
#include "DamageComponent.h"

void EnemyStateManagerComponent::Init()
{
	m_pController = Owner()->GetComponent<CharacterControllerComponent>();
	m_pPlayerObject = Manager::GetScene()->GetGameObject<PlayerObject>();
}

void EnemyStateManagerComponent::Update(float dt)
{
	switch (m_State)
	{
	case State::Idle:
		ChaseToPlayer(dt); // プレイヤーを追う
		break;
	case State::Ready:
		Ready();
		break;
	case State::Attack:
		Attack(dt);
		break;
	case State::Dead:
		Dead();
		break;
	}
}

void EnemyStateManagerComponent::FixedUpdate(float fixedDt)
{
}

void EnemyStateManagerComponent::SetSettings(int type)
{
	auto* cc = Owner()->GetComponent<CharacterControllerComponent>();
	BuildSettings(type);
	cc->SetSettings(m_Settings);
}

void EnemyStateManagerComponent::BuildSettings(int type)
{
	CharacterControllerComponent::Settings s{};
	switch (type)
	{
	case 0: // 通常移動
		s.maxSpeed = 30.0f;
		s.acceleration = 100.0f;
		s.deceleration = 200.0f;
		s.turnAcceleration = 100.0f;
		break;
	case 1: // ゆっくり
		s.maxSpeed = 1.0f;
		s.acceleration = 100.0f;
		s.deceleration = 200.0f;
		s.turnAcceleration = 100.0f;
		break;
	case 2: // 攻撃
		s.maxSpeed = 100.0f;
		s.acceleration = 400.0f;
		s.deceleration = 200.0f;
		s.turnAcceleration = 100.0f;
		break;
	case 3: // その場から動かない
		s.maxSpeed = 0.0f;
		s.acceleration = 400.0f;
		s.deceleration = 200.0f;
		s.turnAcceleration = 100.0f;
		break;
	}
	m_Settings = s;
}

void EnemyStateManagerComponent::Damage()
{

}


void EnemyStateManagerComponent::ChangeState(State newState)
{
	// 終了処理
	switch (m_State)
	{
	case State::Idle:
		break;
	case State::Ready:
		break;
	case State::Attack:
		if (m_ModelAnimeType == Type::Melee)
		{
			// 攻撃判定無効化
			m_pModelAnime->GetAttackObject()->GetComponent<DamageComponent>()->SetActive(false);
		}
		break;
	case State::Dead:
		break;
	}

	m_State = newState;
	// 初期化処理
	switch (m_State)
	{
	case State::Idle:
		if (m_ModelAnimeType == Type::Melee)
		{
			m_pModelAnime->PlayAnimeIdle();
			m_pModelAnime->SetSpeedAnime(1.0f);
			SetSettings(0);
		}
		else if (m_ModelAnimeType == Type::Shot)
		{
			m_pModelAnime->PlayAnimeIdle();
			m_pModelAnime->SetSpeedAnime(1.5f);
			SetSettings(3);
		}
		break;
	case State::Ready:
		if (m_ModelAnimeType == Type::Melee)
		{
			m_pModelAnime->PlayAnimeReady();
			m_pModelAnime->SetSpeedAnime(0.5f);
			SetSettings(1);
		}
		else if (m_ModelAnimeType == Type::Shot)
		{

		}
		break;
	case State::Attack:
	{
		if (m_ModelAnimeType == Type::Melee)
		{
			m_pModelAnime->PlayAnimeAttack();
			m_pModelAnime->SetSpeedAnime(3.0f);
			// ベクトル保存
			Vector3 vect = m_pPlayerObject->Transform()->Position() - Owner()->Transform()->Position();
			m_AttackVect = vect.normalized();
			SetSettings(2);

			// 攻撃判定有効化
			m_pModelAnime->GetAttackObject()->GetComponent<DamageComponent>()->SetActive(true);
		}
		else if (m_ModelAnimeType == Type::Shot)
		{
			m_pModelAnime->PlayAnimeAttack();
			m_pModelAnime->SetSpeedAnime(2.0f);
			// 攻撃生成
			m_ShotTuneTimer = 0.0f;
			m_IsShot = false;
		}
	}
		break;
	case State::Dead:
		m_pModelAnime->PlayAnimeDead();
		m_pModelAnime->SetSpeedAnime(1.5f);
		
		// CC 削除
		Owner()->RemoveComponent(m_pController);
		m_pController = nullptr;
		// コライダー設定
		Collider* col = Owner()->GetComponent<Collider>();
		col->SetBox({ 1.0f, 1.0f, 1.0f });
		col->SetModeSimulate();
		col->SetOffsetPosition({ 0, 5, 0.0f });
		Rigidbody* rigid = Owner()->GetComponent<Rigidbody>();
		rigid->SetBodyTypeDynamic();

		// タグ変更
		m_pTargetObject->SetTag("TargetDead");

		// UI 消去
		dynamic_cast<TargetObject*>(m_pTargetObject)->GetMarkSprite()->RequestDestroy();
		dynamic_cast<TargetObject*>(m_pTargetObject)->GetTargetSprite()->RequestDestroy();

		break;
	}
}

void EnemyStateManagerComponent::ChaseToPlayer(float dt)
{
	Vector3 vect = m_pPlayerObject->Transform()->Position() - Owner()->Transform()->Position();
	m_pController->SetMoveInput(vect.normalized());

	switch (m_ModelAnimeType)
	{
	case Type::Decoy:
		break;
	case Type::Melee:
		// 近づいたら攻撃準備
		if (vect.length() < 100.0f)
		{
			ChangeState(State::Ready);
		}
		break;
	case Type::Shot:
		// 一定間隔で攻撃
		m_ShotTimer += dt;
		if (m_ShotTimer > 3.0f)
		{
			m_ShotTimer = 0.0f;
			ChangeState(State::Attack);
		}
		break;
	case Type::ShotAndBarrier:
		break;
	}


}

void EnemyStateManagerComponent::Attack(float dt)
{
	switch (m_ModelAnimeType)
	{
	case Type::Decoy:
		break;
	case Type::Melee:
		// 直線移動
		m_pController->SetMoveInput(m_AttackVect);

		// 時間経過
		m_AttackTimer += dt;
		if (m_AttackTimer > 2.0f)
		{
			m_AttackTimer = 0.0f;
			ChangeState(State::Idle);
		}
		break;
	case Type::Shot:
	{
		// 向き
		Vector3 vect = m_pPlayerObject->Transform()->Position() - Owner()->Transform()->Position();
		m_pController->SetMoveInput(vect.normalized());
		m_ShotTuneTimer += dt;

		// 攻撃生成
		if (m_ShotTuneTimer > 0.6f && m_IsShot == false)
		{
			Vector3 vect = m_pPlayerObject->Transform()->Position() - Owner()->Transform()->Position();
			auto* bullet = Manager::GetScene()->AddGameObject<BulletObject>(1);
			bullet->Init();
			bullet->Transform()->SetPosition(Owner()->Transform()->Position());
			bullet->GetComponent<BulletStateManagerComponent>()->SetVect(vect.normalized());

			m_IsShot = true;
		}
		// 遷移
		auto animator = m_pModelAnime->GetComponent<AnimatorComponent>();
		if (!animator->IsBlending())
		{
			ChangeState(State::Idle);
		}
	}
		break;
	case Type::ShotAndBarrier:
		break;
	}
}

void EnemyStateManagerComponent::Ready()
{
	Vector3 vect = m_pPlayerObject->Transform()->Position() - Owner()->Transform()->Position();
	m_pController->SetMoveInput(vect.normalized()); 


	auto animator = m_pModelAnime->GetComponent<AnimatorComponent>();
	if (!animator->IsBlending())
		ChangeState(State::Attack);

}

void EnemyStateManagerComponent::Dead()
{
}
