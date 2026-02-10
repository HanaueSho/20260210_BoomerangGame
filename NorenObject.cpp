/*
	NorenObejct.cpp
	20260201 hanaue sho
*/
#include "NorenObject.h"

#include "ModelObject.h"
#include "Renderer.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "ColliderComponent.h"
#include "AnimatorComponent.h"
#include "AnimationClip.h"
#include "AnimatorController.h"
#include "Keyboard.h"
#include "BoneManager.h"
#include "SkinMatrixProviderComponent.h"
#include "BallJointComponent.h"
#include "HingeJointComponent.h"

#include "Texture.h"


void NorenObject::Init()
{
	auto* tf = GetComponent<TransformComponent>();
	float s = 1.0f;
	//tf->SetPosition({ -15, 10, 0 });
	tf->SetScale({ s, s, s });
	tf->SetEulerAngles({ 3.141592F / 2 * -1, 3.141592F / 2 * 1, 0 });

	// Mesh + Skeleton
	std::vector<ID3D11ShaderResourceView*> srvs;
	auto* mf = AddComponent<MeshFilterComponent>();
	m_Skeleton = ModelLoader::BuildSkeletonFromFile("assets\\model\\Noren_001.fbx");
	ModelLoader::LoadSkinnedMeshFromFile(mf, "assets\\model\\Noren_001.fbx", m_Skeleton, srvs, true);

	// Animator
	auto* animator = AddComponent<AnimatorComponent>();
	m_Clip = ModelLoader::BuildAnimationClipFromFile("assets\\model\\Noren_001.fbx", m_Skeleton, 0);
	//assert(m_Clip.duration > 0.0f);
	//assert(!m_Clip.tracks.empty());
	animator->SetSkeleton(&m_Skeleton);
	animator->SetClip(&m_Clip);

	// BoneManager
	m_pBoneManager = new BoneManager(static_cast<GameObject*>(this), &m_Skeleton);

	//SkinMatrixProviderComponent
	m_pProvider = AddComponent<SkinMatrixProviderComponent>();
	m_pProvider->SetUp(&m_Skeleton, animator, m_pBoneManager, Transform());
	m_pProvider->SetMode(SkinMatrixProviderComponent::Mode::Hybrid);
	SetupBones();


	// MeshRenderer
	auto* mr = AddComponent<MeshRendererComponent>();

	// Slot 数
	const int slotCount = mf->MaterialSlotCount();
	mr->ResizeMaterialSlots(slotCount);

	// 共通シェーダー
	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* il = nullptr;
	Renderer::CreateVertexShader(&vs, &il, "shader\\VS_Skinned.cso", Renderer::VertexLayoutType::Skinned);
	Renderer::CreatePixelShader(&ps, "shader\\PS_Toon.cso");

	// 共通マテリアル定数
	MaterialApp m{};
	m.diffuse = Vector4(1, 1, 1, 1);
	m.ambient = Vector4(1, 1, 1, 1);
	m.textureEnable = true;

	// slot ごとに MaterialComponent を作って srv を刺す
	for (int slot = 0; slot < slotCount; slot++)
	{
		auto* mat = AddComponent<MaterialComponent>();

		mat->SetVSPS(vs, ps, il, false, false, false);

		ID3D11ShaderResourceView* srv = nullptr;
		if (slot < (int)srvs.size()) srv = srvs[slot];

		mat->SetMainTexture(srv, nullptr);

		mat->SetMaterial(m);
		mat->SetBlendMode(MaterialComponent::BlendMode::Opaque);

		mr->SetMaterialSlot(slot, mat);
	}
	if (slotCount > 0)
		mr->SetBaseMaterial(mr->GetMaterialSlot(0));

	// アウトラインマテリアル追加
	//auto* matOutline = AddComponent<MaterialComponent>();
	//Renderer::CreateVertexShader(&vs, &il, "shader\\VS_SkinnedOutline.cso", Renderer::VertexLayoutType::Skinned);
	//Renderer::CreatePixelShader(&ps, "shader\\PS_Outline.cso");
	//matOutline->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);
	//matOutline->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Opaque);
	//mr->SetOutlineMaterial(matOutline);
}

void NorenObject::Uninit()
{
	delete m_pBoneManager;
	m_pBoneManager = nullptr;
	GameObject::Uninit();
}

