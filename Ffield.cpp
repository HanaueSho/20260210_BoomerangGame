/*
	field.cpp
	20250514 hanaue sho
*/
#include "field.h"
#include "ColliderComponent.h"
#include "RigidbodyComponent.h"
#include "Random.h"

void Field::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0,0,0 });
	tf->SetScale({ 1,1,1 });
	tf->SetEulerAngles({ 0,0,0 });

	// 2) MeshFilter を追加して頂点バッファ（4頂点の矩形）を作る
	std::vector<float> heights;
	for (int i = 0; i < 100 * 100; i++) heights.push_back(0.0f);
	for (int i = 0; i < 100 * 100; i++) if (i % 1 == 0) heights[i] = Random::RandomRangeStepped(0.0f, 2.0f, 0.1f);
	auto* mf = AddComponent<MeshFilterComponent>();
	MeshFactory::CreateMeshField(mf, { 100, 100, 1.0f, 1.0f, heights, true});

	// 3) Material を追加（シェーダ/テクスチャ/マテリアル）
	auto* mat = AddComponent<MaterialComponent>();

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* il = nullptr;
	Renderer::CreateVertexShader(&vs, &il, "shader\\VS_Lit.cso");
	Renderer::CreatePixelShader(&ps, "shader\\PS_PixelLit.cso");
	mat->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);

	// 旧 Polygon2D と同じ kirby を使う
	ID3D11ShaderResourceView* srv = Texture::LoadAndRegisterKey("assets\\texture\\error.png");
	// サンプラーは Renderer::Init() で 0番に PSSetSamplers 済みなら null でも描ける
	mat->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);

	MaterialApp m{};
	m.diffuse = Vector4(1, 1, 1, 1);
	m.ambient = Vector4(1, 1, 1, 1);
	m.textureEnable = TRUE;
	mat->SetMaterial(m);

	// 透明テクスチャの可能性が高いのでアルファブレンドに
	mat->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Opaque);

	// 4) MeshRenderer を追加（描画実行係）
	AddComponent<MeshRendererComponent>();

	// 当たり判定追加
	Collider* coll = AddComponent<Collider>();
	coll->SetMeshField(100, 100, 1.0f, 1.0f, heights);
	//coll->SetSphere(1);
	coll->SetModeSimulate();

	// Rigidbody
	Rigidbody* rigid = AddComponent<Rigidbody>();
	rigid->AddForce({ 0, 500, 0 });
	rigid->SetMass(5);
	rigid->ComputeBoxInertia({ 1, 1, 1 });
	rigid->SetBodyType(Rigidbody::BodyType::Static);
	//rigid->ComputeSphereInertia(1);

}

