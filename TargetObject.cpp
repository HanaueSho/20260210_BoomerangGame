/*
	TargetObject.cpp
	20260214  hanaue sho
*/
#include "TargetObject.h"

#include "ColliderComponent.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "Renderer.h"
#include "Texture.h"  // Texture::Load 既存
#include "TargetStateManagerComponent.h"
#include "TargetSpriteObject.h"
#include "MarkSpriteObject.h"
#include "LightComponent.h"

void TargetObject::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0, 0, 0 });
	float s = 4.0f;
	tf->SetScale({ s,s,s });
	tf->SetEulerAngles({ 0,0,0 });

	// 2) MeshFilter を追加して頂点バッファ（4頂点の矩形）を作る
	auto* mf = AddComponent<MeshFilterComponent>();
	MeshFactory::CreateSphere(mf, { 1, 6, 6 });

	// 3) Material を追加（シェーダ/テクスチャ/マテリアル）
	auto* mat = AddComponent<MaterialComponent>();

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* il = nullptr;
	Renderer::CreateVertexShader(&vs, &il, "shader\\VS_Lit.cso");
	Renderer::CreatePixelShader(&ps, "shader\\PS_TexUnlit.cso");
	mat->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);

	// テクスチャセット
	ID3D11ShaderResourceView* srv = Texture::LoadAndRegisterKey("assets\\texture\\whiteTexture.png");
	// サンプラーは Renderer::Init() で 0番に PSSetSamplers 済みなら null でも描ける
	mat->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);

	// マテリアルセット
	MaterialApp m{};
	m.diffuse = Vector4(1, 0, 0, 0.5f);
	m.ambient = Vector4(1, 1, 1, 1);
	m.specular = Vector4(0, 0, 0, 1);
	m.textureEnable = false;
	mat->SetMaterial(m);

	// 透明テクスチャの可能性が高いのでアルファブレンドに
	mat->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Alpha);

	// 4) MeshRenderer を追加（描画実行係）
	auto* mr = AddComponent<MeshRendererComponent>();

	// 物理を働かせたいのでコライダーなどを設定
	Collider* coll = AddComponent<Collider>();
	coll->SetSphere(1);
	coll->SetModeTrigger();

	Rigidbody* rigid = AddComponent<Rigidbody>();
	rigid->SetGravityScale(1.0f);
	rigid->SetRestitution(0.0f);
	rigid->SetMass(1.0f);
	rigid->ComputeSphereInertia(1);
	rigid->SetBodyTypeKinematic();

	// State
	auto* state = AddComponent<TargetStateManagerComponent>();
	state->Init();

	// ターゲットスプライト
	{
		m_pTargetSpriteObject = Manager::GetScene()->AddGameObject<TargetSpriteObject>(2);
		m_pTargetSpriteObject->Init();
		m_pTargetSpriteObject->Transform()->SetParent(Transform());
		float scale = 1.0f / 4.0f;
		m_pTargetSpriteObject->Transform()->SetScale({ scale, scale, scale });
		m_pTargetSpriteObject->Transform()->SetPosition({ 0, 5, 0 });
	}
	// マークオブジェクト
	{
		m_pMarkSpriteObject = Manager::GetScene()->AddGameObject<MarkSpriteObject>(2);
		m_pMarkSpriteObject->Init();
		m_pMarkSpriteObject->Transform()->SetParent(Transform());
		float scale = 1.0f / 1.5f;
		m_pMarkSpriteObject->Transform()->SetScale({ scale, scale, scale });
	}

	// LightComponent を追加
	auto* lc = AddComponent<LightComponent>();
	lc->SetDiffuse({ 1.0f, 1.0f, 1.0f });
	lc->SetIntensity(1);
	lc->SetRange(80);
	lc->SetDiffuse({ 0.8f, 0.2f, 0.2f });

	// タグ設定
	SetTag("Target");
}

void TargetObject::Uninit()
{
	if (m_pTargetSpriteObject) m_pTargetSpriteObject->RequestDestroy();
	if (m_pMarkSpriteObject) m_pMarkSpriteObject->RequestDestroy();

	GameObject::Uninit();
}
