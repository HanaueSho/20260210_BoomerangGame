/*
	BulletObject.h
	20260215  hanaue sho
*/
#include "BulletObject.h"

#include "ColliderComponent.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "Renderer.h"
#include "Texture.h"  // Texture::Load 既存

#include "BulletStateManagerComponent.h"
#include "EnemyAttackObject.h"

void BulletObject::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0,0,0 });
	float s = 5;
	tf->SetScale({ s, s, s });
	tf->SetEulerAngles({ 0, 0, 0 });

	// 2) MeshFilter を追加して頂点バッファ（4頂点の矩形）を作る
	auto* mf = AddComponent<MeshFilterComponent>();
	MeshFactory::CreateSphere(mf, { 1, 36, 36 });

	// 3) Material を追加（シェーダ/テクスチャ/マテリアル）
	auto* mat = AddComponent<MaterialComponent>();

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* il = nullptr;
	Renderer::CreateVertexShader(&vs, &il, "shader\\VS_Lit.cso");
	Renderer::CreatePixelShader(&ps, "shader\\PS_TexUnlit.cso");
	mat->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);

	// テクスチャセット
	ID3D11ShaderResourceView* srv = Texture::LoadAndRegisterKey("assets\\texture\\Apple.png");
	// サンプラーは Renderer::Init() で 0番に PSSetSamplers 済みなら null でも描ける
	mat->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);

	// マテリアルセット
	MaterialApp m{};
	m.diffuse = Vector4(0.8f, 0.2f, 0.2f, 0.8f);
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
	coll->SetSphere(1.0f);
	coll->SetModeTrigger();

	// BulletStateManagerComponent
	auto* state = AddComponent<BulletStateManagerComponent>();

	// EnemyAttackObject
	auto* attack = Manager::GetScene()->AddGameObject<EnemyAttackObject>(1);
	attack->Init();
	attack->Transform()->SetParent(Transform());

	// ターゲットにしちゃう
	SetTag("Bullet");
}

