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
	AnimationClip m_ClipIdle;
	AnimationClip m_ClipWalk;
	AnimationClip m_ClipRun;
	AnimationClip m_ClipJump;
	Skeleton m_Skeleton;
	AnimatorController* m_pController = nullptr;
	BoneManager* m_pBoneManager = nullptr;
	SkinMatrixProviderComponent* m_pProvider = nullptr;

	float m_SpeedParam = 0.0f;

public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;

	// ----- 外部から呼び出されるアニメ制御 -----
	void SetIsLocomotion(bool b);
	void SetBlendParam(float blend);
	void SetSpeedAnime(float speed);
	void PlayAnimeIdle();
	void PlayAnimeJump();


	// 骨のセット
	void SetupBones();

	// ゲッター（これらを作るとエラーが出る）
	const BoneManager& GetBoneManager() const { return *m_pBoneManager; }
	const AnimatorController& GetAnimatorController() const { return *m_pController; }
};

#endif