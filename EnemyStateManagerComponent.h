/*
	EnemyStateMaanger.h
	20260215  hanaue sho
*/
#ifndef ENEMYSTATEMANAGERCOMPONENT_H_
#define ENEMYSTATEMANAGERCOMPONENT_H_
#include "Component.h"
#include "ModelAnimeObject.h"
#include "EnemyModelAnimeObject.h"
#include "CharacterControllerComponent.h"

class GameObject;

class EnemyStateManagerComponent : public Component
{
public:
	enum class State
	{
		Idle,
		Ready,
		Attack,
		Dead
	};
private:
	// ----- state -----
	State m_State = State::Idle;

	// ターゲットオブジェクト
	GameObject* m_pTargetObject = nullptr;

	// プレイヤーオブジェクト
	GameObject* m_pPlayerObject = nullptr;

	// アニメーションオブジェクトポインタ（外部参照）
	EnemyModelAnimeObject* m_pModelAnime = nullptr;
	Type m_ModelAnimeType = Type::Melee;

	// コンポーネントポインタ
	CharacterControllerComponent* m_pController = nullptr;
	CharacterControllerComponent::Settings m_Settings;

	// ----- Melee -----
	Vector3 m_AttackVect = {};
	float m_AttackTimer = 0.0f;

	// ----- Shot -----
	float m_ShotTimer = 0.0f;

public:
	// ライフサイクル
	void Init() override;
	void Update(float dt) override;
	void FixedUpdate(float fixedDt) override;

	// State チェンジ
	void ChangeState(State newState);
	void ChangeStateIdle() { ChangeState(State::Idle); }
	void ChangeStateReady() { ChangeState(State::Ready); }
	void ChangeStateAttack() { ChangeState(State::Attack); }
	void ChangeStateDead() { ChangeState(State::Dead); }

	// セッター
	void SetModelAnime(EnemyModelAnimeObject* model)
	{
		m_pModelAnime = model;
		m_ModelAnimeType = model->GetType();
	}

	// CharacterControllerComponent
	void SetSettings(int type);
	void BuildSettings(int type);

	// ダメージ処理
	void Damage();

private:
	void ChaseToPlayer(float dt);
	void Attack(float dt);
	void Ready();
	void Dead();

};

#endif