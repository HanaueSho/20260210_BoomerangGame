/*
    toonLightingPS
    20251017 hanaue sho
    ３段階トゥーンシェーダー
*/
#include "common.hlsl"
#include "LightingCommon.hlsl"

// step(edge, x) : x < edge なら 0, x >= edge なら 1 を返す
// smoothstep(edge0, edge1, x) : x < edge0 なら 0, x >= edge1 なら 1, その間は滑らかに補間される
void ToonWeights(float key, out float wDark, out float wMid, out float wLit)
{
    float s0 = (ToonSmooth01 > 0) ? smoothstep(ToonThreshold01 - ToonSmooth01, ToonThreshold01 + ToonSmooth01, key)
                                  : step(ToonThreshold01, key);
    float s1 = (ToonSmooth12 > 0) ? smoothstep(ToonThreshold12 - ToonSmooth12, ToonThreshold12 + ToonSmooth12, key)
                                  : step(ToonThreshold12, key);
    wDark = 1.0f - s0;
    wMid = s0 * (1.0 - s1);
    wLit = s1;
}

float ExposureMap(float bright)
{
    return bright / (1.0f + bright);
}

void main(in VS_OUT IN, out float4 outDiffuse : SV_Target)
{
    // ----- 法線・視線 -----
    float3 N = normalize(IN.WorldNormal);
    float3 V = normalize(CameraPosition.xyz - IN.WorldPosition);
    
    // ----- アルベド作成 -----
    float4 texColor = (MatTextureEnable != 0) ? MainTex.Sample(MainSamp, IN.TexCoord) : float4(1, 1, 1, 1);
    float3 albedo    = BuildAlbedo(texColor.rgb, IN.Color.rgb);
    float alpha      = texColor.a * IN.Color.a;
    
    // ----- 拡散光の明るさの作成 -----
    float3 normal = normalize(IN.WorldNormal);
    BrightColor bc = ComputeLightBrightAndColor(normal, IN.WorldPosition.xyz);
    float S = bc.bright;
    //S = S / (1.0f + S); // Sを圧縮すると環境光１つだけの時に暗くなりすぎ問題（参照するライトの数によって合わせるべき？）
    
    // ----- ３段トゥーン重み -----
    float wD, wM, wL;
    ToonWeights(saturate(S), wD, wM, wL);
    
    // ----- 直射Toon -----
    float3 colorMix = min(bc.color, 1.0f); // 複数ライトで色跳びしないように１以上を丸める
    //colorMix = 1.0f; // GravityDaze 風ではライトカラーは使わない
    //float3 colorMix = bc.color;
    float3 colLit = ToonTintLit * (albedo * colorMix);
    float3 colMid = ToonTintMid * (albedo * colorMix);
    
    // 暗部はアルベドを弱めて灰色寄りに
    float3 albedoShadow  = lerp(1.0.xxx, albedo, saturate(ToonShadowAlbedoRate)); 
    float3 colDarkAlbedo = ToonTintDark * (albedoShadow * colorMix);
    // デサチュレート（色を抜いて灰色寄りへ）
    float  Y             = dot(colDarkAlbedo, float3(0.2999, 0.587, 0.114));
    float3 colDarkDesat  = lerp(colDarkAlbedo, Y.xxx, saturate(ToonShadowDesaturate));
    // 影の固定色へ上書き寄りにブレンド（模様を消したい度合い）
    float3 colDark       = lerp(colDarkDesat, ToonShadowOverrideColor, saturate(ToonShadowOverrideRate));
    // 段ごとの色（Tint*LightDiffuse）を合成
    float3 toonDirect = colLit * wL + colMid * wM + colDark * wD;
    
    // ----- 環境光の作成 -----
    float3 ambient = BuildAmbient();
    float3 albedoForAmbient = albedo * wL + albedo * wM + albedoShadow * wD;
    float3 toonAmbient      = albedoForAmbient * ambient;
    
    // ----- 鏡面反射・自己発光 -----
    float3 viewDir = normalize(CameraPosition.xyz - IN.WorldPosition);
    DiffSpec dsDir = ComputeDirectionalLight(normal, viewDir);
    DiffSpec dsPt  = ComputePointLightIndexes(IN.WorldPosition, normal, viewDir);
    float3 spec = dsDir.spec + dsPt.spec;
    float3 emis = BuildEmission();
    
    // 最終色
    float3 finalRGB = toonAmbient + toonDirect + spec + emis;
    outDiffuse = float4(finalRGB, alpha);
    //outDiffuse = float4(saturate(bc.bright.xxx), 1);
    //outDiffuse = float4(S.xxx, 1);
}

/*
    ===== 備忘録 =====
    影の濁りがライト色に引っ張られ過ぎる場合は、デサチュレートをLight乗算前にやる手法もある
    float3 albedoDark = albedo;
    float  Ya = dot(albedoDark, float3(0.299, 0.587, 0.114));
    albedoDark = lerp(albedoDark, Ya.xxx, ToonShadowDesaturate);
    float3 colDarkAlb = ToonTintDark * lightRGB * albedoDark;

*/

