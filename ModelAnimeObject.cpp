/*
	ModelAnimeObject.h
	20260208  hanaue sho
*/
#include "ModelAnimeObject.h"
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

#include "Keyboard.h"

#include "Texture.h"

void ModelAnimeObject::Init()
{
	auto* tf = GetComponent<TransformComponent>();
	float s = 1.0f;
	tf->SetPosition({ 0, -5.5f, 0 });
	tf->SetScale({ s, s, s });
	tf->SetEulerAngles({ 3.141592F / 2 * 3, 0, 0 });

	// Mesh + Skeleton
	std::vector<ID3D11ShaderResourceView*> srvs;
	auto* mf = AddComponent<MeshFilterComponent>();
	m_Skeleton = ModelLoader::BuildSkeletonFromFile("assets\\model\\player_001.fbx");
	ModelLoader::LoadSkinnedMeshFromFile(mf, "assets\\model\\player_001.fbx", m_Skeleton, srvs, true);

	// Animator
	auto* animator = AddComponent<AnimatorComponent>();
	m_ClipIdle = ModelLoader::BuildAnimationClipFromFile("assets\\model\\player_001_Idle.fbx", m_Skeleton, 0);
	m_ClipWalk = ModelLoader::BuildAnimationClipFromFile("assets\\model\\player_001_Walk.fbx", m_Skeleton, 0);
	m_ClipRun  = ModelLoader::BuildAnimationClipFromFile("assets\\model\\player_001_Run.fbx", m_Skeleton, 0);
	m_ClipJump = ModelLoader::BuildAnimationClipFromFile("assets\\model\\player_001_Jump.fbx", m_Skeleton, 0);
	assert(m_ClipIdle.duration > 0.0f);
	assert(!m_ClipIdle.tracks.empty());
	animator->SetSkeleton(&m_Skeleton);
	animator->SetClip(&m_ClipIdle);
	animator->SetSpeed(1.3f);

	// AnimatorController
	m_pController = new AnimatorController();
	m_pController->m_Locomotion.clipA = &m_ClipIdle;
	m_pController->m_Locomotion.clipB = &m_ClipRun;
	animator->SetController(m_pController);
	animator->SetIsLocomotion(true);

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

}

void ModelAnimeObject::Uninit()
{
	delete m_pBoneManager;
	m_pBoneManager = nullptr;
	GameObject::Uninit();
}

void ModelAnimeObject::Update(float dt)
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


	if (Keyboard_IsKeyDownTrigger(KK_D1))
	{
		auto animator = GetComponent<AnimatorComponent>();
		animator->CrossFadeFromCurrentPose(&m_ClipIdle, 1.0f, true);
		animator->SetSpeed(1.0f);
	}
	if (Keyboard_IsKeyDownTrigger(KK_D2))
	{
		auto animator = GetComponent<AnimatorComponent>();
		animator->CrossFadeFromCurrentPose(&m_ClipWalk, 1.0f, true);
		animator->SetSpeed(1.3f);
	}
	if (Keyboard_IsKeyDownTrigger(KK_D3))
	{
		auto animator = GetComponent<AnimatorComponent>();
		animator->CrossFadeFromCurrentPose(&m_ClipRun, 1.0f, true);
		animator->SetSpeed(1.8f);
	}
	if (Keyboard_IsKeyDownTrigger(KK_D4))
	{
		auto animator = GetComponent<AnimatorComponent>();
		animator->CrossFadeFromCurrentPose(&m_ClipJump, 0.2f, false);
		animator->SetSpeed(1.0f);
	}
	/*if (Keyboard_IsKeyDown(KK_O))
	{
		m_SpeedParam -= 1.0f * dt; if (m_SpeedParam < 0.0f) m_SpeedParam = 0.0f;
		auto animator = GetComponent<AnimatorComponent>();
		animator->SetBlendParam(m_SpeedParam);
	}
	if (Keyboard_IsKeyDown(KK_P))
	{
		m_SpeedParam += 1.0f * dt; if (m_SpeedParam > 1.0f) m_SpeedParam = 1.0f;
		auto animator = GetComponent<AnimatorComponent>();
		animator->SetBlendParam(m_SpeedParam);
	}*/
}

