/*
	Renderer.cpp
	20251007  hanaue sho
	GPUへ情報を送る経路
*/
#include "Renderer.h"
#include <cstdio>
#include <io.h>
#include <cstring>
#include <memory>
#include <cassert>
using namespace DirectX;

#include "DebugRenderer.h"

// ==================================================
// ----- 静的メンバ変数生成 -----
// ==================================================
ComPtr<ID3D11Device>            Renderer::m_Device{};
ComPtr<ID3D11DeviceContext>     Renderer::m_DeviceContext{};
ComPtr<ID3D11Buffer>		    Renderer::m_CB_Camera;
ComPtr<ID3D11Buffer>		    Renderer::m_CB_Object;
ComPtr<ID3D11Buffer>		    Renderer::m_CB_Material;
ComPtr<ID3D11Buffer>		    Renderer::m_CB_Light;
ComPtr<ID3D11Buffer>		    Renderer::m_CB_PointLights;
ComPtr<ID3D11Buffer>		    Renderer::m_CB_Toon;
ComPtr<ID3D11Buffer>		    Renderer::m_CB_CameraLight;
ComPtr<ID3D11Buffer>		    Renderer::m_CB_Outline;
ComPtr<ID3D11Buffer>		    Renderer::m_CB_PointLightIndexes;
ComPtr<ID3D11Buffer>		    Renderer::m_CB_Skin;
ComPtr<ID3D11Buffer>		    Renderer::m_CB_Sprite;
ComPtr<ID3D11RasterizerState>   Renderer::m_RasterizerStateDefault;
ComPtr<ID3D11SamplerState>	    Renderer::m_SamplerLinearWrap;
ComPtr<ID3D11DepthStencilState> Renderer::m_DepthStateEnable{};
ComPtr<ID3D11DepthStencilState> Renderer::m_DepthStateDisable{};
ComPtr<ID3D11BlendState>		Renderer::m_BlendState{};
ComPtr<ID3D11BlendState>		Renderer::m_BlendStateATC{}; 
ComPtr<ID3D11BlendState>		Renderer::m_BS_Opaque;
ComPtr<ID3D11BlendState>		Renderer::m_BS_Alpha;
ComPtr<ID3D11BlendState>		Renderer::m_BS_Premul;
ComPtr<ID3D11BlendState>		Renderer::m_BS_Add;
ComPtr<ID3D11BlendState>		Renderer::m_BS_Multiply;
ComPtr<ID3D11DepthStencilState> Renderer::m_DS_Opaque;
ComPtr<ID3D11DepthStencilState> Renderer::m_DS_Transparent;
ComPtr<ID3D11DepthStencilState> Renderer::m_DS_ZPrepassWrite;
ComPtr<ID3D11DepthStencilState> Renderer::m_DS_ZEqualReadOnly;
ComPtr<ID3D11DepthStencilState> Renderer::m_DS_OutlineLE;
ComPtr<ID3D11DepthStencilState> Renderer::m_DS_Disable;
ComPtr<ID3D11RasterizerState>   Renderer::m_RS_BackCull;
ComPtr<ID3D11RasterizerState>   Renderer::m_RS_FrontCull;
ComPtr<ID3D11RasterizerState>   Renderer::m_RS_NoCull;
ComPtr<ID3D11RasterizerState>   Renderer::m_RS_WireFrame;
ComPtr<ID3D11RasterizerState>   Renderer::m_RS_ShadowCaster;
DebugRenderer*					Renderer::m_pDebugRenderer;

// ==================================================
// ※ 放置 ※
// ==================================================
D3D_FEATURE_LEVEL        Renderer::m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

