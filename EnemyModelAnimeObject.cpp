/*
	EnemyModelAnimeObject.cpp
	20260214  hanaue sho
*/
#include "EnemyModelAnimeObject.h"
#include "Renderer.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "ColliderComponent.h"
#include "AnimatorComponent.h"
#include "BallJointComponent.h"
#include "HingeJointComponent.h"
#include "AnimationClip.h"
#include "SkinMatrixProviderComponent.h"
#include "AnimatorController.h"
#include "BoneManager.h"
#include "TargetObject.h"

#include "Keyboard.h"

#include "Texture.h"

void EnemyModelAnimeObject::Init()
{
	auto* tf = GetComponent<TransformComponent>();
	float s = 1.0f;
	tf->SetPosition({ 0, 0.0f, 0 });
	tf->SetScale({ s, s, s });
	tf->SetEulerAngles({ 3.141592F / 2 * 3, 0, 0 });

	// Mesh + Skeleton
	std::vector<ID3D11ShaderResourceView*> srvs;
	auto* mf = AddComponent<MeshFilterComponent>();
	switch (m_Type)
	{
	case Type::Melee:
		m_Skeleton = ModelLoader::BuildSkeletonFromFile("assets\\model\\enemy0_001.fbx");
		ModelLoader::LoadSkinnedMeshFromFile(mf, "assets\\model\\enemy0_001.fbx", m_Skeleton, srvs, true);
		break;
	case Type::Shot:
		m_Skeleton = ModelLoader::BuildSkeletonFromFile("assets\\model\\enemy1_001.fbx");
		ModelLoader::LoadSkinnedMeshFromFile(mf, "assets\\model\\enemy1_001.fbx", m_Skeleton, srvs, true);
		break;
	case Type::Barrier:
		m_Skeleton = ModelLoader::BuildSkeletonFromFile("assets\\model\\enemy2_001.fbx");
		ModelLoader::LoadSkinnedMeshFromFile(mf, "assets\\model\\enemy2_001.fbx", m_Skeleton, srvs, true);
		break;
	default:
		assert(false && "No Define Enemy Type");
		break;
	}

	// Animator
	auto* animator = AddComponent<AnimatorComponent>();
	switch (m_Type)
	{
	case Type::Melee:
		m_ClipIdle   = ModelLoader::BuildAnimationClipFromFile("assets\\model\\enemy0_001_Idle.fbx", m_Skeleton, 0);
		m_ClipReady  = ModelLoader::BuildAnimationClipFromFile("assets\\model\\enemy0_001_Ready.fbx", m_Skeleton, 0);
		m_ClipAttack = ModelLoader::BuildAnimationClipFromFile("assets\\model\\enemy0_001_Attack.fbx", m_Skeleton, 0);
		m_ClipDead   = ModelLoader::BuildAnimationClipFromFile("assets\\model\\enemy0_001_Dead.fbx", m_Skeleton, 0);
		break;
	case Type::Shot:
		m_ClipIdle   = ModelLoader::BuildAnimationClipFromFile("assets\\model\\enemy1_001_Idle.fbx", m_Skeleton, 0);
		//m_ClipReady  = ModelLoader::BuildAnimationClipFromFile("assets\\model\\enemy1_001_Ready.fbx", m_Skeleton, 0);
		m_ClipAttack = ModelLoader::BuildAnimationClipFromFile("assets\\model\\enemy1_001_Attack.fbx", m_Skeleton, 0);
		m_ClipDead   = ModelLoader::BuildAnimationClipFromFile("assets\\model\\enemy1_001_Dead.fbx", m_Skeleton, 0);
		break;
	case Type::Barrier:
		m_ClipIdle   = ModelLoader::BuildAnimationClipFromFile("assets\\model\\enemy2_001_Idle.fbx", m_Skeleton, 0);
		//m_ClipReady  = ModelLoader::BuildAnimationClipFromFile("assets\\model\\enemy2_001_Ready.fbx", m_Skeleton, 0);
		//m_ClipAttack = ModelLoader::BuildAnimationClipFromFile("assets\\model\\enemy2_001_Attack.fbx", m_Skeleton, 0);
		//m_ClipDead   = ModelLoader::BuildAnimationClipFromFile("assets\\model\\enemy2_001_Dead.fbx", m_Skeleton, 0);
		break;
	default:
		assert(false && "No Define Enemy Type");
		break;
	}
	assert(m_ClipIdle.duration > 0.0f);
	assert(!m_ClipIdle.tracks.empty());
	animator->SetSkeleton(&m_Skeleton);
	animator->SetClip(&m_ClipIdle);
	animator->SetSpeed(1.3f);
	animator->SetSpeed(4.0f);

	// AnimatorController
	m_pController = new AnimatorController();
	m_pController->m_Locomotion.clipA = &m_ClipIdle;
	m_pController->m_Locomotion.clipB = &m_ClipReady;
	animator->SetController(m_pController);
	animator->SetIsLocomotion(false);

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
	auto* matOutline = AddComponent<MaterialComponent>();
	Renderer::CreateVertexShader(&vs, &il, "shader\\VS_SkinnedOutline.cso", Renderer::VertexLayoutType::Skinned);
	Renderer::CreatePixelShader(&ps, "shader\\PS_Outline.cso");
	matOutline->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);
	matOutline->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Opaque);
	mr->SetOutlineMaterial(matOutline);

	// ----- ターゲット -----
	GameObject* bone = m_pBoneManager->GetBoneObject(0);
	GameObject* target = Manager::GetScene()->AddGameObject<TargetObject>(1);
	target->Init();
	target->Transform()->SetParent(bone->Transform());
	target->Transform()->SetPosition({ 0, 10, 0 });

}

void EnemyModelAnimeObject::Uninit()
{
	delete m_pBoneManager;
	m_pBoneManager = nullptr;
	GameObject::Uninit();
}

void EnemyModelAnimeObject::Update(float dt)
{
	GameObject::Update(dt);

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

void EnemyModelAnimeObject::SetIsLocomotion(bool b)
{
	auto animator = GetComponent<AnimatorComponent>();
	animator->SetIsLocomotion(b);
}
void EnemyModelAnimeObject::SetBlendParam(float blend)
{
	auto animator = GetComponent<AnimatorComponent>();
	animator->SetBlendParam(blend);
}
void EnemyModelAnimeObject::SetSpeedAnime(float speed)
{
	auto animator = GetComponent<AnimatorComponent>();
	animator->SetSpeed(speed);
}
void EnemyModelAnimeObject::PlayAnimeIdle()
{
	auto animator = GetComponent<AnimatorComponent>();
	animator->CrossFadeFromCurrentPose(&m_ClipIdle, 0.1f, true);
}
void EnemyModelAnimeObject::PlayAnimeReady()
{
	auto animator = GetComponent<AnimatorComponent>();
	animator->CrossFadeFromCurrentPose(&m_ClipReady, 0.1f, false);
}
void EnemyModelAnimeObject::PlayAnimeAttack()
{
	auto animator = GetComponent<AnimatorComponent>();
	animator->CrossFadeFromCurrentPose(&m_ClipAttack, 0.1f, true);
}
void EnemyModelAnimeObject::PlayAnimeDead()
{
	auto animator = GetComponent<AnimatorComponent>();
	animator->CrossFadeFromCurrentPose(&m_ClipDead, 0.1f, false);
}

void EnemyModelAnimeObject::SetupBones()
{
	
}

