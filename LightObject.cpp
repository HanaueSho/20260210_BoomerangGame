/*
	lightObject.h
	20251110 hanaue sho
*/
#include "LightObject.h"

#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "LightComponent.h"
#include "ColliderComponent.h"
#include "RigidbodyComponent.h"
#include "Renderer.h"
#include "Texture.h"  // Texture::Load 既存

void LightObject::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 2,0,0 });
	tf->SetScale({ 1.0f, 1.0f, 1.0f });
	tf->SetEulerAngles({ 0,0,0 });

	// 2) MeshFilter を追加して頂点バッファを作る
	auto* mf = AddComponent<MeshFilterComponent>();
	//MeshFactory::CreateSphere(mf, { 1, 12, 12});
	MeshFactory::CreateCapsule(mf, { 1, 1});

	// 3) Material を追加（シェーダ/テクスチャ/マテリアル）
	auto* mat = AddComponent<MaterialComponent>();

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* il = nullptr;
	Renderer::CreateVertexShader(&vs, &il, "shader\\VS_Lit.cso", Renderer::VertexLayoutType::Basic);
	Renderer::CreatePixelShader(&ps, "shader\\PS_TexUnlit.cso");
	mat->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);

	// テクスチャセット
	ID3D11ShaderResourceView* srv = Texture::LoadAndRegisterKey("assets\\texture\\apple.png");
	// サンプラーは Renderer::Init() で 0番に PSSetSamplers 済みなら null でも描ける
	mat->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);

	// マテリアルセット
	MaterialApp m{};
	m.diffuse = Vector4(1, 1, 1, 1);
	m.ambient = Vector4(1, 1, 1, 1);
	m.textureEnable = true;
	mat->SetMaterial(m);

	// 透明テクスチャの可能性が高いのでアルファブレンドに
	mat->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Opaque);

	// 4) MeshRenderer を追加（描画実行係）
	auto* mr = AddComponent<MeshRendererComponent>();

	// 5) LightComponent を追加
	auto* lc = AddComponent<LightComponent>();
	lc->SetDiffuse({ 1.0f, 1.0f, 1.0f });
	lc->SetIntensity(1);
	lc->SetRange(20);

	auto* col = AddComponent<Collider>();
	col->SetCapsule(1, 1);
	//col->SetSphere(1);
	col->SetModeSimulate();

	auto* rb = AddComponent<Rigidbody>();
	rb->AddForce({ 0, 500, 0 });
	rb->SetMass(1);
	rb->ComputeSphereInertia(1);
	rb->SetBodyType(Rigidbody::BodyType::Dynamic);

	// レイヤーセット
	SetPhysicsLayer(2);
}

void LightObject::Update(float dt)
{
	GameObject::Update(dt);
	/*
	m_Radian += 2.0f * dt;
	if (m_Radian > PI * 2) m_Radian -= PI * 2;

	Vector3 pos = Transform()->Position();
	pos.x += 10.0f * dt * sinf(m_Radian);
	Transform()->SetPosition(pos);
	*/
}
