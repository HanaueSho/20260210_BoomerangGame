/*
	PlayerObejct.h
	20260201 hanaue sho
*/
#ifndef PLAYEROBJECT_H_
#define PLAYEROBJECT_H_

#include "gameObject.h"
#include "AnimatorComponent.h"
#include "AnimationClip.h"

class ModelAnimeObject;

class PlayerObject : public GameObject
{
private:
	// ƒAƒjƒŠÖŒW
	ModelAnimeObject* m_pModelAnimeObject = nullptr;

	float m_SpeedParam = 0.0f;

	// ˆÚ“®ŠÖŒW
	Vector3 m_ForwardVector = {0, 0, 1};
public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;

	int GetSign(float f) { if (f < 0) return -1; else return 1; }
};

#endif