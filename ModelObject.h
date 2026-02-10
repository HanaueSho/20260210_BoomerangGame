/*
	ModelObject.h
	20251224  hanaue sho
*/
#ifndef MODELOBJECT_H_
#define MODELOBJECT_H_
#include "GameObject.h"
#include "AnimatorComponent.h"
#include "AnimationClip.h"

class AnimatorController;
class BoneManager;
class SkinMatrixProviderComponent;

class ModelObject : public GameObject
{
private:
	AnimationClip m_Clip;
	AnimationClip m_Clip1;
	AnimationClip m_Clip2;
	Skeleton m_Skeleton;
	AnimatorController* m_pController = nullptr;
	BoneManager* m_pBoneManager = nullptr;
	SkinMatrixProviderComponent* m_pProvider = nullptr;

	float m_SpeedParam = 0.0f;

public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;

	//void SetupBones();
};

#endif