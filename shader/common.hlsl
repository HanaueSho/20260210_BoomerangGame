/*
    common.hlsl
    20251008  hanaue sho
   このファイルは他のシェーダーファイルへインクルードされる
   各種マトリクスやベクトルを受け取る変数を用意
*/
#ifndef COMMON_HLSL_
#define COMMON_HLSL_

// ==================================================
// ----- 定数バッファのレイアウト -----
// CPU 側と合わせる
// ==================================================
// --------------------------------------------------
// カメラバッファ（ｂ０）
// --------------------------------------------------
cbuffer CameraCB : register(b0)
{
    // CPU 側は行ベクトル行列、HLSLでも行ベクトル流儀で
    row_major float4x4 View;
    row_major float4x4 Proj;
    row_major float4x4 ViewProj;
    float3 CameraPosition; // カメラ位置
    float  FOV;            // FovY
    float2 ScreenSize;     // スクリーンサイズ
    float2 InvScreenSize;  // 逆スクリーンサイズ    
}
// --------------------------------------------------
// オブジェクトバッファ（ｂ１）
// --------------------------------------------------
cbuffer ObjectCB : register(b1)
{
    row_major float4x4 World; // ワールド行列
    row_major float4x4 WorldInv; // 法線変換用
}
// --------------------------------------------------
// マテリアルバッファ（ｂ２）
// --------------------------------------------------
cbuffer MaterialCB : register(b2)
{
    float4 MatAmbient;       // 環境光
    float4 MatDiffuse;       // 拡散光
    float4 MatSpecular;      // 鏡面光
    float4 MatEmission;      // 自己発光
    float  MatShininess;     // 光沢度
    int    MatTextureEnable; // テクスチャ有効
    float2 _padMat0; // パディング
}
// --------------------------------------------------
// ライトバッファ（ｂ３）
// int LightEnble, _padL0[3] を先頭にするとデータが送れないので
// --------------------------------------------------
cbuffer LightCB : register(b3)
{
    float4 LightDirection; // ライトの向いている方向（w未使用）（PS内で P→Light へ向くようにマイナスをつける）
    float4 LightDiffuse;   // 拡散光
    float4 LightAmbient;   // 環境光
    int LightEnable;       // ライト有効
    int _padL0[3]; // パディング
}
// --------------------------------------------------
// ポイントライトバッファ（ｂ４）
// 複数のライトを一つのバッファに詰める
// --------------------------------------------------
static const int MAX_POINT = 64;
struct PointLightData
{
    float4 PositionRange; // xyz: 位置, w: range
    float4 ColorIntensity; // rgb: 色, a: 強度
};
cbuffer PointLightCB : register(b4)
{
    PointLightData PointLights[MAX_POINT];
    int PointLightCount; // ※※※これもなぜか先頭だとデータが送られない※※※
    int _padPL0[3];
}
// --------------------------------------------------
// トゥーンバッファ（ｂ５）
// --------------------------------------------------
cbuffer ToonCB : register(b5)
{
    // 境界閾値、ぼかし幅
    float ToonThreshold01; // 境界閾値（暗→中）
    float ToonThreshold12; // 境界閾値（中→明）
    float ToonSmooth01; // 境界のぼかし幅（暗→中）
    float ToonSmooth12; // 境界のぼかし幅（中→明）
    // 各段のティント（albedoに掛ける色）
    float3 ToonTintDark; float _padT0;
    float3 ToonTintMid; float _padT1;
    float3 ToonTintLit; float _padT2;
    // ハイライト線
    float  ToonSpecThreshold;
    float  ToonSpecStrength;
    float2 _padT3; 
    // 影部分の上書き処理
    float3 ToonShadowOverrideColor; float _padT4; // 上書きする色
    float  ToonShadowOverrideRate; // 0:使わない、1:完全上書き（上書き率）
    float  ToonShadowDesaturate;   // 0:そのまま、1:完全モノクロ（脱色）
    float  ToonShadowAlbedoRate; // 0:模様を使わない、1: 模様つかう
}
// --------------------------------------------------
// カメラライトバッファ（ｂ６）
// --------------------------------------------------
cbuffer CameraLightCB : register(b6)
{
    float4 CameraLightDirection; // ライトの方向（w未使用）
    float4 CameraLightDiffuse;   // 拡散光
}
// --------------------------------------------------
// アウトラインバッファ（ｂ７）
// --------------------------------------------------
cbuffer OutlineCB : register(b7)
{
    float  OutlineWidth; // 膨張量（ワールド単位）
    float3 OutlineColor; // 線色（0..1）
}
// --------------------------------------------------
// ポイントライトインデックスバッファ（ｂ８）
// --------------------------------------------------
cbuffer PointLightIndexesCB : register(b8)
{
    int4 PL_ActiveIndexes; // アクティブライトのインデックス
    int  PL_ActiveCount;   // アクティブライトのカウント
    int3 _padPLI; // パディング
}
// --------------------------------------------------
// スキニングバッファ（ｂ９）
// --------------------------------------------------
cbuffer SkinCB : register(b9)
{
    row_major float4x4 BoneMatrixes[128];
    int      BoneCount;
    int3     _padS;
}
// --------------------------------------------------
// スプライトバッファ（ｂ１０）
// --------------------------------------------------
cbuffer SpriteCB : register(b10)
{
    float4 SpriteUV_OS; // xy = uvOffset, zw = uvScale
    float4 SpriteColor;
};

