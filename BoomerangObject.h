/*
	BoomerangObject.h
	20260211  hanaue sho
	ブーメランのオブジェクト
*/
#ifndef BOOMERANGOBJECT_H_
#define BOOMERANGOBJECT_H_
#include "GameObject.h"
#include "ModelLoader.h"

class BoomerangObject : public GameObject
{
public:
	void Init() override;
	void Update(float dt) override;

};


#endif