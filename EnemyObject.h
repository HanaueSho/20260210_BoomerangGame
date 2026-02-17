/*
	EnemyObject.h
	20260212  hanaue sho
*/
#ifndef ENEMYOBJECT_H_
#define ENEMYOBJECT_H_
#include "GameObject.h"
#include "CharacterControllerComponent.h"
#include "EnemyModelAnimeObject.h"

class EnemyObject : public GameObject
{
private:
	EnemyModelAnimeObject* m_pModelAnimeObject = nullptr;
	Type m_Type = Type::Melee;

	GameObject* m_pTargetObject = nullptr;

public:
	void Init() override;
	void Update(float gameDt, float realDt) override;
	void Uninit() override;

	EnemyModelAnimeObject* GetModelAnime() const { return m_pModelAnimeObject; }
	void SetType(Type type) { m_Type = type; }
};

#endif