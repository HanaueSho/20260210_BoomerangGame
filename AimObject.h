/*
	AimObject.h
	20260212  hanaue sho
	エイム中に敵に狙いをつけるオブジェクト
*/
#ifndef AIMOBJECT_H_
#define AIMOBJECT_H_
#include "GameObject.h"

class AimSpriteObject;

class AimObject : public GameObject
{
private:
	AimSpriteObject* m_pAimSprite = nullptr;

public:
	void Init() override;
	void Update(float gameDt, float realDt) override;

	AimSpriteObject* GetAimSprite() { return m_pAimSprite; }
};

#endif