// ==================================================
// ----- テクスチャ・サンプラ -----
// ==================================================
Texture2D MainTex     : register(t0);
SamplerState MainSamp : register(s0);

// ==================================================
// ----- 頂点 I/O -----
// CPU 側 VERTEX_3D:position, normal, diffuse, texcoord と一致させる
// ==================================================
struct VS_IN
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float4 Color    : COLOR; 
    float2 TexCoord : TEXCOORD0;
};
struct VS_OUT
{
    float4 SVPos         : SV_POSITION; // クリップ空間の位置
    float3 WorldPosition : POSITION0;   // ワールド座標
    float3 WorldNormal   : NORMAL0;     // 法線
    float4 Color         : COLOR0;      // 色
    float2 TexCoord      : TEXCOORD0;   // テックスコード
};
struct VS_IN_DEBUG
{
    float3 Position : POSITION;
    float4 Color    : COLOR;
};
struct VS_OUT_DEBUG
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
};
struct VS_IN_SKINNED
{
    float3 Position   : POSITION;
    float3 Normal     : NORMAL;
    float2 TexCoord   : TEXCOORD0;
    uint4 BoneIndex   : BLENDINDICES;
    float4 BoneWeight : BLENDWEIGHT;
};

// ==================================================
// ----- 関数 -----
// ==================================================
// --------------------------------------------------
// ワールド座標変換
// --------------------------------------------------
float3 TransformPositionWorld(float3 pos)
{
    // row_major 前提 → mul(v, M)
    float4 wp = mul(float4(pos, 1.0f), World);
    return wp.xyz;
}
// --------------------------------------------------
// ワールド法線変換
// --------------------------------------------------
float3 TransformNormalWorld(float3 normal)
{
    float3 wn = mul(float4(normal, 0.0f), WorldInv).xyz;
    return normalize(wn);
}
// --------------------------------------------------
// 距離減衰
// Range で０まで落とす。C^2 →　グラデーション
// --------------------------------------------------
float AttenuationSmooth(float dist, float range)
{
    float x = saturate(1.0f - dist / max(range, 1e-5));
    return x * x;  
}
// --------------------------------------------------
// Blinn-Phong
// --------------------------------------------------
struct LambertBlinnResult { float3 diff; float3 spec; };
LambertBlinnResult ComputeLambertBlinn(
                                       float3 N, float3 V, float3 L, // 法線、視線、ライト
                                       float3 lightColor, float intensity, float atten, float shininess // ライト色、ライト強度、減衰係数、光沢度
                                       )
{
    LambertBlinnResult r;
    float NdotL = saturate(dot(N, L)); // 面の向きと光の向きの内積（光が当たっているか）
    float3 H = normalize(L + V); // 光と視線の中間方向（ハーフベクトル）
    float sp = pow(saturate(dot(N, H)), max(1.0, shininess)); // ハイライト
    
    float w = intensity * atten; // （明るさ(intensity)×減衰(atten)）を一つにまとめた重み（点光源から距離が遠いほど attenが小さい）
    r.diff = lightColor * (w * NdotL); // ライト色で着色しつつ w で明るさ、距離効果をかける
    r.spec = lightColor * (w * sp);
    return r;
}

#endif