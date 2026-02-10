/*
	player.cpp
	20250514 hanaue sho
*/
#include "player.h"
#include "Transform.h"
#include "Keyboard.h"


void Player::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0,0,0 });
	tf->SetScale({ 1,1,1 });
	tf->SetEulerAngles({ 0,0,0 });

	// 2) MeshFilter を追加して頂点バッファ（4頂点の矩形）を作る
	auto* mf = AddComponent<MeshFilterComponent>();
	//MeshFactory::CreateCube(mf, { {2.0f, 2.0f, 2.0f}});
	//MeshFactory::CreateSphere(mf, { 1, 12, 12});
	//MeshFactory::CreateCylinder(mf, { 2, 20, 12});
	//MeshFactory::CreateCapsule(mf, { 1, 1});
	MeshFactory::CreateApple(mf, { 1, 36, 36});

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
	//coll->SetBox({1 , 1, 1});
	//coll->SetCapsule(1, 1);
	coll->SetModeSimulate();
	coll->SetOffsetRotation(Quaternion::FromEulerAngles({ 0, 0, 0 }));

	Rigidbody* rigid = AddComponent<Rigidbody>();
	rigid->SetGravityScale(1.0f);
	rigid->SetRestitution(0.0f);
	rigid->SetMass(1.0f);
	rigid->ComputeSphereInertia(1);

	// レイヤーセット
	SetPhysicsLayer(1);
}


void Player::Update(float dt)
{
	GameObject::Update(dt);

	//Transform()->RotateAxis({0, 1, 0}, 0.01f);
	//Transform()->MarkLocalDirty();

	Rigidbody* rigid = GetComponent<Rigidbody>();
	if (Keyboard_IsKeyDownTrigger(KK_P))
	{
		rigid->AddTorque({ 0.0f, 50.0f, 0 });
	}
	if (Keyboard_IsKeyDownTrigger(KK_O))
	{
		rigid->AddTorque({ 0.0f, -50.0f, 0 });
	}


}
