/*
	AnimatorComponent.h
	20251216  hanaue sho
*/
#include "AnimatorComponent.h"
#include "AnimationClip.h"
#include "AnimatorController.h"
#include "Renderer.h"

#pragma comment (lib, "assimp-vc143-mt.lib") // ライブラリリンク
#include "assimp/scene.h"	    // assimp の中間表現の構造体定義
#include "assimp/postprocess.h" // ポストプロセスフラグの定義
#include "assimp/Importer.hpp"  // Impoeter専用

namespace
{
	// BoneTrack からローカル行列を作る
	Vector3 SampleTranslation(const BoneTrack& track, float time)
	{
		if (track.translationKeys.empty()) return Vector3(0, 0, 0);

		// とりあえず time以下で最大のキーを探す
		const TranslationKey* prev = &track.translationKeys[0];
		for (size_t i = 1; i < track.translationKeys.size(); i++)
		{
			if (track.translationKeys[i].time > time) break;
			prev = &track.translationKeys[i];
		}
		return prev->value;
	}
	Quaternion SampleRotation(const BoneTrack& track, float time)
	{
		if (track.rotationKeys.empty()) return Quaternion::Identity();

		// とりあえず time以下で最大のキーを探す
		const RotationKey* prev = &track.rotationKeys[0];
		for (size_t i = 1; i < track.rotationKeys.size(); i++)
		{
			if (track.rotationKeys[i].time > time) break;
			prev = &track.rotationKeys[i];
		}
		return prev->value;
	}
	Vector3 SampleScale(const BoneTrack& track, float time)
	{
		if (track.scaleKeys.empty()) return Vector3(1, 1, 1);

		// とりあえず time以下で最大のキーを探す
		const ScaleKey* prev = &track.scaleKeys[0];
		for (size_t i = 1; i < track.scaleKeys.size(); i++)
		{
			if (track.scaleKeys[i].time > time) break;
			prev = &track.scaleKeys[i];
		}
		return prev->value;
	}
	// ※現在未使用
	void ExtractBindTRS(const Matrix4x4& M, Vector3& outT, Matrix4x4& outR, Vector3& outS)
	{
		outT = Vector3(M.m[3][0], M.m[3][1], M.m[3][2]);

		Vector3 r0(M.m[0][0], M.m[0][1], M.m[0][2]);
		Vector3 r1(M.m[1][0], M.m[1][1], M.m[1][2]);
		Vector3 r2(M.m[2][0], M.m[2][1], M.m[2][2]);

		float sx = r0.length(); if (sx < 1e-8f) { sx = 1.0f; r0 = Vector3(1, 0, 0); }
		float sy = r1.length(); if (sy < 1e-8f) { sy = 1.0f; r1 = Vector3(0, 1, 0); }
		float sz = r2.length(); if (sz < 1e-8f) { sz = 1.0f; r2 = Vector3(0, 0, 1); }

		outS = Vector3(sx, sy, sz);

		r0 *= (1.0f / sx);
		r1 *= (1.0f / sy);
		r2 *= (1.0f / sz);

		if (Vector3::Dot(Vector3::Cross(r0, r1), r2) < 0.0f)
		{
			r2 *= -1.0f;
		}

		outR.identity();
		outR.m[0][0] = r0.x; outR.m[0][1] = r0.y; outR.m[0][2] = r0.z;
		outR.m[1][0] = r1.x; outR.m[1][1] = r1.y; outR.m[1][2] = r1.z;
		outR.m[2][0] = r2.x; outR.m[2][1] = r2.y; outR.m[2][2] = r2.z;
	}
}

void AnimatorComponent::AdvanceTime(float dt, const AnimationClip& clip, bool loop, float& inoutTime)
{
	// 時間更新
	inoutTime += dt * m_Speed;
	if (clip.duration > 0.0f)
	{
		if (loop)
		{
			while (inoutTime > clip.duration)
				inoutTime -= clip.duration;
		}
		else
		{
			if (inoutTime > clip.duration)
				inoutTime = clip.duration;
		}
	}
}

