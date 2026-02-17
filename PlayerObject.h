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
class HealthSpriteObject;

class PlayerObject : public GameObject
{
private:
	// ƒAƒjƒŠÖŒW
	ModelAnimeObject* m_pModelAnimeObject = nullptr;

	// ˆÚ“®ŠÖŒW
	Vector3 m_ForwardVector = {0, 0, 1};

	// ‘Ì—ÍŠÖŒW
	HealthSpriteObject* m_pHealthSprite[6]{};
	int m_IndexHealth = 5;

public:
	void Init() override;
	void Uninit() override;
	void Update(float gameDt, float realDt) override;

	void TakeDamage();

	GameObject* GetBoneObject(int index);
};

#endif