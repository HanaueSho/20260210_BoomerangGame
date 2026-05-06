/*
	OperateSpriteObject.h
	20260217  hanaue sho
*/
#ifndef OPERATESPRITEOBJECT_H_
#define OPERATESPRITEOBJECT_H_
#include "GameObject.h"

class OperateSpriteObject : public GameObject
{
public:
	enum class Type
	{
		Movement,
		Throw,
		Stage1,
		Stage2,
		Ybutton
	};
private:
	float m_MaxDistance = 100.0f;
	float m_Alpha = 1.0f;
	Type m_Type = Type::Movement;

public:
	void Init() override;
	void Update(float gameDt, float realDt);

	void SetTypeTexture(Type type);
};

#endif