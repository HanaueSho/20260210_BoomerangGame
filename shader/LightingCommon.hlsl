/*
    LightingCommon.hlsl
    20251112  hanaue sho
    ライティングの共通部分
    算出式：（アルベド）×（環境光＋拡散光）＋鏡面反射＋自己発光
*/
#ifndef LIGHTINGCOMMON_HLSL_
#define LIGHTINGCOMMON_HLSL_
#include "common.hlsl"

// ==================================================
// ----- 関数 -----
// ==================================================
// --------------------------------------------------
// 視線ベクトル（未正規化）
// --------------------------------------------------
float3 ViewDir(float3 worldPos)
{
    return CameraPosition.xyz - worldPos;
}
float3 ViewDirN(float3 worldPos)
{
    return normalize(CameraPosition.xyz - worldPos);
}

// --------------------------------------------------
// アルベド作成
// （テクスチャカラー）×（頂点カラー）×（マテリアル拡散色）
// --------------------------------------------------
float3 BuildAlbedo(float3 texColor, float3 vertexColor)
{
    return texColor * vertexColor * MatDiffuse.rgb;
}
// --------------------------------------------------
// 環境光作成
// （環境光色）×（マテリアル環境光色）
// --------------------------------------------------
float3 BuildAmbient()
{
    return LightAmbient.rgb * MatAmbient.rgb;
}
// --------------------------------------------------
// 拡散光作成
// （ランバート拡散反射）×（ライトカラー）×（ライト強度）×（減衰）
// ライト強度（intensity）：Diffuse.a とする
// --------------------------------------------------
float3 BuildLambert(float3 Normal, float3 LightDir, float3 lightDiffuse, float intensity, float att)
{
    return saturate(dot(Normal, LightDir)) * lightDiffuse * intensity * att;
}
// --------------------------------------------------
// 鏡面反射作成
// （ブリン・フォン）×（ライトカラー）×（ライト強度）×（減衰）×（マテリアル鏡面色）
// ライト強度（intensity）：Diffuse.a とする
// --------------------------------------------------
float3 BuildSpecular(float3 Normal, float3 LightDir, float3 ViewDir, float3 lightColor, float intensity, float att)
{
    float3 Half = normalize(LightDir + ViewDir);
    float sp = pow(saturate(dot(Normal, Half)), max(1.0f, MatShininess));
    return sp * lightColor * intensity * att * MatSpecular.rgb;
}
// --------------------------------------------------
// 自己発光作成
// （マテリアル発光色）
// --------------------------------------------------
float3 BuildEmission()
{
    return MatEmission.rgb;
}

struct DiffSpec
{
    float3 diff;
    float3 spec;
};
// --------------------------------------------------
// 平行光源の算出
// --------------------------------------------------
DiffSpec ComputeDirectionalLight(float3 Normal, float3 ViewDir)
{
    DiffSpec r;
    r.diff = BuildLambert(Normal, -LightDirection.rgb, LightDiffuse.rgb, LightDiffuse.a, 1.0f);
    r.spec = BuildSpecular(Normal, -LightDirection.rgb, ViewDir, LightDiffuse.rgb, LightDiffuse.a, 1.0f);
    return r;
}
// --------------------------------------------------
// ポイントライトの算出（インデックス）
// 有効なライトのみを計算する
// --------------------------------------------------
DiffSpec ComputePointLightIndexes(float3 worldPosition, float3 Normal, float3 ViewDir)
{
    DiffSpec r;
    [loop]
    for (int i = 0; i < PL_ActiveCount; i++)
    {
        // tips: 固定回数のループにして if (i >= PL_ActiveCount) break;　にすると最適化

        // 有効ライトのインデックスを受け取る
        int index = PL_ActiveIndexes[i];
        
        // ライト向きの算出
        float3 lightVect = PointLights[index].PositionRange.xyz - worldPosition.xyz;
        float dist = length(lightVect);
        float range = PointLights[index].PositionRange.w;
        if (dist >= range) continue; // 有効範囲よりも遠いので終了 
        
        // 減衰算出
        float att = AttenuationSmooth(dist, range);
        
        float3 LightDir = (dist > 1e-5) ? (lightVect / dist) : float3(0, 0, 1);
        float3 lightColor = PointLights[index].ColorIntensity.rgb;
        float  intensity = PointLights[index].ColorIntensity.a;
        r.diff += BuildLambert(Normal, LightDir, lightColor, intensity, att);
        r.spec += BuildSpecular(Normal, LightDir, ViewDir, lightColor, intensity, att);
    }
    return r;
}

// --------------------------------------------------
// 明るさのみを算出
// DirectionalLight は att = 1.0f
// intensity = LightDiffuse.a 
// --------------------------------------------------
float EvalLightWeight(float3 normal, float3 lightDir, float intensity, float att = 1.0f)
{
    float NdotL = saturate(dot(normal, lightDir));
    return NdotL * intensity * att;
}
// --------------------------------------------------
// 環境光とポイントライトの明るさと色を分けて算出
// 色は明るさに寄らず、ライトカラーの合算を渡す
// --------------------------------------------------
struct BrightColor{ float bright; float3 color;};
BrightColor ComputeLightBrightAndColor(float3 normal, float3 position)
{
    float bright = 0.0f; // ambient を最初に入れておく（実際はシーンに合わせて調整する値）
    float sumWeight = 0.0f;
    float3 colorAcc = 0.0f;
    
    // Directional Light
    float weightD = EvalLightWeight(normal, -LightDirection.xyz, LightDiffuse.a, 1.0f);
    bright += weightD; // 明るさを加算
    sumWeight += LightDiffuse.a * 1.0f; // intensity * att
    colorAcc += (LightDiffuse.rgb * LightDiffuse.a); // （ライトカラー）×（明るさ）
    
    // PointLights (Index)
    [loop]
    for (int i = 0; i < PL_ActiveCount; i++)
    {
        // 有効ライトのインデックス
        int index = PL_ActiveIndexes[i];
        
        // ライトの向き
        float3 lightPos = PointLights[index].PositionRange.xyz;
        float range = PointLights[index].PositionRange.w;
        float3 lightVect = lightPos - position;
        float dist = length(lightVect);
        if (dist <= 1e-6 || dist >= range) continue;
        float3 lightDir = lightVect / dist;
        
        // 減衰算出
        float att = AttenuationSmooth(dist, range);
        
        // 重み算出
        float weightP = EvalLightWeight(normal, lightDir, PointLights[index].ColorIntensity.a, att);
        bright += weightP;
        sumWeight += PointLights[index].ColorIntensity.a * att; // intensity * att
        colorAcc += (PointLights[index].ColorIntensity.rgb * PointLights[index].ColorIntensity.a) * att;
    }
    
    BrightColor r;
    r.bright = bright * 1.0f; // ToonExposureScale を掛けて 0..1 に収まるようにする
    r.color = (sumWeight > 1e-6) ? colorAcc/sumWeight : 1.0.xxx;
    //r.color = colorAcc; // 単純にライトカラーのみを積む
    
    return r;
}



#endif