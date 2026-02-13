/*
	polygon.cpp
	20250423 hanaue sho
*/
#include "polygon.h"
#include "SpriteRendererComponent.h"
#include "SpriteAnimationComponent.h"

void Polygon2D::Init()
{
    // 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
    auto* tf = GetComponent<TransformComponent>();
    tf->SetPosition({ 0,0,0 });
    tf->SetScale({ 1,1,1 });
    tf->SetEulerAngles({ 0,0,0 });

    // 2) MeshFilter を追加して頂点バッファ（4頂点の矩形）を作る
    auto* mf = AddComponent<MeshFilterComponent>();
    MeshFactory::CreateQuad(mf, { 200.0f, 200.0f, false });

    // 3) Material を追加（シェーダ/テクスチャ/マテリアル）
    auto* mat = AddComponent<MaterialComponent>();

    ID3D11VertexShader* vs = nullptr;
    ID3D11PixelShader* ps = nullptr;
    ID3D11InputLayout* il = nullptr;
    Renderer::CreateVertexShader(&vs, &il, "shader\\VS_Sprite.cso");
    Renderer::CreatePixelShader(&ps, "shader\\PS_Sprite.cso");
    mat->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);

    // 旧 Polygon2D と同じ kirby を使う
    ID3D11ShaderResourceView* srv = Texture::LoadAndRegisterKey("assets\\texture\\PlayerDead.png");
    // サンプラーは Renderer::Init() で 0番に PSSetSamplers 済みなら null でも描ける
    mat->SetMainTexture(srv, /*sampler*/nullptr, /*takeSrv*/false, /*takeSamp*/false);

    MaterialApp m{};
    m.diffuse = Vector4(1, 1, 1, 1);
    m.ambient = Vector4(1, 1, 1, 1);
    m.textureEnable = TRUE;
    mat->SetMaterial(m);

    // 透明テクスチャの可能性が高いのでアルファブレンドに
    mat->SetBlendMode(/*Alpha*/MaterialComponent::BlendMode::Alpha);

    // 4) MeshRenderer を追加（描画実行係）
    auto* sr = AddComponent<SpriteRendererComponent>();
    sr->SetUI(true);
    sr->SetOnWorld(true);
    sr->SetColor({ 1, 1, 1, 1.0f });

    // SpriteAnimation
    auto* sa = AddComponent<SpriteAnimationComponent>();
    m_Clip.columns = 5;
    m_Clip.rows = 1;
    m_Clip.frameCount = 5;
    m_Clip.fps = 10;
    m_Clip.loopDefault = true;
    sa->SetClip(&m_Clip);
}

