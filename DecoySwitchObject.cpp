/*
	DecoySwitchObject.h
	20260217  hanaue sho
*/
#include "DecoySwitchObject.h"

#include "ColliderComponent.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "Renderer.h"
#include "Texture.h"  // Texture::Load 既存

#include "DecoySwitchComponent.h"

void DecoySwitchObject::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0,0,0 });
	float s = 5;
	tf->SetScale({ s,s,s });
	tf->SetEulerAngles({ 0,0,0 });

	// 2) MeshFilter を追加して頂点バッファ（4頂点の矩形）を作る
	auto* mf = AddComponent<MeshFilterComponent>();
	MeshFactory::CreateCylinder(mf, { 1.0f, 5, 8, 8 });

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
	float col = 0.5f;
	m.diffuse = Vector4(col, col, col / 2, 0.2f);
	m.ambient = Vector4(1, 1, 1, 1);
	m.specular = Vector4(0, 0, 0, 1);
	m.textureEnable = false;
	mat->SetMaterial(m);

	// 透明テクスチャの可能性が高いのでアルファブレンドに
	mat->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Alpha);

	// 4) MeshRenderer を追加（描画実行係）
	auto* mr = AddComponent<MeshRendererComponent>();

	// コライダー設定
	Collider* coll = AddComponent<Collider>();
	coll->SetSphere(1);
	coll->SetModeTrigger();

	// switch
	auto* decoySwitch = AddComponent<DecoySwitchComponent>();
	decoySwitch->Init();
}

void DecoySwitchObject::Update(float gameDt, float realDt)
{
	GameObject::Update(gameDt, realDt);

	const float rad = 1.5f * gameDt;
	Transform()->Rotate({ 0, rad, 0 });

}
