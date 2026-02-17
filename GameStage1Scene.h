/*
	GameStage1Scene.h
	20260217  hanaue sho
*/
#ifndef GAMESTAGE1SCENE_H_
#define GAMESTAGE1SCENE_H_
#include "Scene.h"

class GameStage1Scene : public Scene
{
private:
	bool m_IsFadeChangeScene = false;


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