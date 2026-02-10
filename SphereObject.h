/*
	SphereObejct.h
	20260201 hanaue sho
*/
#ifndef SPHEREOBJECT_H_
#define SPHEREOBJECT_H_
#include "gameObject.h"

class SphereObject : public GameObject
{
private:

public:
	void Init() override;
	void Update(float dt) override;
};

#endif