/*
	ModelObject.h
	20251224  hanaue sho
*/
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

#include "Texture.h"

void ModelObject::Init()
{
	auto* tf = GetComponent<TransformComponent>();
	float s = 1.1f;
	tf->SetPosition({ 0, 3, 0 });
	tf->SetScale({ s, s, s });
	tf->SetEulerAngles({ 3.141592F / 2 * 3, 0, 0 });

	// Mesh + Skeleton
	std::vector<ID3D11ShaderResourceView*> srvs;
	auto* mf = AddComponent<MeshFilterComponent>();
	m_Skeleton = ModelLoader::BuildSkeletonFromFile("assets\\model\\player_001.fbx");
	ModelLoader::LoadSkinnedMeshFromFile(mf, "assets\\model\\player_001.fbx", m_Skeleton, srvs, true);
	//ModelLoader::LoadMeshFromFile(mf, "assets\\model\\Akai.fbx", srvs, true);

	// Animator
	auto* animator = AddComponent<AnimatorComponent>();
	m_Clip = ModelLoader::BuildAnimationClipFromFile("assets\\model\\player_001_Idle.fbx", m_Skeleton, 0);
	m_Clip1 = ModelLoader::BuildAnimationClipFromFile("assets\\model\\player_001_Walk.fbx", m_Skeleton, 0);
	//m_Clip2 = ModelLoader::BuildAnimationClipFromFile("assets\\model\\Akai_FastRun.fbx", m_Skeleton, 0);
	//assert(m_Clip.duration > 0.0f);
	//assert(!m_Clip.tracks.empty());
	animator->SetSkeleton(&m_Skeleton);
	animator->SetClip(&m_Clip);
	animator->SetSpeed(1.3f);

	// AnimatorController
	m_pController = new AnimatorController();
	m_pController->m_Locomotion.clipA = &m_Clip;
	m_pController->m_Locomotion.clipB = &m_Clip1;
	animator->SetController(m_pController);
	animator->SetIsLocomotion(true);

	// BoneManager
	m_pBoneManager = new BoneManager(static_cast<GameObject*>(this), &m_Skeleton);
	for (int i = 0; i < m_Skeleton.bones.size(); i++)
		m_pBoneManager->SetBonePhysics(i, true, false);
	animator->EvaluateNow();
	m_pBoneManager->UpdateFromAnimator(animator->ModelMatrixes(), nullptr);
	for (int i = 0; i < m_pBoneManager->GetBonesSize(); i++)
	{
		auto* coll = m_pBoneManager->GetBoneObject(i)->GetComponent<Collider>();
		coll->SetBox({ 0.3, 0.3, 0.3 });
	}
	//m_pBoneManager->AutoSetupCapsulesFromBindPose(0, m_pBoneManager->GetBonesSize(), 5);

	//SkinMatrixProviderComponent
	m_pProvider = AddComponent<SkinMatrixProviderComponent>();
	m_pProvider->SetUp(&m_Skeleton, animator, m_pBoneManager, Transform());
	m_pProvider->SetMode(SkinMatrixProviderComponent::Mode::Animation);
	for (int i = 0; i < m_pBoneManager->GetBonesSize(); i++)
	{
		m_pProvider->SetBonePhysics(i, true);
	}
	//m_pProvider->SetBonePhysics(64, true);
	//m_pProvider->SetBonePhysics(65, true);
	//m_pProvider->SetBonePhysics(66, true);

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
	//Renderer::CreateVertexShader(&vs, &il, "shader\\VS_Lit.cso", Renderer::VertexLayoutType::Basic);
	Renderer::CreatePixelShader(&ps, "shader\\PS_Toon.cso");

	// 共通マテリアル定数
	MaterialApp m{};
	m.diffuse = Vector4(1, 1, 1, 1);
	m.ambient = Vector4(1, 1, 1, 1);
	m.textureEnable = true;

	// 一時的にテクスチャを設定 -----
	auto* mat = AddComponent<MaterialComponent>();
	mat->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);
	ID3D11ShaderResourceView* srv = Texture::LoadAndRegisterKey("assets\\texture\\whiteTexture.png");
	mat->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);


	// slot ごとに MaterialComponent を作って srv を刺す
	for (int slot = 0; slot < slotCount; slot++)
	{
		auto* mat = AddComponent<MaterialComponent>();

		mat->SetVSPS(vs, ps, il, false, false, false);

		ID3D11ShaderResourceView* srv = nullptr;
		if (slot < (int)srvs.size()) srv = srvs[slot];

		//srv = Texture::LoadAndRegisterKey("assets\\texture\\whiteTexture.png"); // デバッグでバグでばっぐ

		mat->SetMainTexture(srv, nullptr);

		mat->SetMaterial(m);
		mat->SetBlendMode(MaterialComponent::BlendMode::Opaque);

		mr->SetMaterialSlot(slot, mat);
	}
	if (slotCount > 0)
		mr->SetBaseMaterial(mr->GetMaterialSlot(0));

	// アウトラインマテリアル追加
	auto* matOutline = AddComponent<MaterialComponent>();
	Renderer::CreateVertexShader(&vs, &il, "shader\\VS_SkinnedOutline.cso", Renderer::VertexLayoutType::Skinned);
	Renderer::CreatePixelShader(&ps, "shader\\PS_Outline.cso");
	matOutline->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);
	matOutline->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Opaque);
	mr->SetOutlineMaterial(matOutline);

}

