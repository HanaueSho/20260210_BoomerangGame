/*
	Renderer.h
	20251006  hanaue sho
	GPUへ情報を送る経路
*/
#ifndef RENDERER_H_
#define RENDERER_H_
#include <wrl/client.h>
#include <d3d11.h>
#include <DirectXMath.h>
using Microsoft::WRL::ComPtr;
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"

class DebugRenderer;

// ==================================================
// ----- 頂点情報構造体（いずれはMeshへ移動） -----
// ==================================================
struct VERTEX_3D
{
	Vector3 position;
	Vector3 normal;
	Vector4 diffuse;
	Vector2 texcoord;
};
struct VERTEX_SKINNED
{
	Vector3 position;
	Vector3 normal;
	Vector2 texcoord;
	uint8_t boneIndex[4];  // ４本まで対応
	float   boneWeight[4]; // ウェイト
};

// ==================================================
// ----- アプリケーション側で扱う構造体 -----
// ※こちらは 16byte に合わせる必要はない
// ==================================================
// --------------------------------------------------
// マテリアル情報
// --------------------------------------------------
struct MaterialApp
{
	Vector4	ambient;  // 環境光
	Vector4	diffuse;  // 拡散光 
	Vector4	specular; // 鏡面光
	Vector4	emission; // 自己発光
	float	shininess = 32.0f; // 光沢度
	int		textureEnable = 1; // テクスチャ有効
};
// --------------------------------------------------
// ライト情報
// --------------------------------------------------
struct LightApp
{
	int		enable = 0; // ライト有効フラグ
	Vector4	direction;	// ライト方向
	Vector4	diffuse;	// 拡散光
	Vector4	ambient;	// 環境光
};
struct PointLightApp
{
	Vector3	position;	// 環境光
	Vector3	diffuse;	// 拡散光
	float range = 10;	// 距離
	float intensity = 1.0f; // 光沢度
};
// --------------------------------------------------
// トゥーン情報
// --------------------------------------------------
struct ToonApp
{
	float ToonThreshold01; // 境界閾値（暗→中）
	float ToonThreshold12; // 境界閾値（中→明）
	float ToonSmooth01; // 境界のぼかし幅（暗→中）
	float ToonSmooth12; // 境界のぼかし幅（中→明）
	// 各段のティント（albedoに掛ける色）
	Vector3 ToonTintDark; 
	Vector3 ToonTintMid;  
	Vector3 ToonTintLit;  
	// ハイライト線
	float ToonSpecThreshold;
	float ToonSpecStrength;
	// 影部分の上書き処理
	Vector3 ToonShadowOverrideColor; // 上書きする色
	float   ToonShadowOverrideRate;  // 0:使わない、1:完全上書き（上書き率）
	float	ToonShadowDesaturate;	 // 0:そのまま、1:完全モノクロ（脱色）
	float   ToonShadowAlbedoRate;	 // 0:模様を使わない、1: 模様つかう
};
// --------------------------------------------------
// カメラライト情報
// --------------------------------------------------
struct CameraLightApp
{
	Vector4	direction;	// ライト方向
	Vector4	diffuse;	// 拡散光
};
// --------------------------------------------------
// アウトライン情報
// --------------------------------------------------
struct OutlineApp
{
	float outlineWidth;
	Vector3 outlineColor;
};
// --------------------------------------------------
// スプライト情報
// --------------------------------------------------
struct SpriteApp
{
	Vector2 spriteUVOffset;
	Vector2 spriteUVScale;
	Vector4 spriteColor;
};

