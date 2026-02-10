/*
	ModelAnimeObject.h
	20260208  hanaue sho
*/
#ifndef MODELANIMEOBJECT_H_
#define MODELANIMEOBJECT_H_
#include "GameObject.h"
#include "AnimatorComponent.h"
#include "AnimationClip.h"
class BoneManager;
class SkinMatrixProviderComponent;
class AnimatorController;

class ModelAnimeObject : public GameObject
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

	// 骨のセット
	void SetupBones();

	// ゲッター（これらを作るとエラーが出る）
	const BoneManager& GetBoneManager() const { return *m_pBoneManager; }
	const AnimatorController& GetAnimatorController() const { return *m_pController; }

};

#endif