void ModelObject::Uninit()
{
	delete m_pBoneManager;
	m_pBoneManager = nullptr;
	GameObject::Uninit();
}

void ModelObject::Update(float dt)
{
	GameObject::Update(dt);

	auto* animator = GetComponent<AnimatorComponent>();
	auto* provider = GetComponent<SkinMatrixProviderComponent>();
	if(m_pProvider->IsAniamtion())
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


	if (Keyboard_IsKeyDownTrigger(KK_D1))
	{
		auto animator = GetComponent<AnimatorComponent>();
		animator->CrossFade(&m_Clip, 5.0f, true);
	}
	if (Keyboard_IsKeyDownTrigger(KK_D2))
	{
		auto animator = GetComponent<AnimatorComponent>();
		animator->CrossFade(&m_Clip1, 5.0f, true);
	}
	if (Keyboard_IsKeyDownTrigger(KK_D3))
	{
		auto animator = GetComponent<AnimatorComponent>();
		animator->CrossFade(&m_Clip2, 5.0f, true);
	}
	if (Keyboard_IsKeyDown(KK_O))
	{
		m_SpeedParam -= 1.0f * dt; if (m_SpeedParam < 0.0f) m_SpeedParam = 0.0f;
		auto animator = GetComponent<AnimatorComponent>();
		animator->SetSpeedParam(m_SpeedParam);
	}
	if (Keyboard_IsKeyDown(KK_P))
	{
		m_SpeedParam += 1.0f * dt; if (m_SpeedParam > 1.0f) m_SpeedParam = 1.0f;
		auto animator = GetComponent<AnimatorComponent>();
		animator->SetSpeedParam(m_SpeedParam);
	}

	const float speed = 6.0f;
	if (Keyboard_IsKeyDown(KK_W))
	{
		Vector3 pos = Transform()->Position();
		pos.z += speed * dt;
		Transform()->SetPosition(pos);
	}
	if (Keyboard_IsKeyDown(KK_S))
	{
		Vector3 pos = Transform()->Position();
		pos.z += -speed * dt;
		Transform()->SetPosition(pos);
	}
	if (Keyboard_IsKeyDown(KK_A))
	{
		Vector3 pos = Transform()->Position();
		pos.x -= speed * dt;
		Transform()->SetPosition(pos);
	}
	if (Keyboard_IsKeyDown(KK_D))
	{
		Vector3 pos = Transform()->Position();
		pos.x += speed * dt;
		Transform()->SetPosition(pos);
	}
	if (Keyboard_IsKeyDown(KK_Q))
	{
		Vector3 pos = Transform()->Position();
		pos.y += speed * dt;
		Transform()->SetPosition(pos);
	}
	if (Keyboard_IsKeyDown(KK_E))
	{
		Vector3 pos = Transform()->Position();
		pos.y += -speed * dt;
		Transform()->SetPosition(pos);
	}

	if (Keyboard_IsKeyDownTrigger(KK_SPACE))
	{
		for (int i = 0; i < m_pBoneManager->GetBonesSize(); i++)
		{
			auto* rigid = m_pBoneManager->GetBoneObject(i)->GetComponent<Rigidbody>();
			if (rigid) rigid->AddForce({ 0, 500, 0 });
		}
	}
}

