/*
	AppleObejct.cpp
	20260201 hanaue sho
*/
#include "AppleGenerator.h"
#include <cmath>

#include "ColliderComponent.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "Renderer.h"
#include "Texture.h"  // Texture::Load 既存

#include "AppleObject.h"
#include "Manager.h"
#include "Scene.h"
#include "SelfDestroyComponent.h"

void AppleGenerator::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	//tf->SetPosition({ 0,0,0 });
	tf->SetScale({ 2,2,2 });
	tf->SetEulerAngles({ 0,0,0 });
	m_BasePosition = tf->Position();

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

	// レイヤーセット
	SetPhysicsLayer(1);
}

void AppleGenerator::Update(float dt)
{
	GameObject::Update(dt);

	// 時間経過
	m_Timer += dt;

	if (m_Timer > m_Interval)
	{
		m_Timer = 0.0f;
		auto* pApple = Manager::GetScene()->AddGameObject<AppleObject>(1);
		pApple->Init();
		pApple->Transform()->SetPosition(Transform()->Position());
		pApple->Transform()->SetScale({ m_AppleScale, m_AppleScale , m_AppleScale });
		auto* destroy = pApple->AddComponent<SelfDestroyComponent>();
		destroy->SetTimer(20.0f);
	}

	// 移動
	m_Radian += dt;
	if (m_Radian > 3.14f) m_Radian -= 3.14f;
	Vector3 position = m_BasePosition;
	position += m_Move * std::sinf(m_Radian);
	Transform()->SetPosition(position);
}
