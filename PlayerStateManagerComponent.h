/*
	PlayerStateManagerComponent.h
	20260210  hanaue sho
	プレイヤーキャラクターのステートパターン
*/
#ifndef PLAYERSTATEMANAGERCOMPONENT_H_
#define PLAYERSTATEMANAGERCOMPONENT_H_
#include <cassert>
#include "Component.h"
#include "GameObject.h"
#include "AimStateManagerComponent.h"
#include "BoomerangStateManagerComponent.h"
#include "CharacterControllerComponent.h"
#include "PlayerStateInterface.h"
#include "PlayerStateIdle.h"
#include "PlayerStateMove.h"
#include "PlayerStateJump.h"
#include "PlayerStateAim.h"
#include "PlayerStateThrow.h"
#include "PlayerStateDamage.h"
#include "PlayerStateDead.h"
#include "PlayerStatePush.h"
#include "PlayerStateKick.h"
#include "PlayerStateEmote.h"
#include "PlayerStateDance.h"
#include "ModelAnimeObject.h"
#include "CameraFollowComponent.h"
#include "HealthComponent.h"
#include "PlayerObject.h"


enum class PlayerStateId
{
	Idle, // 待機
	Move, // 移動
	Jump, // ジャンプ
	Aim,  // 狙う
	Throw,  // 投げる
	Damage, // ダメージ
	Dead,  // 死亡
	Push,  // プッシュ
	Kick,  // キック
	Emote,  // エモート
	Dance,  // ダンス
};

class PlayerStateManagerComponent : public Component
{
private:
	// 現在のステート
	PlayerStateInterface* m_CurrentState = nullptr;
	PlayerStateId m_CurrentId = PlayerStateId::Idle;
	// State実体
	PlayerStateIdle m_StateIdle;
	PlayerStateMove m_StateMove;
	PlayerStateJump m_StateJump;
	PlayerStateAim  m_StateAim;
	PlayerStateThrow  m_StateThrow;
	PlayerStateDamage m_StateDamage;
	PlayerStateDead  m_StateDead;
	PlayerStatePush  m_StatePush;
	PlayerStateKick  m_StateKick;
	PlayerStateEmote  m_StateEmote;
	PlayerStateDance  m_StateDance;

	// カメラ
	GameObject* m_pCamera = nullptr;

	// アニメーションオブジェクトポインタ（外部参照）
	ModelAnimeObject* m_pModelAnime = nullptr;

	// コンポーネントポインタ
	CharacterControllerComponent* m_pController = nullptr;
	BoomerangStateManagerComponent* m_pBoomerang = nullptr;

	// 無敵関係
	bool m_IsInvinsible = false;
	float m_TimerInvinsible = 0.0f; // 無敵タイマー
	
public:
	// ----- ライフライクル -----
	void Init() override
	{
		m_pController = Owner()->GetComponent<CharacterControllerComponent>();
	}
	void Update(float dt) override
	{
		assert(m_CurrentState && "PlayerStateManagerComponent: m_CurrentState is null. Call SetStateInitial() before Update().");
		if (!m_CurrentState) return;
		m_CurrentState->Update(*this, dt);
		//DebugPrintfState();

		// ----- 無敵 -----
		ConsumeTimeInvinsible(dt);

		// ----- 死亡判定 -----
		if (Owner()->GetComponent<HealthComponent>()->CheckDead())
			ChangeState(PlayerStateId::Dead);

	}
	void FixedUpdate(float fixedDt) override
	{
		assert(m_CurrentState && "PlayerStateManagerComponent: m_CurrentState is null. Call SetStateInitial() before FixedUpdate().");
		if (!m_CurrentState) return;
		m_CurrentState->FixedUpdate(*this, fixedDt);
	}
	UpdateClock Clock() const noexcept override { return UpdateClock::Real; }

	// ----- 初期化セッター -----
	// ステートの初期化
	void SetStateInitial(PlayerStateId id)
	{
		if (m_CurrentState) return; // ２重初期化防止
		PlayerStateInterface* newState = ResolveStateId(id);
		SetStateInitial(newState);
	}
	void SetModelAnime(ModelAnimeObject* model)
	{
		m_pModelAnime = model;
	}
	void SetCameraObject(GameObject* camera)
	{
		m_pCamera = camera;
	}
	void SetBoomerangObject(GameObject* boomerang)
	{
		m_pBoomerang = boomerang->GetComponent<BoomerangStateManagerComponent>();
	}

