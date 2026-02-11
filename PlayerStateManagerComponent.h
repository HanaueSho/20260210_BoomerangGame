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
#include "CharacterControllerComponent.h"
#include "PlayerStateInterface.h"
#include "PlayerStateIdle.h"
#include "PlayerStateMove.h"
#include "PlayerStateJump.h"
#include "PlayerStateAim.h"
#include "PlayerStateThrow.h"
#include "ModelAnimeObject.h"
#include "CameraFollowComponent.h"


enum class PlayerStateId
{
	Idle, // 待機
	Move, // 移動
	Jump, // ジャンプ
	Aim,  // 狙う
	Throw,  // 投げる
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

	// カメラ
	GameObject* m_pCamera = nullptr;

	// アニメーションオブジェクトポインタ（外部参照）
	ModelAnimeObject* m_pModelAnime = nullptr;

	// コンポーネントポインタ
	CharacterControllerComponent* m_pController = nullptr;
	
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
		DebugPrintfState();
	}
	void FixedUpdate(float fixedDt) override
	{
		assert(m_CurrentState && "PlayerStateManagerComponent: m_CurrentState is null. Call SetStateInitial() before FixedUpdate().");
		if (!m_CurrentState) return;
		m_CurrentState->FixedUpdate(*this, fixedDt);
	}

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


	// ----- ステートの切り替え -----
	void ChangeState(PlayerStateId id)
	{
		PlayerStateInterface* newState = ResolveStateId(id);
		bool success = ChangeState(newState);
		if (success) SetStateId(id);
	}

	// ----- ゲッター -----
	CharacterControllerComponent* GetCC() { return m_pController; }
	ModelAnimeObject* GetModelAnime() { return m_pModelAnime; }
	const Vector3& GetCameraForward() const
	{
		return m_pCamera->Transform()->Forward();
	}
	const Vector3& GetCameraRight() const
	{
		return m_pCamera->Transform()->Right();
	}

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
		default:
			printf("[PlayerState]: NoState\n");
			return;
		}
	}
};

#endif