void NorenObject::Update(float dt)
{
	GameObject::Update(dt);

	// ----- アニメーション再生 -----
	auto* animator = GetComponent<AnimatorComponent>();
	auto* provider = GetComponent<SkinMatrixProviderComponent>();
	if (m_pProvider->IsAniamtion())
	{
		if (animator) m_pBoneManager->UpdateFromAnimator(animator->ModelMatrixes(), nullptr);
	}
	else if (m_pProvider->IsRagdoll())
	{
		m_pBoneManager->UpdateFromPhysics(nullptr);
	}
	else if (m_pProvider->IsHybrid())
	{
		if (animator) m_pBoneManager->UpdateFromAnimator(animator->ModelMatrixes(), &provider->UsePhysicsBoneMask());
		m_pBoneManager->UpdateFromPhysics(&provider->UsePhysicsBoneMask());
	}
}

void NorenObject::SetupBones()
{
	auto* animator = GetComponent<AnimatorComponent>();

	// まずは骨の位置を揃える
	animator->EvaluateNow();
	m_pBoneManager->UpdateFromAnimator(animator->ModelMatrixes(), nullptr);

	// 骨の設定
	const int indexRoot0 = 0;
	const int indexBone00 = 1;
	const int indexBone02 = 3;
	const int indexRoot1 = 4;
	const int indexBone10 = 5;
	const int indexBone12 = 7;
	const int indexRoot2 = 8;
	const int indexBone20 = 9;
	const int indexBone22 = 11;
	const int indexRoot3 = 12;
	const int indexBone30 = 13;
	const int indexBone32 = 15;

	// 縦のジョイント
	for (int num = 0; num < 4; num++)// RootBone0 .. 3
	{
		const int indexRoot = indexRoot0 + 4 * num;
		// i % 4 == 0 : i = indexRootX;
		for (int i = indexRoot; i <= indexRoot + 3; i++)
		{
			if (i % 4 == 0) m_pBoneManager->SetBonePhysics(i, true, false);
			else { m_pBoneManager->SetBonePhysics(i, true, true); m_pProvider->SetBonePhysics(i, true); }
			auto* bone = m_pBoneManager->GetBoneObject(i);
			auto* coll = bone->GetComponent<Collider>();
			if (i == indexRoot) { coll->SetSphere(0.2f); coll->SetOffsetPosition({ 0, 0.00f, 0.0f }); }
			if (i == indexRoot + 1) { coll->SetSphere(0.4f); coll->SetOffsetPosition({ 0, 0.0f, 0.0f }); }
			if (i == indexRoot + 2) { coll->SetSphere(0.4f); coll->SetOffsetPosition({ 0, 0.0f, 0.0f }); }
			if (i == indexRoot + 3) { coll->SetSphere(0.4f); coll->SetOffsetPosition({ 0, 0.0f, 0.0f }); }
		}
		for (int i = indexRoot; i < indexRoot + 3; i++) // ジョイント設定
		{
			auto* bone = m_pBoneManager->GetBoneObject(i);
			if (i % 4 == 0)
			{
				auto* joint = bone->AddComponent<BallJointComponent>();
				joint->SetOtherObject(m_pBoneManager->GetBoneObject(i + 1));
				joint->SetLocalAnchorA({ 0, -0.2f, 0 }); // 上向きボーン
				joint->SetLocalAnchorB({ 0, -0.5f, 0 }); // 下向きボーン
				joint->RegisterToPhysicsSystem();
			}
			else
			{
				auto* joint = bone->AddComponent<BallJointComponent>();
				joint->SetOtherObject(m_pBoneManager->GetBoneObject(i + 1));
				joint->SetLocalAnchorA({ 0,  0.5f, 0 }); // 下向きボーン
				joint->SetLocalAnchorB({ 0, -0.5f, 0 }); // 下向きボーン
				joint->RegisterToPhysicsSystem();
			}
		}
	}
	// 横のジョイント
	for (int num = 0; num < 3; num++)
	{
		const int indexBone = indexBone00 + 4 * num;
		for (int i = indexBone; i <= indexBone + 2; i++)
		{
			auto* bone = m_pBoneManager->GetBoneObject(i);
			auto* joint = bone->AddComponent<BallJointComponent>();
			joint->SetOtherObject(m_pBoneManager->GetBoneObject(i + 4));
			joint->SetLocalAnchorA({ 0.0f,  0.0f, 0.5f }); // 下向きボーン
			joint->SetLocalAnchorB({ 0.0f,  0.0f, -0.5f }); // 下向きボーン
			joint->RegisterToPhysicsSystem();
		}
	}
}
