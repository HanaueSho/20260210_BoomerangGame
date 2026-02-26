/*
	PlayerObejct.h
	20260201 hanaue sho
*/
#ifndef PLAYEROBJECT_H_
#define PLAYEROBJECT_H_

#include "gameObject.h"
#include "AnimatorComponent.h"
#include "AnimationClip.h"
#include "AudioSource.h"

class ModelAnimeObject;
class HealthSpriteObject;

class PlayerObject : public GameObject
{
private:
	// アニメ関係
	ModelAnimeObject* m_pModelAnimeObject = nullptr;

	// 移動関係
	Vector3 m_ForwardVector = {0, 0, 1};

	// 体力関係
	HealthSpriteObject* m_pHealthSprite[6]{};
	int m_IndexHealth = 5;

	// オーディオ関係
	AudioSource* m_pAudioMove = nullptr;
	AudioSource* m_pAudioJump = nullptr;
	AudioSource* m_pAudioJumpAir = nullptr;
	AudioSource* m_pAudioGround = nullptr;
	AudioSource* m_pAudioAim = nullptr;
	AudioSource* m_pAudioThrow = nullptr;
	AudioSource* m_pAudioDamage = nullptr;
	AudioSource* m_pAudioDead = nullptr;
	AudioSource* m_pAudioKick = nullptr;
	AudioSource* m_pAudioPush = nullptr;

public:
	void Init() override;
	void Uninit() override;
	void Update(float gameDt, float realDt) override;

	void TakeDamage();

	GameObject* GetBoneObject(int index);
	AudioSource* GetAudioMove() { return m_pAudioMove; }
	AudioSource* GetAudioJump() { return m_pAudioJump; }
	AudioSource* GetAudioJumpAir() { return m_pAudioJumpAir; }
	AudioSource* GetAudioGround() { return m_pAudioGround; }
	AudioSource* GetAudioAim() { return m_pAudioAim; }
	AudioSource* GetAudioThrow() { return m_pAudioThrow; }
	AudioSource* GetAudioDamage() { return m_pAudioDamage; }
	AudioSource* GetAudioDead() { return m_pAudioDead; }
	AudioSource* GetAudioKick() { return m_pAudioKick; }
	AudioSource* GetAudioPush() { return m_pAudioPush; }

};

#endif