/*
	GameStage0Scene.h
	20260215  hanaue sho
*/
#ifndef GAMESTAGE0SCENE_H_
#define GAMESTAGE0SCENE_H_
#include "Scene.h"

class GameStage0Scene : public Scene
{
private:
	bool m_IsFadeChangeScene = false;

	// ÉXÉ|Å[Éìä÷åW
	float m_SpawnTimer = 0.0f;

public:
	void Init() override;
	void Uninit() override;
	void Update(float gameDt, float realDt) override;
	void Draw() override;

private:
	void CreateFences();
	void CreateTents();
	void CreateTrees();
};

#endif