	// ----- ステートの切り替え -----
	void ChangeState(PlayerStateId id)
	{
		PlayerStateInterface* newState = ResolveStateId(id);
		bool success = ChangeState(newState);
		if (success) SetStateId(id);
	}
	void ChangeStateDamage() // ダメージ
	{
		if (m_IsInvinsible) return; // 無敵は戻る
		if (m_CurrentId == PlayerStateId::Dead) return; // 既に死んでいたら戻る
		ChangeState(PlayerStateId::Damage);
		// ヘルス
		dynamic_cast<PlayerObject*>(Owner())->TakeDamage();
	}

	// ----- セッター -----
	void SetInvinsible(bool b) { m_IsInvinsible = b; }

	// ----- ゲッター -----
	CharacterControllerComponent* GetCC() { return m_pController; }
	ModelAnimeObject* GetModelAnime() { return m_pModelAnime; }
	BoomerangStateManagerComponent* GetBoomerang() { return m_pBoomerang; }
	const Vector3& GetCameraForward() const
	{
		return m_pCamera->Transform()->Forward();
	}
	const Vector3& GetCameraRight() const
	{
		return m_pCamera->Transform()->Right();
	}
	PlayerStateId GetStateId() { return m_CurrentId; }

	// カメラ制御
	void SetCameraStateFollow()
	{
		if (!m_pCamera) return;
		auto* follow = m_pCamera->GetComponent<CameraFollowComponent>();
		follow->SetStateFollow();
	}
	void SetCameraStateAim()
	{
		if (!m_pCamera) return;
		auto* follow = m_pCamera->GetComponent<CameraFollowComponent>();
		follow->SetStateAim();
	}

private:
	// ----- StateChange 関係 -----
	PlayerStateInterface* ResolveStateId(PlayerStateId id)
	{
		switch (id)
		{
		case PlayerStateId::Idle:
			return &m_StateIdle;
		case PlayerStateId::Move:
			return &m_StateMove;
		case PlayerStateId::Jump:
			return &m_StateJump;
		case PlayerStateId::Aim:
			return &m_StateAim;
		case PlayerStateId::Throw:
			return &m_StateThrow;
		case PlayerStateId::Damage:
			return &m_StateDamage;
		case PlayerStateId::Dead:
			return &m_StateDead;
		case PlayerStateId::Push:
			return &m_StatePush;
		case PlayerStateId::Kick:
			return &m_StateKick;
		case PlayerStateId::Emote:
			return &m_StateEmote;
		case PlayerStateId::Dance:
			return &m_StateDance;

		default:
			assert(false && "Unknown PlayerStateId");
			return nullptr;
		}
	}
	void SetStateInitial(PlayerStateInterface* state)
	{
		assert(state && "PlayerStateManagerComponent: initial state is null.");
		m_CurrentState = state;
		m_CurrentState->Enter(*this);
	}
	bool ChangeState(PlayerStateInterface* newState)
	{
		if (!newState) return false;
		if (m_CurrentState == newState) return false;

		if (m_CurrentState)
			m_CurrentState->Exit(*this); // 終了処理
		m_CurrentState = newState;
		m_CurrentState->Enter(*this); // 開始処理
		return true;
	}
	void SetStateId(PlayerStateId id)
	{
		m_CurrentId = id;
	}
	void DebugPrintfState()
	{
		switch (m_CurrentId)
		{
		case PlayerStateId::Idle:
			printf("[PlayerState]: Idle\n");
			break;
		case PlayerStateId::Move:
			printf("[PlayerState]: Move\n");
			break;
		case PlayerStateId::Jump:
			printf("[PlayerState]: Jump\n");
			break;
		case PlayerStateId::Aim:
			printf("[PlayerState]: Aim\n");
			break;
		case PlayerStateId::Throw:
			printf("[PlayerState]: Throw\n");
			break;
		case PlayerStateId::Damage:
			printf("[PlayerState]: Damage\n");
			break;
		case PlayerStateId::Dead:
			printf("[PlayerState]: Dead\n");
			break;
		case PlayerStateId::Push:
			printf("[PlayerState]: Push\n");
			break;
		case PlayerStateId::Kick:
			printf("[PlayerState]: Kick\n");
			break;
		case PlayerStateId::Emote:
			printf("[PlayerState]: Emote\n");
			break;
		case PlayerStateId::Dance:
			printf("[PlayerState]: Dance\n");
			break;
		default:
			printf("[PlayerState]: NoState\n");
			return;
		}
	}

	// ----- 無敵時間消費 -----
	void ConsumeTimeInvinsible(float gameDt)
	{
		if (!m_IsInvinsible) return;
		m_TimerInvinsible += gameDt;
		if (m_TimerInvinsible > 5.0f)
		{
			m_TimerInvinsible = 0.0f;
			m_IsInvinsible = false; // 無敵解除
		}
	}

};

#endif