// --------------------------------------------------
// 初期化・終了処理
// --------------------------------------------------
void Renderer::Init(ID3D11Device* dev, ID3D11DeviceContext* ctx)
{
	// --------------------------------------------------
	// デバイス取得
	// --------------------------------------------------
	m_Device = dev;
	m_DeviceContext = ctx;
	m_FeatureLevel = m_Device ? m_Device->GetFeatureLevel() : D3D_FEATURE_LEVEL_11_0; // とりあえず初期化

	// --------------------------------------------------
	// 定数バッファ作成
	// --------------------------------------------------
	m_CB_Camera   = CreateCB<CameraCB>();
	m_CB_Object	  = CreateCB<ObjectCB>();
	m_CB_Material = CreateCB<MaterialCB>();
	m_CB_Light    = CreateCB<LightCB>();
	m_CB_PointLights = CreateCB<PointLightsCB>();
	m_CB_Toon     = CreateCB<ToonCB>();
	m_CB_CameraLight = CreateCB<CameraLightCB>();
	m_CB_Outline  = CreateCB<OutlineCB>();
	m_CB_PointLightIndexes = CreateCB<PointLightIndexesCB>();
	m_CB_Skin	  = CreateCB<SkinCB>();
	m_CB_Sprite	  = CreateCB<SpriteCB>();

	// --------------------------------------------------
	// VS/PS へバインド（スロット 0..8 を予約）
	// --------------------------------------------------
	ID3D11Buffer* cbs[] = {
		m_CB_Camera.Get(),   // b0
		m_CB_Object.Get(),   // b1
		m_CB_Material.Get(), // b2
		m_CB_Light.Get(),	 // b3
		m_CB_PointLights.Get(), // b4
		m_CB_Toon.Get(),	 // b5
		m_CB_CameraLight.Get(), // b6
		m_CB_Outline.Get(),  // b7
		m_CB_PointLightIndexes.Get(),  // b8
		m_CB_Skin.Get(),  // b9
		m_CB_Sprite.Get(),  // b10
	};
	m_DeviceContext->VSSetConstantBuffers(0, 11, cbs);
	m_DeviceContext->PSSetConstantBuffers(0, 11, cbs);

	// ポイントライトゼロ初期化
	PointLightsCB zeroPL{};
	zeroPL.pointLightCount = 0; // カウントを０で初期化
	UpdateCB(m_CB_PointLights.Get(), zeroPL);

	// --------------------------------------------------
	// ラスタライザ初期化
	// --------------------------------------------------
	ApplyDefaultRasterizer();

	// --------------------------------------------------
	// ブレンド、深度ステンシル生成
	// --------------------------------------------------
	CreateBlendStates();
	CreateDepthStates(); 
	CreateRasterizerStates();

	// --------------------------------------------------
	// デバッグレンダラー
	// --------------------------------------------------
	m_pDebugRenderer = new DebugRenderer();
	m_pDebugRenderer->Initialize();
}
void Renderer::Uninit()
{
	if (m_DeviceContext) m_DeviceContext->ClearState();

	// 先にステートを解放
	m_DepthStateEnable.Reset();
	m_DepthStateDisable.Reset();
	m_BlendState.Reset();
	m_BlendStateATC.Reset();

	// ComPtr 系は Reset でOK
	m_CB_Light.Reset();
	m_CB_Material.Reset();
	m_CB_Object.Reset();
	m_CB_Camera.Reset();

	// ブレンドステート
	m_BS_Opaque.Reset();
	m_BS_Alpha.Reset();
	m_BS_Premul.Reset();
	m_BS_Add.Reset();
	m_BS_Multiply.Reset();
	m_DS_Opaque.Reset();
	m_DS_Transparent.Reset();
	m_DS_ZPrepassWrite.Reset();
	m_DS_ZEqualReadOnly.Reset();
	m_DS_OutlineLE.Reset();
	m_DS_Disable.Reset();

	m_DeviceContext.Reset();
	m_Device.Reset();

	if (m_pDebugRenderer)
	{
		delete m_pDebugRenderer;
		m_pDebugRenderer = nullptr;
	}
}

// --------------------------------------------------
// Begin/End
// --------------------------------------------------
void Renderer::BeginFrame(const float clearColor[4])
{
	//float clearColor[4] = { 0.1f, 0.3f, 0.5f, 1.0f };
	// 今は空っぽ
}
void Renderer::EndFrame(int vsync)
{
	// 今は空っぽ
}

