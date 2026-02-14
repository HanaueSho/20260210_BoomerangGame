/*
	EnemyModelAnimeObject.h
	20260214  hanaue sho
*/
#ifndef ENEMYMODELANIMEOBJECT_H_
#define ENEMYMODELANIMEOBJECT_H_
#include "GameObject.h"
#include "AnimatorComponent.h"
#include "AnimationClip.h"
class BoneManager;
class SkinMatrixProviderComponent;
class AnimatorController;

enum class Type
{
	Melee, // 近接
	Shot,  // 遠距離
	ShotAndBarrier, // バリア―
	Barrier, // バリア―
};

class EnemyModelAnimeObject : public GameObject
{
private:
	// Clip
	AnimationClip m_ClipIdle;
	AnimationClip m_ClipReady;
	AnimationClip m_ClipAttack;
	AnimationClip m_ClipDead;
	Skeleton m_Skeleton;
	AnimatorController* m_pController = nullptr;
	BoneManager* m_pBoneManager = nullptr;
	SkinMatrixProviderComponent* m_pProvider = nullptr;

	float m_SpeedParam = 0.0f;

	// Type
	Type m_Type = Type::Barrier;

public:
	void Init() override;
	void Uninit() override;
	void Update(float dt) override;

	// ----- 外部から呼び出されるアニメ制御 -----
	void SetIsLocomotion(bool b);
	void SetBlendParam(float blend);
	void SetSpeedAnime(float speed);
	void PlayAnimeIdle();
	void PlayAnimeReady();
	void PlayAnimeAttack();
	void PlayAnimeDead();

	// 骨のセット
	void SetupBones();

	// ゲッター（これらを作るとエラーが出る）
	const BoneManager& GetBoneManager() const { return *m_pBoneManager; }
	const AnimatorController& GetAnimatorController() const { return *m_pController; }
};

#endif