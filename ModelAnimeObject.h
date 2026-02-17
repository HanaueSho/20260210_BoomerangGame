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
	AnimationClip m_ClipJumpAir;
	AnimationClip m_ClipAim;
	AnimationClip m_ClipThrow;
	AnimationClip m_ClipDamage;
	AnimationClip m_ClipDead;
	AnimationClip m_ClipPush;
	AnimationClip m_ClipKick;
	Skeleton m_Skeleton;
	AnimatorController* m_pController = nullptr;
	BoneManager* m_pBoneManager = nullptr;
	SkinMatrixProviderComponent* m_pProvider = nullptr;

	float m_SpeedParam = 0.0f; // アニメーターの再生速度パラム

	// Blink 処理
	bool  m_IsBlink = false;
	float m_BlinkTimer = 0.0f; // 合計時間タイマー
	float m_BlinkInterval = 0.0f; // インターバル
	float m_BlinkIntervalTimer = 0.0f; // インターバルタイマー
	float m_BlinkAdditionalTime = 0.02f; // 毎回追加分
	bool  m_IsBlinkRed = false;

public:
	void Init() override;
	void Uninit() override;
	void Update(float gameDt, float realDt) override;

	// ----- 外部から呼び出されるアニメ制御 -----
	void SetIsLocomotion(bool b);
	void SetBlendParam(float blend);
	void SetSpeedAnime(float speed);
	void PlayAnimeIdle();
	void PlayAnimeJump();
	void PlayAnimeJumpAir();
	void PlayAnimeAim();
	void PlayAnimeThrow();
	void PlayAnimeDamage();
	void PlayAnimeDead();
	void PlayAnimePush();
	void PlayAnimeKick();

	// 色を変える
	void SetMaterialColorDefault();
	void SetMaterialColorRed();
	void StartBlinkColor();

	// 骨のセット
	void SetupBones();

	// ゲッター（これらを作るとエラーが出る）
	const BoneManager& GetBoneManager() const { return *m_pBoneManager; }
	const AnimatorController& GetAnimatorController() const { return *m_pController; }

private:
	void BlinkColor(float gameDt);
};

#endif