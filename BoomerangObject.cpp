/*
	BoomerangObject.cpp
	20260211  hanaue sho
	ブーメランのオブジェクト
*/
#include "BoomerangObject.h"
#include "Renderer.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "ColliderComponent.h"
#include "ModelLoader.h"
#include "Keyboard.h"
#include "Texture.h"

#include "BoneManager.h"
#include "PlayerObject.h"


void BoomerangObject::Init()
{
	auto* tf = GetComponent<TransformComponent>();
	float s = 1.0f;
	tf->SetPosition({ 0, 0, 0 });
	tf->SetScale({ s, s, s });
	tf->SetEulerAngles({ 3.141592F / 2 * 0, 0, 0 });

	// Mesh
	std::vector<ID3D11ShaderResourceView*> srvs;
	auto* mf = AddComponent<MeshFilterComponent>();
	ModelLoader::LoadMeshFromFile(mf, "assets\\model\\boomerang_001.fbx", srvs, true);

	// MeshRenderer
	auto* mr = AddComponent<MeshRendererComponent>();

	// Slot 数
	const int slotCount = mf->MaterialSlotCount();
	mr->ResizeMaterialSlots(slotCount);

	// 共通シェーダー
	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* il = nullptr;
	Renderer::CreateVertexShader(&vs, &il, "shader\\VS_Lit.cso", Renderer::VertexLayoutType::Basic);
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
	Renderer::CreateVertexShader(&vs, &il, "shader\\VS_Outline.cso", Renderer::VertexLayoutType::Basic);
	Renderer::CreatePixelShader(&ps, "shader\\PS_Outline.cso");
	matOutline->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);
	matOutline->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Opaque);
	mr->SetOutlineMaterial(matOutline);
}

void BoomerangObject::Update(float dt)
{
	GameObject::Update(dt);


	switch (m_State)
	{
	case State::Idle:
			break;
	case State::Aim:
			break;
	case State::Throw:
			break;
	case State::Back:
			break;
	}

}

void BoomerangObject::ChangeState(State newState)
{
	m_State = newState;
	switch (newState)
	{
	case State::Idle:
		break;
	case State::Aim:
		break;
	case State::Throw:
		break;
	case State::Back:
		break;
	}
}

void BoomerangObject::SetPlayerObject(GameObject* player)
{
	m_pPlayerObject = player;
	// 背骨ボーン
	GameObject* bone = dynamic_cast<PlayerObject*>(m_pPlayerObject)->GetBoneObject(2);
	m_pBoneSpineObject = bone;
	//Transform()->SetParent(bone->Transform());
	m_pBoneHandObject = dynamic_cast<PlayerObject*>(m_pPlayerObject)->GetBoneObject(23);
	Transform()->SetParent(m_pBoneHandObject->Transform());
	Transform()->SetLocalPosition(m_OffsetHandPosition);
	Transform()->SetEulerAngles(m_OffsetHandRotation);
}
