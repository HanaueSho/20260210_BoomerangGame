/*
	AppleObejct.cpp
	20260201 hanaue sho
*/
#include "AppleObject.h"

#include "ColliderComponent.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "Renderer.h"
#include "Texture.h"  // Texture::Load 既存

#include "Keyboard.h"

void AppleObject::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0,0,0 });
	tf->SetScale({ 1,1,1 });
	tf->SetEulerAngles({ 0,0,0 });

	// 2) MeshFilter を追加して頂点バッファ（4頂点の矩形）を作る
	auto* mf = AddComponent<MeshFilterComponent>();
	MeshFactory::CreateApple(mf, { 1, 36, 36 });

	// 3) Material を追加（シェーダ/テクスチャ/マテリアル）
	auto* mat = AddComponent<MaterialComponent>();

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* il = nullptr;
	Renderer::CreateVertexShader(&vs, &il, "shader\\VS_Lit.cso");
	Renderer::CreatePixelShader(&ps, "shader\\PS_Toon.cso");
	mat->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);

	// テクスチャセット
	ID3D11ShaderResourceView* srv = Texture::LoadAndRegisterKey("assets\\texture\\appleTexture.png");
	// サンプラーは Renderer::Init() で 0番に PSSetSamplers 済みなら null でも描ける
	mat->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);

	// マテリアルセット
	MaterialApp m{};
	m.diffuse = Vector4(1, 1, 1, 1);
	m.ambient = Vector4(1, 1, 1, 1);
	m.specular = Vector4(0, 0, 0, 1);
	m.textureEnable = TRUE;
	mat->SetMaterial(m);

	// 透明テクスチャの可能性が高いのでアルファブレンドに
	mat->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Opaque);

	// 4) MeshRenderer を追加（描画実行係）
	auto* mr = AddComponent<MeshRendererComponent>();

	// アウトラインマテリアル追加
	auto* matOutline = AddComponent<MaterialComponent>();
	Renderer::CreateVertexShader(&vs, &il, "shader\\VS_Outline.cso");
	Renderer::CreatePixelShader(&ps, "shader\\PS_Outline.cso");
	matOutline->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);
	matOutline->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Opaque);
	mr->SetOutlineMaterial(matOutline);

	// 物理を働かせたいのでコライダーなどを設定
	Collider* coll = AddComponent<Collider>();
	coll->SetSphere(1.1f);
	coll->SetModeSimulate();

	Rigidbody* rigid = AddComponent<Rigidbody>();
	rigid->SetGravityScale(1.0f);
	rigid->SetRestitution(0.0f);
	rigid->SetMass(1.0f);
	rigid->ComputeSphereInertia(1);

	// レイヤーセット
	SetPhysicsLayer(1);
}

void AppleObject::Update(float dt)
{
	GameObject::Update(dt);
	if (auto rigid = GetComponent<Rigidbody>())
	{	
		if (rigid->IsKinematic())
		{
			const float speed = 6.0f;
			if (Keyboard_IsKeyDown(KK_UP))
			{
				Vector3 pos = Transform()->Position();
				pos.z += speed * dt;
				Transform()->SetPosition(pos);
			}
			if (Keyboard_IsKeyDown(KK_DOWN))
			{
				Vector3 pos = Transform()->Position();
				pos.z += -speed * dt;
				Transform()->SetPosition(pos);
			}
			if (Keyboard_IsKeyDown(KK_LEFT))
			{
				Vector3 pos = Transform()->Position();
				pos.x -= speed * dt;
				Transform()->SetPosition(pos);
			}
			if (Keyboard_IsKeyDown(KK_RIGHT))
			{
				Vector3 pos = Transform()->Position();
				pos.x += speed * dt;
				Transform()->SetPosition(pos);
			}
			if (Keyboard_IsKeyDown(KK_SPACE))
			{
				Vector3 pos = Transform()->Position();
				pos.y += speed * dt;
				Transform()->SetPosition(pos);
			}
			if (Keyboard_IsKeyDown(KK_RIGHTSHIFT))
			{
				Vector3 pos = Transform()->Position();
				pos.y += -speed * dt;
				Transform()->SetPosition(pos);
			}
		}
	}
}