// ==================================================
// ----- GPU側へ送る定数バッファ構造体 -----
// ※ cbufferと並びを合わせる
// ==================================================
// --------------------------------------------------
// カメラバッファ（ｂ０）
// --------------------------------------------------
struct alignas(16) CameraCB
{
	float view[16];		// view 行列
	float proj[16];		// projection 行列
	float viewProj[16];	// ViewProjection 行列
	float cameraPos[3]; // カメラ位置
	float fov;			// FovY
	float screenSize[2];	// 画面サイズ
	float invScreenSize[2]; // 逆画面サイズ
};
// --------------------------------------------------
// オブジェクトバッファ（ｂ１）
// --------------------------------------------------
struct alignas(16) ObjectCB
{
	float world[16];	// ワールド行列
	float worldInv[16]; // 法線変換用
};
// --------------------------------------------------
// マテリアルバッファ（ｂ２）
// --------------------------------------------------
struct alignas(16) MaterialCB
{
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float emission[4];
	float shininess;
	int   textureEnable;
	float _padMat0[2]; // 16byte アライン調整
};
// --------------------------------------------------
// ライトバッファ（ｂ３）
// --------------------------------------------------
struct alignas(16) LightCB
{
	float direction[4];
	float diffuse[4];
	float ambient[4];
	int   enable; int _padL0[3];
};
// --------------------------------------------------
// ポイントライトバッファ（ｂ４）
// 複数のポイントライトを一つのバッファへ入れる
// --------------------------------------------------
static const int MAX_POINT = 64; // 上限（ここも合わせる必要があるので注意！）
struct PointLightData
{
	float positionRange[4];
	float colorIntensity[4];
};
struct alignas(16) PointLightsCB
{
	PointLightData pointLights[MAX_POINT];
	int pointLightCount;
	int _padPL0[3];
};
// --------------------------------------------------
// トゥーンバッファ（ｂ５）
// --------------------------------------------------
struct alignas(16) ToonCB
{
	// 境界閾値、ぼかし幅
	float ToonThreshold01; // 境界閾値（暗→中）
	float ToonThreshold12; // 境界閾値（中→明）
	float ToonSmooth01; // 境界のぼかし幅（暗→中）
	float ToonSmooth12; // 境界のぼかし幅（中→明）
	// 各段のティント（albedoに掛ける色）
	float ToonTintDark[3]; float _padT0;
	float ToonTintMid[3];  float _padT1;
	float ToonTintLit[3];  float _padT2;
	// ハイライト線
	float ToonSpecThreshold;
	float ToonSpecStrength;
	float _padT3[2];
	// 影部分の上書き処理
	float ToonShadowOverrideColor[3]; float _padT4; // 上書きする色
	float ToonShadowOverrideRate; // 0:使わない、1:完全上書き（上書き率）
	float ToonShadowDesaturate;   // 0:そのまま、1:完全モノクロ（脱色）
	float ToonShadowAlbedoRate;	 // 0:模様を使わない、1: 模様つかう
};
// --------------------------------------------------
// カメラライトバッファ（ｂ６）
// --------------------------------------------------
struct alignas(16) CameraLightCB
{
	float direction[4];
	float diffuse[4];
};
// --------------------------------------------------
// アウトラインバッファ（ｂ７）
// --------------------------------------------------
struct alignas(16) OutlineCB
{
	float outlineWidth;
	float outlineColor[3];
};
// --------------------------------------------------
// ポイントライトインデックスバッファ（ｂ８）
// --------------------------------------------------
struct alignas(16) PointLightIndexesCB
{
	int PL_activeIndexes[4];
	int PL_avtiveCount;
	float _padPLI[3];
};
// --------------------------------------------------
// スキンバッファ（ｂ９）
// --------------------------------------------------
static const int MAX_MATRIX = 128; // 上限（ここも合わせる必要があるので注意！）
struct alignas(16) SkinCB
{
	Matrix4x4 BoneMatrixes[MAX_MATRIX];
	int       BoneCount;
	int       _padS[3];
};
// --------------------------------------------------
// スプライトバッファ（ｂ１０）
// --------------------------------------------------
struct alignas(16) SpriteCB
{
	float spriteUV_OS[4]; // xy = uvOffset, zw = uvScale
	float spriteColor[4]; // カラー
};

// ==================================================
// ----- アラインのズレチェック -----
// ==================================================
static_assert(sizeof(CameraCB) % 16 == 0, "CameraCB size misaligned");
static_assert(sizeof(ObjectCB) % 16 == 0, "ObjectCB size misaligned");
static_assert(sizeof(MaterialCB) % 16 == 0, "MaterialCB size misaligned");
static_assert(sizeof(LightCB)  % 16 == 0, "LightCB size misaligned");
static_assert(sizeof(PointLightsCB)  % 16 == 0, "PointLightsCB size misaligned");
static_assert(sizeof(ToonCB)   % 16 == 0, "ToonCB size misaligned");
static_assert(sizeof(CameraLightCB)  % 16 == 0, "CameraLightCB size misaligned");
static_assert(sizeof(OutlineCB)  % 16 == 0, "OutlineCB size misaligned");
static_assert(sizeof(PointLightIndexesCB)  % 16 == 0, "PointLightIndexesCB size misaligned");
static_assert(sizeof(SkinCB)   % 16 == 0, "SkinCB size misaligned");
static_assert(sizeof(SpriteCB)   % 16 == 0, "SpriteCB size misaligned");

