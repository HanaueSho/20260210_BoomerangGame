/*
	EnemyObject.h
	20260212  hanaue sho
*/
#ifndef ENEMYOBJECT_H_
#define ENEMYOBJECT_H_
#include "GameObject.h"
#include "CharacterControllerComponent.h"
#include "EnemyModelAnimeObject.h"
#include "AudioSource.h"

class EnemyObject : public GameObject
{
private:
	EnemyModelAnimeObject* m_pModelAnimeObject = nullptr;
	Type m_Type = Type::Melee;

	GameObject* m_pTargetObject = nullptr;

	// オーディオ関係
	AudioSource* m_pAudioAttack = nullptr;
	AudioSource* m_pAudioReady = nullptr;
	AudioSource* m_pAudioShot = nullptr;

public:
	void Init() override;
	void Update(float gameDt, float realDt) override;
	void Uninit() override;

	EnemyModelAnimeObject* GetModelAnime() const { return m_pModelAnimeObject; }
	void SetType(Type type) { m_Type = type; }

	AudioSource* GetAudioAttack() { return m_pAudioAttack; }
	AudioSource* GetAudioReady() { return m_pAudioReady; }
	AudioSource* GetAudioShot() { return m_pAudioShot; }
};

#endif