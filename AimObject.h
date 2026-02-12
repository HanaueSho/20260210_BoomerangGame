/*
	AimObject.h
	20260212  hanaue sho
	エイム中に敵に狙いをつけるオブジェクト
*/
#ifndef AIMOBJECT_H_
#define AIMOBJECT_H_
#include "GameObject.h"

class AimObject : public GameObject
{
private:

public:
	void Init() override;
	void Update(float dt) override;

};

#endif