/*
	TentObject.cpp
	20260214  hanaue sho
*/
#include "TentObject.h"
#include "Renderer.h"
#include "MeshRendererComponent.h"
#include "ColliderComponent.h"
#include "ModelLoader.h"
#include "Texture.h"

void TentObject::Init()
{
	auto* tf = GetComponent<TransformComponent>();
	float s = 1.0f;
	tf->SetPosition({ 0, 0, 0 });
	tf->SetScale({ s, s, s });
	tf->SetEulerAngles({ 3.141592F / 2 * 3, 0, 0 });

	// Mesh
	std::vector<ID3D11ShaderResourceView*> srvs;
	auto* mf = AddComponent<MeshFilterComponent>();
	ModelLoader::LoadMeshFromFile(mf, "assets\\model\\tent_001.fbx", srvs, true);

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

	// Collider
	auto* col = AddComponent<Collider>();
	col->Init();
	col->SetBox({3,4.0f,10});
	col->SetModeSimulate(); 
}