// --------------------------------------------------
// セッター
// --------------------------------------------------
void Renderer::SetCamera(const Matrix4x4& view,
	const Matrix4x4& proj,
	const Vector3& cameraPos,
	const float& fovY,
	uint32_t width, uint32_t height)
{
	Matrix4x4 vp = view * proj;
	auto cb = BuildCameraCB(view, proj, vp, cameraPos, fovY, width, height);
	UpdateCB(m_CB_Camera.Get(), cb);
}
void Renderer::SetObject(const Matrix4x4& world, const Matrix4x4& worldInv)
{
	auto cb = BuildObjectCB(world, worldInv);
	UpdateCB(m_CB_Object.Get(), cb);
}
void Renderer::SetMaterial(const MaterialApp& material)
{
	auto cb = BuildMaterialCB(material);
	UpdateCB(m_CB_Material.Get(), cb);
}
void Renderer::SetLight(const LightApp& light)
{
	auto cb = BuildLightCB(light);
	UpdateCB(m_CB_Light.Get(), cb);
}
void Renderer::SetPointLights(const PointLightApp* lights, int count)
{
	auto cb = BuildPointLightsCB(lights, count);
	UpdateCB(m_CB_PointLights.Get(), cb);
}
void Renderer::SetToon(const ToonApp& toon)
{
	auto cb = BuildToonCB(toon);
	UpdateCB(m_CB_Toon.Get(), cb);
}
void Renderer::SetCameraLight(const CameraLightApp& cameraLight)
{
	auto cb = BuildCameraLightCB(cameraLight);
	UpdateCB(m_CB_CameraLight.Get(), cb);
}
void Renderer::SetOutline(const OutlineApp& outline)
{
	auto cb = BuildOutlineCB(outline);
	UpdateCB(m_CB_Outline.Get(), cb);
}
void Renderer::SetPointLightIndexes(int* indexes, int count)
{
	auto cb = BuildPointLightIndexesCB(indexes, count);
	UpdateCB(m_CB_PointLightIndexes.Get(), cb);
}
void Renderer::SetSkinMatrixes(const Matrix4x4* mats, int count)
{
	auto cb = BuildSkinMatrixesCB(mats, count);
	UpdateCB(m_CB_Skin.Get(), cb);
}
void Renderer::SetSprite(const SpriteApp& sprite)
{
	auto cb = BuildSpriteCB(sprite);
	UpdateCB(m_CB_Sprite.Get(), cb);
}

// --------------------------------------------------
// CBの作成
// --------------------------------------------------
template<class T>
ComPtr<ID3D11Buffer> Renderer::CreateCB()
{
	D3D11_BUFFER_DESC desc{};
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	// 16byte 倍数
	//desc.ByteWidth = (UINT)((sizeof(T) + 15) / 16 * 16);
	desc.ByteWidth = (UINT)((sizeof(T) + 15) & ~15);
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	ComPtr<ID3D11Buffer> buf;
	HRESULT hr = m_Device->CreateBuffer(&desc, nullptr, buf.GetAddressOf());
	assert(SUCCEEDED(hr));
	return buf;
}
// --------------------------------------------------
// CBの更新
// --------------------------------------------------
template<class T>
void Renderer::UpdateCB(ID3D11Buffer* cb, const T& data)
{
	m_DeviceContext->UpdateSubresource(cb, 0, nullptr, &data, 0, 0);
}

