/*
	AnimatorComponent.h
	20251214  hanaue sho
*/
#ifndef ANIMATORCOMPONENT_H_
#define ANIMATORCOMPONENT_H_
#include <vector>
#include "Component.h"
#include "ModelLoader.h"

struct AnimationClip;
class Matrix4x4;
class AnimatorController;

// ローカルTRS 構造体
struct LocalTRS
{
	Vector3 Translation;
	Quaternion Rotation;
	Vector3 Scale;
};

class AnimatorComponent : public Component
{
private:
	// ==================================================
	// ----- 基本構成要素 -----
	// ==================================================
	// --------------------------------------------------
	// スケルトン情報
	// --------------------------------------------------
	Skeleton*	   m_Skeleton  = nullptr;
	// --------------------------------------------------
	// アニメーションクリップ情報
	// --------------------------------------------------
	AnimationClip* m_ClipCurrent = nullptr;
	AnimationClip* m_ClipNext = nullptr;
	// --------------------------------------------------
	// アニメーションの時間
	// --------------------------------------------------
	float m_TimeCurrent = 0.0f; // アニメーション再生中の時間
	float m_TimeNext = 0.0f; 
	// --------------------------------------------------
	// ループ制御フラグ
	// --------------------------------------------------
	bool  m_LoopCurrent = true; 
	bool  m_LoopNext = true; 
	// --------------------------------------------------
	// 再生速度
	// --------------------------------------------------
	float m_Speed = 1.0f;
	// --------------------------------------------------
	// ブレンド制御
	// --------------------------------------------------
	bool m_IsBlending = false;
	float m_BlendElapsed = 0.0f;
	float m_BlendDuration = 0.0f;
	// ==================================================
	// ----- ロコモーション制御 -----
	// ==================================================
	// --------------------------------------------------
	// コントローラー
	// --------------------------------------------------
	AnimatorController* m_Controller = nullptr;
	float m_BlendParam = 0.0f; // コントローラー用の速度
	float m_TimeA = 0.0f; // Blend1D 用
	float m_TimeB = 0.0f; // Blend1D 用
	// --------------------------------------------------
	// 制御フラグ
	// --------------------------------------------------
	bool m_IsLocomotion = true;
	// ==================================================
	// ----- クロスフェード制御 -----
	// ==================================================
	bool m_FadeFromSnapshot = false;
	// ==================================================
	// ----- ボーンごとのTRS, Matrix -----
	// ==================================================
	// --------------------------------------------------
	// ボーンごとの現在のローカル姿勢
	// --------------------------------------------------
	std::vector<LocalTRS> m_TRS_LocalPoseA;
	std::vector<LocalTRS> m_TRS_LocalPoseB;
	std::vector<LocalTRS> m_TRS_LocalPoseOut;
	std::vector<LocalTRS> m_TRS_FadeFromPose; // CrossFade用のスナップショット
	std::vector<Matrix4x4> m_LocalPoseOut;
	// --------------------------------------------------
	// ボーンごとの現在の合成結果姿勢
	// --------------------------------------------------
	std::vector<Matrix4x4> m_ModelPose; // ボーンごとの現在の合成結果姿勢
	// --------------------------------------------------
	// VS に投げる最終行列
	// --------------------------------------------------
	std::vector<Matrix4x4> m_SkinMatrixes; // VS に投げる最終行列
public:
	// ==================================================
	// ----- 姿勢評価 -----
	// ==================================================
	void EvaluateNow();

	// ==================================================
	// ----- セッター -----
	// ==================================================
	void SetSkeleton(Skeleton* s)  noexcept { m_Skeleton = s; }
	void SetClip(AnimationClip* c) noexcept { m_ClipCurrent = c; m_TimeCurrent = 0.0f; }
	void SetLoop(bool b)		   noexcept { m_LoopCurrent = b; }
	void SetSpeed(float s)		   noexcept { m_Speed = s; }
	void SetController(AnimatorController* c) noexcept { m_Controller = c; }
	void SetBlendParam(float s) noexcept { m_BlendParam = s;}
	void SetIsLocomotion(bool b)   noexcept { m_IsLocomotion = b; }

	// ==================================================
	// ----- アクセサ -----
	// ==================================================
	const std::vector<Matrix4x4>& SkinMatrixes()  const noexcept { return m_SkinMatrixes; }
	const std::vector<Matrix4x4>& ModelMatrixes() const noexcept { return m_ModelPose; }
	const std::vector<Matrix4x4>& LocalPoseOut()  const noexcept { return m_LocalPoseOut; }
	int BoneCount() noexcept { return m_Skeleton ? (int)m_Skeleton->bones.size() : 0; }

	// ==================================================
	// ----- クリップ再生 -----
	// ==================================================
	void Play(AnimationClip* c) noexcept 
	{ 
		if (!c) return;
		m_ClipCurrent = c; 
		m_TimeCurrent = 0.0f;

		m_IsBlending = false;
		m_ClipNext = nullptr;
		m_BlendDuration = 0.0f;
		m_BlendElapsed = 0.0f;
	}
	void CrossFade(AnimationClip* clip, float duration, bool loop) 
	{
		if (!clip) return;

		m_IsBlending = true;
		m_FadeFromSnapshot = false;

		m_ClipNext = clip;
		m_TimeNext = 0.0f;
		m_LoopNext = loop;
		m_BlendElapsed = 0.0f;
		m_BlendDuration = duration;
	}
	void CrossFadeFromCurrentPose(AnimationClip* clip, float duration, bool loop)
	{
		if (!clip) return;

		// 現在出力からスナップショット
		const int boneCount = (int)m_Skeleton->bones.size();
		m_TRS_FadeFromPose = m_TRS_LocalPoseOut;
		if ((int)m_TRS_FadeFromPose.size() != boneCount)
			m_TRS_FadeFromPose.resize(boneCount); // 念のため

		m_IsBlending = true;
		m_FadeFromSnapshot = true;

		m_ClipNext = clip;
		m_TimeNext = 0.0f;
		m_LoopNext = loop;
		m_BlendElapsed = 0.0f;
		m_BlendDuration = duration;
	}

	// ==================================================
	// ----- ライフサイクル -----
	// ==================================================
	void Update(float dt) override;

private:
	// 時間経過
	void AdvanceTime(float dt, const AnimationClip& clip, bool loop, float& inoutTime);
	// ポーズ評価
	void EvaluateLocalPose(const AnimationClip& clip, float time, std::vector<Matrix4x4>& outLocalPose);
	void EvaluateLocalPoseTRS(const AnimationClip& clip, float time, std::vector<LocalTRS>& outLocalPose);
	void EvaluateBindPose(std::vector<Matrix4x4>& outLocalPose);
	void EvaluateBindPoseTRS(std::vector<LocalTRS>& outLocalPose);
	// ModelPose と SkinMatrix の作成
	void BuildModelAndSkinFromLocalTRS();
};

#endif