void AnimatorComponent::EvaluateLocalPose(const AnimationClip& clip, float time, std::vector<Matrix4x4>& outLocalPose)
{
	const auto& skeleton = *m_Skeleton;
	const auto& bones = skeleton.bones;
	const int boneCount = (int)bones.size();

	outLocalPose.resize(boneCount);
	// 全ノードを bindLocal で初期化
	for (int i = 0; i < boneCount; i++)
		outLocalPose[i] = bones[i].bindLocal;

	// Clip の BoneTrack で上書き
	for (const BoneTrack& track : clip.tracks)
	{
		int idx = track.boneIndex;
		if (idx < 0 || idx >= (int)boneCount) continue;

		// キーが無い成分はbindを使う
		Vector3 t   = track.translationKeys.empty() ? bones[idx].bindT			  : SampleTranslation(track, time);
		Vector3 s   = track.scaleKeys.empty()		? bones[idx].bindS			  : SampleScale(track, time);
		Matrix4x4 r = track.rotationKeys.empty()	? bones[idx].bindR.ToMatrix() : SampleRotation(track, time).normalized().ToMatrix();

		Matrix4x4 animeLocal = Matrix4x4::CreateTRS(t, r, s);
		outLocalPose[idx] = animeLocal;
	}
}

void AnimatorComponent::EvaluateLocalPoseTRS(const AnimationClip& clip, float time, std::vector<LocalTRS>& outLocalPose)
{
	const auto& skeleton = *m_Skeleton;
	const auto& bones = skeleton.bones;
	const int boneCount = (int)bones.size();

	outLocalPose.resize(boneCount);
	// 全ノードを bindLocal で初期化
	for (int i = 0; i < boneCount; i++)
	{
		outLocalPose[i].Translation = bones[i].bindT;
		outLocalPose[i].Rotation	= bones[i].bindR;
		outLocalPose[i].Scale		= bones[i].bindS;
	}

	// Clip の BoneTrack で上書き
	for (const BoneTrack& track : clip.tracks)
	{
		int idx = track.boneIndex;
		if (idx < 0 || idx >= (int)boneCount) continue;

		// キーが無い成分はbindを使う
		Vector3 t = track.translationKeys.empty() ? bones[idx].bindT : SampleTranslation(track, time);
		Vector3 s = track.scaleKeys.empty() ? bones[idx].bindS : SampleScale(track, time);
		Quaternion r = track.rotationKeys.empty() ? bones[idx].bindR : SampleRotation(track, time).normalized();

		outLocalPose[idx].Translation = t;
		outLocalPose[idx].Rotation = r;
		outLocalPose[idx].Scale = s;
	}
}

void AnimatorComponent::EvaluateBindPose(std::vector<Matrix4x4>& outLocalPose)
{
	const auto& skeleton = *m_Skeleton;
	const auto& bones = skeleton.bones;
	const int boneCount = (int)bones.size();

	outLocalPose.resize(boneCount);
	// 全ノードを bindLocal で初期化
	for (int i = 0; i < boneCount; i++)
		outLocalPose[i] = bones[i].bindLocal;
}

void AnimatorComponent::EvaluateBindPoseTRS(std::vector<LocalTRS>& outLocalPose)
{
	const auto& skeleton = *m_Skeleton;
	const auto& bones = skeleton.bones;
	const int boneCount = (int)bones.size();

	outLocalPose.resize(boneCount);
	// 全ノードを bindLocal で初期化
	for (int i = 0; i < boneCount; i++)
	{
		outLocalPose[i].Translation = bones[i].bindT;
		outLocalPose[i].Rotation = bones[i].bindR;
		outLocalPose[i].Scale = bones[i].bindS;
	}
}