// --------------------------------------------------
// CB構造体の作成
// --------------------------------------------------
static void CopyMat(float dst[16], const Matrix4x4& src)
{
	std::memcpy(dst, &src, sizeof(float) * 16);
}
CameraCB Renderer::BuildCameraCB(const Matrix4x4& view, const Matrix4x4& proj, const Matrix4x4& viewProj, const Vector3& cameraPos, const float& fovY, uint32_t width, uint32_t height)
{
	CameraCB cb{};
	CopyMat(cb.view, view);
	CopyMat(cb.proj, proj);
	CopyMat(cb.viewProj, viewProj);
	cb.cameraPos[0] = cameraPos.x;
	cb.cameraPos[1] = cameraPos.y;
	cb.cameraPos[2] = cameraPos.z;
	cb.fov = fovY;
	cb.screenSize[0] = (float)width;
	cb.screenSize[1] = (float)height;
	cb.invScreenSize[0] = width  ? 1.0f / (float)width  : 0.0f;
	cb.invScreenSize[1] = height ? 1.0f / (float)height : 0.0f;
	return cb;
}
ObjectCB Renderer::BuildObjectCB(const Matrix4x4& world, const Matrix4x4& worldInv)
{
	ObjectCB cb{};
	CopyMat(cb.world, world);
	//Matrix4x4 inv = world.Inverse();
	//Matrix4x4 invT = inv.Transpose();
	CopyMat(cb.worldInv, worldInv);
	return cb;
}
MaterialCB Renderer::BuildMaterialCB(const MaterialApp& m)
{
	MaterialCB cb{};
	std::memcpy(cb.ambient,  &m.ambient,  sizeof(float) * 4);
	std::memcpy(cb.diffuse,  &m.diffuse,  sizeof(float) * 4);
	std::memcpy(cb.specular, &m.specular, sizeof(float) * 4);
	std::memcpy(cb.emission, &m.emission, sizeof(float) * 4);
	cb.shininess = m.shininess;
	cb.textureEnable = m.textureEnable;
	return cb;
}
LightCB Renderer::BuildLightCB(const LightApp& l)
{
	LightCB cb{};
	cb.enable = l.enable;

	cb._padL0[0] = 0;
	cb._padL0[1] = 0;
	cb._padL0[2] = 0;

	cb.direction[0] = l.direction.x;
	cb.direction[1] = l.direction.y;
	cb.direction[2] = l.direction.z;
	cb.direction[3] = l.direction.w;

	cb.diffuse[0] = l.diffuse.x;
	cb.diffuse[1] = l.diffuse.y;
	cb.diffuse[2] = l.diffuse.z;
	cb.diffuse[3] = l.diffuse.w;

	cb.ambient[0] = l.ambient.x;
	cb.ambient[1] = l.ambient.y;
	cb.ambient[2] = l.ambient.z;
	cb.ambient[3] = l.ambient.w;

	//std::memcpy(cb.direction, &l.direction, sizeof(float) * 4);
	//std::memcpy(cb.diffuse,   &l.diffuse,   sizeof(float) * 4);
	//std::memcpy(cb.ambient,   &l.ambient,   sizeof(float) * 4);
	return cb;
}
PointLightsCB Renderer::BuildPointLightsCB(const PointLightApp* lights, int count)
{
	PointLightsCB cb{};
	cb.pointLightCount = (count > MAX_POINT) ? MAX_POINT : count;

	for (int i = 0; i < cb.pointLightCount; i++)
	{
		const auto& L = lights[i];
		cb.pointLights[i].positionRange[0] = L.position.x;
		cb.pointLights[i].positionRange[1] = L.position.y;
		cb.pointLights[i].positionRange[2] = L.position.z;
		cb.pointLights[i].positionRange[3] = L.range;

		cb.pointLights[i].colorIntensity[0] = L.diffuse.x;
		cb.pointLights[i].colorIntensity[1] = L.diffuse.y;
		cb.pointLights[i].colorIntensity[2] = L.diffuse.z;
		cb.pointLights[i].colorIntensity[3] = L.intensity;
	}
	return cb;
}
ToonCB Renderer::BuildToonCB(const ToonApp& t)
{
	ToonCB cb{};
	cb.ToonThreshold01 = t.ToonThreshold01;
	cb.ToonThreshold12 = t.ToonThreshold12;
	cb.ToonSmooth01	   = t.ToonSmooth01;
	cb.ToonSmooth12    = t.ToonSmooth12;

	std::memcpy(cb.ToonTintDark, &t.ToonTintDark, sizeof(float) * 3);
	std::memcpy(cb.ToonTintMid,  &t.ToonTintMid,  sizeof(float) * 3);
	std::memcpy(cb.ToonTintLit,  &t.ToonTintLit,  sizeof(float) * 3);

	cb.ToonSpecThreshold = t.ToonSpecThreshold;
	cb.ToonSpecStrength  = t.ToonSpecStrength;

	std::memcpy(cb.ToonShadowOverrideColor, &t.ToonShadowOverrideColor, sizeof(float) * 3);
	cb.ToonShadowOverrideRate = t.ToonShadowOverrideRate;
	cb.ToonShadowDesaturate = t.ToonShadowDesaturate;
	cb.ToonShadowAlbedoRate = t.ToonShadowAlbedoRate;
	return cb;
}
CameraLightCB Renderer::BuildCameraLightCB(const CameraLightApp& cameraLight)
{
	CameraLightCB cb{};
	std::memcpy(cb.direction, &cameraLight.direction, sizeof(float) * 4);
	std::memcpy(cb.diffuse  , &cameraLight.diffuse  , sizeof(float) * 4);
	return cb;
}
OutlineCB Renderer::BuildOutlineCB(const OutlineApp& outline)
{
	OutlineCB cb{};
	cb.outlineWidth = outline.outlineWidth;
	std::memcpy(cb.outlineColor, &outline.outlineColor, sizeof(float) * 3);
	return cb;
}
PointLightIndexesCB Renderer::BuildPointLightIndexesCB(int* indexes, int count)
{
	PointLightIndexesCB cb{};
	for (int i = 0; i < count; i++)
		cb.PL_activeIndexes[i] = indexes[i];
	cb.PL_avtiveCount = count;
	return cb;
}
SkinCB Renderer::BuildSkinMatrixesCB(const Matrix4x4* mats, int count)
{
	if (count > MAX_MATRIX)
	{
		assert(false && "SkinMatrixes Over MAX_MATRIX");
	}
	SkinCB cb{};
	int n = std::min(count, MAX_MATRIX);

	for (int i = 0; i < n; i++)
		cb.BoneMatrixes[i] = mats[i];
	cb.BoneCount = n;
	return cb;
}
SpriteCB Renderer::BuildSpriteCB(const SpriteApp& sprite)
{
	SpriteCB cb{};
	cb.spriteUV_OS[0] = sprite.spriteUVOffset.x;
	cb.spriteUV_OS[1] = sprite.spriteUVOffset.y;
	cb.spriteUV_OS[2] = sprite.spriteUVScale.x;
	cb.spriteUV_OS[3] = sprite.spriteUVScale.y;

	cb.spriteColor[0] = sprite.spriteColor.x;
	cb.spriteColor[1] = sprite.spriteColor.y;
	cb.spriteColor[2] = sprite.spriteColor.z;
	cb.spriteColor[3] = sprite.spriteColor.w;
	return cb;
}
// --------------------------------------------------
// ブレンドステート作成
// --------------------------------------------------
void Renderer::CreateBlendStates()
{
	auto* dev = Device();

	// 不透明
	D3D11_BLEND_DESC bd{};
	bd.AlphaToCoverageEnable = FALSE;
	bd.IndependentBlendEnable = FALSE;
	bd.RenderTarget[0].BlendEnable = FALSE;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	HRESULT hr = dev->CreateBlendState(&bd, m_BS_Opaque.GetAddressOf());
	assert(SUCCEEDED(hr));
	
	// 通常アルファ
	m_BS_Alpha    = MakeBS(dev, FALSE,
					   D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
					   D3D11_BLEND_ONE,		  D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD
					   );

	// プリマルチα
	m_BS_Premul   = MakeBS(dev, FALSE,
					   D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
					   D3D11_BLEND_ONE,	D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD
					   );

	// 加算（色だけ加算、α保持）
	m_BS_Add	  = MakeBS(dev, FALSE,
					   D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD,
					   D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD
					   );

	// 乗算
	m_BS_Multiply = MakeBS(dev, FALSE,
					   D3D11_BLEND_DEST_COLOR, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
					   D3D11_BLEND_ONE,		   D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD
					   );
}
ComPtr<ID3D11BlendState> Renderer::MakeBS(ID3D11Device* dev, bool atc, D3D11_BLEND src, D3D11_BLEND dst, D3D11_BLEND_OP op, D3D11_BLEND srcA, D3D11_BLEND dstA, D3D11_BLEND_OP opA, UINT8 writeMask)
{
	D3D11_BLEND_DESC bd{};
	bd.AlphaToCoverageEnable = atc ? TRUE : FALSE;
	bd.IndependentBlendEnable = FALSE;
	auto& rt = bd.RenderTarget[0];
	rt.BlendEnable = TRUE;
	rt.SrcBlend = src; 
	rt.DestBlend = dst; 
	rt.BlendOp = op;
	rt.SrcBlendAlpha = srcA;
	rt.DestBlendAlpha = dstA; 
	rt.BlendOpAlpha = opA;
	rt.RenderTargetWriteMask = writeMask;

	ComPtr<ID3D11BlendState> bs;
	HRESULT hr = dev->CreateBlendState(&bd, bs.GetAddressOf());
	assert(SUCCEEDED(hr));
	return bs;
}
// --------------------------------------------------
// デプスステンシルステート作成
// --------------------------------------------------
void Renderer::CreateDepthStates()
{
	auto* dev = Device();

	// 不透明
	m_DS_Opaque = MakeDS(dev, TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS_EQUAL);

	// 半透明（読み専用）
	m_DS_Transparent = MakeDS(dev, TRUE, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS_EQUAL);

	// Zプリパス
	m_DS_ZPrepassWrite = MakeDS(dev, TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS_EQUAL);

	// Z一致のみ着色
	m_DS_ZEqualReadOnly = MakeDS(dev, TRUE, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_EQUAL);

	// アウトライン
	m_DS_OutlineLE = MakeDS(dev, TRUE, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS_EQUAL); // D3D11_COMPARISON_LESS_EQUAL だとつぶれやすい

	// 深度なし
	m_DS_Disable = MakeDS(dev, FALSE, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_ALWAYS);
}
ComPtr<ID3D11DepthStencilState> Renderer::MakeDS(ID3D11Device* dev, BOOL depthEnable, D3D11_DEPTH_WRITE_MASK writeMask, D3D11_COMPARISON_FUNC depthFunc, BOOL stencilEnable, UINT8 readMask, UINT8 writeMaskStencil, D3D11_COMPARISON_FUNC stencilFunc, D3D11_STENCIL_OP passOp, D3D11_STENCIL_OP failOp, D3D11_STENCIL_OP zfailOp)
{
	D3D11_DEPTH_STENCIL_DESC ds{};
	ds.DepthEnable = depthEnable;
	ds.DepthWriteMask = writeMask;
	ds.DepthFunc = depthFunc;

	ds.StencilEnable = stencilEnable;
	ds.StencilReadMask = readMask;
	ds.StencilWriteMask = writeMaskStencil;

	// 前面、背面とも同じにする例
	ds.FrontFace.StencilFunc = stencilFunc;
	ds.FrontFace.StencilPassOp = passOp;
	ds.FrontFace.StencilFailOp = failOp;
	ds.FrontFace.StencilDepthFailOp = zfailOp;

	ds.BackFace = ds.FrontFace;
	ComPtr<ID3D11DepthStencilState> state;
	HRESULT hr = dev->CreateDepthStencilState(&ds, state.GetAddressOf());
	assert(SUCCEEDED(hr));
	return state;
}
// --------------------------------------------------
// ラスタライザステート作成
// --------------------------------------------------
void Renderer::CreateRasterizerStates()
{
	auto* dev = Device();

	// 既定（不透明、通常）
	m_RS_BackCull = MakeRS(dev, D3D11_FILL_SOLID, D3D11_CULL_BACK, FALSE, TRUE, FALSE, FALSE, FALSE, 0, 0, 0);

	// アウトライン（背面法：前面カリング）
	m_RS_FrontCull = MakeRS(dev, D3D11_FILL_SOLID, D3D11_CULL_FRONT, FALSE, TRUE, FALSE, FALSE, FALSE, -2, -1, 0); // 最後から３番目の引数が -2 だと黒く塗りつぶされやすい（０だと平気？）

	// ダブルサイド
	m_RS_NoCull = MakeRS(dev, D3D11_FILL_SOLID, D3D11_CULL_NONE, FALSE, TRUE, FALSE, FALSE, FALSE, 0, 0, 0);

	// ワイヤーフレーム（デバッグ）
	m_RS_WireFrame = MakeRS(dev, D3D11_FILL_WIREFRAME, D3D11_CULL_BACK, FALSE, TRUE, FALSE, FALSE, FALSE, 0, 0, 0);

	// シャドウマップ用
	m_RS_ShadowCaster = MakeRS(dev, D3D11_FILL_SOLID, D3D11_CULL_BACK, FALSE, TRUE, FALSE, FALSE, FALSE, 200, 1.0f, 0.0f);
}
ComPtr<ID3D11RasterizerState> Renderer::MakeRS(ID3D11Device* dev, D3D11_FILL_MODE fill, D3D11_CULL_MODE cull, BOOL frontCCW, BOOL depthClip, BOOL scissor, BOOL multisample, BOOL aaLine, INT depthBias, FLOAT slopeScaledBias, FLOAT depthBiasClamp)
{
	D3D11_RASTERIZER_DESC rd{};
	rd.FillMode = fill; // 三角形の塗りつぶし
	rd.CullMode = cull; // どちらの面を捨てるか
	rd.FrontCounterClockwise = frontCCW; // 反時計回りが表面かのフラグ
	rd.DepthClipEnable = depthClip;		 // クリップ空間のZで自動クリップするかどうか
	rd.ScissorEnable = scissor;			 // シザーテストを使うか
	rd.MultisampleEnable = multisample;  // MSAA ラスタライザの利用
	rd.AntialiasedLineEnable = aaLine;   // 非 MSAA のライン描画のアンリエイリアス（三角形には無関係。ラインのデバッグ描画をきれいにしたいときに TRUE ）
	rd.DepthBias = depthBias;				   // 深度バイアス（Zファイティング抑制・影アクネ対策）
	rd.SlopeScaledDepthBias = slopeScaledBias; // 実際のバイアスは DepthBias * r + SlopScaledDepthBias * slop
	rd.DepthBiasClamp = depthBiasClamp;		   // 上のを DepthBiasClamp でクランプする

	ComPtr<ID3D11RasterizerState> rs;
	HRESULT hr = dev->CreateRasterizerState(&rd, rs.GetAddressOf());
	assert(SUCCEEDED(hr));
	return rs;
}
// --------------------------------------------------
// デフォルトステート作成
// --------------------------------------------------
void Renderer::EnsureCommonStates()
{
	if (!m_Device) return;

	// 深度・ブレンドステートを初期化
	static bool initialized = false;
	if (initialized) return;
	initialized = true;

	ID3D11Device* device = m_Device.Get();
	assert(device);

	// 深度ステートON（深度テスト＆書き込みを行う）
	{
		D3D11_DEPTH_STENCIL_DESC dsOn{};
		dsOn.DepthEnable	  = TRUE;							  // 深度テストを有効
		dsOn.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ALL;		  // 深度バッファへの書き込みON
		dsOn.DepthFunc		  = D3D11_COMPARISON_LESS_EQUAL;	  // 手前なら描画（Zが小さい方）
		dsOn.StencilEnable    = FALSE;							  // ステンシルは今回は無効（シャドウ、マスクのときに使用）
		dsOn.StencilReadMask  = D3D11_DEFAULT_STENCIL_READ_MASK;  // デフォルト
		dsOn.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK; // デフォルト
		HRESULT hr = device->CreateDepthStencilState(&dsOn, m_DepthStateEnable.GetAddressOf());
		assert(SUCCEEDED(hr));
	}
	// 深度ステートOFF（深度テストはするが書き込みはしない）
	{
		D3D11_DEPTH_STENCIL_DESC dsOff{};
		dsOff.DepthEnable	   = TRUE;
		dsOff.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ZERO;      // 深度バッファへの書き込みを止める
		dsOff.DepthFunc		   = D3D11_COMPARISON_LESS_EQUAL;	   // 手前なら描画（Zが小さい方）
		dsOff.StencilEnable	   = FALSE;							   // ステンシルは今回は無効（シャドウ、マスクのときに使用）
		dsOff.StencilReadMask  = D3D11_DEFAULT_STENCIL_READ_MASK;  // デフォルト
		dsOff.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK; // デフォルト
		HRESULT hr = device->CreateDepthStencilState(&dsOff, m_DepthStateDisable.GetAddressOf());
		assert(SUCCEEDED(hr));
	}

	// 通常ブレンド（標準αブレンド）
	{
		D3D11_BLEND_DESC bd{};
		bd.AlphaToCoverageEnable = FALSE;  // マルチサンプリング時のαマスクを無効
		bd.IndependentBlendEnable = FALSE; // 複数RTに個別設定しない（１枚のみ）
		auto& rt = bd.RenderTarget[0];
		rt.BlendEnable			 = TRUE; // ブレンドを有効
		rt.SrcBlend				 = D3D11_BLEND_SRC_ALPHA;	  // ソース：出力α
		rt.DestBlend			 = D3D11_BLEND_INV_SRC_ALPHA; // デスティネーション
		rt.BlendOp				 = D3D11_BLEND_OP_ADD;		  // 色ブレンド：加算
		rt.SrcBlendAlpha		 = D3D11_BLEND_ONE;			  // αブレンド（そのまま）
		rt.DestBlendAlpha		 = D3D11_BLEND_ZERO;		  // αブレンド（無影響）
		rt.BlendOpAlpha			 = D3D11_BLEND_OP_ADD;		  // αブレンド：加算
		rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // RGBA 全て書き込み
		HRESULT hr = device->CreateBlendState(&bd, m_BlendState.GetAddressOf());
		assert(SUCCEEDED(hr));

		// AlphaToCoverage 付き
		bd.AlphaToCoverageEnable = TRUE;
		hr = device->CreateBlendState(&bd, m_BlendStateATC.GetAddressOf());
		assert(SUCCEEDED(hr));
	}

	// 既定ラスタライザ
	{
		D3D11_RASTERIZER_DESC rd{};
		rd.FillMode				 = D3D11_FILL_SOLID;
		rd.CullMode				 = D3D11_CULL_BACK;
		rd.FrontCounterClockwise = FALSE;
		rd.DepthClipEnable		 = TRUE;
		rd.ScissorEnable		 = FALSE;
		rd.MultisampleEnable	 = FALSE;
		rd.AntialiasedLineEnable = FALSE;
		rd.DepthBias			 = 0;
		rd.DepthBiasClamp		 = 0.0f;
		rd.SlopeScaledDepthBias  = 0.0f;

		HRESULT hr = device->CreateRasterizerState(&rd, m_RasterizerStateDefault.GetAddressOf());
		assert(SUCCEEDED(hr));
	}
	// サンプラーステート設定
	{
		if (!m_SamplerLinearWrap)
		{
			D3D11_SAMPLER_DESC sd{};
			sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			sd.MaxLOD = D3D11_FLOAT32_MAX;
			HRESULT hr = m_Device->CreateSamplerState(&sd, m_SamplerLinearWrap.GetAddressOf());
			assert(SUCCEEDED(hr));
		}
	}

}
void Renderer::ApplyDefaultRasterizer()
{
	EnsureCommonStates();
	if (!m_DeviceContext || !m_RasterizerStateDefault) return;
	m_DeviceContext->RSSetState(m_RasterizerStateDefault.Get());
}

