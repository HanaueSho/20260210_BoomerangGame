/*
	GameMainScene.h
	20260211  hanaue sho
*/
#ifndef GAMEMAINSCENE_H_
#define GAMEMAINSCENE_H_
#include "Scene.h"

class EnemyObject;

class GameMainScene : public Scene
{
private:
	EnemyObject* m_pDecoies[5];

public:
	void Init() override;
	void Uninit() override;
	void Update(float gameDt, float realDt) override;
	void Draw() override;

	void ResetDecoies() 
	{
		DestroyDecoies();
		CreateDecoies();
	}

private:
	void CreateSignboards();
	void CreateDecoies();
	void DestroyDecoies();
	void CreateFences();
	void CreateTents();
	void CreateTrees();
	void CreateApples();
	void CreateBoxes();
	void CreateRockes();
	void CreateLights();
	void CreateChains();
	void CreateNorens();
};

#endif