void AnimatorComponent::BuildModelAndSkinFromLocalTRS()
{
	const auto& skeleton = *m_Skeleton;
	const auto& bones = skeleton.bones;
	const int boneCount = (int)bones.size();

	// TRS から Matrix4x4 へ
	m_LocalPoseOut.resize(boneCount);
	for (int i = 0; i < boneCount; i++)
	{
		// 行列へ変換
		m_LocalPoseOut[i] = Matrix4x4::CreateTRS(m_TRS_LocalPoseOut[i].Translation, m_TRS_LocalPoseOut[i].Rotation.ToMatrix(), m_TRS_LocalPoseOut[i].Scale);
	}

	m_ModelPose.resize(boneCount);
	m_SkinMatrixes.resize(boneCount);
	// ノード階層でワールド姿勢を構築
	for (int i = 0; i < boneCount; i++)
	{
		int parent = bones[i].parent;
		if (parent < 0) // ルートボーン：ローカル＝ワールド
			m_ModelPose[i] = m_LocalPoseOut[i];
		else // ローカル×親
			m_ModelPose[i] = m_LocalPoseOut[i] * m_ModelPose[parent];
	}
	// ボーンだけスキン行列に落とす（メッシュ空間→ボーン空間→ボーンワールド）
	for (int i = 0; i < boneCount; i++)
	{
		m_SkinMatrixes[i] = bones[i].invBindPose * m_ModelPose[i];
	}
}

void AnimatorComponent::EvaluateNow()
{
	if (!m_Skeleton) return;

	const auto& skeleton = *m_Skeleton;
	const auto& bones = skeleton.bones;
	const int boneCount = (int)bones.size();
	if (boneCount <= 0) return;

	m_TRS_LocalPoseOut.resize(boneCount);
	// クリップがない
	if (!m_ClipCurrent)
	{
		m_ModelPose.resize(boneCount);
		m_SkinMatrixes.resize(boneCount);
		m_LocalPoseOut.resize(boneCount);
		for (int i = 0; i < boneCount; i++)
		{
			m_ModelPose[i] = bones[i].invBindPose.Inverse();
			m_SkinMatrixes[i] = bones[i].invBindPose * m_ModelPose[i];
		}
		for (int i = 0; i < boneCount; ++i)
		{
			int p = bones[i].parent;
			if (p < 0) m_LocalPoseOut[i] = m_ModelPose[i];
			else       m_LocalPoseOut[i] = m_ModelPose[i] * m_ModelPose[p].Inverse();
		}
		return;
	}
	else // クリップがある
	{
		// 0秒時点の姿勢にする
		EvaluateLocalPoseTRS(*m_ClipCurrent, 0.0f, m_TRS_LocalPoseOut);
	}

	// TRS から Matrix4x4 へ
	m_LocalPoseOut.resize(boneCount);
	for (int i = 0; i < boneCount; i++)
	{
		// 行列へ変換
		m_LocalPoseOut[i] = Matrix4x4::CreateTRS(m_TRS_LocalPoseOut[i].Translation, m_TRS_LocalPoseOut[i].Rotation.ToMatrix(), m_TRS_LocalPoseOut[i].Scale);
	}

	m_ModelPose.resize(boneCount);
	m_SkinMatrixes.resize(boneCount);
	// ノード階層でワールド姿勢を構築
	for (int i = 0; i < boneCount; i++)
	{
		int parent = bones[i].parent;
		if (parent < 0) // ルートボーン：ローカル＝ワールド
			m_ModelPose[i] = m_LocalPoseOut[i];
		else // ローカル×親
			m_ModelPose[i] = m_LocalPoseOut[i] * m_ModelPose[parent];
	}
	// ボーンだけスキン行列に落とす（メッシュ空間→ボーン空間→ボーンワールド）
	for (int i = 0; i < boneCount; i++)
	{
		m_SkinMatrixes[i] = bones[i].invBindPose * m_ModelPose[i];
		//m_SkinMatrixes[i].identity(); // デバッグ用
	}
}

