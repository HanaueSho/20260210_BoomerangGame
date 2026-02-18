/*
	ChainObject.cpp
	20260219  hanaue sho
*/
#include "ChainObject.h"
#include "ModelLoader.h"
#include "Renderer.h"
#include "MeshRendererComponent.h"
#include "ColliderComponent.h"
#include "RigidbodyComponent.h"
#include "BallJointComponent.h"
#include "Texture.h"
#include "Keyboard.h"

void ChainObject::Init()
{
	auto* tf = GetComponent<TransformComponent>();
	float s = 1.0f;
	tf->SetPosition({ 0, 0, 0 });
	tf->SetScale({ s, s, s });
	tf->SetEulerAngles({ 3.141592F / 2 * 0, 0, 0 });

	// Mesh
	std::vector<ID3D11ShaderResourceView*> srvs;
	auto* mf = AddComponent<MeshFilterComponent>();
	ModelLoader::LoadMeshFromFile(mf, "assets\\model\\chain_001.fbx", srvs, true);

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
	m.diffuse = Vector4(0.2f, 0.2, 0.2f, 1);
	m.ambient = Vector4(1, 1, 1, 1);
	m.textureEnable = false;

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

	// Collider
	auto* col = AddComponent<Collider>();
	col->Init();
	col->SetSphere(1.1f);
	col->SetModeSimulate();

	//Rigidbody
	auto* rigid = AddComponent<Rigidbody>();
	rigid->SetGravityScale(5.0f);
	rigid->SetRestitution(0.0f);
	rigid->SetMass(20.0f);
	rigid->ComputeSphereInertia(1);
	rigid->SetBodyTypeKinematic();
}

void ChainObject::Update(float gameDt, float realDt)
{
	GameObject::Update(gameDt, realDt);

	//auto* rigid = GetComponent<Rigidbody>();
	//if (rigid->IsKinematic())
	//{
	//	Vector3 position = Transform()->Position();
	//	if (Keyboard_IsKeyDown(KK_LEFT))
	//		position.x -= gameDt * 5;
	//	if (Keyboard_IsKeyDown(KK_RIGHT))
	//		position.x += gameDt * 5;
	//	if (Keyboard_IsKeyDown(KK_UP))
	//		position.y += gameDt * 5;
	//	if (Keyboard_IsKeyDown(KK_DOWN))
	//		position.y -= gameDt * 5;
	//	Transform()->SetPosition(position);
	//}
}

void ChainObject::CreateChains(int num)
{
	if (num < 1) return;
	
	Vector3 position = Transform()->Position();
	Vector3 scale = Transform()->LossyScale();
	Vector3 axis = -Transform()->Up();
	position += axis.normalized() * 3.0f;

	auto chain = Manager::GetScene()->AddGameObject<ChainObject>(1);
	chain->Init();
	chain->Transform()->SetPosition(position);
	chain->Transform()->SetScale(scale);
	chain->GetComponent<Rigidbody>()->SetBodyTypeDynamic();
	auto* joint = chain->AddComponent<BallJointComponent>();
	joint->SetOtherObject(this);
	joint->SetLocalAnchorB(-chain->Transform()->Up() * 3);
	joint->RegisterToPhysicsSystem();

	chain->CreateChains(num - 1);
}
