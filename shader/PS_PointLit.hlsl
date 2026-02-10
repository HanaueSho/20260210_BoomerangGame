/*
    pointLightingPS
    20251016 hanaue sho
    線形HDR
*/
#include "common.hlsl"
#include "LightingCommon.hlsl"

void main(in VS_OUT IN, out float4 outDiffuse : SV_Target)
{
    // アルベドの作成
    float4 texColor = (MatTextureEnable != 0) ? MainTex.Sample(MainSamp, IN.TexCoord) : float4(1, 1, 1, 1);
    float3 albedo = BuildAlbedo(texColor.rgb, IN.Color.rgb);
    
    // 環境光の作成
    float3 ambient = BuildAmbient();
    
    // 拡散光・鏡面反射の作成
    float3 normal = normalize(IN.WorldNormal);
    float3 viewDir = normalize(CameraPosition.xyz - IN.WorldPosition);
    float3 light = float3(0, 0, 0), spec = float3(0, 0, 0);
    DiffSpec diffSpec;
    diffSpec = ComputeDirectionalLight(normal, viewDir);
    light += diffSpec.diff;
    spec += diffSpec.spec;
    diffSpec = ComputePointLightIndexes(IN.WorldPosition, normal, viewDir);
    light += diffSpec.diff;
    spec += diffSpec.spec;
    
    // 自己発光の作成
    float3 emission = BuildEmission();
    
    outDiffuse.rgb = albedo * (ambient + light) + spec + emission;
    outDiffuse.a = texColor.a * IN.Color.a;
}
/*
    ※※※備忘録※※※
    以前のコード

    // 法線・視線
    float3 N = normalize(IN.WorldNormal);
    float3 V = normalize(CameraPosition.xyz - IN.WorldPosition);
    
    // ベースカラー
    float4 baseColor = (MatTextureEnable != 0) ? MainTex.Sample(MainSamp, IN.TexCoord) : float4(1, 1, 1, 1);
    
    // マテリアル係数
    float3 kA = MatAmbient.rgb;
    float3 kD = MatDiffuse.rgb;
    float3 kS = MatSpecular.rgb;
    
    // ----- Ambient 算出 -----
    float3 lit = kA * LightAmbient.rgb; // マテリアルの Ambient × ライトの Ambient
    
    // ----- Directional -----
    if (LightEnable != 0)
    {
        float3 Ld = normalize(-LightDirection.rgb);
        LambertBlinnResult r = ComputeLambertBlinn(N, V, Ld, LightDiffuse.rgb, LightDiffuse.a, 1.0f, MatShininess);
        lit += kD * r.diff + kS * r.spec; // 色を加算
    }
    
    // ----- ポイントライト -----
    [loop]
    for (int i = 0; i < PointLightCount; i++)
    {
        float3 Lvec = PointLights[i].PositionRange.xyz - IN.WorldPosition;
        float d = length(Lvec);
        float r = PointLights[i].PositionRange.w;
        if (d >= r) continue; // 有効範囲よりも遠いので早期リターン
        float att = AttenuationSmooth(d, r);
        
        float3 Ldir = (d > 1e-5) ? (Lvec / d) : float3(0, 0, 1);
        LambertBlinnResult lbr = ComputeLambertBlinn(N, V, Ldir, PointLights[i].ColorIntensity.rgb, PointLights[i].ColorIntensity.a, att, MatShininess);
        lit += kD * lbr.diff + kS * lbr.spec;
    }
    
    // ----- 頂点色＆テクスチャ乗算 -----
    float3 finalRGB = lit * (baseColor.rgb * IN.Color.rgb);
    float finalA = baseColor.a * IN.Color.a;
    
    outDiffuse = float4(finalRGB, finalA);
*/