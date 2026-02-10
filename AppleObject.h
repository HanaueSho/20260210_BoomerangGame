/*
	AppleObejct.h
	20260201 hanaue sho
*/
#ifndef APPLEOBJECT_H_
#define APPLEOBJECT_H_

#include "gameObject.h"

class AppleObject : public GameObject
{
private:

public:
	void Init() override;
	void Update(float dt) override;
};

#endif