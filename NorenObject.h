/*
	NorenObejct.h
	20260201 hanaue sho
*/
#ifndef NORENOBJECT_H_
#define NORENOBJECT_H_

#include "gameObject.h"
#include "AnimatorComponent.h"
#include "AnimationClip.h"

class AnimatorController;
class BoneManager;
class SkinMatrixProviderComponent;

class NorenObject : public GameObject
{
private:
	AnimationClip m_Clip;
	Skeleton m_Skeleton;
	BoneManager* m_pBoneManager = nullptr;
	SkinMatrixProviderComponent* m_pProvider = nullptr;

	float m_SpeedParam = 0.0f;

public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;

	void SetupBones();
};

#endif