// ==================================================
// ----- Renderer クラス -----
// ==================================================
class Renderer
{
public:
	// ==================================================
	// ----- 列挙体 -----
	// ==================================================
	// --------------------------------------------------
	// ブレンドモード
	// --------------------------------------------------
	enum class BlendMode { Opaque, Alpha, Premultiplied, Additive, Multiply};
	enum class DepthMode 
	{ 
		Opaque,  // 不透明（深度情報更新）
		Transparent,   // 半透明（深度情報非更新）
		ZPrepassWrite, // Ｚプリ１パス目
		ZEqualReadOnly, // Ｚプリ２パス目
		OutlineLE, // アウトライン、押し出し描画
		DisableDepth, // デバッグ、画面効果など（深度無効）
	};
	// --------------------------------------------------
	// インプットレイアウトモード
	// --------------------------------------------------
	enum class VertexLayoutType
	{
		Basic,   // VERTEX_3D
		Skinned, // VERTEX_SKINNED
	};
	// --------------------------------------------------
	// ラスタライザモード
	// --------------------------------------------------
	enum class RasterizerMode
	{
		BackCullSolid,  // 通常
		FrontCullSolid, // 裏側
		NoneCullSolid,  // 両面
		WireFrame,		// ワイヤーフレーム
		ShadowCaster,   // シャドウマップ用
	};

private:
	// ==================================================
	// ----- DirectX コアオブジェクト -----
	// ==================================================
	static ComPtr<ID3D11Device>           m_Device;
	static ComPtr<ID3D11DeviceContext>    m_DeviceContext;

	// ==================================================
	// ----- 定数バッファ -----
	// ==================================================
	static ComPtr<ID3D11Buffer> m_CB_Camera;      // b0
	static ComPtr<ID3D11Buffer>	m_CB_Object;      // b1
	static ComPtr<ID3D11Buffer>	m_CB_Material;    // b2
	static ComPtr<ID3D11Buffer>	m_CB_Light;	      // b3
	static ComPtr<ID3D11Buffer>	m_CB_PointLights; // b4
	static ComPtr<ID3D11Buffer>	m_CB_Toon;		  // b5
	static ComPtr<ID3D11Buffer>	m_CB_CameraLight; // b6
	static ComPtr<ID3D11Buffer>	m_CB_Outline;	  // b7
	static ComPtr<ID3D11Buffer>	m_CB_PointLightIndexes;	// b8
	static ComPtr<ID3D11Buffer>	m_CB_Skin;		  // b9
	static ComPtr<ID3D11Buffer>	m_CB_Sprite;	  // b10

	// ==================================================
	// ----- ブレンド・デプスステンシル -----
	// ==================================================
	static ComPtr<ID3D11DepthStencilState> m_DepthStateEnable;
	static ComPtr<ID3D11DepthStencilState> m_DepthStateDisable;
	static ComPtr<ID3D11BlendState>		   m_BlendState;
	static ComPtr<ID3D11BlendState>		   m_BlendStateATC;
	static ComPtr<ID3D11RasterizerState>   m_RasterizerStateDefault;
	static ComPtr<ID3D11SamplerState>	   m_SamplerLinearWrap;

	// ==================================================
	// ----- ブレンド・デプスステンシルステート -----
	// ==================================================
	static ComPtr<ID3D11BlendState> m_BS_Opaque;
	static ComPtr<ID3D11BlendState> m_BS_Alpha;
	static ComPtr<ID3D11BlendState> m_BS_Premul;
	static ComPtr<ID3D11BlendState> m_BS_Add;
	static ComPtr<ID3D11BlendState> m_BS_Multiply;
	static ComPtr<ID3D11DepthStencilState> m_DS_Opaque;
	static ComPtr<ID3D11DepthStencilState> m_DS_Transparent;
	static ComPtr<ID3D11DepthStencilState> m_DS_ZPrepassWrite;
	static ComPtr<ID3D11DepthStencilState> m_DS_ZEqualReadOnly;
	static ComPtr<ID3D11DepthStencilState> m_DS_OutlineLE;
	static ComPtr<ID3D11DepthStencilState> m_DS_Disable;		

	// ==================================================
	// ----- ラスタライザステート -----
	// ==================================================
	static ComPtr<ID3D11RasterizerState> m_RS_BackCull;
	static ComPtr<ID3D11RasterizerState> m_RS_FrontCull;
	static ComPtr<ID3D11RasterizerState> m_RS_NoCull;
	static ComPtr<ID3D11RasterizerState> m_RS_WireFrame;
	static ComPtr<ID3D11RasterizerState> m_RS_ShadowCaster;