void ModelAnimeObject::SetIsLocomotion(bool b)
{
	auto animator = GetComponent<AnimatorComponent>();
	animator->SetIsLocomotion(b);
}
void ModelAnimeObject::SetBlendParam(float blend)
{
	auto animator = GetComponent<AnimatorComponent>();
	animator->SetBlendParam(blend);
}
void ModelAnimeObject::SetSpeedAnime(float speed)
{
	auto animator = GetComponent<AnimatorComponent>();
	animator->SetSpeed(speed);
}
void ModelAnimeObject::PlayAnimeIdle()
{
	auto animator = GetComponent<AnimatorComponent>();
	animator->CrossFade(&m_ClipIdle, 0.1f, true);
}
void ModelAnimeObject::PlayAnimeJump()
{
	auto animator = GetComponent<AnimatorComponent>();
	animator->CrossFade(&m_ClipJump, 0.2f, false);
}

void ModelAnimeObject::SetupBones()
{
	auto* animator = GetComponent<AnimatorComponent>();

	// まずは骨の位置を揃える
	animator->EvaluateNow();
	m_pBoneManager->UpdateFromAnimator(animator->ModelMatrixes(), nullptr);

	// 骨の設定
	const int indexHip = 0; // Spin0..2 : 1..3
	const int indexNeck = 4;
	const int indexHead = 5;
	const int indexEar0L = 6;
	const int indexEar2L = 8;
	const int indexEar0R = 9;
	const int indexEar2R = 11;
	const int indexMuffler0 = 12;
	const int indexMuffler7 = 19;
	const int indexShoulderL = 20; // UpperArmL: 21, LowerArmL: 22
	const int indexHandL = 23;
	const int indexFinger0L = 24;
	const int indexFinger1L = 25;
	const int indexThumb0L = 26;
	const int indexThumb2L = 28;
	const int indexShoulderR = 29;// UpperArmR: 30, LowerArmR: 31
	const int indexHandR = 32;
	const int indexFinger0R = 33;
	const int indexFinger1R = 34;
	const int indexThumb0R = 35;
	const int indexThumb2R = 37;
	const int indexTail0 = 38;
	const int indexTail7 = 45;
	const int indexUpperLegL = 46; // LowerLegL: 47
	const int indexFootL = 48;
	const int indexToeL = 49;
	const int indexUpperLegR = 50; // LowerLegR: 51
	const int indexFootR = 52;
	const int indexToeR = 53;

	// Hip ~ Neck -----
	for (int i = indexHip; i <= indexNeck; i++)
	{
		m_pBoneManager->SetBonePhysics(i, true, false);
		auto* bone = m_pBoneManager->GetBoneObject(i);
		auto* coll = bone->GetComponent<Collider>();
		if (i == indexHip)coll->SetSphere(0.5f);
		if (i == indexHip + 1) { coll->SetSphere(0.5f); }
		if (i == indexHip + 2) { coll->SetSphere(0.7f); }
		if (i == indexHip + 3) { coll->SetSphere(0.7f); coll->SetOffsetPosition({ 0, 0.50f, 0.0f }); }
		if (i == indexHip + 4) { coll->SetSphere(0.3f); coll->SetOffsetPosition({ 0, 0.50f, 0.0f }); }
	}
	// Head -----
	{
		m_pBoneManager->SetBonePhysics(indexHead, true, false);
		auto* bone = m_pBoneManager->GetBoneObject(indexHead);
		auto* coll = bone->GetComponent<Collider>();
		coll->SetSphere(0.7f);
		coll->SetOffsetPosition({ 0, 0.78f, 0.3f });
	}
	// Ear -----
	for (int i = indexEar0L; i <= indexEar2L; i++)
	{
		if (i == indexEar0L) m_pBoneManager->SetBonePhysics(i, true, false);
		else { m_pBoneManager->SetBonePhysics(i, true, true); m_pProvider->SetBonePhysics(i, true); }
		auto* bone = m_pBoneManager->GetBoneObject(i);
		auto* coll = bone->GetComponent<Collider>();
		if (i == indexEar0L) { coll->SetSphere(0.2f);		 coll->SetOffsetPosition({ 0, 0.00f, 0.0f }); }
		if (i == indexEar0L + 1) { coll->SetCapsule(0.2f, 0.4f); coll->SetOffsetPosition({ 0, 0.00f, 0.0f }); }
		if (i == indexEar0L + 2) { coll->SetCapsule(0.1f, 0.2f); coll->SetOffsetPosition({ 0, 0.12f, -0.2f }); }
	}
	for (int i = indexEar0R; i <= indexEar2R; i++)
	{
		if (i == indexEar0R) m_pBoneManager->SetBonePhysics(i, true, false);
		else { m_pBoneManager->SetBonePhysics(i, true, true); m_pProvider->SetBonePhysics(i, true); }
		auto* bone = m_pBoneManager->GetBoneObject(i);
		auto* coll = bone->GetComponent<Collider>();
		if (i == indexEar0R) { coll->SetSphere(0.2f);		 coll->SetOffsetPosition({ 0, 0.00f, 0.0f }); }
		if (i == indexEar0R + 1) { coll->SetCapsule(0.2f, 0.4f); coll->SetOffsetPosition({ 0, 0.00f, 0.0f }); }
		if (i == indexEar0R + 2) { coll->SetCapsule(0.1f, 0.2f); coll->SetOffsetPosition({ 0, 0.12f, -0.2f }); }
	}
	for (int i = indexEar0L; i <= indexEar2L; i++) // ジョイント設定
	{
		auto* bone = m_pBoneManager->GetBoneObject(i);
		if (i == indexEar0L)
		{
			auto* joint = bone->AddComponent<BallJointComponent>();
			joint->SetOtherObject(m_pBoneManager->GetBoneObject(i + 1));
			joint->SetLocalAnchorA({ 0, 0.4f, 0 });
			joint->SetLocalAnchorB({ 0, -0.4f, 0 });
			joint->RegisterToPhysicsSystem();
		}
		if (i == indexEar0L + 1)
		{
			auto* joint = bone->AddComponent<BallJointComponent>();
			joint->SetOtherObject(m_pBoneManager->GetBoneObject(i + 1));
			joint->SetLocalAnchorA({ 0, 0.4f, 0 });
			joint->SetLocalAnchorB({ 0, -0.3f, 0.0f });
			joint->RegisterToPhysicsSystem();
			auto* rigid = bone->GetComponent<Rigidbody>(); // 上向きに落とす
			rigid->SetGravityScale(-0.5f);
		}
		if (i == indexEar0L + 2)
		{
			auto* rigid = bone->GetComponent<Rigidbody>(); // 上向きに落とす
			rigid->SetGravityScale(-0.5f);
		}
	}
	for (int i = indexEar0R; i <= indexEar2R; i++) // ジョイント設定
	{
		auto* bone = m_pBoneManager->GetBoneObject(i);
		if (i == indexEar0R)
		{
			auto* joint = bone->AddComponent<BallJointComponent>();
			joint->SetOtherObject(m_pBoneManager->GetBoneObject(i + 1));
			joint->SetLocalAnchorA({ 0, 0.4f, 0 });
			joint->SetLocalAnchorB({ 0, -0.4f, 0 });
			joint->RegisterToPhysicsSystem();
		}
		if (i == indexEar0R + 1)
		{
			auto* joint = bone->AddComponent<BallJointComponent>();
			joint->SetOtherObject(m_pBoneManager->GetBoneObject(i + 1));
			joint->SetLocalAnchorA({ 0, 0.4f, 0 });
			joint->SetLocalAnchorB({ 0, -0.3f, 0.0f });
			joint->RegisterToPhysicsSystem();
			auto* rigid = bone->GetComponent<Rigidbody>(); // 上向きに落とす
			rigid->SetGravityScale(-0.5f);
		}
		if (i == indexEar0R + 2)
		{
			auto* rigid = bone->GetComponent<Rigidbody>(); // 上向きに落とす
			rigid->SetGravityScale(-0.5f);
		}
	}
	// Shoulder ~ Hand -----
	for (int i = indexShoulderL; i <= indexHandL; i++)
	{
		m_pBoneManager->SetBonePhysics(i, true, false);
		auto* bone = m_pBoneManager->GetBoneObject(i);
		auto* coll = bone->GetComponent<Collider>();
		if (i == indexShoulderL) { coll->SetCapsule(0.2f, 0.4f); coll->SetOffsetPosition({ 0, 0.2f, 0.0f }); }
		if (i == indexShoulderL + 1) { coll->SetCapsule(0.2f, 1.4f); coll->SetOffsetPosition({ 0, 0.9f, 0.0f }); }
		if (i == indexShoulderL + 2) { coll->SetCapsule(0.2f, 1.2f); coll->SetOffsetPosition({ 0, 0.9f, 0.0f }); }
		if (i == indexShoulderL + 3) { coll->SetSphere(0.15f); }
	}
	for (int i = indexShoulderR; i <= indexHandR; i++)
	{
		m_pBoneManager->SetBonePhysics(i, true, false);
		auto* bone = m_pBoneManager->GetBoneObject(i);
		auto* coll = bone->GetComponent<Collider>();
		if (i == indexShoulderR) { coll->SetCapsule(0.2f, 0.4f); coll->SetOffsetPosition({ 0, 0.2f, 0.0f }); }
		if (i == indexShoulderR + 1) { coll->SetCapsule(0.2f, 1.4f); coll->SetOffsetPosition({ 0, 0.9f, 0.0f }); }
		if (i == indexShoulderR + 2) { coll->SetCapsule(0.2f, 1.2f); coll->SetOffsetPosition({ 0, 0.9f, 0.0f }); }
		if (i == indexShoulderR + 3) { coll->SetSphere(0.15f); }
	}
	// Finger ~ Thumb -----
	for (int i = indexFinger0L; i <= indexThumb2L; i++)
	{
		m_pBoneManager->SetBonePhysics(i, true, false);
		auto* bone = m_pBoneManager->GetBoneObject(i);
		auto* coll = bone->GetComponent<Collider>();
		if (i == indexFinger0L) { coll->SetSphere(0.15f); coll->SetOffsetPosition({ 0, 0.05f, 0.0f }); }
		if (i == indexFinger0L + 1) { coll->SetSphere(0.10f); coll->SetOffsetPosition({ 0, 0.10f, 0.0f }); }
		if (i == indexFinger0L + 2) { coll->SetSphere(0.10f); coll->SetOffsetPosition({ 0, 0.00f, 0.0f }); }
		if (i == indexFinger0L + 3) { coll->SetSphere(0.10f); coll->SetOffsetPosition({ 0, 0.00f, 0.0f }); }
		if (i == indexFinger0L + 4) { coll->SetSphere(0.05f); coll->SetOffsetPosition({ 0, 0.10f, 0.0f }); }
	}
	for (int i = indexFinger0R; i <= indexThumb2R; i++)
	{
		m_pBoneManager->SetBonePhysics(i, true, false);
		auto* bone = m_pBoneManager->GetBoneObject(i);
		auto* coll = bone->GetComponent<Collider>();
		if (i == indexFinger0R) { coll->SetSphere(0.15f); coll->SetOffsetPosition({ 0, 0.05f, 0.0f }); }
		if (i == indexFinger0R + 1) { coll->SetSphere(0.10f); coll->SetOffsetPosition({ 0, 0.10f, 0.0f }); }
		if (i == indexFinger0R + 2) { coll->SetSphere(0.10f); coll->SetOffsetPosition({ 0, 0.00f, 0.0f }); }
		if (i == indexFinger0R + 3) { coll->SetSphere(0.10f); coll->SetOffsetPosition({ 0, 0.00f, 0.0f }); }
		if (i == indexFinger0R + 4) { coll->SetSphere(0.05f); coll->SetOffsetPosition({ 0, 0.10f, 0.0f }); }
	}
	// Muffler -----
	for (int i = indexMuffler0; i <= indexMuffler7; i++)
	{
		if (i == indexMuffler0) m_pBoneManager->SetBonePhysics(i, true, false);
		else { m_pBoneManager->SetBonePhysics(i, true, true); m_pProvider->SetBonePhysics(i, true); }
		auto* bone = m_pBoneManager->GetBoneObject(i);
		auto* coll = bone->GetComponent<Collider>();
		coll->SetBox({ 0.2f, 0.2f, 0.03f });
		auto* rigid = bone->GetComponent<Rigidbody>();
		rigid->ComputeBoxInertia({ 0.2f, 0.2f, 0.03f });
	}
	for (int i = indexMuffler0; i <= indexMuffler7; i++) // ジョイント設定
	{
		auto* bone = m_pBoneManager->GetBoneObject(i);
		if (1 == indexMuffler0)
		{
			auto* joint = bone->AddComponent<HingeJointComponent>();
			joint->SetOtherObject(m_pBoneManager->GetBoneObject(i + 1));
			joint->SetLocalAnchorA({ 0, 0.6f, 0 });
			joint->SetLocalAnchorB({ 0, 0.0f, 0 });
			joint->SetEnableLimit(true);
			joint->SetLimitMin(-1.0f);
			joint->SetLimitMax(0.0f);
			joint->SetAxis({ 1, 0, 0 });
			joint->RegisterToPhysicsSystem();
		}
		else if (i != indexMuffler7)
		{
			auto* joint = bone->AddComponent<HingeJointComponent>();
			joint->SetOtherObject(m_pBoneManager->GetBoneObject(i + 1));
			joint->SetLocalAnchorA({ 0, 0.6f, 0 });
			joint->SetLocalAnchorB({ 0, 0.0f, 0 });
			joint->SetEnableLimit(true);
			joint->SetLimitMin(-1.0f);
			joint->SetLimitMax(0.0f);
			joint->SetAxis({ 1, 0, 0 });
			joint->RegisterToPhysicsSystem();
		}
	}
	// Tail -----
	for (int i = indexTail0; i <= indexTail7; i++)
	{
		if (i == indexTail0) m_pBoneManager->SetBonePhysics(i, true, false);
		else { m_pBoneManager->SetBonePhysics(i, true, true); m_pProvider->SetBonePhysics(i, true); }
		auto* bone = m_pBoneManager->GetBoneObject(i);
		auto* coll = bone->GetComponent<Collider>();
		coll->SetBox({ 0.2, 0.2, 0.1 });
		coll->SetOffsetPosition({ 0, 0, 0 });
	}
	for (int i = indexTail0; i <= indexTail7; i++) // ジョイント設定
	{
		auto* bone = m_pBoneManager->GetBoneObject(i);
		if (i != indexTail7)
		{
			auto* joint = bone->AddComponent<BallJointComponent>();
			joint->SetOtherObject(m_pBoneManager->GetBoneObject(i + 1));
			joint->SetLocalAnchorA({ 0, 0.3f, 0 });
			joint->SetLocalAnchorB({ 0, -0.3f, 0 });
			joint->RegisterToPhysicsSystem();
		}
	}
	// Leg ~ Toe
	for (int i = indexUpperLegL; i <= indexToeL; i++)
	{
		m_pBoneManager->SetBonePhysics(i, true, false);
		auto* bone = m_pBoneManager->GetBoneObject(i);
		auto* coll = bone->GetComponent<Collider>();
		if (i == indexUpperLegL) { coll->SetCapsule(0.5f, 2.6f); coll->SetOffsetPosition({ 0, 1.30f, 0.0f }); }
		if (i == indexUpperLegL + 1) { coll->SetCapsule(0.4f, 1.0f); coll->SetOffsetPosition({ 0, 1.10f, 0.0f }); }
		if (i == indexUpperLegL + 2) { coll->SetCapsule(0.3f, 1.0f); coll->SetOffsetPosition({ 0, 1.00f, 0.0f }); }
		if (i == indexUpperLegL + 3) { coll->SetCapsule(0.1f, 0.4f); coll->SetOffsetPosition({ 0, 0.60f, 0.0f }); }
	}
	for (int i = indexUpperLegR; i <= indexToeR; i++)
	{
		m_pBoneManager->SetBonePhysics(i, true, false);
		auto* bone = m_pBoneManager->GetBoneObject(i);
		auto* coll = bone->GetComponent<Collider>();
		if (i == indexUpperLegR) { coll->SetCapsule(0.5f, 2.6f); coll->SetOffsetPosition({ 0, 1.30f, 0.0f }); }
		if (i == indexUpperLegR + 1) { coll->SetCapsule(0.4f, 1.0f); coll->SetOffsetPosition({ 0, 1.10f, 0.0f }); }
		if (i == indexUpperLegR + 2) { coll->SetCapsule(0.3f, 1.0f); coll->SetOffsetPosition({ 0, 1.00f, 0.0f }); }
		if (i == indexUpperLegR + 3) { coll->SetCapsule(0.1f, 0.4f); coll->SetOffsetPosition({ 0, 0.60f, 0.0f }); }
	}

	//ボーンの Rigidbody のレイヤーを設定する
	for (int i = 0; i < m_pBoneManager->GetBonesSize(); i++)
	{
		auto* bone = m_pBoneManager->GetBoneObject(i);
		bone->SetPhysicsLayer(30);
	}
}

