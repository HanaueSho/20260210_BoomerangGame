/*
	TargetObject.h
	20260214  hanaue sho
*/
#ifndef TARGETOBJECT_H_
#define TARGETOBJECT_H_
#include "GameObject.h"

class TargetSpriteObject;
class MarkSpriteObject;

class TargetObject : public GameObject
{
private:
	TargetSpriteObject* m_pTargetSpriteObject = nullptr;
	MarkSpriteObject* m_pMarkSpriteObject = nullptr;
public:
	void Init() override;
	void Uninit() override;

	TargetSpriteObject* GetTargetSprite() { return m_pTargetSpriteObject; }
	MarkSpriteObject*   GetMarkSprite()   { return m_pMarkSpriteObject; }
};

#endif