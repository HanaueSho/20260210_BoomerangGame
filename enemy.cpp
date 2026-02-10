/*
	enemy.cpp
	20250514 hanaue sho
*/
#include "enemy.h"
#include "Collision.h"
#include "Keyboard.h"


void Enemy::Init()
{
	// 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
	auto* tf = GetComponent<TransformComponent>();
	tf->SetPosition({ 0,0,0 });
	tf->SetScale({ 1, 1, 1 });
	tf->SetEulerAngles({ 0,0,0 });

	// 2) MeshFilter を追加して頂点バッファ（4頂点の矩形）を作る
	auto* mf = AddComponent<MeshFilterComponent>();
	MeshFactory::CreateCube(mf, { {2.0f, 2.0f, 2.0f}});
	//MeshFactory::CreateSphere(mf, { 1, 12, 12});
	//MeshFactory::CreateCylinder(mf, { 2, 20, 12 });
	//MeshFactory::CreateCapsule(mf, { 1, 12, 12});

	// 3) Material を追加（シェーダ/テクスチャ/マテリアル）
	auto* mat = AddComponent<MaterialComponent>();

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* il = nullptr;
	Renderer::CreateVertexShader(&vs, &il, "shader\\VS_Lit.cso");
	Renderer::CreatePixelShader(&ps, "shader\\PS_PointLit.cso");
	mat->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);

	// 旧 Polygon2D と同じ kirby を使う
	//ID3D11ShaderResourceView* srv = Texture::LoadAndRegisterKey("assets\\texture\\apple.png");
	ID3D11ShaderResourceView* srv = Texture::GetByKey("apple");
	// サンプラーは Renderer::Init() で 0番に PSSetSamplers 済みなら null でも描ける
	mat->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);

	MaterialApp m{};
	m.diffuse  = Vector4(1, 1, 1, 1);
	m.ambient  = Vector4(1, 1, 1, 1);
	m.specular = Vector4(0, 0, 0, 1);
	m.shininess = 32.0f;
	m.textureEnable = TRUE;
	mat->SetMaterial(m);

	// 透明テクスチャの可能性が高いのでアルファブレンドに
	mat->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Opaque);

	// 4) MeshRenderer を追加（描画実行係）
	AddComponent<MeshRendererComponent>();

	// 当たり判定追加
	Collider* coll = AddComponent<Collider>();
	coll->SetBox({1, 1, 1});
	//coll->SetSphere(1);
	coll->SetModeSimulate();
	coll->SetOffsetPosition({ 0, 0, 0 });
	coll->SetOffsetRotation(Quaternion::FromEulerAngles({0, 0, 0}));

	// Rigidbody
	Rigidbody* rigid = AddComponent<Rigidbody>();
	rigid->AddForce({ 0, 500, 0 });
	rigid->SetMass(5);
	rigid->ComputeBoxInertia({ 1, 2, 3 });
	rigid->SetBodyType(Rigidbody::BodyType::Dynamic);
	//rigid->SetCenterOfMassLocal({ 3, 0, 0 });
	//rigid->ComputeSphereInertia(1);

	// レイヤーセット
	SetPhysicsLayer(0);
}

void Enemy::Update(float dt)
{
	GameObject::Update(dt);

	if (GetComponent<Rigidbody>()->IsKinematic())
	{
		const float value = 10.0f * dt;
		if (Keyboard_IsKeyDown(KK_A))
		{
			Vector3 position = Transform()->Position();
			position.x -= value;
			Transform()->SetPosition(position);
		}
		if (Keyboard_IsKeyDown(KK_D))
		{
			Vector3 position = Transform()->Position();
			position.x += value;
			Transform()->SetPosition(position);
		}
		if (Keyboard_IsKeyDown(KK_W))
		{
			Vector3 position = Transform()->Position();
			position.z += value;
			Transform()->SetPosition(position);
		}
		if (Keyboard_IsKeyDown(KK_S))
		{
			Vector3 position = Transform()->Position();
			position.z -= value;
			Transform()->SetPosition(position);
		}
		if (Keyboard_IsKeyDown(KK_Q))
		{
			Vector3 position = Transform()->Position();
			position.y += value;
			Transform()->SetPosition(position);
		}
		if (Keyboard_IsKeyDown(KK_E))
		{
			Vector3 position = Transform()->Position();
			position.y -= value;
			Transform()->SetPosition(position);
		}
	}

	float weight = 5.0f;

	//GetComponent<Rigidbody>()->AddTorque({ 0.0f, 80, 0 });
	//GetComponent<Rigidbody>()->AddTorque({ 80.0f, 0, 0 });
	if (Keyboard_IsKeyDownTrigger(KK_SPACE))
	{
		GetComponent<Rigidbody>()->AddForce({0, 300 * weight, 0});
		//GetComponent<Rigidbody>()->AddTorque({ 10.0f, 0, 10 });
	}
	if (Keyboard_IsKeyDownTrigger(KK_LEFT))
	{
		GetComponent<Rigidbody>()->AddForce({ -300 * weight, 0, 0});
	}
	if (Keyboard_IsKeyDownTrigger(KK_RIGHT))
	{
		GetComponent<Rigidbody>()->AddForce({ 300 * weight, 0, 0 });
	}
	if (Keyboard_IsKeyDownTrigger(KK_UP))
	{
		GetComponent<Rigidbody>()->AddForce({ 0, 0, 300 * weight });
	}
	if (Keyboard_IsKeyDownTrigger(KK_DOWN))
	{
		GetComponent<Rigidbody>()->AddForce({ 0, 0, -300 * weight });
	}

	if (Keyboard_IsKeyDownTrigger(KK_D1))
		GetComponent<Rigidbody>()->AddTorque({ 0.0f, 0, 200 });
	if (Keyboard_IsKeyDownTrigger(KK_D2))
		GetComponent<Rigidbody>()->AddTorque({ 0.0f, 0, -200 });
}

