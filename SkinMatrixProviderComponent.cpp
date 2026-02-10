/*
	SkinMatrixProviderComponent.cpp
	20260123  hanaue sho
*/
#include "SkinMatrixProviderComponent.h"
#include "ModelLoader.h"
#include "Matrix4x4.h"
#include "TransformComponent.h"
#include "AnimatorComponent.h"
#include "BoneManager.h"

void SkinMatrixProviderComponent::SetUp(Skeleton* skeleton, AnimatorComponent* anime, BoneManager* boneManager, TransformComponent* ownerTrans)
{
	m_pSkeleton = skeleton;
	m_pAnimator = anime;
	m_pBoneManager = boneManager;
	m_pOwnerTransform = ownerTrans;

	const int boneCount = (int)m_pSkeleton->bones.size();
	m_UsePhysicsBone.assign(boneCount, 0);
}

const Matrix4x4* SkinMatrixProviderComponent::GetSkinMatrixPtr(int& outCount)
{
	outCount = 0;

	if (!m_pSkeleton) return nullptr;
	const int boneCount = (int)m_pSkeleton->bones.size();
	if (boneCount <= 0) return nullptr;

	// Animation
	if (m_Mode == Mode::Animation)
	{
		if (!m_pAnimator) return nullptr;
		const auto& skin = m_pAnimator->SkinMatrixes();
		outCount = (int)skin.size();
		return (outCount > 0) ? skin.data() : nullptr;
	}
	// Ragdoll
	if (m_Mode == Mode::Ragdoll)
	{
		BuildPhysicsSkinMatrixes();
		outCount = (int)m_SkinPhys.size();
		return (outCount > 0) ? m_SkinPhys.data() : nullptr;
	}
	// Hybrid
	BuildHybridSkinMatrixes();
	outCount = (int)m_SkinOut.size();
	return (outCount > 0) ? m_SkinOut.data() : nullptr;
}

void SkinMatrixProviderComponent::BuildPhysicsSkinMatrixes()
{
	if (!m_pSkeleton || !m_pBoneManager || !m_pOwnerTransform) return;

	const int boneCount = (int)m_pSkeleton->bones.size();
	m_SkinPhys.resize(boneCount);

	const Matrix4x4 ownerWorld = m_pOwnerTransform->WorldMatrix();
	const Matrix4x4 invOwnerWorld = ownerWorld.Inverse();

	for (int i = 0; i < boneCount; i++)
	{
		const Matrix4x4& boneWorld = m_pBoneManager->GetBoneWorldMatrix(i);

		// boneModel = boneWorld * invOwnerWorld
		Matrix4x4 boneModel = boneWorld * invOwnerWorld;

		// skin = invBindPose * boneModel
		m_SkinPhys[i] = m_pSkeleton->bones[i].invBindPose * boneModel;
	}
}

void SkinMatrixProviderComponent::BuildHybridSkinMatrixes()
{
	if (!m_pSkeleton || !m_pBoneManager || !m_pOwnerTransform || !m_pAnimator) return;

	const auto& bones = m_pSkeleton->bones;
	const int boneCount = (int)m_pSkeleton->bones.size();
	if (boneCount <= 0) return;

	// Hybrid はマスク必須
	if ((int)m_UsePhysicsBone.size() != boneCount) return;

	// animeLocal を取得（Animator が Update 済みである前提）
	const auto& animeLocal = m_pAnimator->LocalPoseOut();
	if ((int)animeLocal.size() < boneCount) return;

	m_FinalModel.resize(boneCount);
	m_SkinOut.resize(boneCount);

	const Matrix4x4 ownerWorld = m_pOwnerTransform->WorldMatrix();
	const Matrix4x4 invOwnerWorld = ownerWorld.Inverse();

	// bones は親→子順にソートされている前提
	const bool hasMask = (m_UsePhysicsBone.size() == (size_t)boneCount);
	for (int i = 0; i < boneCount; i++)
	{
		const int parent = bones[i].parent;

		if (hasMask && m_UsePhysicsBone[i])
		{
			// ----- Physics 駆動 : boneWorld -> boneModel に戻す -----
			const Matrix4x4& boneWorld = m_pBoneManager->GetBoneWorldMatrix(i);

			// boneModel = boneWorld * invOwnerWorld
			Matrix4x4 boneModel = boneWorld * invOwnerWorld;

			// 最終モデル姿勢はこれで確定
			m_FinalModel[i] = boneModel;
		}
		else
		{
			// ----- Animation 駆動 : animeLocal を親に最終に合成
			if (parent < 0) m_FinalModel[i] = animeLocal[i];
			else			m_FinalModel[i] = animeLocal[i] * m_FinalModel[parent];
		}
		// Skin = invBindPose * finalModel
		m_SkinOut[i] = bones[i].invBindPose * m_FinalModel[i];
	}
}
