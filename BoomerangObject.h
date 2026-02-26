/*
	BoomerangObject.h
	20260211  hanaue sho
	ブーメランのオブジェクト
*/
#ifndef BOOMERANGOBJECT_H_
#define BOOMERANGOBJECT_H_
#include "GameObject.h"
#include "ModelLoader.h"
#include "AudioSource.h"

class BoomerangObject : public GameObject
{
private:
	// オーディオ関係
	AudioSource* m_pAudioFlight = nullptr;
	AudioSource* m_pAudioHit = nullptr;

public:
	void Init() override;
	void Update(float gameDt, float realDt) override;

	AudioSource* GetAudioFlight() { return m_pAudioFlight; }
	AudioSource* GetAudioHit() { return m_pAudioHit; }
};


#endif