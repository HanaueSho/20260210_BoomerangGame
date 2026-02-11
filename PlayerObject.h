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

	// ‚¢‚ç‚È‚¢‚à‚Ì«««««
	float m_SpeedParam = 0.0f; // ‚à‚¤‚¢‚ç‚È‚¢

	// ˆÚ“®ŠÖŒW
	Vector3 m_ForwardVector = {0, 0, 1};
public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;

	GameObject* GetBoneObject(int index);
};

#endif