/*
    ===== 備忘録 =====
    // 法線・視線
    float3 N = normalize(IN.WorldNormal);
    float3 V = normalize(CameraPosition.xyz - IN.WorldPosition);
    
    // ベースカラー
    float4 baseColor = (MatTextureEnable != 0) ? MainTex.Sample(MainSamp, IN.TexCoord) : float4(1, 1, 1, 1);
    float3 albedo    = baseColor.rgb * IN.Color.rgb * MatDiffuse.rgb;
    float alpha      = baseColor.a * IN.Color.a;
    
    // マテリアル係数
    float3 kA = MatAmbient.rgb;
    float3 kS = MatSpecular.rgb;
    
    // ----- Ambient 算出 -----
    float3 ambient = kA * LightAmbient.rgb; // マテリアルの Ambient × ライトの Ambient
    
    // ----- 累計バッファ -----
    float3 spec = 0; // 鏡面にアルベドはかけない
    float3 diffuse = 0; // 拡散反射
    
    // ----- Directional -----
    if (LightEnable != 0)
    {
        float3 Ld = normalize(-LightDirection.rgb);
        float NdotL = saturate(dot(N, Ld));
        
        // ３段トゥーン重み
        float wD, wM, wL;
        ToonWeights(NdotL, wD, wM, wL);
        
        // ライト色（RGB×α）
        float3 lightRGB = LightDiffuse.rgb * LightDiffuse.a;
        
        // 明部、中部（Tint*Light*Albed）
        float3 colLit = ToonTintLit * lightRGB * albedo;
        float3 colMid = ToonTintMid * lightRGB * albedo;
        
        // 暗部はアルベドを弱めて灰色寄りに
        float3 albedoShadow = lerp(1.0.xxx, albedo, saturate(ToonShadowAlbedoRate));
        float3 colDarkAlb = ToonTintDark * lightRGB * albedoShadow;
        
        // デサチュレート（色を抜いて灰色寄りへ）
        float Y = dot(colDarkAlb, float3(0.299, 0.587, 0.114)); // 輝度
        float3 colDarkDesat = lerp(colDarkAlb, Y.xxx, saturate(ToonShadowDesaturate));
        
        // 影の固定色へ上書き寄りにブレンド（模様を消したい度合い）
        float3 colDark = lerp(colDarkDesat, ToonShadowOverrideColor, saturate(ToonShadowOverrideRate));
        
        // ----- 段ごとの色（Tint*LightDiffuse）を合成 -----
        float3 toonDir = wL * colLit + wM * colMid + wD * colDark;
        diffuse += toonDir; // 拡散反射を足しこむ
        
        // 線ハイライト（Blinn:N・H を閾値で２値化）
        float3 H = normalize(Ld + V);
        float nh = saturate(dot(N, H));
        float lin = step(ToonSpecThreshold, nh) * ToonSpecStrength;
        spec += kS * lightRGB * lin; // スペキュラに足し合わせる
    }
    
    // ----- ポイントライト -----
    [loop]
    for (int i = 0; i < PL_ActiveCount; i++)
    {
        int index = PL_ActiveIndexes[i];
        float3 Lvec = PointLights[index].PositionRange.xyz - IN.WorldPosition;
        float d = length(Lvec);
        float r = PointLights[index].PositionRange.w;
        if (d >= r) continue; // 有効範囲よりも遠いので早期リターン
        float att = AttenuationSmooth(d, r);
        
        float3 Ldir = (d > 1e-5) ? (Lvec / d) : float3(0, 0, 1);
        float  NdotL = saturate(dot(N, Ldir));
        
        // ３段トゥーン重み
        float wD, wM, wL;
        ToonWeights(NdotL, wD, wM, wL);
        
        // ライト色（RGB×α）
        float3 lightRGB = PointLights[index].ColorIntensity.rgb * PointLights[index].ColorIntensity.a;
        
        // 明部、中部（Tint*Light*Albed）
        float3 colLit = ToonTintLit * lightRGB * albedo;
        float3 colMid = ToonTintMid * lightRGB * albedo;
        
        // 暗部はアルベドを弱めて灰色寄りに
        float3 albedoShadow = lerp(1.0.xxx, albedo, saturate(ToonShadowAlbedoRate));
        float3 colDarkAlb = ToonTintDark * lightRGB * albedoShadow;
        
        // デサチュレート（色を抜いて灰色寄りへ）
        float Y = dot(colDarkAlb, float3(0.299, 0.587, 0.114)); // 輝度
        float3 colDarkDesat = lerp(colDarkAlb, Y.xxx, saturate(ToonShadowDesaturate));
        
        // 影の固定色へ上書き寄りにブレンド（模様を消したい度合い）
        float3 colDark = lerp(colDarkDesat, ToonShadowOverrideColor, saturate(ToonShadowOverrideRate));
        
        // ----- 段ごとの色（Tint*LightDiffuse）を合成 -----
        float3 toonPt = wL * colLit + wM * colMid + wD * colDark;
        diffuse += toonPt * att; // 拡散反射を足しこむ（減衰込み）
        
        // 鏡面反射
        float3 H = normalize(Ldir + V);
        float nh = saturate(dot(N, H));
        float lin = step(ToonSpecThreshold, nh) * ToonSpecStrength;
        spec += kS * lightRGB * lin * att; // スペキュラに足し合わせる（減衰込み）
    }
    
    // ----- 頂点色＆テクスチャ乗算 -----    
    float3 albedoShadowAmbient = lerp(1.0.xxx, albedo, saturate(ToonShadowAlbedoRate)); // 暗部の環境光にもアルベド率
    float3 Ld = normalize(-LightDirection.rgb); // 暗部か判定用
    float3 finalAlbedoAmbient = ambient * lerp(albedo, albedoShadowAmbient, 1 - saturate(dot(N, Ld)));
    float3 finalRGB = finalAlbedoAmbient + diffuse + spec + MatEmission.rgb;
    float finalA = alpha;
    
    outDiffuse = float4(finalRGB, finalA);



*/