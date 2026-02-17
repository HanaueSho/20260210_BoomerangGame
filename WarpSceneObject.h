/*
	WarpSceneObject.h
	20260217  hanaue sho
*/
#ifndef WARPSCENEOBJECT_H_
#define WARPSCENEOBJECT_H_
#include "GameObject.h"

class WarpSceneObject : public GameObject
{
private:

public:
	void Init() override;
	void Update(float gameDt, float realDt) override; 

};

#endif