/*
	GameStage0Scene.h
	20260215  hanaue sho
*/
#ifndef GAMESTAGE0SCENE_H_
#define GAMESTAGE0SCENE_H_
#include "Scene.h"

class ResultSpriteObject;
class EnemyObject;

class GameStage0Scene : public Scene
{
public:
	enum class State
	{
		First,
		Second,
		Third,
		GameClear,
		GameOver,
	};

private:
	State m_State = State::First;
	// フェード処理
	bool m_IsFadeChangeScene = false;

	// スポーン関係
	float m_SpawnTimer = 0.0f;

	// リザルト表示
	ResultSpriteObject* m_pResultSprite = nullptr;

	// エネミー管理
	EnemyObject* m_pEnemyMeleeFirst = nullptr;
	EnemyObject* m_pEnemyShotSecond = nullptr;
	EnemyObject* m_pEnemyMeleeThird[3];
	EnemyObject* m_pEnemyShotThird[3];

	// クリアのマージ
	float m_ClearTimer = 0.0f;

public:
	void Init() override;
	void Uninit() override;
	void Update(float gameDt, float realDt) override;
	void Draw() override;

	void ChangeState(State state);

private:
	void CreateFences();
	void CreateTents();
	void CreateTrees();
};

#endif