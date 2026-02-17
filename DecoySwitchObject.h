/*
	DecoySwitchObject.h
	20260217  hanaue sho
*/
#ifndef DECOYSWITCHOBJECT_H_
#define DECOYSWITCHOBJECT_H_
#include "GameObject.h"

class DecoySwitchObject : public GameObject
{
private:

public:
	void Init() override;
	void Update(float gameDt, float realDt) override;

};

#endif