void Renderer::SetDepthEnable(bool enable)
{
	EnsureCommonStates();
	if (!m_DeviceContext) return;
	m_DeviceContext->OMSetDepthStencilState(enable ? m_DepthStateEnable.Get() : m_DepthStateDisable.Get(), 0);
}
void Renderer::SetATCEnable(bool enable)
{
	EnsureCommonStates();
	if (!m_DeviceContext) return;
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_DeviceContext->OMSetBlendState(enable ? m_BlendStateATC.Get() : m_BlendState.Get(), blendFactor, 0xffffffff);
}
void Renderer::SetBlendState(BlendMode mode)
{
	ID3D11BlendState* bs = nullptr;
	switch (mode)
	{
	case BlendMode::Opaque:		   bs = m_BS_Opaque.Get();	 break;
	case BlendMode::Alpha:		   bs = m_BS_Alpha.Get();	 break;
	case BlendMode::Premultiplied: bs = m_BS_Premul.Get();	 break;
	case BlendMode::Additive:	   bs = m_BS_Add.Get();		 break;
	case BlendMode::Multiply:	   bs = m_BS_Multiply.Get(); break;
	}
	static  const FLOAT blendFactor[4] = { 1, 1, 1, 1 }; // BLEND_FACTOR を使っていないのでまだ未使用
	DeviceContext()->OMSetBlendState(bs, nullptr, 0xFFFFFFFF);
}
void Renderer::SetDepthState(DepthMode mode, UINT stencilRef)
{
	ID3D11DepthStencilState* ds = nullptr;
	switch (mode)
	{
	case DepthMode::Opaque:			ds = m_DS_Opaque.Get();			break;
	case DepthMode::Transparent:	ds = m_DS_Transparent.Get();	break;
	case DepthMode::ZPrepassWrite:  ds = m_DS_ZPrepassWrite.Get();	break;
	case DepthMode::ZEqualReadOnly: ds = m_DS_ZEqualReadOnly.Get(); break;
	case DepthMode::OutlineLE:		ds = m_DS_OutlineLE.Get();		break;
	case DepthMode::DisableDepth:	ds = m_DS_Disable.Get();		break;
	}
	DeviceContext()->OMSetDepthStencilState(ds, stencilRef); // Ref はステンシル使う時に意味ある
}
void Renderer::SetRasterizerState(RasterizerMode mode)
{
	ID3D11RasterizerState* rs = nullptr;
	switch (mode)
	{
	case RasterizerMode::BackCullSolid:  rs = m_RS_BackCull.Get();	   break;
	case RasterizerMode::FrontCullSolid: rs = m_RS_FrontCull.Get();    break;
	case RasterizerMode::NoneCullSolid:  rs = m_RS_NoCull.Get();	   break;
	case RasterizerMode::WireFrame:		 rs = m_RS_WireFrame.Get();	   break;
	case RasterizerMode::ShadowCaster:	 rs = m_RS_ShadowCaster.Get(); break;
	}
	DeviceContext()->RSSetState(rs);
}