	// ==================================================
	// デバッグレンダラー
	// ==================================================
	static DebugRenderer* m_pDebugRenderer;

	// ==================================================
	// ※　一旦放置（将来的に使う）　※
	// ==================================================
	static D3D_FEATURE_LEVEL       m_FeatureLevel;

public:
	// ==================================================
	// ----- 初期化・終了処理 -----
	// ==================================================
	static void Init(ID3D11Device* dev,	ID3D11DeviceContext* ctx);
	static void Uninit();

	// ==================================================
	// ----- フレーム制御 -----
	// ==================================================
	static void BeginFrame(const float clearColor[4]);
	static void EndFrame(int vsync = 1);

	// ==================================================
	// ----- 各種設定 -----
	// ==================================================
	static void SetCamera(const Matrix4x4& view,
						  const Matrix4x4& proj,
						  const Vector3& cameraPos,
						  const float& fovY,
						  uint32_t width, uint32_t height);
	static void SetObject			(const Matrix4x4& world, const Matrix4x4& worldInvTranspose);
	static void SetMaterial			(const MaterialApp& material);
	static void SetLight			(const LightApp& light);
	static void SetPointLights		(const PointLightApp* lights, int count);
	static void SetToon				(const ToonApp& toon);
	static void SetCameraLight		(const CameraLightApp& cameraLight);
	static void SetOutline			(const OutlineApp& outline);
	static void SetPointLightIndexes(int* indexes, int count);
	static void SetSkinMatrixes		(const Matrix4x4* mats, int count);
	static void SetSprite			(const SpriteApp& sprite);

	static void CreateVertexShader(ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName, VertexLayoutType layoutType = VertexLayoutType::Basic);
	static void CreatePixelShader(ID3D11PixelShader** PixelShader, const char* FileName);

	static void ApplyDefaultRasterizer();
	static void SetDepthEnable(bool Enable);
	static void SetATCEnable(bool Enable);

	static ID3D11SamplerState* DefaultLinearWrapSampler() { return m_SamplerLinearWrap.Get(); }

	// --------------------------------------------------
	// ブレンドステート、深度設定
	// --------------------------------------------------
	static void SetBlendState(BlendMode mode);
	static void SetDepthState(DepthMode mode, UINT stencilRef = 0);
	static void SetRasterizerState(RasterizerMode mode);

	// ==================================================
	// ----- DirectXの基礎取得 -----
	// ==================================================
	static ID3D11Device*		Device( void ){ return m_Device.Get(); }
	static ID3D11DeviceContext* DeviceContext( void ){ return m_DeviceContext.Get(); }

	// ==================================================
	// デバッグレンダラー
	// ==================================================
	static DebugRenderer* GetDebugRenderer() { return m_pDebugRenderer; }
private:
	// ==================================================
	// ----- 内部ユーティリティ -----
	// ==================================================
	// --------------------------------------------------
	// 定数バッファ作成
	// --------------------------------------------------
	template<class T>
	static ComPtr<ID3D11Buffer> CreateCB();

	template<class T>
	static void UpdateCB(ID3D11Buffer* cb, const T& data);

	static CameraCB BuildCameraCB(const Matrix4x4& view,
								  const Matrix4x4& proj,
								  const Matrix4x4& viewProj,
								  const Vector3& cameraPos,
								  const float& fovY ,
								  uint32_t width, uint32_t height);
	static ObjectCB   BuildObjectCB(const Matrix4x4& world, const Matrix4x4& worldInvTranspose);
	static MaterialCB BuildMaterialCB(const MaterialApp& m);
	static LightCB	  BuildLightCB(const LightApp& l);
	static PointLightsCB BuildPointLightsCB(const PointLightApp* lights, int count);
	static ToonCB	  BuildToonCB(const ToonApp& t);
	static CameraLightCB BuildCameraLightCB(const CameraLightApp& cameraLight);
	static OutlineCB  BuildOutlineCB(const OutlineApp& outline);
	static PointLightIndexesCB  BuildPointLightIndexesCB(int* indexes, int count);
	static SkinCB     BuildSkinMatrixesCB(const Matrix4x4* mats, int count);
	static SpriteCB   BuildSpriteCB(const SpriteApp& sprite);

