/*
	GameManagerObject.h
	20260217  hanaue sho
*/
#ifndef GAMEMANAGEROBJECT_H_
#define GAMEMANAGEROBJECT_H_
#include "GameObject.h"

class GameManagerObject : public GameObject
{
public:
	enum class State
	{
		None,
		Active,
		Success, // ゲームクリア
		Failed,  // ゲームオーバー
	};

private:
	State m_State = State::None;
	GameObject* m_pPlayer = nullptr;
	GameObject* m_pSpriteStart = nullptr;   // スタート表示
	GameObject* m_pSpriteSuccess = nullptr; // クリア表示
	GameObject* m_pSpriteFailed = nullptr;  // ゲームオーバー表示

public:
	void Init() override;
	void Update(float gameDt, float realDt);

	void ChangeState(State newState);

};

#endif