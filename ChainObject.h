/*
	ChainObject.h
	20260219  hanaue sho
*/
#ifndef CHAINOBJECT_H_
#define CHAINOBJECT_H_
#include "GameObject.h"

class ChainObject : public GameObject
{
private:
public:
	void Init() override;
	void Update(float gameDt, float realDt) override; 

	void CreateChains(int num);
};

#endif