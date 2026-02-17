/*
	GameStage1Scene.h
	20260217  hanaue sho
*/
#ifndef GAMESTAGE1SCENE_H_
#define GAMESTAGE1SCENE_H_
#include "Scene.h"

class ResultSpriteObject;

class GameStage1Scene : public Scene
{
public:
	enum class State
	{
		None,
		GameClear,
		GameOver,
	};
private:
	State m_State = State::None;
	bool m_IsFadeChangeScene = false;

	// リザルト表示
	ResultSpriteObject* m_pResultSprite = nullptr;

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