void AnimatorComponent::Update(float dt)
{
	if (!m_Skeleton) return;

	const auto& skeleton = *m_Skeleton;
	const auto& bones = skeleton.bones;
	const int boneCount = (int)bones.size();
	if (boneCount == 0) return;

	// ==================================================
	// ----- ブレンド分岐 -----
	// ==================================================
	// --------------------------------------------------
	// クロスフェード中
	// --------------------------------------------------
	if (m_IsBlending && m_ClipNext) // ブレンド中
	{
		const auto& clipNext = *m_ClipNext;

		// 時間経過
		m_BlendElapsed += dt;
		float alpha = (m_BlendDuration <= 0) ? 1 : Vector3::Clamp(m_BlendElapsed / m_BlendDuration, 0.0f, 1.0f);

		// ----- B側の評価 -----
		// 時間更新
		AdvanceTime(dt, clipNext, m_LoopNext, m_TimeNext);
		// アニメーションのポーズ評価
		EvaluateLocalPoseTRS(clipNext, m_TimeNext, m_TRS_LocalPoseB);

		// ----- A側の評価 -----
		if (m_FadeFromSnapshot)
		{
			// スナップショットをそのままAにする
			if ((int)m_TRS_FadeFromPose.size() != boneCount)
				m_TRS_FadeFromPose.resize(boneCount);
			m_TRS_LocalPoseA = m_TRS_FadeFromPose;
		}
		else
		{
			// クリップAから A を作る
			if (!m_ClipCurrent) return;
			const auto& clipCurrent = *m_ClipCurrent;
			AdvanceTime(dt, clipCurrent, m_LoopCurrent, m_TimeCurrent);
			EvaluateLocalPoseTRS(clipCurrent, m_TimeCurrent, m_TRS_LocalPoseA);
		}


		// ----- A/B をブレンドして out を作る -----
		m_TRS_LocalPoseOut.resize(boneCount);
		m_LocalPoseOut.resize(boneCount);
		for (int i = 0; i < boneCount; i++)
		{
			m_TRS_LocalPoseOut[i].Translation = Vector3::Lerp(m_TRS_LocalPoseA[i].Translation, m_TRS_LocalPoseB[i].Translation, alpha);
			m_TRS_LocalPoseOut[i].Rotation    = Quaternion::NlerpShortest(m_TRS_LocalPoseA[i].Rotation, m_TRS_LocalPoseB[i].Rotation, alpha);
			m_TRS_LocalPoseOut[i].Scale		  = Vector3::Lerp(m_TRS_LocalPoseA[i].Scale, m_TRS_LocalPoseB[i].Scale, alpha);
			// 行列へ変換
			m_LocalPoseOut[i] = Matrix4x4::CreateTRS(m_TRS_LocalPoseOut[i].Translation, m_TRS_LocalPoseOut[i].Rotation.ToMatrix(), m_TRS_LocalPoseOut[i].Scale);
		}

		m_ModelPose.resize(boneCount);
		m_SkinMatrixes.resize(boneCount);
		// ノード階層でワールド姿勢を構築
		for (int i = 0; i < boneCount; i++)
		{
			int parent = bones[i].parent;
			if (parent < 0) // ルートボーン：ローカル＝ワールド
				m_ModelPose[i] = m_LocalPoseOut[i];
			else // ローカル×親
				m_ModelPose[i] = m_LocalPoseOut[i] * m_ModelPose[parent];
		}

		// ボーンだけスキン行列に落とす（メッシュ空間→ボーン空間→ボーンワールド）
		for (int i = 0; i < boneCount; i++)
		{
			m_SkinMatrixes[i] = bones[i].invBindPose * m_ModelPose[i];
		}

		// ブレンド終了判定
		if (m_BlendElapsed >= m_BlendDuration)
		{
			m_IsBlending = false;
			m_ClipCurrent = m_ClipNext;
			m_TimeCurrent = m_TimeNext;
			m_LoopCurrent = m_LoopNext;
			m_ClipNext = nullptr;
			m_BlendElapsed = 0.0f;
			m_BlendDuration = 0.0f;
		}
		return;
	}
	// --------------------------------------------------
	// AnimatorController の常時ブレンド
	// --------------------------------------------------
	if (m_Controller && m_IsLocomotion)
	{
		const Blend1DResult r = m_Controller->EvaluateLocomotion(m_BlendParam);
		if (r.IsValid())
		{
			// 時間更新
			AdvanceTime(dt, *r.clipA, r.loopA, m_TimeA);
			AdvanceTime(dt, *r.clipB, r.loopB, m_TimeB);
		 
			// アニメーションのポーズ評価
			EvaluateLocalPoseTRS(*r.clipA, m_TimeA, m_TRS_LocalPoseA);
			EvaluateLocalPoseTRS(*r.clipB, m_TimeB, m_TRS_LocalPoseB);

			const float alpha = Vector3::Clamp(r.alpha, 0.0f, 1.0f);

			// ここでブレンド処理
			m_TRS_LocalPoseOut.resize(boneCount);
			m_LocalPoseOut.resize(boneCount);
			for (int i = 0; i < boneCount; i++)
			{
				m_TRS_LocalPoseOut[i].Translation = Vector3::Lerp(m_TRS_LocalPoseA[i].Translation, m_TRS_LocalPoseB[i].Translation, alpha);
				m_TRS_LocalPoseOut[i].Rotation = Quaternion::NlerpShortest(m_TRS_LocalPoseA[i].Rotation, m_TRS_LocalPoseB[i].Rotation, alpha);
				m_TRS_LocalPoseOut[i].Scale = Vector3::Lerp(m_TRS_LocalPoseA[i].Scale, m_TRS_LocalPoseB[i].Scale, alpha);
				// 行列へ変換
				m_LocalPoseOut[i] = Matrix4x4::CreateTRS(m_TRS_LocalPoseOut[i].Translation, m_TRS_LocalPoseOut[i].Rotation.ToMatrix(), m_TRS_LocalPoseOut[i].Scale);
			}

			m_ModelPose.resize(boneCount);
			m_SkinMatrixes.resize(boneCount);
			// ノード階層でワールド姿勢を構築
			for (int i = 0; i < boneCount; i++)
			{
				int parent = bones[i].parent;
				if (parent < 0) // ルートボーン：ローカル＝ワールド
					m_ModelPose[i] = m_LocalPoseOut[i];
				else // ローカル×親
					m_ModelPose[i] = m_LocalPoseOut[i] * m_ModelPose[parent];
			}
			// ボーンだけスキン行列に落とす（メッシュ空間→ボーン空間→ボーンワールド）
			for (int i = 0; i < boneCount; i++)
			{
				m_SkinMatrixes[i] = bones[i].invBindPose * m_ModelPose[i];
			}
			return;
		}
	}

	// --------------------------------------------------
	// 単一クリップ中
	// --------------------------------------------------
	if (!m_ClipCurrent) return;
	{
		const auto& clipCurrent = *m_ClipCurrent;

		// 時間更新
		AdvanceTime(dt, clipCurrent, m_LoopCurrent, m_TimeCurrent);

		// アニメーションのポーズ評価
		m_LocalPoseOut.resize(boneCount);
		EvaluateLocalPoseTRS(clipCurrent, m_TimeCurrent, m_TRS_LocalPoseOut);
		for (int i = 0; i < boneCount; i++)
		{
			// 行列へ変換
			m_LocalPoseOut[i] = Matrix4x4::CreateTRS(m_TRS_LocalPoseOut[i].Translation, m_TRS_LocalPoseOut[i].Rotation.ToMatrix(), m_TRS_LocalPoseOut[i].Scale);
		}

		m_ModelPose.resize(boneCount);
		m_SkinMatrixes.resize(boneCount);

		// ノード階層でワールド姿勢を構築
		for (int i = 0; i < boneCount; i++)
		{
			int parent = bones[i].parent;
			if (parent < 0) // ルートボーン：ローカル＝ワールド
				m_ModelPose[i] = m_LocalPoseOut[i];
			else // ローカル×親
				m_ModelPose[i] = m_LocalPoseOut[i] * m_ModelPose[parent];
		}
		// ボーンだけスキン行列に落とす（メッシュ空間→ボーン空間→ボーンワールド）
		for (int i = 0; i < boneCount; i++)
		{
			m_SkinMatrixes[i] = bones[i].invBindPose * m_ModelPose[i];
		}
	}
}
