/*
	CubeObejct.h
	20260201 hanaue sho
*/
#ifndef CUBEOBJECT_H_
#define CUBEOBJECT_H_
#include "gameObject.h"

class CubeObject : public GameObject
{
private:

public:
	void Init() override;
	void Update(float dt) override;
};

#endif