/*
	GameMainScene.h
	20260211  hanaue sho
*/
#ifndef GAMEMAINSCENE_H_
#define GAMEMAINSCENE_H_
#include "Scene.h"

class GameMainScene : public Scene
{
private:

public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;
	void Draw() override;

};

#endif