void Renderer::CreateVertexShader(ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName, VertexLayoutType layoutType)
{
	// ファイルネームから探す
	FILE* file = fopen(FileName, "rb");
	assert(file);
	long int fsize = _filelength(_fileno(file));
	std::unique_ptr<unsigned char[]> buffer(new unsigned char[fsize]);
	fread(buffer.get(), fsize, 1, file);
	fclose(file);

	m_Device->CreateVertexShader(buffer.get(), fsize, NULL, VertexShader);

	// レイアウト設定
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc;
	if (layoutType == VertexLayoutType::Basic)
	{
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(VERTEX_3D, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,	 0, (UINT)offsetof(VERTEX_3D, normal),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetof(VERTEX_3D, diffuse),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		 0, (UINT)offsetof(VERTEX_3D, texcoord), D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		layoutDesc.assign(std::begin(layout), std::end(layout));
	}
	else if (layoutType == VertexLayoutType::Skinned)
	{
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, (UINT)offsetof(VERTEX_SKINNED, position),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,	 0, (UINT)offsetof(VERTEX_SKINNED, normal),     D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,		 0, (UINT)offsetof(VERTEX_SKINNED, texcoord),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT,		 0, (UINT)offsetof(VERTEX_SKINNED, boneIndex),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetof(VERTEX_SKINNED, boneWeight), D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		layoutDesc.assign(std::begin(layout), std::end(layout));
	}


	m_Device->CreateInputLayout(layoutDesc.data(),
		(UINT)layoutDesc.size(),
		buffer.get(),
		fsize,
		VertexLayout);
}
void Renderer::CreatePixelShader(ID3D11PixelShader** PixelShader, const char* FileName)
{
	FILE* file = fopen(FileName, "rb");
	assert(file);
	long int fsize = _filelength(_fileno(file));
	std::unique_ptr<unsigned char[]> buffer(new unsigned char[fsize]);
	fread(buffer.get(), fsize, 1, file);
	fclose(file);

	m_Device->CreatePixelShader(buffer.get(), fsize, NULL, PixelShader);
}
