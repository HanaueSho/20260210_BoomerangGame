/*
    pixelLightingPS
    20251008 hanaue sho
*/
#include "common.hlsl"

void main(in VS_OUT IN, out float4 outDiffuse : SV_Target)
{
    // ピクセルの法線を正規化
    float3 normal = normalize(IN.WorldNormal);
    
    // 物体表面へ向かう光ベクトル
    float3 L = normalize(-LightDirection.xyz);
    
    // 物体からカメラへ向かうベクトル
    float3 V = normalize(CameraPosition.xyz - IN.WorldPosition);
    
    // Lambert
    float NdotL = saturate(dot(normal, L));
    
    // Blinn-Phong
    float3 H = normalize(L + V);
    float spec = pow(saturate(dot(normal, H)), max(1.0, MatShininess));
    
    // テクスチャ
    float4 baseColor = (MatTextureEnable != 0) ? MainTex.Sample(MainSamp, IN.TexCoord) : float4(1, 1, 1, 1);
    baseColor = MainTex.Sample(MainSamp, IN.TexCoord);
    
    // 係数
    float3 ambient = MatAmbient.rgb * LightAmbient.rgb;
    float3 diffuse = MatDiffuse.rgb * IN.Color.rgb * LightDiffuse.rgb * NdotL;
    float3 specular = MatSpecular.rgb * spec;
    
    float3 litRGB = (ambient + diffuse + specular) * (baseColor.rgb * IN.Color.rgb);
    
    outDiffuse.rgb = litRGB;
    outDiffuse.a = baseColor.a * IN.Color.a;
}
