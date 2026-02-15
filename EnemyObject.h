/*
	EnemyObject.h
	20260212  hanaue sho
*/
#ifndef ENEMYOBJECT_H_
#define ENEMYOBJECT_H_
#include "GameObject.h"
#include "CharacterControllerComponent.h"

class EnemyModelAnimeObject;

class EnemyObject : public GameObject
{
private:
	EnemyModelAnimeObject* m_pModelAnimeObject = nullptr;

public:
	void Init() override;
	void Update(float dt) override;

	void SetSettings(int type);

private:
};

#endif