	// --------------------------------------------------
	// ブレンドステート作成
	// --------------------------------------------------
	static void CreateBlendStates();
	static ComPtr<ID3D11BlendState> MakeBS(ID3D11Device* dev, 
										   bool atc,
										   D3D11_BLEND src, D3D11_BLEND dst, D3D11_BLEND_OP op,
										   D3D11_BLEND srcA, D3D11_BLEND dstA, D3D11_BLEND_OP opA,
										   UINT8 writeMask = D3D11_COLOR_WRITE_ENABLE_ALL
										  );
	// --------------------------------------------------
	// デプスステンシルステート作成
	// --------------------------------------------------
	static void CreateDepthStates();
	static ComPtr<ID3D11DepthStencilState> MakeDS(
												  ID3D11Device* dev,
												  BOOL depthEnable,
												  D3D11_DEPTH_WRITE_MASK writeMask,
												  D3D11_COMPARISON_FUNC depthFunc,
												  BOOL stencilEnable = FALSE,
												  UINT8 readMask = 0xFF, UINT8 writeMaskStencil = 0xff,
												  D3D11_COMPARISON_FUNC stencilFunc = D3D11_COMPARISON_ALWAYS,
												  D3D11_STENCIL_OP passOp = D3D11_STENCIL_OP_KEEP,
												  D3D11_STENCIL_OP failOp = D3D11_STENCIL_OP_KEEP,
												  D3D11_STENCIL_OP zfailOp = D3D11_STENCIL_OP_KEEP
												 );
	// --------------------------------------------------
	// ラスタライザステート作成
	// --------------------------------------------------
	static void CreateRasterizerStates();
	static ComPtr<ID3D11RasterizerState> MakeRS(
												ID3D11Device* dev,
												D3D11_FILL_MODE fill,
												D3D11_CULL_MODE cull,
												BOOL frontCCW,
												BOOL depthClip,
												BOOL scissor,
												BOOL multisample,
												BOOL aaLine,
												INT  depthBias = 0,
												FLOAT slopeScaledBias = 0.0f,
												FLOAT depthBiasClamp = 0.0f
											   );



	// 共通ステートの遅延初期化（深度／ブレンド／ラスタライザ）
	static void EnsureCommonStates();
};


// ==================================================
// ----- 構造体プリセット -----
// ==================================================
enum class ToonPreset
{
	GravityRushLike,
	GravityRush2Like,
	Debug,
};
inline ToonApp MakeToon(ToonPreset p)
{
	switch (p)
	{
	case ToonPreset::GravityRushLike:
		{
			ToonApp toon = {};
			toon.ToonThreshold01 = 0.10f;
			toon.ToonThreshold12 = 0.30f;
			toon.ToonSmooth01 = 0.03f;
			toon.ToonSmooth12 = 0.01f;
			toon.ToonTintDark = { 0.15f, 0.20f, 0.30f }; // 暗い部分は青っぽく
			toon.ToonTintMid = { 0.70f, 0.75f, 0.80f }; // やや寒色寄り
			toon.ToonTintLit = { 1.00f, 1.00f, 1.00f }; // アルベドそのまま
			toon.ToonSpecThreshold = 0.95f;
			toon.ToonSpecStrength = 1.0f;
			toon.ToonShadowOverrideColor = { 0.25f, 0.30f, 0.45f };
			toon.ToonShadowOverrideRate = 0.70f; // ブレンド率
			toon.ToonShadowDesaturate = 0.60f;  // 脱色率
			toon.ToonShadowAlbedoRate = 0.0f; // テクスチャ率
			return toon;
		}
	case ToonPreset::GravityRush2Like:
		{
			ToonApp toon = {};
			toon.ToonThreshold01 = 0.10f;
			toon.ToonThreshold12 = 0.50f;
			toon.ToonSmooth01 = 0.03f;
			toon.ToonSmooth12 = 0.03f;
			toon.ToonTintDark = { 0.45f, 0.50f, 0.60f }; // 暗い部分は青っぽく
			toon.ToonTintMid = { 0.70f, 0.75f, 0.80f }; // やや寒色寄り
			toon.ToonTintLit = { 1.00f, 1.00f, 1.00f }; // アルベドそのまま
			toon.ToonSpecThreshold = 0.95f;
			toon.ToonSpecStrength = 1.0f;
			toon.ToonShadowOverrideColor = { 0.00f, 0.00f, 0.00f };
			toon.ToonShadowOverrideRate = 0.00f; // ブレンド率（０で上書きしない）
			toon.ToonShadowDesaturate = 0.0f;  // 脱色率
			toon.ToonShadowAlbedoRate = 1.0f; // テクスチャ率
			return toon;
		}
	}
}

#endif