/*
    TargetSpriteObject.h
    20260215  hanaue sho
*/
#include "TargetSpriteObject.h"

#include "SpriteRendererComponent.h"
#include "SpriteAnimationComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "renderer.h"
#include "texture.h"  // Texture::Load 既存
#include "BillboardComponent.h"

void TargetSpriteObject::Init()
{
    // 1) Transform（既に GameObject ctor で追加済み）を取得して初期姿勢を入れておく
    auto* tf = GetComponent<TransformComponent>();
    tf->SetPosition({ 0, 0, 0 });
    tf->SetScale({ 1,1,1 });
    tf->SetEulerAngles({ 0,0,0 });

    // 2) MeshFilter を追加して頂点バッファ（4頂点の矩形）を作る
    auto* mf = AddComponent<MeshFilterComponent>();
    MeshFactory::CreateQuad3D(mf, { 20.0f, 20.0f, true });

    // 3) Material を追加（シェーダ/テクスチャ/マテリアル）
    auto* mat = AddComponent<MaterialComponent>();

    ID3D11VertexShader* vs = nullptr;
    ID3D11PixelShader* ps = nullptr;
    ID3D11InputLayout* il = nullptr;
    Renderer::CreateVertexShader(&vs, &il, "shader\\VS_Sprite.cso");
    Renderer::CreatePixelShader(&ps, "shader\\PS_Sprite.cso");
    mat->SetVSPS(vs, ps, il, /*takeVS*/true, /*takePS*/true, /*takeIL*/true);

    // 旧 Polygon2D と同じ kirby を使う
    ID3D11ShaderResourceView* srv = Texture::LoadAndRegisterKey("assets\\texture\\targetTexture_001.png");
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
    sr->SetColor({ 1, 1, 1, m_Alpha });

    // Billboard
    auto* bb = AddComponent<BillboardComponent>();

}

void TargetSpriteObject::Update(float dt)
{
    GameObject::Update(dt);

    switch (m_State)
    {
    case State::None:
        break;
    case State::FadeIn:
    {
        m_Timer += dt * m_Speed;
        if (m_Timer > m_Time)
        {
            m_Timer = m_Time;
            m_State = State::None;
        }
        m_Alpha = m_Timer / m_Time;
        auto* sr = GetComponent<SpriteRendererComponent>();
        sr->SetColor({ 1, 1, 1, m_Alpha });
    }
    break;
    case State::FadeOut:
    {
        m_Timer -= dt * m_Speed;
        if (m_Timer < 0)
        {
            m_Timer = 0;
            m_State = State::None;
        }
        m_Alpha = m_Timer / m_Time;
        auto* sr = GetComponent<SpriteRendererComponent>();
        sr->SetColor({ 1, 1, 1, m_Alpha });
    }
    break;
    }
}
