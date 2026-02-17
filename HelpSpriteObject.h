/*
	HelpSpriteObject.h
	20260218  hanaue sho
*/
#ifndef HelpSpriteObject_H_
#define HelpSpriteObject_H_
#include "GameObject.h"

class HelpSpriteObject : public GameObject
{
private:
	float m_Alpha = 1.0f;

public:
	void Init() override;
	void Update(float gameDt, float realDt) override;
};


#endif